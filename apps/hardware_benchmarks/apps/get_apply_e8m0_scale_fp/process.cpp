#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
#include "get_apply_e8m0_scale_fp.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "get_apply_e8m0_scale_fp_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("get_apply_e8m0_scale_fp", { "mu_input.mat" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        get_apply_e8m0_scale_fp(proc.inputs["mu_input.mat"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["mu_input.mat"], proc.output,
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
            get_apply_e8m0_scale_fp_clockwork(proc.inputs["mu_input.mat"], proc.output);
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
    auto vec_width_fake_env = getenv("vec_width_fake");
    auto vec_width_env = getenv("vec_width");
    auto mu_i_env = getenv("mu_i");

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 3136;
    auto vec_width_fake = vec_width_fake_env ? atoi(vec_width_fake_env) : 32;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 64;
    auto mu_i = mu_i_env ? atoi(mu_i_env) : 32;
    const int block_size = 64;

    // Assert that vec_height is even for proper bit packing
    assert((vec_height % 2 == 0) && "vec_height must be even for proper bit packing");

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // Input
    processor.inputs["mu_input.mat"] = Buffer<uint16_t>(vec_width_fake, vec_height);
    auto real_mu_input = Buffer<uint16_t>(mu_i, vec_height, int(vec_width / mu_i));
    for (int z = 0; z < real_mu_input.dim(2).extent(); z++) {
        for (int y = 0; y < real_mu_input.dim(1).extent(); y++) {
            for (int x = 0; x < real_mu_input.dim(0).extent(); x++) {
                // // Use [-10, 10], which is a typical range for activations of most models
                // real_mu_input(x, y, z) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 20.0f - 10.0f);
                // Use [-1024, 1024], which helps check scale output since otherwise scales are all the same
                real_mu_input(x, y, z) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2048.0f - 1024.0f);
            }
        }
    }

    // Gold scale output - packed along dimension 0
    processor.output = Buffer<uint16_t>(int(vec_height));
    auto real_output = Buffer<uint16_t>(int(vec_height / 2), int(vec_width / block_size));
    for (int y = 0; y < real_mu_input.dim(1).extent(); y++) {
        // For each block along width (e.g., block_size=64, vec_width=128 -> 2 blocks)
        for (int b = 0; b < int(vec_width / block_size); b++) {
            float max_val = 0.0f;
            uint16_t max_val_bf16 = float_to_bfloat16_process(max_val);

            // For channels in the block (e.g., 64 channels)
            for (int ch = 0; ch < block_size; ch++) {
                int channel_idx = b * block_size + ch;
                int x = channel_idx % mu_i;
                int z = channel_idx / mu_i;
                if (z < real_mu_input.dim(2).extent() && x < real_mu_input.dim(0).extent()) {
                    max_val_bf16 = abs_max(max_val_bf16, real_mu_input(x, y, z));
                }
            }

            // Pack every two 8-bit scale into one 16-bit word along dimension 0
            uint16_t packed_scale = get_shared_exp(max_val_bf16);
            if (y & 1) {
                // Odd y: pack with the previous even y value
                real_output(int(y / 2), b) = bit8_pack(packed_scale, real_output(int(y / 2), b));
            } else {
                // Even y: store as low byte, will be packed with next odd y
                real_output(int(y / 2), b) = packed_scale;
            }
        }
    }

    // Gold quantized output in INT8 with packing of two 8-bit INT8 into one 16-bit word
    auto quantized_output_packed = Buffer<uint16_t>(int(block_size / 2), vec_height, int(vec_width / block_size));
    int overflow_count = 0;
    for (int b = 0; b < int(vec_width / block_size); b++) {
        for (int p = 0; p < vec_height; p++) {
            for (int c = 0; c < int(block_size / 2); c++) {
                int c0 = b * block_size + (c * 2);
                int c1 = c0 + 1;

                // Channel c0, write low byte first
                int b0 = c0 / block_size;
                // Extract the appropriate 8-bit scale from packed data
                uint8_t exp0;
                if (p & 1) {
                    // Odd pixel: extract high byte from packed data
                    exp0 = static_cast<uint8_t>((real_output(int(p / 2), b0) >> 8) & 0xFF);
                } else {
                    // Even pixel: extract low byte from packed data
                    exp0 = static_cast<uint8_t>(real_output(int(p / 2), b0) & 0xFF);
                }
                float actual_scale0 = powf(2.0f, static_cast<float>(exp0) - 127.0f);
                int ch_in_tile0 = c0 % mu_i;
                int tile_idx0 = c0 / mu_i;
                uint8_t q0 = 0;
                if (tile_idx0 < real_mu_input.dim(2).extent() && ch_in_tile0 < real_mu_input.dim(0).extent()) {
                    float f0 = bfloat16_to_float_process(real_mu_input(ch_in_tile0, p, tile_idx0));
                    int q = lroundf(f0 / actual_scale0);
                    // For quantized data, we clamp to INT8 range.
                    // We should do this in quant_e8m0 test as well. But unfortunately, we missed it and the quantized data might overflow.
                    // Zircon hardware will have the overflow problem as well, when the quantized data is >= 127.5 in some corner cases.
                    if (q < -128) q = -128;
                    // We comment out the gold overflow saturation to match the Zircon hardware behavior.
                    // if (q > 127)  q = 127;
                    if (q > 127) overflow_count++;
                    q0 = static_cast<uint8_t>(static_cast<int8_t>(q));
                }

                // Write low byte for this [word within 64, pixel, 64ch tile]
                quantized_output_packed(c, p, b) = static_cast<uint16_t>(q0);

                // Channel c1, packed with low byte
                int b1 = c1 / block_size;
                // Extract the appropriate 8-bit scale from packed data
                uint8_t exp1;
                if (p & 1) {
                    // Odd pixel: extract high byte from packed data
                    exp1 = static_cast<uint8_t>((real_output(int(p / 2), b1) >> 8) & 0xFF);
                } else {
                    // Even pixel: extract low byte from packed data
                    exp1 = static_cast<uint8_t>(real_output(int(p / 2), b1) & 0xFF);
                }
                float actual_scale1 = powf(2.0f, static_cast<float>(exp1) - 127.0f);
                int ch_in_tile1 = c1 % mu_i;
                int tile_idx1 = c1 / mu_i;
                uint8_t q1 = 0;
                if (tile_idx1 < real_mu_input.dim(2).extent() && ch_in_tile1 < real_mu_input.dim(0).extent()) {
                    float f1 = bfloat16_to_float_process(real_mu_input(ch_in_tile1, p, tile_idx1));
                    int q = lroundf(f1 / actual_scale1);
                    if (q < -128) q = -128;
                    // if (q > 127)  q = 127;
                    if (q > 127) overflow_count++;
                    q1 = static_cast<uint8_t>(static_cast<int8_t>(q));
                }

                // pack high byte
                quantized_output_packed(c, p, b) = bit8_pack(q1, quantized_output_packed(c, p, b));
            }
        }
    }
    std::cout << "[WARNING] Overflow max INT8 (127) count: " << overflow_count << " percentage: " << static_cast<float>(overflow_count) / static_cast<float>(vec_height * vec_width) * 100.0f << "%" << std::endl;

    // Save packed quantized output
    save_halide_buffer_to_raw(real_mu_input, "bin/mu_input_host_stencil.raw");
    save_halide_buffer_to_raw(real_output, "bin/hw_scale_output.raw");
    save_halide_buffer_to_raw(quantized_output_packed, "bin/quantized_output_stencil.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {},
        {
            tensor_spec{"hw_scale_output_stencil", {"x_coord", "E64_packed", "bank_toggle_mode"}},
            tensor_spec{"quantized_output_stencil", {"x_coord", "E64_packed"}}
        },
        {
            tensor_spec{"mu_input_host_stencil", {"x_coord"}}
        }
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}