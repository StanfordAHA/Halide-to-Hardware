#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestAbsolute : public Halide::Generator<UnitTestAbsolute> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = i16(input(x, y));

        Func abso, absdo;
        abso(x,y) = abs(hw_input(x,y) - 100);
        absdo(x,y) = absd( abso(x,y) , i16(30) );

        Func hw_output("hw_output");
        hw_output(x, y) = absdo(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.bound(x, 0, 64);
          hw_output.bound(y, 0, 64);
          
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
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

HALIDE_REGISTER_GENERATOR(UnitTestAbsolute, absolute)
