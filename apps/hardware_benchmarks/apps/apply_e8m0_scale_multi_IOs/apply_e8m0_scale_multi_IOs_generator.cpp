#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Apply_E8M0_Scale_Multi_IOs : public Halide::Generator<Apply_E8M0_Scale_Multi_IOs> {
public:
    Input<Buffer<uint16_t>> input_bf_act{ "input_bf_act", 2 };
    Input<Buffer<uint16_t>> input_scale{ "input_scale", 2 };
    Output<Buffer<uint16_t>> output_mxint8_act{ "output_mxint8_act", 2 };

    // e.g. head dimension
    GeneratorParam<int> head_dim{ "head_dim", 64 };
    // e.g. sequence length * number of heads
    GeneratorParam<int> seq_heads_prod{ "seq_heads_prod", 1536 };
    // e.g. input glb unrolling, should match MU OC dimension
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    void generate() {
        /* THE ALGORITHM */
        // Input 1: BF16 input from 32 software GLB IOs stored in 4 GLB Tiles
        // Input 2: Packed e8m0 scale stored in 2 GLB Tiles with 16 software GLB IOs
        // Output: Quantized packed output with 16 software GLB IOs stored in 2 GLB Tiles

        // Dimention variables
        Var d("d"), s("s");
        // input_bf_act buffers
        Func input_bf_act_hw_input("input_bf_act_hw_input"), input_bf_act_host("input_bf_act_host"), input_bf_act_glb("input_bf_act_glb"), input_bf_act_cgra("input_bf_act_cgra");
        // input_scale buffers
        Func input_scale_hw_input("input_scale_hw_input"), input_scale_host("input_scale_host"), input_scale_glb("input_scale_glb"), input_scale_cgra("input_scale_cgra");
        Func input_scale_unpack("input_scale_unpack"), input_scale_cgra_unpacked("input_scale_cgra_unpacked");
        // output_mxint8_act buffers
        Func hw_output_mxint8_act("hw_output_mxint8_act"), output_mxint8_act_glb("output_mxint8_act_glb"), output_mxint8_act_cgra("output_mxint8_act_cgra");
        Func output_mxint8_act_unpacked("output_mxint8_act_unpacked");

        // input_bf_act buffers assignment
        input_bf_act_hw_input(d, s) = bf16(input_bf_act(d, s));
        input_bf_act_host(d, s) = input_bf_act_hw_input(d, s);
        input_bf_act_glb(d, s) = input_bf_act_host(d, s);
        input_bf_act_cgra(d, s) = input_bf_act_glb(d, s);

        // input_scale buffers assignment
        input_scale_hw_input(d, s) = bf16(input_scale(d, s));
        input_scale_host(d, s) = input_scale_hw_input(d, s);
        input_scale_glb(d, s) = input_scale_host(d, s);
        input_scale_cgra(d, s) = input_scale_glb(d, s);

        // Unpack packed e8m0 scales (each 16-bit holds two 8-bit values)
        if (get_target().has_feature(Target::Clockwork)) {
            input_scale_unpack(d, s) = bit8_unpack(bf16(input_scale_cgra(d, s)));
        } else {
            input_scale_unpack(d, s) = bit8_unpack(input_scale_cgra(d, s));
        }
        input_scale_cgra_unpacked(d, s) =
            // [1] is the lower 8 bits, [0] is the upper 8 bits
            select((d % 2) == 0, input_scale_unpack(d / 2, s)[1], input_scale_unpack(d / 2, s)[0]);

        // Apply e8m0 scale
        output_mxint8_act_unpacked(d, s) = e8m0_quant(input_bf_act_cgra(d, s), input_scale_cgra_unpacked(d, s));
        // Pack every two adajcent x values into one 16-bit word
        output_mxint8_act_cgra(d, s) = bit8_pack(output_mxint8_act_unpacked(2 * d + 1, s), output_mxint8_act_unpacked(2 * d, s));

        // output_mxint8_act buffers assignment
        output_mxint8_act_glb(d, s) = output_mxint8_act_cgra(d, s);
        hw_output_mxint8_act(d, s) = output_mxint8_act_glb(d, s);
        output_mxint8_act(d, s) = u16(hw_output_mxint8_act(d, s));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define bound for input_bf_act buffers
            input_bf_act_host.bound(d, 0, glb_i).bound(s, 0, seq_heads_prod);
            input_bf_act_glb.bound(d, 0, glb_i).bound(s, 0, seq_heads_prod);
            input_bf_act_cgra.bound_extent(d, glb_i);

            // Define bound for input_scale buffers
            input_scale_host.bound(d, 0, int(glb_i/2)).bound(s, 0, seq_heads_prod);
            input_scale_glb.bound(d, 0, int(glb_i/2)).bound(s, 0, seq_heads_prod);
            input_scale_cgra.bound_extent(d, int(glb_i/2));

            // Define bound for output_mxint8_act buffers
            output_mxint8_act.bound(d, 0, int(glb_i/2)).bound(s, 0, seq_heads_prod);
            hw_output_mxint8_act.bound(d, 0, int(glb_i/2)).bound(s, 0, seq_heads_prod);
            output_mxint8_act_glb.bound_extent(d, int(glb_i/2));
            output_mxint8_act_cgra.bound_extent(d, int(glb_i/2));

            // Define loop variables for scale_output output
            Var s_host, s_glb, s_cgra;
            Var d_host, d_glb, d_cgra;

            hw_output_mxint8_act.compute_root()
                .split(d, d_host, d_glb, glb_i / 2)
                .split(s, s_host, s_glb, seq_heads_prod)
                .reorder(d_glb, s_glb, d_host, s_host)
                .hw_accelerate(s_glb, s_host);
            hw_output_mxint8_act.unroll(d_glb, glb_i / 2);

            output_mxint8_act_glb.compute_at(hw_output_mxint8_act, s_host)
                .split(d, d_glb, d_cgra, glb_i / 2)
                .split(s, s_glb, s_cgra, seq_heads_prod)
                .reorder(d_cgra, s_cgra, d_glb, s_glb);
            output_mxint8_act_glb.unroll(d_cgra, glb_i / 2);

            output_mxint8_act_cgra.compute_at(output_mxint8_act_glb, s_glb).unroll(d, glb_i / 2);

            // Input buffers tiling schedule for input_bf_act
            input_bf_act_host.compute_root().accelerator_input();
            input_bf_act_glb.compute_at(hw_output_mxint8_act, s_host).unroll(d, glb_i);
            input_bf_act_cgra.compute_at(output_mxint8_act_glb, s_glb)
                .split(d, d_glb, d_cgra, glb_i)
                .split(s, s_glb, s_cgra, seq_heads_prod)
                .reorder(d_cgra, s_cgra, d_glb, s_glb)
                .unroll(d_cgra, glb_i);

            // Input buffers tiling schedule for input_scale
            input_scale_host.compute_root().accelerator_input();
            input_scale_glb.compute_at(hw_output_mxint8_act, s_host).unroll(d, glb_i / 2);
            input_scale_cgra.compute_at(output_mxint8_act_glb, s_glb)
                .split(d, d_glb, d_cgra, glb_i / 2)
                .split(s, s_glb, s_cgra, seq_heads_prod)
                .reorder(d_cgra, s_cgra, d_glb, s_glb)
                .unroll(d_cgra, glb_i / 2);

        } else {  // schedule to CPU
            output_mxint8_act_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Apply_E8M0_Scale_Multi_IOs, apply_e8m0_scale_multi_IOs)

