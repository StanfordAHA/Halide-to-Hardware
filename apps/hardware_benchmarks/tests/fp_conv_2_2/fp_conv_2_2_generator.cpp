#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel"), fp_kernel("fp_kernel");
        Func conv("conv");
        RDom r(0, 2,
               0, 2);

        kernel(x,y) = bfloat16_t(0.f);
        kernel(0,0) = bfloat16_t(1.f);      kernel(0,1) = bfloat16_t(1.3f);
        kernel(1,0) = bfloat16_t(1.f);      kernel(1,1) = bfloat16_t(4.f);
        fp_kernel(x, y) = cast<bfloat16_t>(kernel(x, y));

        conv(x, y) = cast<bfloat16_t>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));
        conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y) % 256);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-1);
          output.bound(y, 0, 64-1);
          conv.bound(x, 0, 64-1);
          conv.bound(y, 0, 64-1);
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);

          conv.linebuffer();
          
          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, fp_conv_2_2)
