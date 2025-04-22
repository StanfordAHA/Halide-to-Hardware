#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include "hw_support_utils.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>

#if defined(WITH_CPU)
#include "rope_pass2_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "rope_pass2_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("rope_pass2_fp", { "input_lower", "input_upper", "cos_lower", "sin_lower" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        rope_pass2_fp(proc.inputs["input_lower"], proc.inputs["input_upper"], proc.inputs["cos_lower"], proc.inputs["sin_lower"], proc.output);
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
            rope_pass2_fp_clockwork(proc.inputs["input_lower"], proc.inputs["input_upper"], proc.inputs["cos_lower"], proc.inputs["sin_lower"], proc.output);
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

    auto n_heads_env = getenv("n_heads");
    auto seq_len_env = getenv("seq_len");
    auto head_dim_half_env = getenv("head_dim_half");

    auto n_heads = n_heads_env ? atoi(n_heads_env) : 8;
    auto seq_len = seq_len_env ? atoi(seq_len_env) : 32;
    auto head_dim_half = head_dim_half_env ? atoi(head_dim_half_env) : 64;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["input_lower"] = Buffer<uint16_t>(n_heads, seq_len, head_dim_half);
    processor.inputs["input_upper"] = Buffer<uint16_t>(n_heads, seq_len, head_dim_half);
    processor.inputs["cos_lower"] = Buffer<uint16_t>(n_heads, seq_len, head_dim_half);
    processor.inputs["sin_lower"] = Buffer<uint16_t>(n_heads, seq_len, head_dim_half);
    auto input_lower_copy_stencil = processor.inputs["input_lower"];
    auto input_upper_copy_stencil = processor.inputs["input_upper"];
    auto cos_lower_copy_stencil = processor.inputs["cos_lower"];
    auto sin_lower_copy_stencil = processor.inputs["sin_lower"];
    for (int n = 0; n < n_heads; n++) {
        for (int s = 0; s < seq_len; s++) {
            for (int h = 0; h < head_dim_half; h++) {
                input_lower_copy_stencil(n, s, h) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 32.0f - 16.0f);
                input_upper_copy_stencil(n, s, h) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 32.0f - 16.0f);
                cos_lower_copy_stencil(n, s, h) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f);
                sin_lower_copy_stencil(n, s, h) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f);
            }
        }
    }

    // Gold output: upper half of rope
    processor.output = Buffer<uint16_t>(n_heads, seq_len, head_dim_half);
    for (int n = 0; n < n_heads; n++) {
        for (int s = 0; s < seq_len; s++) {
            for (int h = 0; h < head_dim_half; h++) {
                processor.output(n, s, h) = float_to_bfloat16_process(bfloat16_to_float_process(input_lower_copy_stencil(n, s, h)) * bfloat16_to_float_process(cos_lower_copy_stencil(n, s, h)) + bfloat16_to_float_process(input_upper_copy_stencil(n, s, h)) * bfloat16_to_float_process(sin_lower_copy_stencil(n, s, h)));
            }
        }
    }

    saveHalideBufferToRawBigEndian(processor.inputs["input_lower"], "bin/input_lower_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.inputs["input_upper"], "bin/input_upper_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.inputs["cos_lower"], "bin/cos_lower_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.inputs["sin_lower"], "bin/sin_lower_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.output, "bin/hw_output.raw");

    auto output = processor.process_command(argc, argv);

    return output;
}