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
#include "layer_norm_pass2_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "layer_norm_pass2_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("layer_norm_pass2_fp", { "input.mat", "vec_avg.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        layer_norm_pass2_fp(proc.inputs["input.mat"], proc.inputs["vec_avg.mat"], proc.output);
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
            layer_norm_pass2_fp_clockwork(proc.inputs["input.mat"], proc.inputs["vec_avg.mat"], proc.output);
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

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 2;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 80;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["input.mat"] = Buffer<uint16_t>(vec_width, vec_height);
    auto input_copy_stencil = processor.inputs["input.mat"];
    for (int y = 0; y < input_copy_stencil.dim(1).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(0).extent(); x++) {
            input_copy_stencil(x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 14.0f - 7.0f);
        }
    }

    // Vec avg input
    processor.inputs["vec_avg.mat"] = Buffer<uint16_t>(vec_width, vec_height);
    auto vec_avg = Buffer<uint16_t>(1);
    vec_avg(0) = float_to_bfloat16_process(7.0f);

    // Gold output
    processor.output = Buffer<uint16_t>(vec_height);
    auto real_output = Buffer<uint16_t>(1);
    float sum = 0.0f;
    for (int y = 0; y < processor.inputs["input.mat"].dim(1).extent(); y++) {
        for (int x = 0; x < processor.inputs["input.mat"].dim(0).extent(); x++) {
            sum += (bfloat16_to_float_process(processor.inputs["input.mat"](x, y)) - bfloat16_to_float_process(vec_avg(0)))
                   * (bfloat16_to_float_process(processor.inputs["input.mat"](x, y)) - bfloat16_to_float_process(vec_avg(0)));
        }
    }
    const float vec_size = float(vec_width) * float(vec_height);
    real_output(0) = float_to_bfloat16_process(sqrt(sum / vec_size));

    save_halide_buffer_to_raw(processor.inputs["input.mat"], "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(vec_avg, "bin/vec_avg_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    auto output = processor.process_command(argc, argv);

    return output;
}