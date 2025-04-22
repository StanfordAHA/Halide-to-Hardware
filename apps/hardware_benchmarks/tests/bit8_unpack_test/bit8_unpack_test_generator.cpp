#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class BIT8_UNPACK : public Halide::Generator<BIT8_UNPACK> {
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
        Func unpack_a("unpack_a"), unpack_b("unpack_b");
        Func new_pack("new_pack");

        hw_input(w, x, y) = input(w, x, y);

        // Call int8tobf16_unpack and extract both values
        if (get_target().has_feature(Target::Clockwork)) {
            unpack(w, x, y) = bit8_unpack(bf16(hw_input(w, x, y)));
        } else {
            unpack(w, x, y) = bit8_unpack(hw_input(w, x, y));
        }
        unpack_a(w, x, y) = unpack(w, x, y)[0];
        unpack_b(w, x, y) = unpack(w, x, y)[1];

        if (get_target().has_feature(Target::Clockwork)) {
            new_pack(w, x, y) = bit8_pack(bf16(unpack_b(w, x, y)), bf16(unpack_a(w, x, y)));
        } else {
            new_pack(w, x, y) = bit8_pack(unpack_b(w, x, y), unpack_a(w, x, y));
        }

        hw_output(w, x, y) = new_pack(w, x, y);

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

            new_pack.compute_at(hw_output, xo).unroll(w, unroll);

            hw_input.stream_to_accelerator();
            hw_input.in().unroll(w, unroll);

        } else {
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BIT8_UNPACK, bit8_unpack_test)
