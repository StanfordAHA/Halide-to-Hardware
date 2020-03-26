#include "Halide.h"

namespace {

using namespace Halide;
using namespace ConciseCasts;

class UnitTestFPComp : public Halide::Generator<UnitTestFPComp> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func lt, gt, le, ge, equal, not_equal;
        lt(x,y) = hw_input(x,y) <  Expr(bfloat16_t(100.3f));
        ge(x,y) = hw_input(x,y) >= Expr(bfloat16_t(80.2f));
        le(x,y) = hw_input(x,y) <= Expr(bfloat16_t(42.42f));
        gt(x,y) = hw_input(x,y) >  Expr(bfloat16_t(3.6f));
        not_equal(x,y)= hw_input(x,y) != bfloat16_t(6.f);
        equal(x,y) = hw_input(x,y) == bfloat16_t(66.f);

        Func hw_output("hw_output");
        hw_output(x, y) = select((lt(x,y) && ge(x,y)) ||
                                 (le(x,y) && gt(x,y) && not_equal(x,y)) ||
                                 equal(x,y),
                                 255, 0);
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

HALIDE_REGISTER_GENERATOR(UnitTestFPComp, fp_comp)
