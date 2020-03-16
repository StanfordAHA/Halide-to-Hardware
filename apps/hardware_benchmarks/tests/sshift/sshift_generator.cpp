#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestSshift : public Halide::Generator<UnitTestSshift> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y));

	Func negatives, shiftr, shiftl;
        negatives(x,y) = hw_input(x,y) - 100;
        shiftr(x,y) = negatives(x,y) >> 12;
        shiftl(x,y) = negatives(x,y) << 12;

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(shiftr(x,y) + shiftl(x,y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
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

HALIDE_REGISTER_GENERATOR(UnitTestSshift, sshift)
