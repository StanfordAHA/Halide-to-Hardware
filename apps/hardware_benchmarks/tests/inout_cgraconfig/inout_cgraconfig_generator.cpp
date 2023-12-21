#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class InOutTestCgraConfig : public Halide::Generator<InOutTestCgraConfig> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Input<Buffer<uint8_t>>  weight{"weight", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

  int ksize = 2;
  //int imgsize = 64-ksize+1;
  int imgsize = 64;
  int height = imgsize;
  int width = imgsize;


    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        conv(x, y) = u16(0);

        Func hw_input("hw_input"), hw_weight("hw_weight");
        // Test that input padding is allowed
        hw_input(x, y) = u16(input(clamp(x, 0, width - 1), clamp(y, 0, height - 1)));
        
        // Second input
        hw_weight(x, y) = u16(weight(x, y));

        // This one has the hw_weight input much smaller
        //conv(x, y)  += u16(hw_weight(r.x, r.y)) * hw_input(x + r.x, y + r.y);
        
        conv(x, y)  += hw_weight(x, y) * hw_input(x + r.x, y + r.y);
        //conv(x, y)  += hw_weight(x, y) * hw_input(x + 1, y + 1);
        //conv(x, y)  += hw_weight(x, y) * hw_input(x, y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);

        // Output is a different bitwidth and value than the cgra output
        output(x, y) = u8(hw_output(x,y) + 6);

        output.bound(x, 0, imgsize);
        output.bound(y, 0, imgsize);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          // This tests that this input can be unrolled for multiple pixels/cycle
          //hw_input.in().unroll(x, ksize);
          
          hw_input.stream_to_accelerator();
          hw_weight.stream_to_accelerator();


        } else {  // schedule to CPU
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(InOutTestCgraConfig, inout_cgraconfig)
