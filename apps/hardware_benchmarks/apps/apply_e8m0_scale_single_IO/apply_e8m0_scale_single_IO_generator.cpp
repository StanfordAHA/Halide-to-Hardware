#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Apply_E8M0_Scale_Single_IO : public Halide::Generator<Apply_E8M0_Scale_Single_IO> {
public:
    Input<Buffer<uint16_t>> input_bf_act{ "input_bf_act", 2 };
    Input<Buffer<uint16_t>> input_scale{ "input_scale", 2 };
    Output<Buffer<uint16_t>> output_mxint8_act{ "output_mxint8_act", 2 };

    // e.g. 56x56 image input, y dim
    GeneratorParam<int> vec_height{ "vec_height", 3136 };
    // e.g. 32 channels, x dim
    GeneratorParam<int> glb_i{ "glb_i", 32 };
    GeneratorParam<int> vec_width{ "vec_width", 64 };

    // Fake to help aha flow run
    GeneratorParam<int> dim0_fake{ "dim0_fake", 3072 };

    void generate() {
        /* THE ALGORITHM */
        // Input 1: BF16 input from 32 software GLB IOs stored in 4 GLB Tiles
        // Input 2: Unpacked e8m0 scale stored in 1 GLB Tile with 1 software GLB IO
        // Output 1: Packed e8m0 scale to two software GLB IOs stored in 2 GLB Tiles, with bank toggle mode
        // Output 2: Quantized packed output with 16 software GLB IOs stored in 2 GLB Tiles

        // Dimention variables
        Var x("x"), y("y");
        // input_bf_act buffers
        Func input_bf_act_hw_input("input_bf_act_hw_input"), input_bf_act_host("input_bf_act_host"), input_bf_act_glb("input_bf_act_glb"), input_bf_act_cgra("input_bf_act_cgra");
        // input_scale buffers
        Func input_scale_hw_input("input_scale_hw_input"), input_scale_host("input_scale_host"), input_scale_glb("input_scale_glb"), input_scale_cgra("input_scale_cgra");
        // output_mxint8_act buffers
        Func hw_output_mxint8_act("hw_output_mxint8_act"), output_mxint8_act_glb("output_mxint8_act_glb"), output_mxint8_act_cgra("output_mxint8_act_cgra");
        Func output_mxint8_act_unpacked("output_mxint8_act_unpacked");

        // input_bf_act buffers assignment
        input_bf_act_hw_input(x, y) = bf16(input_bf_act(x, y));
        input_bf_act_host(x, y) = input_bf_act_hw_input(x, y);
        input_bf_act_glb(x, y) = input_bf_act_host(x, y);
        input_bf_act_cgra(x, y) = input_bf_act_glb(x, y);

        // input_scale buffers assignment
        input_scale_hw_input(x, y) = bf16(input_scale(x, y));
        input_scale_host(x, y) = input_scale_hw_input(x, y);
        input_scale_glb(x, y) = input_scale_host(x, y);
        input_scale_cgra(x, y) = input_scale_glb(x, y);

        // Apply e8m0 scale
        output_mxint8_act_unpacked(x, y) = e8m0_quant(input_bf_act_cgra(x, y), input_scale_cgra(x, y));
        // Pack every two adajcent x values into one 16-bit word
        output_mxint8_act_cgra(x, y) = bit8_pack(output_mxint8_act_unpacked(2 * x + 1, y), output_mxint8_act_unpacked(2 * x, y));

        // output_mxint8_act buffers assignment
        output_mxint8_act_glb(x, y) = output_mxint8_act_cgra(x, y);
        hw_output_mxint8_act(x, y) = output_mxint8_act_glb(x, y);
        output_mxint8_act(x, y) = u16(hw_output_mxint8_act(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define bound for input_bf_act buffers
            input_bf_act_host.bound(x, 0, glb_i).bound(y, 0, vec_height);
            input_bf_act_glb.bound(x, 0, glb_i).bound(y, 0, vec_height);
            input_bf_act_cgra.bound_extent(x, glb_i);

            // Define bound for input_scale buffers
            input_scale_host.bound(x, 0, glb_i).bound(y, 0, vec_height);
            input_scale_glb.bound(x, 0, glb_i).bound(y, 0, vec_height);
            input_scale_cgra.bound_extent(x, glb_i);

            // Define bound for output_mxint8_act buffers
            output_mxint8_act.bound(x, 0, int(glb_i/2)).bound(y, 0, vec_height);
            hw_output_mxint8_act.bound(x, 0, int(glb_i/2)).bound(y, 0, vec_height);
            output_mxint8_act_glb.bound_extent(x, int(glb_i/2));
            output_mxint8_act_cgra.bound_extent(x, int(glb_i/2));

            // Define loop variables for scale_output output
            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            hw_output_mxint8_act.compute_root()
                .split(x, x_host, x_glb, glb_i / 2)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host);
            hw_output_mxint8_act.unroll(x_glb, glb_i / 2);

            output_mxint8_act_glb.compute_at(hw_output_mxint8_act, y_host)
                .split(x, x_glb, x_cgra, glb_i / 2)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb);
            output_mxint8_act_glb.unroll(x_cgra, glb_i / 2);

            output_mxint8_act_cgra.compute_at(output_mxint8_act_glb, y_glb).unroll(x, glb_i / 2);

            // Input buffers tiling schedule for input_bf_act
            input_bf_act_host.compute_root().accelerator_input();
            input_bf_act_glb.compute_at(hw_output_mxint8_act, y_host).unroll(x, glb_i);
            input_bf_act_cgra.compute_at(output_mxint8_act_glb, y_glb)
                .split(x, x_glb, x_cgra, glb_i)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, glb_i);

            // Input buffers tiling schedule for input_scale
            input_scale_host.compute_root().accelerator_input();
            input_scale_glb.compute_at(hw_output_mxint8_act, y_host).unroll(x, glb_i);
            input_scale_cgra.compute_at(output_mxint8_act_glb, y_glb)
                .split(x, x_glb, x_cgra, glb_i)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, glb_i);

        } else {  // schedule to CPU
            output_mxint8_act_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Apply_E8M0_Scale_Single_IO, apply_e8m0_scale_single_IO)

