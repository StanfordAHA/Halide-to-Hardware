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

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, 3,               0, 3);

        kernel(x,y) = 0;
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

        //conv(x, y) = 0;

        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        //conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        conv(x,y) =
          kernel(0,0)*hw_input(x,y) +
          kernel(1,0)*hw_input(x+1,y) +
          kernel(2,0)*hw_input(x+2,y) +
          kernel(0,1)*hw_input(x,y+1) +
          kernel(1,1)*hw_input(x+1,y+1) +
          kernel(2,1)*hw_input(x+2,y+1) +
          kernel(0,2)*hw_input(x,y+2) +
          kernel(1,2)*hw_input(x+1,y+2) +
          kernel(2,2)*hw_input(x+2,y+2);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-2, 64-2)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);

          conv.linebuffer();

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
//          kernel.compute_root();
//          conv.compute_root();
//          conv.update()
//            .unroll(r.x, 3)
//            .unroll(r.y, 3);
          Var xi,yi, xo,yo;
          //output.tile(x, y, xo,yo, xi,yi, 64,64);
          
          hw_input.in().store_root().compute_at(output, x).unroll(x, 2);
          //output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_3)
