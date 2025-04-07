#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class RopePass1 : public Halide::Generator<RopePass1> {
public:
    // 3D input and output buffers.
    Input<Buffer<uint16_t>> input_upper{"input_upper", 3};
    Input<Buffer<uint16_t>> input_lower{"input_lower", 3};
    Input<Buffer<uint16_t>> cos_upper{"cos_upper", 3};
    Input<Buffer<uint16_t>> sin_upper{"sin_upper", 3};
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
        Func hw_input_upper("hw_input_upper"), input_upper_host("input_upper_host"),
             input_upper_glb("input_upper_glb"), input_upper_cgra("input_upper_cgra");
        hw_input_upper(n, s, h) = bf16(input_upper(n, s, h));
        input_upper_host(n, s, h) = hw_input_upper(n, s, h);
        input_upper_glb(n, s, h) = input_upper_host(n, s, h);
        input_upper_cgra(n, s, h) = input_upper_glb(n, s, h);


        Func hw_input_lower("hw_input_lower"), input_lower_host("input_lower_host"),
             input_lower_glb("input_lower_glb"), input_lower_cgra("input_lower_cgra");
        hw_input_lower(n, s, h) = bf16(input_lower(n, s, h));
        input_lower_host(n, s, h) = hw_input_lower(n, s, h);
        input_lower_glb(n, s, h) = input_lower_host(n, s, h);
        input_lower_cgra(n, s, h) = input_lower_glb(n, s, h);

        Func hw_cos_upper("hw_cos_upper"), cos_upper_host("cos_upper_host"),
             cos_upper_glb("cos_upper_glb"), cos_upper_cgra("cos_upper_cgra");
        hw_cos_upper(n, s, h) = bf16(cos_upper(n, s, h));
        cos_upper_host(n, s, h) = hw_cos_upper(n, s, h);
        cos_upper_glb(n, s, h) = cos_upper_host(n, s, h);
        cos_upper_cgra(n, s, h) = cos_upper_glb(n, s, h);

        Func hw_sin_upper("hw_sin_upper"), sin_upper_host("sin_upper_host"),
             sin_upper_glb("sin_upper_glb"), sin_upper_cgra("sin_upper_cgra");
        hw_sin_upper(n, s, h) = bf16(sin_upper(n, s, h));
        sin_upper_host(n, s, h) = hw_sin_upper(n, s, h);
        sin_upper_glb(n, s, h) = sin_upper_host(n, s, h);
        sin_upper_cgra(n, s, h) = sin_upper_glb(n, s, h);

        // in_upper * cos_upper - in_lower * sin_upper
        Func output_cgra("output_cgra");
        output_cgra(n, s, h) = input_upper_cgra(n, s, h) * cos_upper_cgra(n, s, h) -
                               input_lower_cgra(n, s, h) * sin_upper_cgra(n, s, h);

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
            input_upper_host.compute_root().accelerator_input();
            input_upper_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            input_upper_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);

            input_lower_host.compute_root().accelerator_input();
            input_lower_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            input_lower_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);

            cos_upper_host.compute_root().accelerator_input();
            cos_upper_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            cos_upper_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);

            sin_upper_host.compute_root().accelerator_input();
            sin_upper_glb.compute_at(hw_output, s_host).unroll(n, glb_i);
            sin_upper_cgra.compute_at(output_glb, s_glb).unroll(n, glb_i);
        } else {  // CPU schedule fallback.
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(RopePass1, rope_pass1_fp)
