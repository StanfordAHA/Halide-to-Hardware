#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class RoPE : public Halide::Generator<RoPE> {
public:
    // 3D input and output buffers.
    Input<Buffer<uint16_t>> input{ "input", 3 };
    Input<Buffer<uint16_t>> cos_buf{ "cos_buf", 3 };
    Input<Buffer<uint16_t>> sin_buf{ "sin_buf", 3 };
    Output<Buffer<uint16_t>> output{ "output", 3 };

    // Dimensions.
    GeneratorParam<int> n_heads{ "n_heads", 8 };
    GeneratorParam<int> seq_len{ "seq_len", 32 };
    GeneratorParam<int> head_dim_half{ "head_dim_half", 32 };

    // glb_i and glb_o control unrolling for inner stages.
    GeneratorParam<int> glb_i{ "glb_i", 1 };
    GeneratorParam<int> glb_o{ "glb_o", 1 };

    void generate() {
        // Define loop variables: h for head-dimension, s for sequence, and n for heads.
        Var h("h"), s("s"), n("n");

        // Pipeline for inputs
        Func hw_input("hw_input"), input_host("input_host"),
            input_glb("input_glb"), input_cgra("input_cgra");
        hw_input(h, s, n) = bf16(input(h, s, n));
        input_host(h, s, n) = hw_input(h, s, n);
        input_glb(h, s, n) = input_host(h, s, n);
        input_cgra(h, s, n) = input_glb(h, s, n);

        Func hw_cos("hw_cos"), cos_host("cos_host"),
            cos_glb("cos_glb"), cos_cgra("cos_cgra");
        hw_cos(h, s, n) = bf16(cos_buf(h, s, n));
        cos_host(h, s, n) = hw_cos(h, s, n);
        cos_glb(h, s, n) = cos_host(h, s, n);
        cos_cgra(h, s, n) = cos_glb(h, s, n);

        Func hw_sin("hw_sin"), sin_host("sin_host"),
            sin_glb("sin_glb"), sin_cgra("sin_cgra");
        hw_sin(h, s, n) = bf16(sin_buf(h, s, n));
        sin_host(h, s, n) = hw_sin(h, s, n);
        sin_glb(h, s, n) = sin_host(h, s, n);
        sin_cgra(h, s, n) = sin_glb(h, s, n);

        // input * cos - input * sin
        Func output_cgra("output_cgra");
        output_cgra(h, s, n) = input_cgra(h, s, n) * cos_cgra(h, s, n) -
                               input_cgra(h, s, n) * sin_cgra(h, s, n);

        Func output_glb("output_glb"), hw_output("hw_output");
        output_glb(h, s, n) = output_cgra(h, s, n);
        hw_output(h, s, n) = output_glb(h, s, n);
        output(h, s, n) = u16(hw_output(h, s, n));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            // Bound the output dimensions.
            output.bound(h, 0, head_dim_half).bound(s, 0, seq_len).bound(n, 0, n_heads);
            hw_output.bound(h, 0, head_dim_half).bound(s, 0, seq_len).bound(n, 0, n_heads);
            output_cgra.bound(h, 0, head_dim_half).bound(s, 0, seq_len).bound(n, 0, n_heads);

            Var h_host, h_glb, h_cgra;
            Var s_host, s_glb, s_cgra;
            Var n_host, n_glb, n_cgra;

            // Host loop level
            hw_output.compute_root();
            hw_output
                .split(h, h_host, h_glb, head_dim_half)
                .split(s, s_host, s_glb, seq_len)
                .split(n, n_host, n_glb, n_heads)
                .reorder(h_glb, s_glb, n_glb, h_host, s_host, n_host)
                .hw_accelerate(n_glb, n_host)
                .unroll(h_glb, glb_o);

            // GLB loop level
            output_glb.compute_at(hw_output, n_host);
            output_glb
                .split(h, h_glb, h_cgra, head_dim_half)
                .split(s, s_glb, s_cgra, seq_len)
                .split(n, n_glb, n_cgra, n_heads)
                .reorder(h_cgra, s_cgra, n_cgra, h_glb, s_glb, n_glb)
                .unroll(h_cgra, glb_o);

            output_cgra.compute_at(output_glb, n_glb).unroll(h, glb_o);

            // Input buffers
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, n_host).unroll(h, glb_i);
            input_cgra
                .compute_at(output_glb, n_glb)
                .split(h, h_glb, h_cgra, head_dim_half)
                .split(s, s_glb, s_cgra, seq_len)
                .split(n, n_glb, n_cgra, n_heads)
                .reorder(h_cgra, s_cgra, n_cgra, h_glb, s_glb, n_glb)
                .unroll(h_cgra, glb_i);

            cos_host.compute_root().accelerator_input();
            cos_glb.compute_at(hw_output, n_host).unroll(h, glb_i);
            cos_cgra
                .compute_at(output_glb, n_glb)
                .split(h, h_glb, h_cgra, head_dim_half)
                .split(s, s_glb, s_cgra, seq_len)
                .split(n, n_glb, n_cgra, n_heads)
                .reorder(h_cgra, s_cgra, n_cgra, h_glb, s_glb, n_glb)
                .unroll(h_cgra, glb_i);

            sin_host.compute_root().accelerator_input();
            sin_glb.compute_at(hw_output, n_host).unroll(h, glb_i);
            sin_cgra
                .compute_at(output_glb, n_glb)
                .split(h, h_glb, h_cgra, head_dim_half)
                .split(s, s_glb, s_cgra, seq_len)
                .split(n, n_glb, n_cgra, n_heads)
                .reorder(h_cgra, s_cgra, n_cgra, h_glb, s_glb, n_glb)
                .unroll(h_cgra, glb_i);
        } else {  // CPU schedule fallback.
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(RoPE, rope_fp)
