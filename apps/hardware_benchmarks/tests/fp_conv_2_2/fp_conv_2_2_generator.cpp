#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

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
        fp_kernel(x) = cast<bfloat16_t>(2*x);

        conv(x, y) = cast<bfloat16_t>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));
        conv(x, y)  += fp_kernel(r.x + 2*r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(clamp(hw_output(x,y), 0, 255));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-1);
          output.bound(y, 0, 64-1);

          hw_output.compute_root();
          
          hw_output
            .tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);

           conv.update()
             .unroll(r.x, 2)
             .unroll(r.y, 2);

          conv.compute_at(hw_output, xo);
          //fp_kernel.compute_at(hw_output, xo);

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
