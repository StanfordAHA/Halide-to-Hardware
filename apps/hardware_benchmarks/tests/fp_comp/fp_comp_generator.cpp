#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestFPComp : public Halide::Generator<UnitTestFPComp> {
public:
    Input<Buffer<float>>  input{"input", 2};
    Output<Buffer<float>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));

        Func lt, gt, le, ge, eq, neq;
        Expr const_k = 100.3f;
        lt(x,y) = hw_input(x,y) < const_k;
        ge(x,y) = hw_input(x,y) >= Expr(80.2f);
        le(x,y) = hw_input(x,y) <= Expr(42.42f);
        gt(x,y) = hw_input(x,y) > Expr(9.6f);
        neq(x,y)= hw_input(x,y) != 6.66f;
        eq(x,y) = hw_input(x,y) == 66.6f;

        Func hw_output("hw_output");
        hw_output(x, y) = cast<float>(select((lt(x,y) && ge(x,y)) ||
                                             (le(x,y) && gt(x,y) && neq(x,y)) ||
                                             eq(x,y),
                                             255, 0));
        output(x, y) = hw_output(x,y);

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

HALIDE_REGISTER_GENERATOR(UnitTestFPComp, fp_comp)
