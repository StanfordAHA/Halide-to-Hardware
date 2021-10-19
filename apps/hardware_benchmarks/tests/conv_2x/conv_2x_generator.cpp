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
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 62);
          output.bound(y, 0, 62);
          
          hw_output.in().compute_root();

          hw_output.in()
            .tile(x, y, xo, yo, xi, yi, 62, 62)
            .hw_accelerate(xi, xo);
          hw_output.in().store_in(MemoryType::GLB);
          hw_output.in().unroll(xi, 2, TailStrategy::RoundUp);;
          
          hw_output.tile(x,y, xo,yo, xi,yi, 62, 62);
          hw_output.compute_at(hw_output.in(), xo);
          hw_output.unroll(xi, 2, TailStrategy::RoundUp);

          conv.unroll(x, 2, TailStrategy::RoundUp);
          conv.update()
            .unroll(x, 2, TailStrategy::RoundUp)
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          kernel.compute_at(hw_output, xo);
          conv.compute_at(hw_output, xo);

          hw_input.in().in().compute_at(hw_output, xo); // represents the mem tile
          hw_input.in().in().unroll(x, 2, TailStrategy::RoundUp);
          
          hw_input.in().compute_at(hw_output.in(), xo);
          hw_input.in().store_in(MemoryType::GLB);
          hw_input.in().unroll(x, 2, TailStrategy::RoundUp);
            
          hw_input.compute_root()
            .accelerator_input();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvTest, conv_2x)
