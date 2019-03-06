#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestCounter : public Halide::Generator<UnitTestCounter> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y));

        Func countx, county;
        countx(x,y) = x + hw_input(x,y);
        county(x,y) = y + hw_input(x,y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(countx(x, y) + county(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestCounter, counter)
