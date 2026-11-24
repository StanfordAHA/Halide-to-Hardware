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
#include "transpose_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "transpose_fp_clockwork.h"
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
    ManyInOneOut_ProcessController<uint16_t> processor("transpose_fp", { "input", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        transpose_fp(proc.inputs["input"], proc.output);
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
            transpose_fp_clockwork(proc.inputs["input"], proc.output);
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

    // One pass of the batched transpose [batch, X, Y] -> [batch, Y, X] (strait transpose_bf16; the same env as the
    // hook). The input is spread over in_lanes lanes (lane = y mod in_lanes, word = (b X + x) Y / in_lanes + y div
    // in_lanes); this pass reads lane k = TRANSPOSE_KERNEL_ID, so the input is its GLB tile (8 lanes; lane k mod 8 is
    // read). The output is the whole transposed tensor [batch * Y rows, X columns]; the pass writes rows (b, y) with
    // y = k mod in_lanes, the rest stays 0 (the GLB starts at 0, and the testbench reads the whole output back).
    const int batch = env_int("TRANSPOSE_BATCH", 4);
    const int X = env_int("TRANSPOSE_X", 64);
    const int Y = env_int("TRANSPOSE_Y", 256);
    const int in_lanes = env_int("TRANSPOSE_IN_LANES", 32);
    const int k = env_int("TRANSPOSE_KERNEL_ID", 0);
    const int tile_lanes = 8;
    const int g_in = Y / in_lanes;
    const int words = batch * X * g_in;
    if (Y % in_lanes != 0 || k < 0 || k >= in_lanes) {
        std::cerr << "[ERROR] TRANSPOSE_Y " << Y << " / TRANSPOSE_IN_LANES " << in_lanes << " or TRANSPOSE_KERNEL_ID " << k
                  << " invalid" << std::endl;
        return 1;
    }

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input: word w, lane l of the pass's GLB tile
    auto real_input = Buffer<uint16_t>(tile_lanes, words);
    for (int w = 0; w < words; w++) {
        for (int l = 0; l < tile_lanes; l++) {
            real_input(l, w) = float_to_bfloat16_process(random_value(-1.0f, 1.0f));
        }
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): hw_input_stencil.raw, written to the app dir by
    // voyager/scripts/aha_flow/parse_dnnLayer_tensors.py (row-major [words][8 lanes], big-endian, CGRA_EXTRA_INPUTS)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        std::cout << "Loaded hw_input_stencil.raw" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: out(x, b Y + y) = in[b, x, y] for y = k mod in_lanes, else 0
    auto real_output = Buffer<uint16_t>(X, batch * Y);
    real_output.fill(0);
    for (int b = 0; b < batch; b++) {
        for (int yg = 0; yg < g_in; yg++) {
            for (int x = 0; x < X; x++) {
                real_output(x, b * Y + yg * in_lanes + k) = real_input(k % tile_lanes, (b * X + x) * g_in + yg);
            }
        }
    }

    // Placeholder definitions for the Halide inputs: the generator's bounds (HALIDE_GEN_ARGS vec_width x vec_height),
    // which the Halide pipeline checks; the real input and gold above have their own shapes
    const int vec_width = env_int("vec_width", 64);
    const int vec_height = env_int("vec_height", 1024);
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
