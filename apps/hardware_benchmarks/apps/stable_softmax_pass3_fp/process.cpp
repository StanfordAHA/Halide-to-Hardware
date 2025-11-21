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
#include "stable_softmax_pass3_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "stable_softmax_pass3_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("stable_softmax_pass3_fp", { "input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        stable_softmax_pass3_fp(proc.inputs["input.mat"], proc.output);
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
            stable_softmax_pass3_fp_clockwork(proc.inputs["input.mat"], proc.output);
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

    auto vec_width_fake_env = getenv("vec_width_fake");
    auto vec_height_fake_env = getenv("vec_height_fake");
    auto vec_width_env = getenv("vec_width");
    auto vec_height_env = getenv("vec_height");

    auto vec_width_fake = vec_width_fake_env ? atoi(vec_width_fake_env) : 128;
    auto vec_height_fake = vec_height_fake_env ? atoi(vec_height_fake_env) : 4;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 128;
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 128;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Softmax pass 1 input
    auto softmax_pass1_input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < softmax_pass1_input.dim(1).extent(); y++) {
        for (int x = 0; x < softmax_pass1_input.dim(0).extent(); x++) {
            softmax_pass1_input(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Softmax pass 2 input vec max
    auto vec_max_pass2_input = Buffer<uint16_t>(vec_height);
    for (int y = 0; y < softmax_pass1_input.dim(1).extent(); y++) {
        // Initialize to 0 as pond is hardcoded to be 0 in Zircon
        float max_val = 0.0f;
        for (int x = 0; x < softmax_pass1_input.dim(0).extent(); x++) {
            max_val = std::max(max_val, bfloat16_to_float_process(softmax_pass1_input(x, y)));
        }
        vec_max_pass2_input(y) = float_to_bfloat16_process(max_val);
    }

    // Softmax pass 3 input exp(x - vec max)
    auto exp_pass3_input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < exp_pass3_input.dim(1).extent(); y++) {
        for (int x = 0; x < exp_pass3_input.dim(0).extent(); x++) {
            exp_pass3_input(x, y) = float_to_bfloat16_process(exp(bfloat16_to_float_process(softmax_pass1_input(x, y)) - bfloat16_to_float_process(vec_max_pass2_input(y))));
        }
    }

    // Softmax pass 3 output - using reference softmax implementation
    auto softmax_pass3_output = Buffer<uint16_t>(vec_width, vec_height);
    // Use reference implementation for stable, well-tested softmax
    reference_softmax_2d(softmax_pass1_input, softmax_pass3_output);

    // Define fake processor input and output placeholer buffers
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_width_fake, vec_height_fake);
    processor.output = Buffer<uint16_t>(vec_width_fake, vec_height_fake);

    save_halide_buffer_to_raw(exp_pass3_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(softmax_pass3_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
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