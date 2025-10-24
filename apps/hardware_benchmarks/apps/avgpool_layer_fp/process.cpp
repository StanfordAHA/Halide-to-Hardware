#include "bf16_op.h"
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
#include "avgpool_layer_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "avgpool_layer_fp_clockwork.h"
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("avgpool_layer_fp", { "input" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        avgpool_layer_fp(proc.inputs["input"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.output,
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
            avgpool_layer_fp_clockwork(proc.inputs["input"], proc.output);
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

    auto in_img_env = getenv("in_img");
    auto n_ic_env = getenv("n_ic");

    auto in_img = in_img_env ? atoi(in_img_env) : 7;
    auto n_ic = n_ic_env ? atoi(n_ic_env) : 512;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["input"] = Buffer<uint16_t>(n_ic, in_img, in_img);
    auto input_copy_stencil = processor.inputs["input"];
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
            for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
                input_copy_stencil(c, x, y) = float_to_bfloat16_process(static_cast<float>((rand() / (float) RAND_MAX) * 60.0f - 30.0f));
            }
        }
    }

    // Gold output
    processor.output = Buffer<uint16_t>(n_ic, 1, 1);
    auto gold_output = processor.output;
    for (int c = 0; c < gold_output.dim(0).extent(); c++) {
        float sum = 0.0f;
        for (int y = 0; y < in_img; y++) {
            for (int x = 0; x < in_img; x++) {
                sum += bfloat16_to_float_process(input_copy_stencil(c, x, y));
            }
        }
        float avg = sum / (in_img * in_img);
        gold_output(c, 0, 0) = float_to_bfloat16_process(avg);
    }


    save_halide_buffer_to_raw(processor.inputs["input"], "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(gold_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    const config_spec spec = {
        {tensor_spec{"input_host_stencil", {"x_coord"}}},
        {tensor_spec{"hw_output", {"x_coord"}}}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}