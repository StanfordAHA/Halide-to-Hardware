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
        Func blur("blur");
        RDom r(0, 2,
               0, 2);

        kernel(x,y) = 0;
        kernel(0,0) = 1;      kernel(0,1) = 1;
        kernel(1,0) = 1;      kernel(1,1) = 1;
 
        blur(x, y) = cast<uint16_t>(0);

        Func hw_input("hw_input");
        Func brighten;
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        brighten(x, y) = hw_input(x, y) * 2;
        blur(x, y)  += cast<uint16_t>(kernel(r.x, r.y)) * brighten(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = blur(x, y) / 4;
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();

          output.bound(x, 0, 64-1);
          output.bound(y, 0, 64-1);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);

          blur.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);

          blur.linebuffer();

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-1);
          output.bound(y, 0, 64-1);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 64-1, 64-1)
            .hw_accelerate(xi, xo);
          
          kernel.compute_at(blur, x);
          blur.compute_at(hw_output, xo);
          blur.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);
          
          brighten.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          kernel.compute_root();
          blur.compute_root();
          blur.update()
            .unroll(r.x, 2)
            .unroll(r.y, 2);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, brighten_and_blur)
