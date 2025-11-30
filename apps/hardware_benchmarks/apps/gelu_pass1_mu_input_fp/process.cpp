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
#include "gelu_pass1_mu_input_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "gelu_pass1_mu_input_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("gelu_pass1_mu_input_fp", { "input" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        gelu_pass1_mu_input_fp(proc.inputs["input"], proc.output);
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
            gelu_pass1_mu_input_fp_clockwork(proc.inputs["input"], proc.output);
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
    auto mu_i_env = getenv("mu_i");

    auto vec_width = vec_width_env ? atoi(vec_width_env) : 3072;
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 64;
    auto mu_i = mu_i_env ? atoi(mu_i_env) : 32;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    auto input_copy_stencil = processor.inputs["input"];
    for (int y = 0; y < input_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
            input_copy_stencil(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Computed half lanes output
    // 0,1,2,...,mu_i/2-1, mu_i, mu_i+1, mu_i+2, ... (interleaved pattern)
    auto computed_output = Buffer<uint16_t>(int(vec_width / 2), vec_height);
    for (int y = 0; y < computed_output.dim(1).extent(); y++) {
        for (int x = 0; x < computed_output.dim(0).extent(); x++) {
            int input_x = (x % (mu_i / 2)) + (x / (mu_i / 2)) * mu_i;
            computed_output(x, y) = float_to_bfloat16_process(
                bfloat16_to_float_process(processor.inputs["input"](input_x, y)) /
                (1.0f + expf(-1.702f * bfloat16_to_float_process(processor.inputs["input"](input_x, y))))
            );
        }
    }

    // Pass through half lanes output
    // The second half of each mu_i group (mu_i/2, mu_i/2+1, ..., mu_i-1, mu_i+mu_i/2, ...)
    auto pass_through_output = Buffer<uint16_t>(int(vec_width / 2), vec_height);
    for (int y = 0; y < pass_through_output.dim(1).extent(); y++) {
        for (int x = 0; x < pass_through_output.dim(0).extent(); x++) {
            int input_x = (x % (mu_i / 2)) + mu_i / 2 + (x / (mu_i / 2)) * mu_i;
            pass_through_output(x, y) = processor.inputs["input"](input_x, y);
        }
    }

    save_halide_buffer_to_raw(processor.inputs["input"], "bin/mu_input_host_stencil.raw");
    save_halide_buffer_to_raw(computed_output, "bin/hw_output.raw");
    save_halide_buffer_to_raw(pass_through_output, "bin/hw_activation_output.raw");

    // Fake placeholder for processor output
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {},
        {
            tensor_spec{"hw_output", {"x_coord"}},
            tensor_spec{"hw_activation_output", {"x_coord"}}
        },
        {
            tensor_spec{"mu_input_host_stencil", {"x_coord"}}
        }
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}