#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestDiv : public Halide::Generator<UnitTestDiv> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func udiv, sdiv, urem, srem;
        udiv(x,y) = hw_input(x,y) / 5;
        urem(x,y) = hw_input(x,y) % 7;
        sdiv(x,y) = i16(udiv(x,y) - urem(x,y)) / -100;
        //srem(x,y) = sdiv(x,y) % -3;
        srem(x,y) = -3 % min(sdiv(x,y), -1);
        // could also test with data in the denominator

        Func hw_output("hw_output");
        hw_output(x, y) = srem(x, y);
        output(x, y) = u8(hw_output(x,y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
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

HALIDE_REGISTER_GENERATOR(UnitTestDiv, divide)
