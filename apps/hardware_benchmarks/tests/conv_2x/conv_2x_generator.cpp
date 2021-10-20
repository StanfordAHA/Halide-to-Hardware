#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

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

        conv(x, y) = u16(0);

        Func hw_input("hw_input"), hw_input_copy;
        hw_input(x, y) = u16(input(x, y));
        hw_input_copy(x, y) = hw_input(x, y);
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input_copy(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

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
          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 62);
          output.bound(y, 0, 62);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 62, 62)
            .hw_accelerate(xi, xo);


          hw_output.unroll(xi, 2);
            
          kernel.compute_at(hw_output, xo);

          conv.update()
            .unroll(x, 2)
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          hw_input_copy.unroll(x,2);
          hw_input_copy.compute_at(hw_output, xo);
          hw_input.accelerator_input();
          hw_input.compute_root();

        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvTest, conv_2x)
