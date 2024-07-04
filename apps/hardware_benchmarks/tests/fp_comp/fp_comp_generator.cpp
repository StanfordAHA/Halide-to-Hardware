#include "Halide.h"

namespace {

using namespace Halide;
using namespace ConciseCasts;

class UnitTestFPComp : public Halide::Generator<UnitTestFPComp> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int imgsize = 64;
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func lt, gt, le, ge, eq, ne;
        lt(x,y) = hw_input(x,y) <  Expr(bfloat16_t(6.6f));
        gt(x,y) = hw_input(x,y) > Expr(bfloat16_t(3.3f));

        le(x,y) = hw_input(x,y) <=  Expr(bfloat16_t(16.6f));
        ge(x,y) = hw_input(x,y) >= Expr(bfloat16_t(13.3f));

        eq(x,y) = hw_input(x,y) == bfloat16_t(66.0f);

        Func hw_output("hw_output");
        hw_output(x, y) = u16(select((lt(x,y) && gt(x,y)) || (le(x,y) && ge(x,y)) || eq(x,y), u16(0), u16(1)));
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;
        
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);
          
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFPComp, fp_comp)
