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

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, 1,
               0, 1);

        kernel(x,y) = 0;
        kernel(0,0) = 11;

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y) = undef<uint16_t>();  // temporary placeholder, halide will skip initialization internally
        conv(x, y) = select(
          r.x == 0 && r.y == 0,
          u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y),
          conv(x, y) + u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y)
        );

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          conv.bound(x, 0, 64);
          conv.bound(y, 0, 64);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);

          conv.linebuffer();

          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);
          //kernel.compute_at(conv, x);
          //kernel.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);

          conv.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();


        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_1_1)
