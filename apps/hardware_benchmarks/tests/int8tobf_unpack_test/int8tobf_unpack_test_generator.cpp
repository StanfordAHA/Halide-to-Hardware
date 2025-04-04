#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class INT8TOBF16_UNPACK : public Halide::Generator<INT8TOBF16_UNPACK> {
public:
    Input<Buffer<uint16_t>> input{ "input", 3 };
    Output<Buffer<uint16_t>> output{ "output", 3 };

    GeneratorParam<int> out_img{ "out_img", 8 };
    GeneratorParam<int> n_oc{ "n_oc", 16 };
    GeneratorParam<int> unroll{ "unroll", 8 };

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), w("w");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
        Func unpack("unpack");
        Func mult("mult"), unpack_a("unpack_a"), unpack_b("unpack_b");

        hw_input(w, x, y) = input(w, x, y);

        // Call int8tobf16_unpack and extract both values
        unpack(w, x, y) = int2f_unpack(hw_input(w, x, y));
        unpack_a(w, x, y) = unpack(w, x, y)[0];
        unpack_b(w, x, y) = unpack(w, x, y)[1];

        mult(w, x, y) = cast<bfloat16_t>(unpack_a(w, x, y)) * cast<bfloat16_t>(unpack_b(w, x, y));
        hw_output(w, x, y) = mult(w, x, y);

        output(w, x, y) = cast<uint16_t>(hw_output(w, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
            Var xi, yi, xo, yo;

            output.bound(x, 0, out_img);
            output.bound(y, 0, out_img);
            output.bound(w, 0, n_oc);

            hw_output.compute_root();

            hw_output
                .tile(x, y, xo, yo, xi, yi, out_img, out_img)
                .reorder(w, xi, yi, xo, yo)
                .hw_accelerate(xi, xo);
            hw_output.unroll(w, unroll);

            mult.compute_at(hw_output, xo).unroll(w, unroll);

            hw_input.stream_to_accelerator();
            hw_input.in().unroll(w, unroll);

        } else {
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(INT8TOBF16_UNPACK, int8tobf_unpack_test)
