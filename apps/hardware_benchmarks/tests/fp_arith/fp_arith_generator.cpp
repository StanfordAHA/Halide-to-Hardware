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
        hw_input(x, y) = bf16(input(x, y));
        hw_input_bfloat(x, y) = hw_input(x, y);

        Func mult, div, add, add2, sub, mod, neg;
        neg(x,y)  = bfloat16_t(13.3);//-Expr(bfloat16_t(13.3));
        mult(x,y) = hw_input_bfloat(x,y) * neg(x,y);
        div(x,y)  = bfloat16_t(4.56) / hw_input_bfloat(x,y);
        //mod(x,y)  = hw_input_bfloat(x,y);// % 16;
        add(x,y)  = div(x,y) + hw_input_bfloat(x,y) + bfloat16_t(9.8);
        //add2(x,y) = add(x,y) + mult(x,y); // these don't work for some reason
        //sub(x,y)  =  mult(x,y) - add(x,y);
        sub(x,y)  = bfloat16_t(2.6) * mult(x,y) - add(x,y);
        

        Func hw_output("hw_output");
        hw_output(x,y) = sub(x,y);
        //hw_output(x,y) = add(x,y);
        output(x,y) = u8(clamp(hw_output(x,y), 0, 255));

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
          // CPU schedule must be similar for bfloat16 for outputs to be bit exact
          output.compute_root();

          Var xi,yi, xo,yo;
          hw_output
            .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .compute_root();

          //sub.compute_at(hw_output, xo);
          add.compute_at(hw_output, xo);
          mult.compute_at(hw_output, xo);
          //div.compute_at(hw_output, xo);
          //mod.compute_at(hw_output, xo);



        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFPArith, fp_arith)
