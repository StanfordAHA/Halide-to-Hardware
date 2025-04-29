#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class RopePass2 : public Halide::Generator<RopePass2> {
public:
    // 3D input and output buffers.
    Input<Buffer<uint16_t>> input_lower{"input_lower", 3};
    Input<Buffer<uint16_t>> input_upper{"input_upper", 3};
    Input<Buffer<uint16_t>> cos_lower{"cos_lower", 3};
    Input<Buffer<uint16_t>> sin_lower{"sin_lower", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    // Dimensions.
    GeneratorParam<int> n_heads{"n_heads", 8};
    GeneratorParam<int> seq_len{"seq_len", 32};
    GeneratorParam<int> head_dim_half{"head_dim_half", 64};

    // glb_i and glb_o control unrolling for inner stages.
    GeneratorParam<int> glb_i{"glb_i", 1};
    GeneratorParam<int> glb_o{"glb_o", 1};

    void generate() {
        // Define loop variables: n for heads, s for sequence, and h for head-dimension.
        Var n("n"), s("s"), h("h");

        // Pipeline for inputs
        Func hw_input_lower("hw_input_lower"), input_lower_host("input_lower_host"),
             input_lower_glb("input_lower_glb"), input_lower_cgra("input_lower_cgra");
        hw_input_lower(n, s, h) = bf16(input_lower(n, s, h));
        input_lower_host(n, s, h) = hw_input_lower(n, s, h);
        input_lower_glb(n, s, h) = input_lower_host(n, s, h);
        input_lower_cgra(n, s, h) = input_lower_glb(n, s, h);

        Func hw_input_upper("hw_input_upper"), input_upper_host("input_upper_host"),
             input_upper_glb("input_upper_glb"), input_upper_cgra("input_upper_cgra");
        hw_input_upper(n, s, h) = bf16(input_upper(n, s, h));
        input_upper_host(n, s, h) = hw_input_upper(n, s, h);
        input_upper_glb(n, s, h) = input_upper_host(n, s, h);
        input_upper_cgra(n, s, h) = input_upper_glb(n, s, h);

        Func hw_cos_lower("hw_cos_lower"), cos_lower_host("cos_lower_host"),
             cos_lower_glb("cos_lower_glb"), cos_lower_cgra("cos_lower_cgra");
        hw_cos_lower(n, s, h) = bf16(cos_lower(n, s, h));
        cos_lower_host(n, s, h) = hw_cos_lower(n, s, h);
        cos_lower_glb(n, s, h) = cos_lower_host(n, s, h);
        cos_lower_cgra(n, s, h) = cos_lower_glb(n, s, h);

        Func hw_sin_lower("hw_sin_lower"), sin_lower_host("sin_lower_host"),
             sin_lower_glb("sin_lower_glb"), sin_lower_cgra("sin_lower_cgra");
        hw_sin_lower(n, s, h) = bf16(sin_lower(n, s, h));
        sin_lower_host(n, s, h) = hw_sin_lower(n, s, h);
        sin_lower_glb(n, s, h) = sin_lower_host(n, s, h);
        sin_lower_cgra(n, s, h) = sin_lower_glb(n, s, h);

        // in_upper * cos_upper - in_lower * sin_upper
        Func output_cgra("output_cgra");
        output_cgra(n, s, h) = input_lower_cgra(n, s, h) * cos_lower_cgra(n, s, h) +
                               input_upper_cgra(n, s, h) * sin_lower_cgra(n, s, h);

        Func output_glb("output_glb"), hw_output("hw_output");
        output_glb(n, s, h) = output_cgra(n, s, h);
        hw_output(n, s, h) = output_glb(n, s, h);
        output(n, s, h) = u16(hw_output(n, s, h));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
            // Bound the output dimensions.
            output.bound(n, 0, n_heads);
            output.bound(s, 0, seq_len);
            output.bound(h, 0, head_dim_half);
            hw_output.bound(n, 0, n_heads);
            hw_output.bound(s, 0, seq_len);
            hw_output.bound(h, 0, head_dim_half);
            output_glb.bound(n, 0, n_heads);
            output_glb.bound(s, 0, seq_len);
            output_glb.bound(h, 0, head_dim_half);
            output_cgra.bound(n, 0, n_heads);
            output_cgra.bound(s, 0, seq_len);
            output_cgra.bound(h, 0, head_dim_half);

            // Create host, global buffer, and CGRA loop levels.
            Var s_host, s_glb, s_cgra, h_host, h_glb, h_cgra;
            hw_output.compute_root();
            hw_output.tile(s, h, s_host, h_host, s_glb, h_glb, seq_len, head_dim_half)
                     .reorder(n, s_glb, h_glb, s_host, h_host)
                     .hw_accelerate(s_glb, s_host);

            output_glb.compute_at(hw_output, s_host);
            output_glb.tile(s, h, s_glb, h_glb, s_cgra, h_cgra, seq_len, head_dim_half)
                      .reorder(n, s_cgra, h_cgra, s_glb, h_glb);
            hw_output.unroll(n, glb_o);
            output_glb.unroll(n, glb_o);

            output_cgra.compute_at(output_glb, s_glb)
                        .unroll(n, glb_o);

            // Schedule the input buffers
            input_lower_host.compute_root().accelerator_input();
            input_lower_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            input_lower_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);

            input_upper_host.compute_root().accelerator_input();
            input_upper_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            input_upper_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);

            cos_lower_host.compute_root().accelerator_input();
            cos_lower_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            cos_lower_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);

            sin_lower_host.compute_root().accelerator_input();
            sin_lower_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            sin_lower_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);
        } else {  // CPU schedule fallback.
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(RopePass2, rope_pass2_fp)
