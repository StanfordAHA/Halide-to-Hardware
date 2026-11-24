#include "coreir.h"
#include "halide_image_io.h"
#include "hardware_process_helper.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "mamba_exp_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mamba_exp_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

static int env_int(const char *name, int fallback) {
    auto value = getenv(name);
    return value ? atoi(value) : fallback;
}

static float random_value(float lo, float hi) {
    return lo + (static_cast<float>(rand()) / RAND_MAX) * (hi - lo);
}

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("mamba_exp_fp", { "input", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mamba_exp_fp(proc.inputs["input"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"],
                                    proc.output, "self.in_arg_0_0_0", "self.out_0_0");
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
            mamba_exp_fp_clockwork(proc.inputs["input"], proc.output);
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

    // One run = exp of a [vec_height x vec_width] tensor, elementwise (Mamba2 exp_2 = exp(a_cum), a_cum in its producer's
    // layout: rows 64 c + h, columns i)
    const int vec_width = env_int("vec_width", 256);
    const int vec_height = env_int("vec_height", 256);

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            real_input(x, y) = float_to_bfloat16_process(random_value(-20.0f, 0.0f));
        }
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): hw_input_stencil.raw, written to the app dir by
    // voyager/scripts/aha_flow/parse_dnnLayer_tensors.py (row-major, big-endian, CGRA_EXTRA_INPUTS)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        std::cout << "Loaded hw_input_stencil.raw" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: bf16(exp(x))
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            real_output(x, y) = float_to_bfloat16_process(expf(bfloat16_to_float_process(real_input(x, y))));
        }
    }

    // Placeholder definitions for the Halide inputs
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

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
