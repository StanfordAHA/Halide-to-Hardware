#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestScomp : public Halide::Generator<UnitTestScomp> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y));

        Func lt, gt, le, ge;
        lt(x,y) = hw_input(x,y) < cast<int16_t>(100);
        gt(x,y) = -16 < hw_input(x,y);
        le(x,y) = hw_input(x,y) <= 42;
        ge(x,y) = hw_input(x,y) >= -1;

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(select(lt(x,y) && gt(x,y) && le(x,y) && ge(x,y), 255, 0));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);
          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestScomp, scomp)
