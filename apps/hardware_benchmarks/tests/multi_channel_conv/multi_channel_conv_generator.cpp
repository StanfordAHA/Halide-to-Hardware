#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");

        int imgsize = 62;
        int channels = 3;
        int ksize = 3;

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,
               0, ksize);

        kernel(x,y,z) = 0;
        kernel(0,0,0) = 11;      kernel(0,1,0) = 12;      kernel(0,2,0) = 13;
        kernel(1,0,0) = 14;      kernel(1,1,0) = 0;       kernel(1,2,0) = 16;
        kernel(2,0,0) = 17;      kernel(2,1,0) = 18;      kernel(2,2,0) = 19;

        kernel(0,0,1) = 21;      kernel(0,1,1) = 22;      kernel(0,2,1) = 23;
        kernel(1,0,1) = 24;      kernel(1,1,1) = 2;       kernel(1,2,1) = 26;
        kernel(2,0,1) = 27;      kernel(2,1,1) = 28;      kernel(2,2,1) = 29;

        kernel(0,0,2) = 31;      kernel(0,1,2) = 32;      kernel(0,2,2) = 33;
        kernel(1,0,2) = 34;      kernel(1,1,2) = 3;       kernel(1,2,2) = 36;
        kernel(2,0,2) = 37;      kernel(2,1,2) = 38;      kernel(2,2,2) = 39;

        conv(x, y, z) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y, z)  += u16(kernel(r.x, r.y, z) * hw_input(x + r.x, y + r.y));

        Func hw_output("hw_output");
        hw_output(x, y, z) = conv(x, y, z);
        output(x, y, z) = u8(hw_output(x,y,z));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(z, 0, 3);

          hw_output.compute_root();
          
          hw_output
            .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .reorder(xi, yi, z, xo, yo)
            .hw_accelerate(xi, xo);

          conv.compute_at(hw_output,xo);
          conv.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, multi_channel_conv)
