#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestArith : public Halide::Generator<UnitTestArith> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func mult, div, add, sub, mod;
        mult(x,y) = hw_input(x,y) * 13;
        div(x,y) = hw_input(x,y) / 4;
        mod(x,y) = hw_input(x,y) % 16;
        add(x,y) = div(x,y) + mod(x,y);
        sub(x,y) = mult(x,y) - add(x,y);

        Func hw_output("hw_output");
        hw_output(x, y) = sub(x, y);
        output(x, y) = u8(hw_output(x,y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
          
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

HALIDE_REGISTER_GENERATOR(UnitTestArith, arith)
