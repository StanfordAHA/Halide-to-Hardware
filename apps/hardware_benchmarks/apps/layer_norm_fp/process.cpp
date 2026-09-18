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
#include "layer_norm_fp.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "layer_norm_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("layer_norm_fp", { "input", "weight", "bias", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        layer_norm_fp(proc.inputs["input"], proc.inputs["weight"], proc.inputs["bias"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_CLOCKWORK)
    auto clockwork_process = [&](auto &proc) {
        RDAI_Platform *rdai_platform = RDAI_register_platform(&rdai_clockwork_sim_ops);
        if (rdai_platform) {
            printf("[RUN_INFO] found an RDAI platform\n");
            layer_norm_fp_clockwork(proc.inputs["input"], proc.inputs["weight"], proc.inputs["bias"], proc.output);
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

    auto vec_width = vec_width_env ? atoi(vec_width_env) : 768;
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 128;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < real_input.dim(1).extent(); y++) {
        for (int x = 0; x < real_input.dim(0).extent(); x++) {
            real_input(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    auto real_weight = Buffer<uint16_t>(vec_width);
    for (int x = 0; x < real_weight.dim(0).extent(); x++) {
        real_weight(x) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
    }

    auto real_bias = Buffer<uint16_t>(vec_width);
    for (int x = 0; x < real_bias.dim(0).extent(); x++) {
        real_bias(x) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
    }

    const char *use_real_input = getenv("USE_REAL_INPUT");
    if (use_real_input && std::string(use_real_input) == "1") {
        // Original activations and full per-channel parameters from Voyager.
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        load_raw_to_halide_buffer("hw_weight_stencil.raw", real_weight);
        load_raw_to_halide_buffer("hw_bias_stencil.raw", real_bias);
    }

    // Independent CPU reference. Keep reductions in double precision and round
    // the final result to BF16; the CGRA graph uses the existing BF16 datapath.
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; ++y) {
        double mean = 0.0;
        for (int x = 0; x < vec_width; ++x)
            mean += bfloat16_to_float_process(real_input(x, y));
        mean /= vec_width;
        double variance = 0.0;
        for (int x = 0; x < vec_width; ++x) {
            double centered = bfloat16_to_float_process(real_input(x, y)) - mean;
            variance += centered * centered;
        }
        double scale = 1.0 / sqrt(variance / vec_width);
        for (int x = 0; x < vec_width; ++x) {
            double normalized = (bfloat16_to_float_process(real_input(x, y)) - mean) * scale;
            real_output(x, y) = float_to_bfloat16_process(
                normalized * bfloat16_to_float_process(real_weight(x)) +
                bfloat16_to_float_process(real_bias(x)));
        }
    }

    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["weight"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["bias"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);
    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_weight, "bin/weight_host_stencil.raw");
    save_halide_buffer_to_raw(real_bias, "bin/bias_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"weight_host_stencil", {"x_coord"}},
            tensor_spec{"bias_host_stencil", {"x_coord"}},
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
