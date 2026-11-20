#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// Cumsum along dim -2: the tensor is vec_height / scan_rows matrices of
// scan_rows rows and vec_width columns, and
// output(x, y) = input(x, y0) + input(x, y0 + 1) + ... + input(x, y),
// where y0 is the first row of y's matrix. The Strait hook replaces the CGRA
// graph with col_prefix_sum_bf16, which streams glb_i consecutive columns
// per cycle and keeps each column's running sum in the Pond of its lane.
class ColPrefixSum : public Halide::Generator<ColPrefixSum> {
public:
    Input<Buffer<uint16_t>> input{ "input", 2 };
    Output<Buffer<uint16_t>> output{ "output", 2 };

    GeneratorParam<int> vec_width{ "vec_width", 128 };
    GeneratorParam<int> vec_height{ "vec_height", 1024 };
    GeneratorParam<int> scan_rows{ "scan_rows", 128 };
    GeneratorParam<int> glb_i{ "glb_i", 32 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        hw_input(x, y) = bf16(input(x, y));
        input_host(x, y) = hw_input(x, y);
        input_glb(x, y) = input_host(x, y);
        input_cgra(x, y) = input_glb(x, y);

        RDom r(0, scan_rows, "r");
        Expr first_row = (y / scan_rows) * scan_rows;
        Func prefix("prefix");
        prefix(x, y) = 0.0f;
        prefix(x, y) += select(r.x <= y - first_row, f32(input_cgra(x, clamp(first_row + r.x, 0, vec_height - 1))), 0.0f);
        output_cgra(x, y) = bf16(prefix(x, y));

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
            prefix.compute_at(output_glb, y_glb);

            // Input streaming
            input_host.compute_root().accelerator_input();
            input_glb.compute_at(hw_output, y_host).unroll(x, 1);
            input_cgra.compute_at(output_glb, y_glb).unroll(x, 1);

        } else {  // schedule to CPU
            output_cgra.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ColPrefixSum, col_prefix_sum_fp)
