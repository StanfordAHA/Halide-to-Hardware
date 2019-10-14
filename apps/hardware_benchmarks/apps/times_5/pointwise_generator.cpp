#include "Halide.h"

namespace {

using namespace Halide;

class PointwiseMultiplication : public Halide::Generator<PointwiseMultiplication> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {

        Var x("x"), y("y");

        Func hw_input, hw_output;
        Func mult("mult");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        
        mult(x, y) = 5* hw_input(x, y);
        hw_output(x, y) = mult(x, y);
        output(x, y) = hw_output(x, y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          //.unroll(y, 2).unroll(x, 2);
          //hw_input.tile(x, y, xo, yo, xi, yi, 2, 2);
          //hw_input.linebuffer();

          hw_output.compute_root();

          //hw_output.accelerate(x);
          //hw_output.tile(x, xo, xi).accelerate(x);
          //hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
          hw_output.tile(x,y, xo,yo, xi,yi, 100, 100)
            //.unroll(yi, 10)
            .unroll(xi, 10)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();

        } else {
          mult.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseMultiplication, pointwise)
