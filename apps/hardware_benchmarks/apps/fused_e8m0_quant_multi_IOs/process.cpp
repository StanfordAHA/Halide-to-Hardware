#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"
#include <cassert>
#include <cmath>

#if defined(WITH_CPU)
#include "fused_e8m0_quant_multi_IOs.h"
#endif

#if defined(WITH_COREIR)
#include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
#include "clockwork_sim_platform.h"
#include "fused_e8m0_quant_multi_IOs_clockwork.h"
#include "rdai_api.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> functions;
    ManyInOneOut_ProcessController<uint16_t> processor("fused_e8m0_quant_multi_IOs", { "input_bf_act" });

#if defined(WITH_CPU)
    auto cpu_process = [&](auto &proc) {
        fused_e8m0_quant_multi_IOs(proc.inputs["input_bf_act"], proc.output);
    };
    functions["cpu"] = [&]() {
        cpu_process(processor);
    };
#endif

#if defined(WITH_COREIR)
    auto coreir_process = [&](auto &proc) {
        run_coreir_on_interpreter<>("bin/design_top.json",
                                    proc.inputs["input_bf_act"], proc.output,
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
            fused_e8m0_quant_multi_IOs_clockwork(proc.inputs["input_bf_act"], proc.output);
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

    // Shape: seq_heads_prod rows of head_dim columns; MX blocks are 64
    // consecutive rows of one column (cumulative scale along dim -2), as in
    // get_e8m0_scale_accum_gb_input + apply_e8m0_scale_multi_IOs.
    auto head_dim_env = getenv("head_dim");
    auto seq_heads_prod_env = getenv("seq_heads_prod");
    auto glb_i_env = getenv("glb_i");

    auto head_dim = head_dim_env ? atoi(head_dim_env) : 64;
    auto seq_heads_prod = seq_heads_prod_env ? atoi(seq_heads_prod_env) : 1536;
    auto glb_i = glb_i_env ? atoi(glb_i_env) : 32;
    const int block_size = 64;

    assert((head_dim % 2) == 0 && "head_dim must be even for packing");
    assert((seq_heads_prod % block_size) == 0 && "seq_heads_prod must be divisible by block_size");
    const int num_blocks = seq_heads_prod / block_size;

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

    // Block scales: shared exponent of the largest magnitude of 64 rows of one column
    auto real_input_scale = Buffer<uint16_t>(head_dim, num_blocks);
    for (int b = 0; b < num_blocks; b++) {
        for (int d = 0; d < head_dim; d++) {
            uint16_t max_val_bf16 = float_to_bfloat16_process(0.0f);
            for (int r = 0; r < block_size; r++) {
                max_val_bf16 = abs_max(max_val_bf16, real_input_bf_act(d, b * block_size + r));
            }
            real_input_scale(d, b) = get_shared_exp(max_val_bf16);
        }
    }

    // Gold hw_scale_packed_output: two adjacent columns per word (odd column in the high byte)
    auto real_scale_packed_output = Buffer<uint16_t>(head_dim / 2, num_blocks);
    for (int b = 0; b < num_blocks; b++) {
        for (int d_pack = 0; d_pack < head_dim / 2; d_pack++) {
            real_scale_packed_output(d_pack, b) = bit8_pack(real_input_scale(2 * d_pack + 1, b), real_input_scale(2 * d_pack, b));
        }
    }

    // Gold hw_output_mxint8_act: two adjacent columns per word (odd column in the high byte)
    processor.output = Buffer<uint16_t>(int(glb_i / 2), seq_heads_prod);
    auto real_quantized_output_packed = Buffer<uint16_t>(head_dim / 2, seq_heads_prod);
    int overflow_count = 0;
    auto quantize = [&](int d, int s) -> uint8_t {
        uint8_t exp = static_cast<uint8_t>(real_input_scale(d, s / block_size) & 0xFF);
        float actual_scale = powf(2.0f, static_cast<float>(exp) - 127.0f);
        float f = bfloat16_to_float_process(real_input_bf_act(d, s));
        int q = lroundf(f / actual_scale);
        if (q < -128) q = -128;
        if (q > 127) overflow_count++;
        return static_cast<uint8_t>(static_cast<int8_t>(q));
    };
    for (int s = 0; s < seq_heads_prod; s++) {
        for (int d_pack = 0; d_pack < head_dim / 2; d_pack++) {
            real_quantized_output_packed(d_pack, s) = bit8_pack(quantize(2 * d_pack + 1, s), quantize(2 * d_pack, s));
        }
    }
    std::cout << "[WARNING] Overflow max INT8 (127) count: " << overflow_count
              << " percentage: "
              << static_cast<float>(overflow_count) / static_cast<float>(seq_heads_prod * head_dim) * 100.0f
              << "%" << std::endl;

    save_halide_buffer_to_raw(real_input_bf_act, "bin/input_bf_act_host_stencil.raw");
    save_halide_buffer_to_raw(real_scale_packed_output, "bin/hw_scale_packed_output.raw");
    save_halide_buffer_to_raw(real_quantized_output_packed, "bin/hw_output_mxint8_act.raw");

    // Create glb bank config
    using namespace glb_cfg;
    // inputs, outputs, mu_inputs
    const config_spec spec = {
        {
            tensor_spec{"input_bf_act_host", {"x_coord"}}
        },
        {
            tensor_spec{"hw_scale_packed_output", {"x_coord"}},
            tensor_spec{"hw_output_mxint8_act", {"x_coord"}}
        },
        {}
    };
    write_glb_bank_config(spec);

    auto output = processor.process_command(argc, argv);

    return output;
}
