#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"
#include <cassert>
#include <cmath>

#if defined(WITH_CPU)
#include "apply_e8m0_scale_single_IO.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "apply_e8m0_scale_single_IO_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("apply_e8m0_scale_single_IO", { "input_bf_act", "input_scale" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        apply_e8m0_scale_single_IO(proc.inputs["input_bf_act"], proc.inputs["input_scale"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input_bf_act"], proc.inputs["input_scale"], proc.output,
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
            apply_e8m0_scale_single_IO_clockwork(proc.inputs["input_bf_act"], proc.inputs["input_scale"], proc.output);
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
    auto glb_i_env = getenv("glb_i");
    auto vec_width_env = getenv("vec_width");

    auto vec_height = vec_height_env ? atoi(vec_height_env) : 3136;
    auto glb_i = glb_i_env ? atoi(glb_i_env) : 32;
    auto vec_width = vec_width_env ? atoi(vec_width_env) : 64;
    const int block_size = 64;

    assert((vec_width % 2) == 0 && "vec_width must be even for packing");

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // input_bf_act
    processor.inputs["input_bf_act"] = Buffer<uint16_t>(glb_i, vec_height);
    auto real_input_bf_act = Buffer<uint16_t>(vec_width, vec_height);
    for (int h = 0; h < real_input_bf_act.dim(1).extent(); h++) {
        for (int w = 0; w < real_input_bf_act.dim(0).extent(); w++) {
            // Use [-1024, 1024], which helps check scale output since otherwise scales are all the same
            real_input_bf_act(w, h) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2048.0f - 1024.0f);
        }
    }

    // input_scale calculated from input_bf_act
    processor.inputs["input_scale"] = Buffer<uint16_t>(glb_i, vec_height);
    auto real_input_scale = Buffer<uint16_t>(int(vec_width / block_size), vec_height);
    for (int h = 0; h < real_input_bf_act.dim(1).extent(); h++) {
        // For each block along width (e.g., block_size=64, vec_width=128 -> 2 blocks)
        for (int b = 0; b < int(vec_width / block_size); b++) {
            float max_val = 0.0f;
            uint16_t max_val_bf16 = float_to_bfloat16_process(max_val);

            // For channels in the block (e.g., 64 channels)
            for (int ch = 0; ch < block_size; ch++) {
                int channel_idx = b * block_size + ch;
                if (channel_idx < real_input_bf_act.dim(0).extent()) {
                    max_val_bf16 = abs_max(max_val_bf16, real_input_bf_act(channel_idx, h));
                }
            }

            // Store scale directly without packing
            uint16_t scale = get_shared_exp(max_val_bf16);
            real_input_scale(b, h) = scale;
        }
    }

    // Gold scale_packed_output
    const int scale_blocks = real_input_scale.dim(0).extent();
    const int packed_blocks = (scale_blocks + 1) / 2;
    auto real_scale_packed_output = Buffer<uint16_t>(packed_blocks, real_input_scale.dim(1).extent());
    for (int h = 0; h < real_input_scale.dim(1).extent(); h++) {
        for (int b = 0; b < scale_blocks; b++) {
            uint16_t scale = real_input_scale(b, h);
            int packed_idx = b / 2;
            if (b & 1) {
                real_scale_packed_output(packed_idx, h) = bit8_pack(scale, real_scale_packed_output(packed_idx, h));
            } else {
                real_scale_packed_output(packed_idx, h) = scale;
            }
        }
    }

    // Gold hw_output_mxint8_act
    processor.output = Buffer<uint16_t>(int(glb_i / 2), int(vec_height));
    auto real_quantized_output_packed = Buffer<uint16_t>(int(vec_width / 2), vec_height);
    int overflow_count = 0;
    for (int b = 0; b < int(vec_width / block_size); b++) {
        for (int h = 0; h < vec_height; h++) {
            uint8_t exp = static_cast<uint8_t>(real_input_scale(b, h) & 0xFF);
            float actual_scale = powf(2.0f, static_cast<float>(exp) - 127.0f);
            for (int c = 0; c < int(block_size / 2); c++) {
                int c0 = b * block_size + (c * 2);
                int c1 = c0 + 1;

                uint8_t q0 = 0;
                if (c0 < real_input_bf_act.dim(0).extent()) {
                    float f0 = bfloat16_to_float_process(real_input_bf_act(c0, h));
                    int q = lroundf(f0 / actual_scale);
                    if (q < -128) q = -128;
                    if (q > 127) overflow_count++;
                    q0 = static_cast<uint8_t>(static_cast<int8_t>(q));
                }

                uint8_t q1 = 0;
                if (c1 < real_input_bf_act.dim(0).extent()) {
                    float f1 = bfloat16_to_float_process(real_input_bf_act(c1, h));
                    int q = lroundf(f1 / actual_scale);
                    if (q < -128) q = -128;
                    if (q > 127) overflow_count++;
                    q1 = static_cast<uint8_t>(static_cast<int8_t>(q));
                }

                int out_x = b * int(block_size / 2) + c;
                uint16_t packed_val = bit8_pack(q1, q0);
                real_quantized_output_packed(out_x, h) = packed_val;
            }
        }
    }
    std::cout << "[WARNING] Overflow max INT8 (127) count: " << overflow_count
              << " percentage: "
              << static_cast<float>(overflow_count) / static_cast<float>(vec_height * vec_width) * 100.0f
              << "%" << std::endl;

    // Save outputs
    save_halide_buffer_to_raw(real_input_bf_act, "bin/input_bf_act_host_stencil.raw");
    save_halide_buffer_to_raw(real_input_scale, "bin/input_scale_host_stencil.raw");
    save_halide_buffer_to_raw(real_scale_packed_output, "bin/hw_scale_packed_output.raw");
    save_halide_buffer_to_raw(real_quantized_output_packed, "bin/hw_output_mxint8_act.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_bf_act_host", {"x_coord"}},
            tensor_spec{"input_scale_host", {"x_coord", "E64_packed"}}
        },
        {
            tensor_spec{"hw_scale_packed_output", {"x_coord", "E64_packed", "bank_toggle_mode"}},
            tensor_spec{"hw_output_mxint8_act", {"x_coord"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}