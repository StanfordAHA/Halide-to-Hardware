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
    ManyInOneOut_ProcessController<uint16_t> processor("stable_softmax_pass3_fp", { "input.mat", "pass2_sum.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        stable_softmax_pass3_fp(proc.inputs["input.mat"], proc.inputs["pass2_sum.mat"], proc.output);
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
            stable_softmax_pass3_fp_clockwork(proc.inputs["input.mat"], proc.inputs["pass2_sum.mat"], proc.output);
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

    auto vec_len_env = getenv("vec_len");

    auto vec_len = vec_len_env ? atoi(vec_len_env) : 512;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_len);
    auto input_copy_stencil = processor.inputs["input.mat"];
    for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
        input_copy_stencil(x) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 14.0f - 7.0f);
    }

    // Pass2 sum input
    processor.inputs["pass2_sum.mat"] = Buffer<uint16_t>(vec_len);
    auto pass2_sum = Buffer<uint16_t>(1);
    pass2_sum(0) = float_to_bfloat16_process(7.0f);

    // Gold output
    processor.output = Buffer<uint16_t>(vec_len);
    for (int x = 0; x < processor.inputs["input.mat"].dim(0).extent(); x++) {
        processor.output(x) = float_to_bfloat16_process(bfloat16_to_float_process(processor.inputs["input.mat"](x)) / bfloat16_to_float_process(pass2_sum(0)));
    }

    saveHalideBufferToRawBigEndian(processor.inputs["input.mat"], "bin/input_host_stencil.raw");
    saveHalideBufferToRawBigEndian(pass2_sum, "bin/pass2_sum_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.output, "bin/hw_output.raw");

    auto output = processor.process_command(argc, argv);

    return output;
}