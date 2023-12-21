#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
    int ksize = 4;
    int imgsize = 61;
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel"), fp_kernel("fp_kernel");
        Func conv("conv");
        RDom r(0, 4,
               0, 4);

        kernel(x,y) = bfloat16_t(0.f);
        kernel(0,0) = bfloat16_t(11.f);      kernel(0,1) = bfloat16_t(12.f);      kernel(0,2) = bfloat16_t(13.f);      kernel(0,3) = bfloat16_t(23.f);
        kernel(1,0) = bfloat16_t(14.f);      kernel(1,1) = bfloat16_t(0.f);       kernel(1,2) = bfloat16_t(16.f);      kernel(1,3) = bfloat16_t(21.f);
        kernel(2,0) = bfloat16_t(17.f);      kernel(2,1) = bfloat16_t(18.f);      kernel(2,2) = bfloat16_t(19.f);      kernel(2,3) = bfloat16_t(32.f);
        kernel(3,0) = bfloat16_t(20.f);      kernel(3,1) = bfloat16_t(29.f);      kernel(3,2) = bfloat16_t(22.f);      kernel(3,3) = bfloat16_t(24.f);
        fp_kernel(x) = cast<bfloat16_t>(4*x);

        conv(x, y) = cast<bfloat16_t>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));
        conv(x, y)  += fp_kernel(r.x + 4* r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(clamp(hw_output(x,y), 0, 255));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x)
            .unroll(r.y);

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
