#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class BF2INT8_PACK : public Halide::Generator<BF2INT8_PACK> {
public:
    Input<Buffer<uint16_t>> input{ "input", 3 };
    Output<Buffer<uint16_t>> output{ "output", 3 };

    GeneratorParam<int> out_img{ "out_img", 16 };
    GeneratorParam<int> n_oc{ "n_oc", 32 };

    GeneratorParam<int> unroll{ "unroll", 8 };

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), w("w");
        Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
        Func mult;

        hw_input(w, x, y) = cast<bfloat16_t>(input(w, x, y));

        mult(w, x, y) = hw_input(w, x, y) * cast<bfloat16_t>(2.0f);

        hw_output(w, x, y) = f2int_pack(hw_input(w, x, y), mult(w, x, y));
        output(w, x, y) = cast<uint16_t>(hw_output(w, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
            Var xi, yi, xo, yo;

            output.bound(x, 0, out_img);
            output.bound(y, 0, out_img);
            output.bound(w, 0, n_oc);

            // hw_input.compute_root();
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

HALIDE_REGISTER_GENERATOR(BF2INT8_PACK, bf2int8_pack_test)
