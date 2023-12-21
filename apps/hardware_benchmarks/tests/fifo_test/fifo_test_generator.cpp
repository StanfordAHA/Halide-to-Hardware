#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

float gamma = 1.0f;
float contrast = 1.0f;

class UnitTestFIFO : public Halide::Generator<UnitTestFIFO> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func conv, merge;
        conv(x, y) = hw_input(x, y) + hw_input(x+1, y+1);
        merge(x, y) = conv(x, y) + hw_input(x, y);

        Func hw_output("hw_output");
        hw_output(x, y) = merge(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 63);
          output.bound(y, 0, 63);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 63, 63)
            .hw_accelerate(xi, xo);

          merge.compute_at(hw_output, xo);
          conv.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFIFO, fifo_test)
