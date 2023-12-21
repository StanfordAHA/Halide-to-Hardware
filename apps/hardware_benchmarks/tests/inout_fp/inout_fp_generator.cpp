#include "Halide.h"

namespace {

using namespace Halide;

class PointwiseKernel : public Halide::Generator<PointwiseKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func product("product");

        Func hw_input("hw_input");
        //hw_input(x, y) = cast<bfloat16_t>(reinterpret<float>((input(x, y)>>16)<<16));
        //hw_input(x, y) = reinterpret<float>(input(x, y));
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        Expr pi = bfloat16_t(3.1415926535f);
        product(x, y)  = cast<bfloat16_t>(hw_input(x, y)) * pi;

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint16_t>(product(x, y));
        //output(x, y) = cast<float>(hw_output(x,y));
        //Expr output_value = cast<uint16_t>(hw_output(x,y));
        //output(x,y) = cast<uint8_t>(output_value);
        output(x,y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);
          
          product.compute_at(hw_output, xo);
          
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PointwiseKernel, inout_fp)
