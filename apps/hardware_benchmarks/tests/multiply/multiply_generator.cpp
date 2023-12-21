#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestMul : public Halide::Generator<UnitTestMul> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    Expr umul0(Expr a, Expr b) {
      return i16(( (i32(a)&0xffff) * (i32(b)&0xffff) ) >> 0);
    }

    Expr umul1(Expr a, Expr b) {
      return i16(( (i32(a)&0xffff) * (i32(b)&0xffff) ) >> 8);
    }

    Expr umul2(Expr a, Expr b) {
      return i16(( (i32(a)&0xffff) * (i32(b)&0xffff) ) >> 16);
    }

    Expr mul0(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 0);
    }

    Expr mul1(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 8);
    }

    Expr mul2(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 16);
    }

  
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = i16(input(x, y));

        Func mult0, mult1, mult2;
        mult0(x,y) = mul0(hw_input(x,y), i16(-7));
        //mult1(x,y) = mul1(15361, 100 * hw_input(x,y));
        mult1(x,y) = mul1(i16(15361), hw_input(x,y));
        mult2(x,y) = mul2(mult1(x,y), mult0(x,y));

        Func hw_output("hw_output");
        hw_output(x, y) = mult2(x, y);
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

HALIDE_REGISTER_GENERATOR(UnitTestMul, multiply)
