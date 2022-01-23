#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class PointwiseKernel : public Halide::Generator<PointwiseKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func product("product");

        Func hw_input("hw_input");
        hw_input(x, y) = bf16(input(x, y));
        Expr pi = bfloat16_t(3.1415926535f); // this should be stored as: 0x4049 == 3.140625
        //Expr pi = bfloat16_t(13.3f); // this should be stored as: 0x4049 == 3.140625
        product(x, y)  = hw_input(x, y) * pi;

        // with pi, round to even outputs 7 * pi = 22.0
        //    while trunction outputs     7 * pi = 21.875
        Func hw_output("hw_output");
        hw_output(x, y) = product(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
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

HALIDE_REGISTER_GENERATOR(PointwiseKernel, fp_pointwise)
