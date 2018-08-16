#include "Halide.h"

namespace {

using namespace Halide;

class PointwiseMultiplication : public Halide::Generator<PointwiseMultiplication> {
public:
    Input<Buffer<int16_t>>  input{"input", 2};
    Output<Buffer<int16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func mult("mult");

        output(x, y) = cast<int16_t>(input(x,y) * 2);

        /* THE SCHEDULE */
        mult.compute_root();
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseMultiplication, pointwise)
