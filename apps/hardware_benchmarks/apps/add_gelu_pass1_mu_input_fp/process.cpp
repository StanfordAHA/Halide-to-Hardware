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
#include "add_gelu_pass1_mu_input_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "add_gelu_pass1_mu_input_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("add_gelu_pass1_mu_input_fp", { "input", "input_psum0" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        add_gelu_pass1_mu_input_fp(proc.inputs["input"], proc.inputs["input_psum0"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input.mat"], proc.inputs["input_psum0.mat"], proc.output,
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
            add_gelu_pass1_mu_input_fp_clockwork(proc.inputs["input"], proc.inputs["input_psum0"], proc.output);
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

    // MU Input
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    auto input_copy_stencil = processor.inputs["input"];
    for (int y = 0; y < input_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
            input_copy_stencil(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Complete PSUM0
    processor.inputs["input_psum0"] = Buffer<uint16_t>(vec_width, vec_height);
    auto complete_psum0 = processor.inputs["input_psum0"];
    for (int y = 0; y < complete_psum0.dim(1).extent(); y++) {
        for (int x = 0; x < complete_psum0.dim(0).extent(); x++) {
            complete_psum0(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
        }
    }

    // Upper half of PSUM0 as Input
    auto real_input_psum0 = Buffer<uint16_t>(int(vec_width / 2), vec_height);
    for (int y = 0; y < real_input_psum0.dim(1).extent(); y++) {
        for (int x = 0; x < real_input_psum0.dim(0).extent(); x++) {
            int input_x = (x % (mu_i / 2)) + (x / (mu_i / 2)) * mu_i;
            real_input_psum0(x, y) = complete_psum0(input_x, y);
        }
    }

    // Computed upper half lanes output
    // 0,1,2,...,mu_i/2-1, mu_i, mu_i+1, mu_i+2, ... (interleaved pattern)
    auto computed_output = Buffer<uint16_t>(int(vec_width / 2), vec_height);
    for (int y = 0; y < computed_output.dim(1).extent(); y++) {
        for (int x = 0; x < computed_output.dim(0).extent(); x++) {
            int input_x = (x % (mu_i / 2)) + (x / (mu_i / 2)) * mu_i;
            // First add input with psum elementwise
            float input_val = bfloat16_to_float_process(processor.inputs["input"](input_x, y));
            float psum_val = bfloat16_to_float_process(real_input_psum0(x, y));
            float sum = input_val + psum_val;
            // Then apply GELU
            computed_output(x, y) = float_to_bfloat16_process(
                sum / (1.0f + expf(-1.702f * sum))
            );
        }
    }

    // Pass through of lower half lanes output
    // The second half of each mu_i group (mu_i/2, mu_i/2+1, ..., mu_i-1, mu_i+mu_i/2, ...)
    auto pass_through_output = Buffer<uint16_t>(int(vec_width / 2), vec_height);
    for (int y = 0; y < pass_through_output.dim(1).extent(); y++) {
        for (int x = 0; x < pass_through_output.dim(0).extent(); x++) {
            int input_x = (x % (mu_i / 2)) + mu_i / 2 + (x / (mu_i / 2)) * mu_i;
            float input_val = bfloat16_to_float_process(processor.inputs["input"](input_x, y));
            pass_through_output(x, y) = float_to_bfloat16_process(input_val);
        }
    }

    save_halide_buffer_to_raw(processor.inputs["input"], "bin/mu_input_host_stencil.raw");
    save_halide_buffer_to_raw(real_input_psum0, "bin/input_psum0_host_stencil.raw");
    save_halide_buffer_to_raw(computed_output, "bin/hw_output.raw");
    save_halide_buffer_to_raw(pass_through_output, "bin/hw_psum1_lower_output.raw");

    // Fake placeholder for processor output
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_psum0_host_stencil", {"x_coord"}}
        },
        {
            tensor_spec{"hw_add_gelu_upper_output", {"x_coord"}},
            tensor_spec{"hw_psum1_lower_output", {"x_coord"}}
        },
        {
            tensor_spec{"mu_input_host_stencil", {"x_coord"}}
        }
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}