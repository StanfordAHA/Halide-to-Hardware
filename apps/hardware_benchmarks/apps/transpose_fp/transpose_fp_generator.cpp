#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Transpose : public Halide::Generator<Transpose> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 64 };
    GeneratorParam<int> vec_height{ "vec_height", 1024 };
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    void generate() {
        /* THE ALGORITHM */
        // One pass of a (batched) transpose. Only the IO names and shapes matter here: the strait template
        // transpose_bf16 replaces the graph (one input lane broadcast to glb_i filter MEMs), and the hook sets the
        // input (one GLB tile's 8 lanes) and output (the whole transposed tensor) shapes.

        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        output_cgra(x, y) = input_cgra(x, y) * input_cgra(x, y);

        output_glb(x, y) = output_cgra(x, y);
        hw_output(x, y) = output_glb(x, y);
        output(x, y) = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            input_host.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_glb.bound(x, 0, vec_width).bound(y, 0, vec_height);
            input_cgra.bound_extent(x, vec_width);

            output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            hw_output.bound(x, 0, vec_width).bound(y, 0, vec_height);
            output_cgra.bound_extent(x, vec_width);

            Var y_host, y_glb, y_cgra;
            Var x_host, x_glb, x_cgra;

            // Produce loop levels: host, global buffer, cgra
            // L3 loop level
            hw_output.compute_root();
            hw_output
                .split(x, x_host, x_glb, vec_width)
                .split(y, y_host, y_glb, vec_height)
                .reorder(x_glb, y_glb, x_host, y_host)
                .hw_accelerate(y_glb, y_host)
                .unroll(x_glb, 1);

            // L2 loop level
            output_glb.compute_at(hw_output, y_host);
            output_glb
                .split(x, x_glb, x_cgra, vec_width)
                .split(y, y_glb, y_cgra, vec_height)
                .reorder(x_cgra, y_cgra, x_glb, y_glb)
                .unroll(x_cgra, 1);

            // L1 loop level
            output_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

            // Input streaming
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, 1);
            input_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

        } else {  // schedule to CPU
            output_cgra.compute_root().unroll(x, glb_i);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Transpose, transpose_fp)
