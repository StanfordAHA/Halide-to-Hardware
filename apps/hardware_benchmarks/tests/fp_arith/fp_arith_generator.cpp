#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestFPArith : public Halide::Generator<UnitTestFPArith> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int imgsize = 64;
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input"), hw_input_bfloat;
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        hw_input_bfloat(x, y) = cast<bfloat16_t>(hw_input(x, y));

        Func mult, div, add, sub, mod, neg;
        neg(x,y)  = Expr(bfloat16_t(13.3));//-Expr(bfloat16_t(13.3));
        mult(x,y) = hw_input_bfloat(x,y) * neg(x,y);
        div(x,y)  = hw_input_bfloat(x,y) / Expr(bfloat16_t(4.56));
        mod(x,y)  = hw_input_bfloat(x,y);// % 16;
        add(x,y)  = div(x,y) + Expr(bfloat16_t(9.8));
        sub(x,y)  = mult(x,y) - add(x,y);

        Func hw_output("hw_output");
        hw_output(x,y) = u16(mult(x,y));
        output(x,y) = u8(hw_output(x,y));

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

          mult.compute_at(hw_output, xo);
          div.compute_at(hw_output, xo);
          mod.compute_at(hw_output, xo);
          add.compute_at(hw_output, xo);
          sub.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFPArith, fp_arith)
