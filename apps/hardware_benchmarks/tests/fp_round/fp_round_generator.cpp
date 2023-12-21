#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestFPRound : public Halide::Generator<UnitTestFPRound> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func rnd, ceiling, flr;
        ceiling(x,y) = cast<bfloat16_t>(ceil(hw_input(x,y)));
        flr(x,y) = cast<bfloat16_t>(floor(hw_input(x,y)));
        rnd(x,y) = cast<bfloat16_t>(round(hw_input(x,y)));
        
        Func hw_output("hw_output");
        hw_output(x, y) = rnd(x,y) + ceiling(x,y) + flr(x,y);
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

HALIDE_REGISTER_GENERATOR(UnitTestFPRound, fp_round)
