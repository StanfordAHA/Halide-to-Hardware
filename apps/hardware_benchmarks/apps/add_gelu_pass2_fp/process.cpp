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
#include "add_gelu_pass2_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "add_gelu_pass2_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("add_gelu_pass2_fp", { "input", "input_psum0_lower" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        add_gelu_pass2_fp(proc.inputs["input"], proc.inputs["input_psum0_lower"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.inputs["input_psum0_lower"], proc.output,
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
            add_gelu_pass2_fp_clockwork(proc.inputs["input"], proc.inputs["input_psum0_lower"], proc.output);
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

    // Complete PSUM0
    auto complete_psum0 = Buffer<uint16_t>(vec_width * 2, vec_height);
    for (int y = 0; y < complete_psum0.dim(1).extent(); y++) {
        for (int x = 0; x < complete_psum0.dim(0).extent(); x++) {
            complete_psum0(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
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

    // Lower half of PSUM0 as Input
    auto real_input_psum0_lower = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < real_input_psum0_lower.dim(1).extent(); y++) {
        for (int x = 0; x < real_input_psum0_lower.dim(0).extent(); x++) {
            int input_x = (x % glb_i) + glb_i + (x / glb_i) * glb_i * 2;
            real_input_psum0_lower(x, y) = complete_psum0(input_x, y);
        }
    }

    // Gold output of add + GELU of second half lanes with lower half PSUM0
    auto gold_add_gelu_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < gold_add_gelu_output.dim(1).extent(); y++) {
        for (int x = 0; x < gold_add_gelu_output.dim(0).extent(); x++) {
            float pass_through_val = bfloat16_to_float_process(pass_through_output(x, y));
            float psum_val = bfloat16_to_float_process(real_input_psum0_lower(x, y));
            float sum = pass_through_val + psum_val;
            float add_gelu_val = sum / (1.0f + expf(-1.702f * sum));
            gold_add_gelu_output(x, y) = float_to_bfloat16_process(add_gelu_val);
        }
    }

    // Fake processor input and output placeholer buffers
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["input_psum0_lower"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(pass_through_output, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_input_psum0_lower, "bin/input_psum0_lower_host_stencil.raw");
    save_halide_buffer_to_raw(gold_add_gelu_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"input_psum0_lower_host_stencil", {"x_coord"}}
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