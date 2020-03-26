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
        kernel(1,0) = uint16_t(14);
        kernel(2,0) = uint16_t(17);

        conv(x, y) = uint16_t(0);

        Func input16("input16");
        Func hw_output;
        input16(x, y) = cast<uint16_t>(input(x, y));
        conv(x, y) += kernel(r.x, r.y) * input16(x + r.x, y + r.y);

        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x, y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();

          output.bound(x, 0, 64);
          output.bound(y, 0, 64-2);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 3);

          conv.linebuffer();
          
          input16.compute_at(hw_output, xi).store_at(hw_output, xo);
          input16.stream_to_accelerator();
          
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
