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
#include "rope_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "rdai_api.h"
#include "rope_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("rope_fp", { "input", "cos_buf", "sin_buf" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        rope_fp(proc.inputs["input"], proc.inputs["cos_buf"], proc.inputs["sin_buf"], proc.output);
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
            rope_fp_clockwork(proc.inputs["input"], proc.inputs["cos_buf"], proc.inputs["sin_buf"], proc.output);
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

    auto n_heads_env = getenv("n_heads");
    auto seq_len_env = getenv("seq_len");
    auto head_dim_half_env = getenv("head_dim_half");
    auto glb_i_env = getenv("glb_i");

    auto n_heads = n_heads_env ? atoi(n_heads_env) : 32;
    auto seq_len = seq_len_env ? atoi(seq_len_env) : 512;
    auto head_dim_half = head_dim_half_env ? atoi(head_dim_half_env) : 32;
    auto head_dim = 2 * head_dim_half;
    auto glb_i = glb_i_env ? atoi(glb_i_env) : 16;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Processor inputs match generator extents (placeholders for Halide pipeline)
    processor.inputs["input"] = Buffer<uint16_t>(head_dim_half, seq_len, n_heads);
    processor.inputs["cos_buf"] = Buffer<uint16_t>(head_dim_half, seq_len, n_heads);
    processor.inputs["sin_buf"] = Buffer<uint16_t>(head_dim_half, seq_len, n_heads);
    processor.output = Buffer<uint16_t>(head_dim_half, seq_len, n_heads);

    // Real tensors
    Buffer<uint16_t> real_input(head_dim, seq_len, n_heads);   // (64, 512, 32)
    Buffer<uint16_t> real_cos(head_dim, seq_len, 1);           // (64, 512, 1)
    Buffer<uint16_t> real_sin(head_dim, seq_len, 1);           // (64, 512, 1)
    Buffer<uint16_t> real_output(head_dim, seq_len, n_heads);  // (64, 512, 32)

    // Fill input with random data
    for (int n = 0; n < n_heads; n++) {
        for (int s = 0; s < seq_len; s++) {
            for (int h = 0; h < head_dim; h++) {
                real_input(h, s, n) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 32.0f - 16.0f);
            }
        }
    }

    // Fill cos/sin with random data (broadcast over n_heads, same for upper and lower halves)
    for (int s = 0; s < seq_len; s++) {
        for (int h = 0; h < head_dim_half; h++) {
            float cos_val = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            float sin_val = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            real_cos(h, s, 0) = float_to_bfloat16_process(cos_val);
            real_cos(h + head_dim_half, s, 0) = float_to_bfloat16_process(cos_val);
            real_sin(h, s, 0) = float_to_bfloat16_process(sin_val);
            real_sin(h + head_dim_half, s, 0) = float_to_bfloat16_process(sin_val);
        }
    }

    // Gold output: full rotate-half RoPE
    // out[0:D/2]   = in[0:D/2]   * cos[0:D/2]   - in[D/2:D]   * sin[0:D/2]
    // out[D/2:D]   = in[D/2:D]   * cos[D/2:D]   + in[0:D/2]   * sin[D/2:D]
    for (int n = 0; n < n_heads; n++) {
        for (int s = 0; s < seq_len; s++) {
            for (int h = 0; h < head_dim_half; h++) {
                float in_lower = bfloat16_to_float_process(real_input(h, s, n));
                float in_upper = bfloat16_to_float_process(real_input(h + head_dim_half, s, n));
                float cos_lower = bfloat16_to_float_process(real_cos(h, s, 0));
                float sin_lower = bfloat16_to_float_process(real_sin(h, s, 0));
                float cos_upper = bfloat16_to_float_process(real_cos(h + head_dim_half, s, 0));
                float sin_upper = bfloat16_to_float_process(real_sin(h + head_dim_half, s, 0));

                // Lower half output: out[0:D/2]
                real_output(h, s, n) = float_to_bfloat16_process(
                    in_lower * cos_lower - in_upper * sin_lower);

                // Upper half output: out[D/2:D]
                real_output(h + head_dim_half, s, n) = float_to_bfloat16_process(
                    in_upper * cos_upper + in_lower * sin_upper);
            }
        }
    }

    // Tile the head dimension: take glb_i consecutive, skip glb_i, repeat.
    // Raw files only contain half the head dim (the selected tiles).
    int num_tiles = head_dim_half / glb_i;
    Buffer<uint16_t> tiled_input(head_dim_half, seq_len, n_heads);
    Buffer<uint16_t> tiled_cos(head_dim_half, seq_len, 1);
    Buffer<uint16_t> tiled_sin(head_dim_half, seq_len, 1);
    Buffer<uint16_t> tiled_output(head_dim_half, seq_len, n_heads);

    for (int t = 0; t < num_tiles; t++) {
        for (int j = 0; j < glb_i; j++) {
            int src_h = t * 2 * glb_i + j; // int src_h = t * 2 * glb_i + glb_i + j; to select another half
            int dst_h = t * glb_i + j;
            for (int s = 0; s < seq_len; s++) {
                for (int n = 0; n < n_heads; n++) {
                    tiled_input(dst_h, s, n) = real_input(src_h, s, n);
                    tiled_output(dst_h, s, n) = real_output(src_h, s, n);
                }
                tiled_cos(dst_h, s, 0) = real_cos(src_h, s, 0);
                tiled_sin(dst_h, s, 0) = real_sin(src_h, s, 0);
            }
        }
    }

    auto real_input_env = getenv("USE_REAL_INPUT");
    bool use_random_tensors = true;
    if (real_input_env && strcmp(real_input_env, "1") == 0) {
        use_random_tensors = false;
    }

    if (use_random_tensors) {
        // Save tiled tensors to raw files for CGRA testing
        std::cout << "Generating random tensors" << std::endl;
        save_halide_buffer_to_raw(tiled_input, "bin/input_host_stencil.raw");
        save_halide_buffer_to_raw(tiled_cos, "bin/cos_host_stencil.raw");
        save_halide_buffer_to_raw(tiled_sin, "bin/sin_host_stencil.raw");
        save_halide_buffer_to_raw(tiled_output, "bin/hw_output.raw");
    } else {


         // Copy the pre-existing raw file into the bin folder for now
        int ret = system("cp hw_input_stencil.raw bin/input_host_stencil.raw");
        if (ret != 0) {
            std::cerr << "Error: Failed to copy hw_input_stencil.raw to bin folder. "
                        "hw_input_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow/parse_dnnLayer_tensors.py "
                        "(system call returned " << ret << ")" << std::endl;
            return 1;
        }
        std::cout << "Copying pre-existing hw_input_stencil.raw to bin/input_host_stencil.raw" << std::endl;

        // Copy the pre-existing raw file into the bin folder for now
        ret = system("cp hw_weight_stencil.raw bin/cos_host_stencil.raw");
        if (ret != 0) {
            std::cerr << "Error: Failed to copy hw_weight_stencil.raw to bin folder. "
                        "hw_weight_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow/parse_dnnLayer_tensors.py "
                        "(system call returned " << ret << ")" << std::endl;
            return 1;
        }
        std::cout << "Copying pre-existing hw_weight_stencil.raw to bin/cos_host_stencil.raw" << std::endl;

        // Copy the pre-existing raw file into the bin folder for now
        ret = system("cp hw_bias_stencil.raw bin/sin_host_stencil.raw");
        if (ret != 0) {
            std::cerr << "Error: Failed to copy hw_bias_stencil.raw to bin folder. "
                        "hw_bias_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow/parse_dnnLayer_tensors.py "
                        "(system call returned " << ret << ")" << std::endl;
            return 1;
        }
        std::cout << "Copying pre-existing hw_bias_stencil.raw to bin/sin_host_stencil.raw" << std::endl;
    }

    // Create glb bank config
    using namespace glb_cfg;
    const config_spec spec = {
        {
            tensor_spec{"input_host_stencil", {"x_coord"}},
            tensor_spec{"cos_host_stencil", {"x_coord"}},
            tensor_spec{"sin_host_stencil", {"x_coord"}}
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
