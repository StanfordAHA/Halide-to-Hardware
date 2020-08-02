#include "Halide.h"

namespace {

using namespace Halide;

float gamma = 1.0f;
float contrast = 1.0f;

class UnitTestFIFO : public Halide::Generator<UnitTestFIFO> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y));

        conv(x, y) = hw_input(x, y) + hw_input(x+1, y+1);
        merge(x, y) = conv(x, y) + hw_input(x, y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(merge(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          curve.compute_at(hw_output, yi).unroll(x);  // synthesize curve to a ROM

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFIFO, fifo)
