#include "Halide.h"

namespace {

using namespace Halide;

float gamma = 1.0f;
float contrast = 1.0f;

class ConvTest : public Halide::Generator<ConvTest> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int ksize = 3;
    int imgsize = 62;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        kernel(x,y) = 0;
        kernel(0,0) = 17;     kernel(0,1) = 4;       kernel(0,2) = 6;
        kernel(1,0) = 7;      kernel(1,1) = 19;      kernel(1,2) = 4;
        kernel(2,0) = 5;      kernel(2,1) = 21;      kernel(2,2) = 15;

        conv(x, y) = 0;

        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);
          hw_output.unroll(x, 2);

          conv.update()
            .unroll(x, 2)
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          conv.linebuffer();

          hw_input.unroll(x, 2);
          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvTest, conv_2x)
