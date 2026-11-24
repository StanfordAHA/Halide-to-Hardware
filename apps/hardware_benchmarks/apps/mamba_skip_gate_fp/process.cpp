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
#include "mamba_skip_gate_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mamba_skip_gate_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

static int env_int(const char *name, int fallback) {
    auto value = getenv(name);
    return value ? atoi(value) : fallback;
}

static float random_value(float scale) {
    return ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * scale;
}

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("mamba_skip_gate_fp", { "input", "weight", "yheads", "gate", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mamba_skip_gate_fp(proc.inputs["input"], proc.inputs["weight"], proc.inputs["yheads"], proc.inputs["gate"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input"], proc.inputs["weight"], proc.inputs["yheads"], proc.inputs["gate"],
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
            mamba_skip_gate_fp_clockwork(proc.inputs["input"], proc.inputs["weight"], proc.inputs["yheads"], proc.inputs["gate"], proc.output);
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

    // One run = vec_width / head_dim heads x vec_height positions: x and silu(z) are position-major slabs
    // [vec_height x vec_width], Y is the run's heads-first blocks [chunk, head, row in chunk, p]. D is the run's D values
    // (IO_INNERMOST_TILE_PARAMS: the parser slices it), or the whole per-head vector with the run at D[d_start ...].
    const int vec_width = env_int("vec_width", 128);
    const int vec_height = env_int("vec_height", 1024);
    const int head_dim = env_int("SKIP_GATE_HEAD_DIM", 64);
    const int chunk_rows = env_int("SKIP_GATE_CHUNK_ROWS", 256);
    const int heads = vec_width / head_dim;
    auto tile_params_env = getenv("IO_INNERMOST_TILE_PARAMS");
    const bool per_run_d = tile_params_env && strcmp(tile_params_env, "1") == 0;
    const int num_heads = per_run_d ? heads : env_int("SKIP_GATE_NUM_HEADS", 64);
    const int d_start = per_run_d ? 0 : heads * env_int("IO_INNERMOST_DIM_TILING_IDX", 0);
    if (vec_width % head_dim != 0 || vec_height % chunk_rows != 0 || d_start + heads > num_heads) {
        std::cerr << "Error: inconsistent shape: vec_width " << vec_width << ", vec_height " << vec_height << ", head_dim "
                  << head_dim << ", chunk_rows " << chunk_rows << ", heads " << d_start << ".." << d_start + heads - 1
                  << " of " << num_heads << std::endl;
        return 1;
    }

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_input = Buffer<uint16_t>(vec_width, vec_height);
    auto real_gate = Buffer<uint16_t>(vec_width, vec_height);
    auto real_yheads = Buffer<uint16_t>(head_dim, vec_height * heads);
    auto real_weight = Buffer<uint16_t>(num_heads);
    for (int y = 0; y < vec_height; y++) {
        for (int x = 0; x < vec_width; x++) {
            real_input(x, y) = float_to_bfloat16_process(random_value(4.0f));
            real_gate(x, y) = float_to_bfloat16_process(random_value(4.0f));
        }
    }
    for (int y = 0; y < real_yheads.dim(1).extent(); y++) {
        for (int x = 0; x < head_dim; x++) {
            real_yheads(x, y) = float_to_bfloat16_process(random_value(4.0f));
        }
    }
    for (int h = 0; h < num_heads; h++) {
        real_weight(h) = float_to_bfloat16_process(random_value(2.0f));
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): the tensors that voyager/scripts/aha_flow/parse_dnnLayer_tensors.py wrote
    // to the app dir (row-major, big-endian): hw_input_stencil.raw (x), hw_weight_stencil.raw (D, CGRA_WEIGHT_NODE),
    // hw_yheads_stencil.raw and hw_gate_stencil.raw (CGRA_EXTRA_INPUTS).
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_input_stencil.raw", real_input);
        load_raw_to_halide_buffer("hw_weight_stencil.raw", real_weight);
        load_raw_to_halide_buffer("hw_yheads_stencil.raw", real_yheads);
        load_raw_to_halide_buffer("hw_gate_stencil.raw", real_gate);
        std::cout << "Loaded hw_{input,weight,yheads,gate}_stencil.raw (heads " << d_start << ".." << d_start + heads - 1
                  << ")" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: g = bf16(bf16(Y[permuted] + bf16(x * D)) * gate), each op rounded once, as the graph does
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int y = 0; y < vec_height; y++) {
        const int chunk = y / chunk_rows, row = y % chunk_rows;
        for (int x = 0; x < vec_width; x++) {
            const int h = x / head_dim, p = x % head_dim;
            const float xd = bfloat16_to_float_process(float_to_bfloat16_process(
                bfloat16_to_float_process(real_input(x, y)) * bfloat16_to_float_process(real_weight(d_start + h))));
            const float yv = bfloat16_to_float_process(real_yheads(p, (chunk * heads + h) * chunk_rows + row));
            const float s = bfloat16_to_float_process(float_to_bfloat16_process(yv + xd));
            real_output(x, y) = float_to_bfloat16_process(s * bfloat16_to_float_process(real_gate(x, y)));
        }
    }

    // Placeholder definitions for the Halide inputs
    processor.inputs["input"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["weight"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["yheads"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["gate"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(real_input, "bin/input_host_stencil.raw");
    save_halide_buffer_to_raw(real_weight, "bin/weight_host_stencil.raw");
    save_halide_buffer_to_raw(real_yheads, "bin/yheads_host_stencil.raw");
    save_halide_buffer_to_raw(real_gate, "bin/gate_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config: D is one unpacked IO (WEIGHT_HOST_STENCIL_PACKED 0)
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"weight_host_stencil", {"x_coord", "E64_packed"}},
            tensor_spec{"yheads_host_stencil", {"x_coord"}},
            tensor_spec{"gate_host_stencil", {"x_coord"}},
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
