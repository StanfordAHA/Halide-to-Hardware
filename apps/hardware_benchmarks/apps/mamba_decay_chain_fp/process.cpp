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
#include "mamba_decay_chain_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "mamba_decay_chain_fp_clockwork.h"
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
    ManyInOneOut_ProcessController<uint16_t> processor("mamba_decay_chain_fp", { "a", "mask_strict", "mask_lower", "output" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        mamba_decay_chain_fp(proc.inputs["a"], proc.inputs["mask_strict"], proc.inputs["mask_lower"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["a"], proc.inputs["mask_strict"], proc.inputs["mask_lower"],
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
            mamba_decay_chain_fp_clockwork(proc.inputs["a"], proc.inputs["mask_strict"], proc.inputs["mask_lower"], proc.output);
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

    // One run = num_mats = vec_height / scan_rows heads x scan_rows rows of one 16-lane pass (vec_width columns: column
    // m unroll + j is lane j's group m). a (`a`) is one packed GLB tile of mul_6's 16-lane layout [a_rows x a_lanes]:
    // head s's value at row i is a(a_lane, a_start + a_row_words i + s). The masks are [scan_rows x vec_width], the output
    // [vec_height x vec_width], row s scan_rows + i.
    const int vec_width = env_int("vec_width", 128);
    const int vec_height = env_int("vec_height", 1024);
    const int scan_rows = env_int("DECAY_CHAIN_SCAN_ROWS", 256);
    const int a_lanes = env_int("DECAY_CHAIN_A_LANES", 8);
    const int a_lane = env_int("DECAY_CHAIN_A_LANE", 0);
    const int a_rows = env_int("DECAY_CHAIN_A_ROWS", 4096);
    const int a_row_words = env_int("DECAY_CHAIN_A_ROW_WORDS", 4);
    const int a_start = env_int("DECAY_CHAIN_A_START", 0);
    const int num_mats = vec_height / scan_rows;
    if (vec_height % scan_rows != 0 || a_lane < 0 || a_lane >= a_lanes ||
        a_start + a_row_words * (scan_rows - 1) + num_mats - 1 >= a_rows) {
        std::cerr << "Error: inconsistent shape: vec_height " << vec_height << ", scan_rows " << scan_rows << ", a lane "
                  << a_lane << " of " << a_lanes << ", a words " << a_start << " + " << a_row_words << " x "
                  << scan_rows - 1 << " of " << a_rows << std::endl;
        return 1;
    }

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    auto real_a = Buffer<uint16_t>(a_lanes, a_rows);
    auto real_mask_strict = Buffer<uint16_t>(vec_width, scan_rows);
    auto real_mask_lower = Buffer<uint16_t>(vec_width, scan_rows);
    for (int y = 0; y < a_rows; y++) {
        for (int x = 0; x < a_lanes; x++) {
            real_a(x, y) = float_to_bfloat16_process(random_value(-0.8f, 0.0f));
        }
    }
    // Random runs: column x stands for column x of a 256-wide matrix
    for (int i = 0; i < scan_rows; i++) {
        for (int x = 0; x < vec_width; x++) {
            real_mask_strict(x, i) = float_to_bfloat16_process(i > x ? 1.0f : 0.0f);
            real_mask_lower(x, i) = float_to_bfloat16_process(i >= x ? 1.0f : 0.0f);
        }
    }

    // USE_REAL_INPUT=1 (voyager_cgra tests): the tensors that voyager/scripts/aha_flow/parse_dnnLayer_tensors.py wrote
    // to the app dir (row-major, big-endian, CGRA_EXTRA_INPUTS): hw_a_stencil.raw (mul_6's GLB tile),
    // hw_mask_strict_stencil.raw and hw_mask_lower_stencil.raw (the pass's half of the masks)
    auto real_input_env = getenv("USE_REAL_INPUT");
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        load_raw_to_halide_buffer("hw_a_stencil.raw", real_a);
        load_raw_to_halide_buffer("hw_mask_strict_stencil.raw", real_mask_strict);
        load_raw_to_halide_buffer("hw_mask_lower_stencil.raw", real_mask_lower);
        std::cout << "Loaded hw_{a,mask_strict,mask_lower}_stencil.raw" << std::endl;
    } else {
        std::cout << "Generating random tensors" << std::endl;
    }

    // Gold: bf16(exp(S)) * lower, S the column scan of a * strict with a bf16 rounding after each add
    auto real_output = Buffer<uint16_t>(vec_width, vec_height);
    for (int s = 0; s < num_mats; s++) {
        for (int x = 0; x < vec_width; x++) {
            float sum = 0.0f;
            for (int i = 0; i < scan_rows; i++) {
                const float a = bfloat16_to_float_process(real_a(a_lane, a_start + a_row_words * i + s));
                const float strict = bfloat16_to_float_process(real_mask_strict(x, i));
                sum = bfloat16_to_float_process(float_to_bfloat16_process(sum + a * strict));
                const float e = bfloat16_to_float_process(float_to_bfloat16_process(expf(sum)));
                real_output(x, s * scan_rows + i) =
                    float_to_bfloat16_process(e * bfloat16_to_float_process(real_mask_lower(x, i)));
            }
        }
    }

    // Placeholder definitions for the Halide inputs
    processor.inputs["a"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["mask_strict"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.inputs["mask_lower"] = Buffer<uint16_t>(vec_width, vec_height);
    processor.output = Buffer<uint16_t>(vec_width, vec_height);

    save_halide_buffer_to_raw(real_a, "bin/a_host_stencil.raw");
    save_halide_buffer_to_raw(real_mask_strict, "bin/mask_strict_host_stencil.raw");
    save_halide_buffer_to_raw(real_mask_lower, "bin/mask_lower_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"a_host_stencil", {"x_coord"}},
            tensor_spec{"mask_strict_host_stencil", {"x_coord"}},
            tensor_spec{"mask_lower_host_stencil", {"x_coord"}},
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
