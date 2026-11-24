#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "add_softplus_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "add_softplus_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("add_softplus_fp", { "input.mat", "bias.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        add_softplus_fp(proc.inputs["input.mat"], proc.inputs["bias.mat"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input.mat"], proc.output,
                                    "self.in_arg_0_0_0", "self.out_0_0");
    };
    functions["coreir"] = [&]() {
        coreir_process(processor);
    };
#endif

#if defined(WITH_CLOCKWORK)
    auto clockwork_process = [&](auto &proc) {
        RDAI_Platform *rdai_platform = RDAI_register_platform(&rdai_clockwork_sim_ops);
        if (rdai_platform) {
            printf("[RUN_INFO] found an RDAI platform\n");
            add_softplus_fp_clockwork(proc.inputs["input.mat"], proc.inputs["bias.mat"], proc.output);
            RDAI_unregister_platform(rdai_platform);
        } else {
            printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
    };
    functions["clockwork"] = [&]() {
        clockwork_process(processor);
    };
#endif

    processor.run_calls = functions;

    auto vec_height_env = getenv("vec_height");
    auto vec_width_env = getenv("vec_width");
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 64;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 1024;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Activation in [-8, 8] and one bias per row in [-4, 4], so s = input + bias
    // covers [-12, 12], the range where softplus changes from exp(s) to s.
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_width, vec_height);
    auto input = processor.inputs["input.mat"];
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            input(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 16.0f - 8.0f);
        }
    }
    auto bias = Buffer<uint16_t>(vec_height);
    processor.inputs["bias.mat"] = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; y++) {
        bias(y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 8.0f - 4.0f);
        for (int x = 0; x < vec_width; x++) {
            processor.inputs["bias.mat"](x, y) = bias(y);
        }
    }

    // USE_REAL_INPUT: the activation written to the app dir by voyager/scripts/aha_flow/parse_dnnLayer_tensors.py
    // (row-major, big-endian). ZERO_BIAS: a zero bias, for an input whose bias was already added (Mamba2's dt_bias
    // is added in the MU by dt_proj), so the kernel computes softplus(input).
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        // INPUT_RAW_PATH / BIAS_RAW_PATH: an input and a per-row bias from other files (same layouts), e.g. the
        // carry add of a split-row cumsum: the first pass's output and the carries
        auto input_path_env = getenv("INPUT_RAW_PATH");
        std::string input_path = input_path_env ? std::string(input_path_env) : std::string("hw_input_stencil.raw");
        load_raw_to_halide_buffer(input_path, input);
        std::cout << "Loaded the real input " << input_path << std::endl;
        auto bias_path_env = getenv("BIAS_RAW_PATH");
        if (bias_path_env) {
            load_raw_to_halide_buffer(bias_path_env, bias);
            for (int y = 0; y < vec_height; y++)
                for (int x = 0; x < vec_width; x++)
                    processor.inputs["bias.mat"](x, y) = bias(y);
            std::cout << "Loaded the real bias " << bias_path_env << std::endl;
        }
    }
    auto zero_bias_env = getenv("ZERO_BIAS");
    if (zero_bias_env && strcmp(zero_bias_env, "1") == 0) {
        for (int y = 0; y < vec_height; y++) {
            bias(y) = 0;
            for (int x = 0; x < vec_width; x++) {
                processor.inputs["bias.mat"](x, y) = 0;
            }
        }
        std::cout << "Using a zero bias" << std::endl;
    }

    // CPU reference in double precision, rounded once to BF16. ADD_SOFTPLUS_OP=add: input + bias only.
    auto op_env = getenv("ADD_SOFTPLUS_OP");
    bool add_only = op_env && strcmp(op_env, "add") == 0;
    processor.output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            double s = bfloat16_to_float_process(input(x, y)) + bfloat16_to_float_process(bias(y));
            double softplus = add_only ? s : std::max(s, 0.0) + log1p(exp(-fabs(s)));
            processor.output(x, y) = float_to_bfloat16_process(softplus);
        }
    }

    save_halide_buffer_to_raw(input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(bias, "bin/bias_host_stencil.raw");
    save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"bias_host_stencil", {"x_coord", "E64_packed"}}
        },
        {
            tensor_spec{"hw_output", {"x_coord"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}
