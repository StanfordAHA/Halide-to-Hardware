#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");

        int channels = 3;
        int filter = 3;

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, filter,
               0, filter);

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

        conv(x, y, z) = 0;

        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        conv(x, y, z)  += kernel(r.x, r.y, z) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y, z) = cast<uint8_t>(conv(x, y, z));
        output(x, y, z) = hw_output(x,y,z);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-(filter-1), 64-(filter-1))
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, filter)
            .unroll(r.y, filter);

          //conv.linebuffer();

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

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, multi_channel_conv)
