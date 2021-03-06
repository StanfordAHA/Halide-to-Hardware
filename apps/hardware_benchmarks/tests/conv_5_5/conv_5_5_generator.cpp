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
        RDom r(0, 5,
               0, 5);

        kernel(x,y) = 0;
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;      kernel(0,3) = 23;      kernel(0,4) = 33;
        kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;      kernel(1,3) = 21;      kernel(1,4) = 31;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;      kernel(2,3) = 26;      kernel(2,4) = 32;
        kernel(3,0) = 20;      kernel(3,1) = 29;      kernel(3,2) = 22;      kernel(3,3) = 24;      kernel(3,4) = 34;
        kernel(4,0) = 30;      kernel(4,1) = 39;      kernel(4,2) = 32;      kernel(4,3) = 34;      kernel(4,4) = 37;

        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x,y));


        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();

          output.bound(x, 0, 64-4);
          output.bound(y, 0, 64-4);
          conv.bound(x, 0, 64-4);
          conv.bound(y, 0, 64-4);

          hw_output.tile(x,y, xo,yo, xi,yi, 64-4, 64-4)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 5)
            .unroll(r.y, 5);

          conv.linebuffer();

          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-4);
          output.bound(y, 0, 64-4);

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 64-4, 64-4)
            .hw_accelerate(xi, xo);

          kernel.compute_at(hw_output, xo);
          conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x, 5)
            .unroll(r.y, 5);

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, 5)
            .unroll(r.y, 5);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_5_5)
