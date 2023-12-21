#include "Halide.h"

namespace {

using namespace Halide;
using namespace ConciseCasts;

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
               0, 3);

        kernel(x,y) = uint16_t(0);
        kernel(0,0) = uint16_t(11);
        kernel(0,1) = uint16_t(14);
        kernel(0,2) = uint16_t(17);

        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();

          output.bound(x, 0, 64);
          output.bound(y, 0, 64-2);
          conv.bound(x, 0, 64);
          conv.bound(y, 0, 64-2);

          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 3);

          conv.linebuffer();

          //kernel.compute_at(hw_output, yi);
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64-2);

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            .hw_accelerate(xi, xo);

          kernel.compute_at(hw_output, xo);
          conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 3);

          hw_input.stream_to_accelerator();


        } else {  // schedule to CPU
          kernel.compute_root();

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 3);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_1)
