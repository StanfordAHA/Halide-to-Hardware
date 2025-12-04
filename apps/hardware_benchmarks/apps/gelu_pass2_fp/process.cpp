#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "gelu_pass2_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "gelu_pass2_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("gelu_pass2_fp", { "input" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        gelu_pass2_fp(proc.inputs["input"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.output,
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
            gelu_pass2_fp_clockwork(proc.inputs["input"], proc.output);
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

    auto vec_width_env = getenv("vec_width");
    auto vec_height_env = getenv("vec_height");
    auto glb_i_env = getenv("glb_i");

    auto vec_width = vec_width_env ? atoi(vec_width_env) : 1536;
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 64;
    auto glb_i = glb_i_env ? atoi(glb_i_env) : 16;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input of pass 1
    auto gelu_input_activation = Buffer<uint16_t>(vec_width * 2, vec_height);
    for (int y = 0; y < gelu_input_activation.dim(1).extent(); y++) {
        for (int x = 0; x < gelu_input_activation.dim(0).extent(); x++) {
            gelu_input_activation(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Pass through second half lanes output from pass 1
    auto pass_through_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < pass_through_output.dim(1).extent(); y++) {
        for (int x = 0; x < pass_through_output.dim(0).extent(); x++) {
            int input_x = (x % glb_i) + glb_i + (x / glb_i) * glb_i * 2;
            pass_through_output(x, y) = gelu_input_activation(input_x, y);
        }
    }

    // Gold output of GELU of second half lanes
    auto gold_gelu_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < gold_gelu_output.dim(1).extent(); y++) {
        for (int x = 0; x < gold_gelu_output.dim(0).extent(); x++) {
            float input_val = bfloat16_to_float_process(pass_through_output(x, y));
            float gelu_val = input_val / (1.0f + expf(-1.702f * input_val));
            gold_gelu_output(x, y) = float_to_bfloat16_process(gelu_val);
        }
    }

    // Fake processor input and output placeholer buffers
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(pass_through_output, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(gold_gelu_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}}
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