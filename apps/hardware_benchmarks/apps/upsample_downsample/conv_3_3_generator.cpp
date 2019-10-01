#include "Halide.h"

namespace {

using namespace Halide;

// UNet?
class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");


        Func downsample("downsample");


        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        downsample(x, y) = hw_input(x / 2, y / 2);


        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(downsample(x, y));
        
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;
         
          //kernel.compute_at(hw_input, xo);
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-2, 64-2)
            .hw_accelerate(xi, xo);

          //conv.update()
            //.unroll(r.x, 3)
            //.unroll(r.y, 3);

          //conv.linebuffer();

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          //kernel.compute_root();
          //conv.compute_root();
          //conv.update()
            //.unroll(r.x, 3)
            //.unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_3)
