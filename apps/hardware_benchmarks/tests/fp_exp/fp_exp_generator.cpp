#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestFPExp : public Halide::Generator<UnitTestFPExp> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func exponential, natural_log, power, square_root;
        exponential(x,y) = exp(hw_input(x,y));
        natural_log(x,y) = exponential(x,y) + log(hw_input(x,y));
        power(x,y) = pow(hw_input(x,y), hw_input(x,y)) + natural_log(x,y);
        square_root(x,y) = sqrt(power(x,y) + 1);

        Func hw_output("hw_output");
        //hw_output(x, y) = u8(square_root(x, y));
        hw_output(x, y) = power(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output
            .tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);
          
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFPExp, fp_exp)
