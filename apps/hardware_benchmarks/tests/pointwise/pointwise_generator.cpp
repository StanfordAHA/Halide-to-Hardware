#include "Halide.h"

namespace {

using namespace Halide;

class PointwiseMultiplication : public Halide::Generator<PointwiseMultiplication> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func mult("mult");
        mult(x, y) = input(x,y) * 2;
        output(x, y) = cast<uint16_t>(mult(x, y));

        /* THE SCHEDULE */
        mult.compute_root();
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseMultiplication, pointwise)
