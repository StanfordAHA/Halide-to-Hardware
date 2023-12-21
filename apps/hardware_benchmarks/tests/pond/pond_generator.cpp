#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

float gamma = 1.0f;
float contrast = 1.0f;

class MemTestPond : public Halide::Generator<MemTestPond> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */
        // similar to conv_2input, look at resnet process.cpp

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, 4,
               0, 1);

        kernel(x,y) = 0;
        kernel(0,0) = 1;  kernel(1,0) = 3;  kernel(2,0) = 7;  kernel(3,0) = 18; //pass the kernel in as an input
        
        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        
        conv(x, y) = u16(0);
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);
        
        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-3);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-3, 64)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MemTestPond, pond)
