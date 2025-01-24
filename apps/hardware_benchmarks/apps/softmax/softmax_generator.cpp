#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class Softmax : public Halide::Generator<Softmax> {
public:
    Input<Buffer<uint16_t>> input{ "input", 1 };
    Output<Buffer<uint16_t>> output{ "output", 1 };

    GeneratorParam<int32_t> vec_len{ "vec_len", 32 };
    GeneratorParam<int32_t> unroll{ "unroll", 1 };

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");
        RDom r(0, vec_len, "r");

        Func hw_input("hw_input");
        hw_input(x) = bf16(input(x));

        Func sum_input("sum_input");
        sum_input(y) = bf16(0.0f);
        sum_input(y) += hw_input(r.x);

        Func hw_output("hw_output");
        hw_output(y) = sum_input(y);

        output(y) = u16(hw_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            hw_input.bound(x, 0, vec_len);
            output.bound(y, 0, 1);

            Var yi, yo;
            hw_output.compute_root();
            hw_output.split(y, yo, yi, 1).hw_accelerate(yi, yo);
            // hw_output.unroll(xi, unroll);

            sum_input.compute_at(hw_output, yo);
            sum_input.update();
            // sum_input.update().unroll(r.x, unroll);

            hw_input.stream_to_accelerator();
            // hw_input.in().unroll(r.x, unroll);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Softmax, softmax)
