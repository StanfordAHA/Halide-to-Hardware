#include "Halide.h"

namespace {

using namespace Halide;

class PointwiseMultiplication : public Halide::Generator<PointwiseMultiplication> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input, hw_output;
        Func mult("mult");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        
        mult(x, y) = hw_input(x,y) * 2;
        hw_output(x, y) = cast<uint8_t>(mult(x, y));
        output(x, y) = hw_output(x, y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          //hw_input.compute_root();
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            .hw_accelerate(xi, xo);
          hw_output.bound(x, 0, 64);
          hw_output.bound(y, 0, 64);

          hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);

          hw_input.stream_to_accelerator();

        } else {
          mult.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseMultiplication, pointwise)
