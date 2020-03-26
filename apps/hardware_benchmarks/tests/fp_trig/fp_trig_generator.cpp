#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestFPTrig : public Halide::Generator<UnitTestFPTrig> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func sine, cosine, tangent;
        sine(x,y)    = sin(hw_input(x,y));
        cosine(x,y)  = cos(hw_input(x,y));
        tangent(x,y) = tan(hw_input(x,y));

        Func hw_output("hw_output");
        hw_output(x, y) = sine(x,y) + cosine(x,y) + tangent(x,y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFPTrig, fp_trig)
