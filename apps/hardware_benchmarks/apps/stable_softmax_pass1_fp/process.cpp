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
#include "stable_softmax_pass1_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "stable_softmax_pass1_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("stable_softmax_pass1_fp", { "input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        stable_softmax_pass1_fp(proc.inputs["input.mat"], proc.output);
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
            stable_softmax_pass1_fp_clockwork(proc.inputs["input.mat"], proc.output);
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
    auto vec_height_fake_env = getenv("vec_height_fake");
    auto vec_width_fake_env = getenv("vec_width_fake");

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 2;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 80;
    auto vec_height_fake = vec_height_fake_env ? atoi(vec_height_fake_env) : 4;
    auto vec_width_fake = vec_width_fake_env ? atoi(vec_width_fake_env) : 512;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;
    // Input
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_width_fake, vec_height_fake);
    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < real_input.dim(1).extent(); y++) {
        for (int x = 0; x < real_input.dim(0).extent(); x++) {
            real_input(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Gold output
    processor.output = Buffer<uint16_t>(vec_height_fake);
    auto real_output = Buffer<uint16_t>(vec_height);
    for (int y = 0; y < real_input.dim(1).extent(); y++) {
        // Initialize to 0 as pond is hardcoded to be 0 in Zircon
        float max_val = 0.0f;
        for (int x = 0; x < real_input.dim(0).extent(); x++) {
            max_val = std::max(max_val, bfloat16_to_float_process(real_input(x, y)));
        }
        real_output(y) = float_to_bfloat16_process(max_val);
    }

    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}}
        },
        {
            tensor_spec{"hw_output", {"x_coord", "E64_packed"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}