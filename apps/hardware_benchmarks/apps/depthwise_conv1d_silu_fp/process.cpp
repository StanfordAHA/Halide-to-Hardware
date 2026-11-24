#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "depthwise_conv1d_silu_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "depthwise_conv1d_silu_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("depthwise_conv1d_silu_fp", { "input.mat", "weight.mat", "bias.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        depthwise_conv1d_silu_fp(proc.inputs["input.mat"], proc.inputs["weight.mat"], proc.inputs["bias.mat"], proc.output);
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
            depthwise_conv1d_silu_fp_clockwork(proc.inputs["input.mat"], proc.inputs["weight.mat"], proc.inputs["bias.mat"], proc.output);
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

    // x: channel (vec_width), y: sequence position (vec_height)
    auto vec_height_env = getenv("vec_height");
    auto vec_width_env = getenv("vec_width");
    auto length = vec_height_env ? atoi(vec_height_env) : 1024;
    auto channels = vec_width_env ? atoi(vec_width_env) : 8;
    const int taps = 4;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto rnd = [](float lo, float hi) { return lo + (hi - lo) * static_cast<float>(rand()) / RAND_MAX; };
    // Activation [length, channels], weights [channels, 1, 4], bias [channels].
    auto input = Buffer<uint16_t>(channels, length);
    for (int t = 0; t < length; t++)
        for (int c = 0; c < channels; c++)
            input(c, t) = float_to_bfloat16_process(rnd(-2.0f, 2.0f));
    auto weight = Buffer<uint16_t>(taps, channels);
    for (int c = 0; c < channels; c++)
        for (int k = 0; k < taps; k++)
            weight(k, c) = float_to_bfloat16_process(rnd(-0.5f, 0.5f));
    auto bias = Buffer<uint16_t>(channels);
    for (int c = 0; c < channels; c++)
        bias(c) = float_to_bfloat16_process(rnd(-0.5f, 0.5f));

    // USE_REAL_INPUT: replace them with the activation, weights and bias written to the app dir by
    // voyager/scripts/aha_flow/parse_dnnLayer_tensors.py (same layouts, big-endian)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", input);
        load_raw_to_halide_buffer("hw_weight_stencil.raw", weight);
        load_raw_to_halide_buffer("hw_bias_stencil.raw", bias);
        std::cout << "Loaded the real hw_input_stencil.raw, hw_weight_stencil.raw and hw_bias_stencil.raw" << std::endl;
    }

    // CPU reference in double precision, rounded once to BF16:
    // y[t, c] = bias[c] + sum_k w[c, k] * x[t + k - 3, c] (zero before t = 0), out = y * sigmoid(y).
    processor.output = Buffer<uint16_t>(channels, length);
    for (int t = 0; t < length; t++) {
        for (int c = 0; c < channels; c++) {
            double y = bfloat16_to_float_process(bias(c));
            for (int k = 0; k < taps; k++) {
                int src = t + k - (taps - 1);
                if (src >= 0)
                    y += double(bfloat16_to_float_process(weight(k, c))) * bfloat16_to_float_process(input(c, src));
            }
            processor.output(c, t) = float_to_bfloat16_process(y / (1.0 + exp(-y)));
        }
    }

    processor.inputs["input.mat"] = input;
    processor.inputs["weight.mat"] = Buffer<uint16_t>(channels, length);
    processor.inputs["bias.mat"] = Buffer<uint16_t>(channels, length);
    save_halide_buffer_to_raw(input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(weight, "bin/weight_host_stencil.raw");
    save_halide_buffer_to_raw(bias, "bin/bias_host_stencil.raw");
    save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"weight_host_stencil", {"x_coord"}},
            tensor_spec{"bias_host_stencil", {"x_coord"}}
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
