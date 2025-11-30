#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"
#include <cassert>
#include <cmath>

#if defined(WITH_CPU)
#include "apply_e8m0_scale_multi_IOs.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "apply_e8m0_scale_multi_IOs_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("apply_e8m0_scale_multi_IOs", { "input_bf_act", "input_scale" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        apply_e8m0_scale_multi_IOs(proc.inputs["input_bf_act"], proc.inputs["input_scale"], proc.output);
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
            apply_e8m0_scale_multi_IOs_clockwork(proc.inputs["input_bf_act"], proc.inputs["input_scale"], proc.output);
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

    auto head_dim_env = getenv("head_dim");
    auto seq_heads_prod_env = getenv("seq_heads_prod");
    auto glb_i_env = getenv("glb_i");

    auto head_dim = head_dim_env ? atoi(head_dim_env) : 64;
    auto seq_heads_prod = seq_heads_prod_env ? atoi(seq_heads_prod_env) : 1536;
    auto glb_i = glb_i_env ? atoi(glb_i_env) : 32;
    const int block_size = 64;

    std::cout << "using inputs set within process.cpp" << std::endl;
    processor.inputs_preset = true;

    // input_bf_act
    processor.inputs["input_bf_act"] = Buffer<uint16_t>(glb_i, seq_heads_prod);
    auto real_input_bf_act = Buffer<uint16_t>(head_dim, seq_heads_prod);
    for (int s = 0; s < real_input_bf_act.dim(1).extent(); s++) {
        for (int d = 0; d < real_input_bf_act.dim(0).extent(); d++) {
            // Use [-1024, 1024], which helps check scale output since otherwise scales are all the same
            real_input_bf_act(d, s) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 2048.0f - 1024.0f);
        }
    }

    // input_scale calculated from input_bf_act
    processor.inputs["input_scale"] = Buffer<uint16_t>(glb_i / 2, seq_heads_prod);
    auto real_input_scale = Buffer<uint16_t>(head_dim / 2, seq_heads_prod / block_size);
    auto real_input_scale_unpacked = Buffer<uint16_t>(head_dim, seq_heads_prod / block_size);
    for (int s = 0; s < real_input_scale.dim(1).extent(); s++) {
        const int base = s * block_size;
        for (int d_pack = 0; d_pack < real_input_scale.dim(0).extent(); d_pack++) {
            const int d0 = 2 * d_pack;
            const int d1 = d0 + 1;

            uint16_t max0 = real_input_bf_act(d0, base);
            uint16_t max1 = real_input_bf_act(d1, base);
            for (int r = 1; r < block_size; r++) {
                max0 = abs_max(max0, real_input_bf_act(d0, base + r));
                max1 = abs_max(max1, real_input_bf_act(d1, base + r));
            }
            const uint16_t scale0 = get_shared_exp(max0);
            const uint16_t scale1 = get_shared_exp(max1);

            real_input_scale(d_pack, s) = bit8_pack(scale1, scale0);
            real_input_scale_unpacked(d0, s) = scale0;
            real_input_scale_unpacked(d1, s) = scale1;
        }
    }



    // Gold hw_output_mxint8_act
    processor.output = Buffer<uint16_t>(int(glb_i / 2), seq_heads_prod);
    auto real_quantized_output_packed = Buffer<uint16_t>(head_dim / 2, seq_heads_prod);
    int overflow_count = 0;

    assert((seq_heads_prod % block_size) == 0 && "seq_heads_prod must be divisible by block_size");

    for (int s = 0; s < seq_heads_prod; s++) {
        const int scale_block = s / block_size;
        for (int d_pack = 0; d_pack < real_quantized_output_packed.dim(0).extent(); d_pack++) {
            const int d0 = 2 * d_pack;
            const int d1 = d0 + 1;

            auto quantize_channel = [&](int d) -> uint8_t {
                if (d >= real_input_bf_act.dim(0).extent()) {
                    return 0;
                }
                if (scale_block >= real_input_scale_unpacked.dim(1).extent()) {
                    return 0;
                }

                const uint16_t scale_e8m0 = real_input_scale_unpacked(d, scale_block);
                const float actual_scale = powf(2.0f, static_cast<float>(scale_e8m0) - 127.0f);

                const float value = bfloat16_to_float_process(real_input_bf_act(d, s));
                int q = lroundf(value / actual_scale);
                if (q < -128) {
                    q = -128;
                }
                if (q > 127) {
                    overflow_count++;
                }

                return static_cast<uint8_t>(static_cast<int8_t>(q));
            };

            const uint8_t q0 = quantize_channel(d0);
            const uint8_t q1 = quantize_channel(d1);
            real_quantized_output_packed(d_pack, s) = bit8_pack(q1, q0);
        }
    }

    std::cout << "[WARNING] Overflow max INT8 (127) count: " << overflow_count
              << " percentage: "
              << static_cast<float>(overflow_count) / static_cast<float>(head_dim * seq_heads_prod) * 100.0f
              << "%" << std::endl;

    // Save outputs
    save_halide_buffer_to_raw(real_input_bf_act, "bin/input_bf_act_host_stencil.raw");
    save_halide_buffer_to_raw(real_input_scale, "bin/input_scale_host_stencil.raw");
    save_halide_buffer_to_raw(real_quantized_output_packed, "bin/hw_output_mxint8_act.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_bf_act_host", {"x_coord"}},
            tensor_spec{"input_scale_host", {"x_coord"}}
        },
        {
            tensor_spec{"hw_output_mxint8_act", {"x_coord"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}