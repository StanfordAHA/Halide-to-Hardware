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
#include "mamba_state_decay_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mamba_state_decay_fp_clockwork.h"
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
    ManyInOneOut_ProcessController<uint16_t> processor("mamba_state_decay_fp", { "input", "decay", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mamba_state_decay_fp(proc.inputs["input"], proc.inputs["decay"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.inputs["decay"],
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
            mamba_state_decay_fp_clockwork(proc.inputs["input"], proc.inputs["decay"], proc.output);
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

    // One run = one pass (16 of the layout's 32 lanes) of the num_heads x slots heads of one chunk-decay GLB tile (lane j,
    // word slot s: head h0 + j + 32 s) x vec_height state rows. Of each head it sees the hold B = head_dim / layout_lanes
    // groups g of `unroll` channels: the state (`input`) and the output are [vec_height x vec_width], column
    // (B (num_heads s + h) + g) unroll + j; the chunk decay (`decay`) is [slots x num_heads], the same for every row.
    const int vec_width = env_int("vec_width", 512);
    const int vec_height = env_int("vec_height", 128);
    const int num_heads = env_int("MUL_GATHER_NUM_HEADS", 8);
    const int slots = env_int("MUL_GATHER_SLOTS", 2);
    const int head_dim = env_int("MUL_GATHER_HEAD_DIM", 64);
    const int layout_lanes = env_int("MUL_GATHER_LAYOUT_LANES", 32);
    const int hold = head_dim / layout_lanes;
    if (hold < 1 || slots < 1 || head_dim % layout_lanes != 0 || vec_width % (num_heads * slots * hold) != 0) {
        std::cerr << "Error: inconsistent shape: vec_width " << vec_width << ", num_heads " << num_heads << ", slots "
                  << slots << ", head_dim " << head_dim << ", layout_lanes " << layout_lanes << std::endl;
        return 1;
    }
    const int unroll = vec_width / (num_heads * slots * hold);
    // Lane source (MUL_GATHER_SCALAR_LANE set): the decay is one packed GLB tile [decay_rows words x decay_lanes] (exp_2's
    // output GLB tile 3) and head h's decay is word decay_start + decay_head_stride h of lane decay_lane
    const int decay_lane = env_int("MUL_GATHER_SCALAR_LANE", -1);
    const int decay_lanes = env_int("MUL_GATHER_SCALAR_LANES", 8);
    const int decay_head_stride = env_int("MUL_GATHER_SCALAR_HEAD_STRIDE", 8);
    const int decay_start = env_int("MUL_GATHER_SCALAR_START", 0);
    const int decay_rows = env_int("MUL_GATHER_SCALAR_ROWS", 2048);
    const bool lane_source = decay_lane >= 0;
    if (lane_source && (slots != 1 || decay_lane >= decay_lanes ||
                        decay_start + decay_head_stride * (num_heads - 1) >= decay_rows)) {
        std::cerr << "Error: lane-source decay: slots " << slots << ", lane " << decay_lane << " of " << decay_lanes
                  << ", words " << decay_start << " + " << decay_head_stride << " x " << num_heads - 1 << " of "
                  << decay_rows << std::endl;
        return 1;
    }

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    auto real_decay = lane_source ? Buffer<uint16_t>(decay_lanes, decay_rows) : Buffer<uint16_t>(num_heads, slots);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            real_input(x, y) = float_to_bfloat16_process(random_value(-4.0f, 4.0f));
        }
    }
    for (int y = 0; y < real_decay.dim(1).extent(); y++) {
        for (int x = 0; x < real_decay.dim(0).extent(); x++) {
            real_decay(x, y) = float_to_bfloat16_process(random_value(0.0f, 1.0f));
        }
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): the tensors that voyager/scripts/aha_flow/parse_dnnLayer_tensors.py wrote
    // to the app dir (row-major, big-endian, CGRA_EXTRA_INPUTS): hw_input_stencil.raw (the pass's half of the run's
    // state heads) and hw_decay_stencil.raw (the run's chunk decays)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        load_raw_to_halide_buffer("hw_decay_stencil.raw", real_decay);
        std::cout << "Loaded hw_{input,decay}_stencil.raw" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: bf16(state * decay[head])
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int n = 0; n < vec_height; n++) {
        for (int x = 0; x < vec_width; x++) {
            const int head = x / (hold * unroll);
            const float d = bfloat16_to_float_process(
                lane_source ? real_decay(decay_lane, decay_start + decay_head_stride * head)
                            : real_decay(head % num_heads, head / num_heads));
            real_output(x, n) = float_to_bfloat16_process(bfloat16_to_float_process(real_input(x, n)) * d);
        }
    }

    // Placeholder definitions for the Halide inputs
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["decay"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_decay, "bin/decay_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
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
