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
#include "mamba_x_end_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mamba_x_end_fp_clockwork.h"
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
    ManyInOneOut_ProcessController<uint16_t> processor("mamba_x_end_fp", { "input", "dt", "decay", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mamba_x_end_fp(proc.inputs["input"], proc.inputs["dt"], proc.inputs["decay"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.inputs["dt"], proc.inputs["decay"],
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
            mamba_x_end_fp_clockwork(proc.inputs["input"], proc.inputs["dt"], proc.inputs["decay"], proc.output);
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
    // hold B = head_dim / layout_lanes groups g of `unroll` channels: x (`input`) and X_end (`output`) are
    // [vec_height x vec_width], column (B h + g) unroll + j, position-major; dt and decay_to_end (`decay`) are
    // [vec_height x num_heads].
    const int vec_width = env_int("vec_width", 256);
    const int vec_height = env_int("vec_height", 256);
    const int num_heads = env_int("MUL_GATHER_NUM_HEADS", 8);
    const int head_dim = env_int("MUL_GATHER_HEAD_DIM", 64);
    const int layout_lanes = env_int("MUL_GATHER_LAYOUT_LANES", 32);
    const int hold = head_dim / layout_lanes;
    if (hold < 1 || head_dim % layout_lanes != 0 || vec_width % (num_heads * hold) != 0) {
        std::cerr << "Error: inconsistent shape: vec_width " << vec_width << ", num_heads " << num_heads << ", head_dim "
                  << head_dim << ", layout_lanes " << layout_lanes << std::endl;
        return 1;
    }
    const int unroll = vec_width / (num_heads * hold);

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    auto real_dt = Buffer<uint16_t>(num_heads, vec_height);
    auto real_decay = Buffer<uint16_t>(num_heads, vec_height);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            real_input(x, y) = float_to_bfloat16_process(random_value(-4.0f, 4.0f));
        }
        for (int h = 0; h < num_heads; h++) {
            real_dt(h, y) = float_to_bfloat16_process(random_value(0.0f, 2.0f));
            real_decay(h, y) = float_to_bfloat16_process(random_value(0.0f, 1.0f));
        }
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): the tensors that voyager/scripts/aha_flow/parse_dnnLayer_tensors.py wrote
    // to the app dir (row-major, big-endian, CGRA_EXTRA_INPUTS): hw_input_stencil.raw (the pass's half of the x slab),
    // hw_dt_stencil.raw and hw_decay_stencil.raw (the run's heads)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        load_raw_to_halide_buffer("hw_dt_stencil.raw", real_dt);
        load_raw_to_halide_buffer("hw_decay_stencil.raw", real_decay);
        std::cout << "Loaded hw_{input,dt,decay}_stencil.raw" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: X_end = bf16(bf16(x * dt) * decay_to_end), position-major, each multiply rounded once as the graph does
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int i = 0; i < vec_height; i++) {
        for (int x = 0; x < vec_width; x++) {
            const int h = x / (hold * unroll);
            const float xdt = bfloat16_to_float_process(float_to_bfloat16_process(
                bfloat16_to_float_process(real_input(x, i)) * bfloat16_to_float_process(real_dt(h, i))));
            real_output(x, i) = float_to_bfloat16_process(xdt * bfloat16_to_float_process(real_decay(h, i)));
        }
    }

    // Placeholder definitions for the Halide inputs
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["dt"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["decay"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_dt, "bin/dt_host_stencil.raw");
    save_halide_buffer_to_raw(real_decay, "bin/decay_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"dt_host_stencil", {"x_coord"}},
            tensor_spec{"decay_host_stencil", {"x_coord"}},
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
