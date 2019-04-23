#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel"), fp_kernel("fp_kernel");
        Func conv("conv");
        RDom r(0, 2,
               0, 2);

        kernel(x,y) = 0.f;
        kernel(0,0) = 11.f;      kernel(0,1) = 12.f;
        kernel(1,0) = 14.f;      kernel(1,1) = 0.f;
        fp_kernel(x, y) = kernel(x, y);

        conv(x, y) = cast<float>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<float>(input(x, y));
        conv(x, y)  += fp_kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint16_t>(conv(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);

          conv.linebuffer();

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

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, fp_conv_3_3)
