#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// One-pass MX quantization along dim -2: the E8M0 scale of every block of 64
// rows of one column is computed from the input itself, and the input is
// quantized with it. The Strait hook replaces the CGRA graph with
// fused_accumulator_quant_pack_bf16 and adds the packed scale output; this
// algorithm only fixes the IO names (its scale is that of one glb_i-wide
// pass). process.cpp holds the reference for both outputs.
class Fused_E8M0_Quant_Multi_IOs : public Halide::Generator<Fused_E8M0_Quant_Multi_IOs> {
public:
    Input<Buffer<uint16_t>> input_bf_act{ "input_bf_act", 2 };
    Output<Buffer<uint16_t>> output_mxint8_act{ "output_mxint8_act", 2 };

    // e.g. sequence length * number of heads (rows, the scaled dimension)
    GeneratorParam<int> seq_heads_prod{ "seq_heads_prod", 1536 };
    // e.g. input glb unrolling, should match MU OC dimension
    GeneratorParam<int> glb_i{ "glb_i", 32 };
    // e.g. head dimension (columns)
    GeneratorParam<int> head_dim{ "head_dim", 64 };

    void generate() {
        /* THE ALGORITHM */
        // Input: BF16 input from 32 software GLB IOs stored in 4 GLB Tiles
        // Output 1: Packed e8m0 scale with 16 software GLB IOs stored in 2 GLB Tiles (added by the Strait hook)
        // Output 2: Quantized packed output with 16 software GLB IOs stored in 2 GLB Tiles

        // Dimention variables
        Var x("x"), y("y");
        // input_bf_act buffers
        Func input_bf_act_hw_input("input_bf_act_hw_input"), input_bf_act_host("input_bf_act_host"), input_bf_act_glb("input_bf_act_glb"), input_bf_act_cgra("input_bf_act_cgra");
        // output_mxint8_act buffers
        Func hw_output_mxint8_act("hw_output_mxint8_act"), output_mxint8_act_glb("output_mxint8_act_glb"), output_mxint8_act_cgra("output_mxint8_act_cgra");
        Func output_mxint8_act_unpacked("output_mxint8_act_unpacked");

        // input_bf_act buffers assignment
        input_bf_act_hw_input(x, y) = bf16(input_bf_act(x, y));
        input_bf_act_host(x, y) = input_bf_act_hw_input(x, y);
        input_bf_act_glb(x, y) = input_bf_act_host(x, y);
        input_bf_act_cgra(x, y) = input_bf_act_glb(x, y);

        // Scale of one pass: shared exponent of the largest magnitude
        RDom lane(0, glb_i, "lane");
        Func pass_max("pass_max"), pass_scale("pass_scale");
        pass_max(y) = bf16(0.0f);
        pass_max(y) = abs_max_bf16(pass_max(y), input_bf_act_cgra(lane, y));
        pass_scale(y) = get_shared_exp(pass_max(y));

        // Apply e8m0 scale
        output_mxint8_act_unpacked(x, y) = e8m0_quant(input_bf_act_cgra(x, y), pass_scale(y));
        // Pack every two adajcent x values into one 16-bit word
        output_mxint8_act_cgra(x, y) = bit8_pack(output_mxint8_act_unpacked(2 * x + 1, y), output_mxint8_act_unpacked(2 * x, y));

        // output_mxint8_act buffers assignment
        output_mxint8_act_glb(x, y) = output_mxint8_act_cgra(x, y);
        hw_output_mxint8_act(x, y) = output_mxint8_act_glb(x, y);
        output_mxint8_act(x, y) = u16(hw_output_mxint8_act(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define bound for input_bf_act buffers
            input_bf_act_host.bound(x, 0, glb_i).bound(y, 0, seq_heads_prod);
            input_bf_act_glb.bound(x, 0, glb_i).bound(y, 0, seq_heads_prod);
            input_bf_act_cgra.bound_extent(x, glb_i);

            // Define bound for output_mxint8_act buffers
            output_mxint8_act.bound(x, 0, int(glb_i/2)).bound(y, 0, seq_heads_prod);
            hw_output_mxint8_act.bound(x, 0, int(glb_i/2)).bound(y, 0, seq_heads_prod);
            output_mxint8_act_glb.bound_extent(x, int(glb_i/2));
            output_mxint8_act_cgra.bound_extent(x, int(glb_i/2));

            // Define loop variables for scale_output output
            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            hw_output_mxint8_act.compute_root()
                .split(x, x_host, x_glb, glb_i / 2)
                .split(y, y_host, y_glb, seq_heads_prod)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host);
            hw_output_mxint8_act.unroll(x_glb, 1);

            output_mxint8_act_glb.compute_at(hw_output_mxint8_act, y_host)
                .split(x, x_glb, x_cgra, glb_i / 2)
                .split(y, y_glb, y_cgra, seq_heads_prod)
                .reorder(x_cgra, y_cgra, x_glb, y_glb);
            output_mxint8_act_glb.unroll(x_cgra, 1);

            output_mxint8_act_cgra.compute_at(output_mxint8_act_glb, y_glb).unroll(x, 1);

            // Input buffers tiling schedule for input_bf_act
            input_bf_act_host.compute_root().accelerator_input();
            input_bf_act_glb.compute_at(hw_output_mxint8_act, y_host).unroll(x, 1);
            input_bf_act_cgra.compute_at(output_mxint8_act_glb, y_glb)
                .split(x, x_glb, x_cgra, glb_i)
                .split(y, y_glb, y_cgra, seq_heads_prod)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, 1);

            pass_max.compute_at(output_mxint8_act_glb, y_glb);

        } else {  // schedule to CPU
            output_mxint8_act_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Fused_E8M0_Quant_Multi_IOs, fused_e8m0_quant_multi_IOs)

