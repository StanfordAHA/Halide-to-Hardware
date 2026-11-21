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
#include "col_prefix_sum_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "col_prefix_sum_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    OneInOneOut_ProcessController<uint16_t> processor("col_prefix_sum_fp");

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        col_prefix_sum_fp(proc.input, proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.input, proc.output,
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
            col_prefix_sum_fp_clockwork(proc.input, proc.output);
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
    auto scan_rows_env = getenv("scan_rows");
    auto vec_height = vec_height_env ? atoi(vec_height_env) : 1024;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 128;
    auto scan_rows = scan_rows_env ? atoi(scan_rows_env) : 128;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Values in [-1, 0], like the Mamba decay terms dt * A that the SSD
    // segment sum adds down the rows, so every column is a decreasing running sum.
    processor.input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            processor.input(x, y) = float_to_bfloat16_process(-static_cast<float>(rand()) / RAND_MAX);
        }
    }

    // CPU reference: running sum down each column of each matrix of
    // scan_rows rows, in double precision, rounded once to BF16.
    processor.output = Buffer<uint16_t>(vec_width, vec_height);
    for (int x = 0; x < vec_width; x++) {
        double sum = 0.0;
        for (int y = 0; y < vec_height; y++) {
            if (y % scan_rows == 0) sum = 0.0;
            sum += bfloat16_to_float_process(processor.input(x, y));
            processor.output(x, y) = float_to_bfloat16_process(sum);
        }
    }

    save_halide_buffer_to_raw(processor.input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

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
