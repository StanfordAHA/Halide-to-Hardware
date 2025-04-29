#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class UnitTestFPArith : public Halide::Generator<UnitTestFPArith> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    int imgsize = 64;
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input"), hw_input_bfloat("hw_input_bfloat");
        hw_input(x, y) = bf16(input(x, y));
        hw_input_bfloat(x, y) = hw_input(x, y);

        Func get_shared_expo("get_shared_expo");
        get_shared_expo(x, y) = get_shared_exp(hw_input_bfloat(x, y));

        Func hw_output("hw_output");
        hw_output(x, y) = u16(get_shared_expo(x, y));
        output(x, y)    = u16(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          // CPU schedule must be similar for bfloat16 for outputs to be bit exact
          output.compute_root();

          Var xi,yi, xo,yo;
          hw_output
            .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestFPArith, fp_get_shared_exp_test)
