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

        Func mult, div, add, add2, sub, mod, neg, expo;
        mult(x,y) = hw_input_bfloat(x,y) * bfloat16_t(1.576);
        add(x,y)  = mult(x,y) + bfloat16_t(9.8);
        div(x,y)  = bfloat16_t(4.56) / add(x,y);
        sub(x,y)  = div(x,y) - bfloat16_t(2.6);
        expo(x,y)  = exp(sub(x,y));

        Func hw_output("hw_output");
        hw_output(x,y) = expo(x,y);
        output(x,y) = u8(clamp(hw_output(x,y), 0, bfloat16_t(254.5)));

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
