#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestFPInvTrig : public Halide::Generator<UnitTestFPInvTrig> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func arcsine, arccosine, arctangent;
        arcsine(x,y)    = asin(hw_input(x,y));
        arccosine(x,y)  = acos(hw_input(x,y));
        arctangent(x,y) = atan(hw_input(x,y));

        Func hw_output("hw_output");
        hw_output(x, y) = arcsine(x,y) + arccosine(x,y) + arctangent(x,y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

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

HALIDE_REGISTER_GENERATOR(UnitTestFPInvTrig, fp_invtrig)
