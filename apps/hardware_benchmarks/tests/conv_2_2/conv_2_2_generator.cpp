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
        RDom r(0, 2,
               0, 2);

        kernel(x,y) = 0;
        kernel(0,0) = 11;      kernel(0,1) = 12;
        kernel(1,0) = 14;      kernel(1,1) = 0;
 
        conv(x, y) = cast<uint16_t>(0);

        Func hw_input("hw_input");
        Func hw_input_copy("hw_input_copy");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        hw_input_copy(x, y) = hw_input(x, y);
        conv(x, y)  += cast<uint16_t>(kernel(r.x, r.y)) * hw_input_copy(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();

          output.bound(x, 0, 64-1);
          output.bound(y, 0, 64-1);
          conv.bound(x, 0, 64-1);
          conv.bound(y, 0, 64-1);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);

          conv.linebuffer();

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-1);
          output.bound(y, 0, 64-1);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);

            
          kernel.compute_at(conv, x);
          conv.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);

          hw_input_copy.compute_at(hw_output, xo);
          hw_input.compute_root();
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

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_2_2)
