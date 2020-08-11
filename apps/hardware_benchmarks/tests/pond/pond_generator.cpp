#include "Halide.h"

namespace {

using namespace Halide;

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
        hw_input(x, y) = cast<int16_t>(input(x, y));
        
        conv(x, y) = 0;
        conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        
        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(lookup(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-3);
          output.bound(y, 0, 64);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-3, 64)
            .hw_accelerate(xi, xo);

          conv.linebuffer();

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MemTestPond, pond)
