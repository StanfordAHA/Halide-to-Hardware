#include "Halide.h"

namespace {

using namespace Halide;

float gamma = 1.0f;
float contrast = 1.0f;

class UnitTestFIFO : public Halide::Generator<UnitTestFIFO> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input"), input_copy;
        input_copy(x, y) = cast<int16_t>(input(x, y));
        hw_input(x, y) = input_copy(x, y);

        Func conv, merge;
        conv(x, y) = hw_input(x, y) + hw_input(x+1, y+1);
        merge(x, y) = conv(x, y) + hw_input(x, y);

        Func hw_output("hw_output");
        hw_output(x, y) = merge(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 63);
          output.bound(y, 0, 63);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          merge.compute_at(hw_output, xo);
          conv.compute_at(hw_output, xo);

          hw_input.compute_at(hw_output, xo).store_at(hw_output, xo);
          input_copy.accelerator_input();
          input_copy.compute_root();
          
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFIFO, fifo_test)
