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

        int filter = 3;
        int stride = 2;

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, filter,
               0, filter);

        kernel(x,y) = 0;
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

        conv(x, y) = u16(0);

        Func input_copy, hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x * stride + r.x, y * stride + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x,y));

        output.bound(x, 0, 31);
        output.bound(y, 0, 31);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, (64 - (filter - 1))/stride, (64 - (filter - 1))/stride )
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, filter)
            .unroll(r.y, filter);

          conv.linebuffer();

          kernel.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, (64 - (filter - 1))/stride, (64 - (filter - 1))/stride )
              .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, filter)
            .unroll(r.y, filter);

          conv.compute_at(hw_output, xo);

          kernel.compute_at(conv, x);

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, filter)
            .unroll(r.y, filter);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, strided_conv)
