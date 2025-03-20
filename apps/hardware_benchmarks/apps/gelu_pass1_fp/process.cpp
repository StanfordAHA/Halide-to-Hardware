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
#include "gelu_pass1_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "gelu_pass1_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("gelu_pass1_fp", { "input" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        gelu_pass1_fp(proc.inputs["input"], proc.output);
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
            gelu_pass1_fp_clockwork(proc.inputs["input"], proc.output);
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
    processor.inputs["input"] = Buffer<uint16_t>(vec_len);
    auto input_copy_stencil = processor.inputs["input"];
    for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
        input_copy_stencil(x) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 2.0f);
    }

    // Gold output
    processor.output = Buffer<uint16_t>(vec_len);
    for (int x = 0; x < processor.inputs["input"].dim(0).extent(); x++) {
        processor.output(x) = float_to_bfloat16_process(
            1.0f + expf(-1.702f * bfloat16_to_float_process(processor.inputs["input"](x)))
        );
    }

    saveHalideBufferToRawBigEndian(processor.inputs["input"], "bin/input_host_stencil.raw");
    saveHalideBufferToRawBigEndian(processor.output, "bin/hw_output.raw");

    auto output = processor.process_command(argc, argv);

    return output;
}