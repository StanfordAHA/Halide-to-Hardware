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
#include "mamba_x_dt_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mamba_x_dt_fp_clockwork.h"
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
    ManyInOneOut_ProcessController<uint16_t> processor("mamba_x_dt_fp", { "input", "dt", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mamba_x_dt_fp(proc.inputs["input"], proc.inputs["dt"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.inputs["dt"],
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
            mamba_x_dt_fp_clockwork(proc.inputs["input"], proc.inputs["dt"], proc.output);
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

    // One run = one pass (16 of the layout's 32 lanes) of num_heads heads x vec_height positions. Of head h it sees the
    // hold B = head_dim / layout_lanes groups g of `unroll` channels, so x (`input`) is [vec_height x vec_width] with
    // column (B h + g) unroll + j, dt is [vec_height x num_heads], and X (`output`) is heads-first
    // [num_heads vec_height x B unroll]: row h vec_height + i, column g unroll + j.
    const int vec_width = env_int("vec_width", 256);
    const int vec_height = env_int("vec_height", 256);
    const int num_heads = env_int("MUL_X_DT_NUM_HEADS", 8);
    const int head_dim = env_int("MUL_X_DT_HEAD_DIM", 64);
    const int layout_lanes = env_int("MUL_X_DT_LAYOUT_LANES", 32);
    const int hold = head_dim / layout_lanes;
    if (hold < 1 || head_dim % layout_lanes != 0 || vec_width % (num_heads * hold) != 0) {
        std::cerr << "Error: inconsistent shape: vec_width " << vec_width << ", num_heads " << num_heads << ", head_dim "
                  << head_dim << ", layout_lanes " << layout_lanes << std::endl;
        return 1;
    }
    const int unroll = vec_width / (num_heads * hold);
    // dt in its chip layout (softplus's 32-lane GLB tile, word 2 l + s): position i's value is word
    // dt_start + dt_row_stride * i of dt_rows words per lane (defaults: the run's slice, one word per position)
    const int dt_row_stride = env_int("MUL_X_DT_DT_ROW_STRIDE", 1);
    const int dt_start = env_int("MUL_X_DT_DT_START", 0);
    const int dt_rows = env_int("MUL_X_DT_DT_ROWS", vec_height);
    if (dt_start + dt_row_stride * (vec_height - 1) >= dt_rows) {
        std::cerr << "Error: dt words " << dt_start << " + " << dt_row_stride << " x " << vec_height - 1
                  << " exceed the " << dt_rows << " loaded" << std::endl;
        return 1;
    }

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    auto real_dt = Buffer<uint16_t>(num_heads, dt_rows);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            real_input(x, y) = float_to_bfloat16_process(random_value(-4.0f, 4.0f));
        }
    }
    for (int y = 0; y < dt_rows; y++) {
        for (int h = 0; h < num_heads; h++) {
            real_dt(h, y) = float_to_bfloat16_process(random_value(0.0f, 2.0f));
        }
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): the tensors that voyager/scripts/aha_flow/parse_dnnLayer_tensors.py wrote
    // to the app dir (row-major, big-endian, CGRA_EXTRA_INPUTS): hw_input_stencil.raw (the pass's half of the x slab)
    // and hw_dt_stencil.raw (the run's dt heads)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        load_raw_to_halide_buffer("hw_dt_stencil.raw", real_dt);
        std::cout << "Loaded hw_{input,dt}_stencil.raw" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: X = bf16(x * dt), heads-first
    auto real_output = Buffer<uint16_t>(hold * unroll, num_heads * vec_height);
    for (int h = 0; h < num_heads; h++) {
        for (int i = 0; i < vec_height; i++) {
            const float d = bfloat16_to_float_process(real_dt(h, dt_start + dt_row_stride * i));
            for (int g = 0; g < hold; g++) {
                for (int j = 0; j < unroll; j++) {
                    const float x = bfloat16_to_float_process(real_input((hold * h + g) * unroll + j, i));
                    real_output(g * unroll + j, h * vec_height + i) = float_to_bfloat16_process(x * d);
                }
            }
        }
    }

    // Placeholder definitions for the Halide inputs
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["dt"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_dt, "bin/dt_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"dt_host_stencil", {"x_coord"}},
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
