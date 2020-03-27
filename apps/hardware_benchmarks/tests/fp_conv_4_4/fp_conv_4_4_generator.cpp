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
        RDom r(0, 4,
               0, 4);

        kernel(x,y) = 0.f;
        kernel(0,0) = 11.f;      kernel(0,1) = 12.f;      kernel(0,2) = 13.f;      kernel(0,3) = 23.f;
        kernel(1,0) = 14.f;      kernel(1,1) = 0.f;       kernel(1,2) = 16.f;      kernel(1,3) = 21.f;
        kernel(2,0) = 17.f;      kernel(2,1) = 18.f;      kernel(2,2) = 19.f;      kernel(2,3) = 32.f;
        kernel(3,0) = 20.f;      kernel(3,1) = 29.f;      kernel(3,2) = 22.f;      kernel(3,3) = 24.f;
        fp_kernel(x, y) = cast<bfloat16_t>(kernel(x, y));

        conv(x, y) = cast<bfloat16_t>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));
        conv(x, y)  += fp_kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y) % 256);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();

          output.bound(x, 0, 64-3);
          output.bound(y, 0, 64-3);
          conv.bound(x, 0, 64-3);
          conv.bound(y, 0, 64-3);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-3, 64-3)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 4)
            .unroll(r.y, 4);

          conv.linebuffer();

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, 4)
            .unroll(r.y, 4);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, fp_conv_4_4)
