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

        Func mult("mult"), sub("sub"), expo("expo");
        mult(x, y) = hw_input_bfloat(x, y) * bfloat16_t(0.02f);
        sub(x, y)  = mult(x, y) - bfloat16_t(1.0f);
        expo(x, y)  = exp(sub(x, y));

        // A simple mult2ear path to compare against expo:
        Func mult2("mult2");
        mult2(x, y) = hw_input_bfloat(x, y) * bfloat16_t(0.1f);

        // abs_max to flip branches many times
        Func absolute_max("absolute_max");
        absolute_max(x, y) = abs_max_bf16(expo(x, y), mult2(x, y));

        // A simple add/div/sub path to compare against abs_max:
        Func add("add"), div("div"), sub2("sub2"), mult3("mult3");
        add(x, y) = absolute_max(x, y) + bfloat16_t(3.0f);
        div(x, y) = bfloat16_t(20.0f) / add(x, y);
        sub2(x, y) = div(x, y) - add(x, y);
        mult3(x, y) = sub2(x, y) * bfloat16_t(5.0f);

        Func hw_output("hw_output");
        hw_output(x, y) = mult3(x, y);
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

HALIDE_REGISTER_GENERATOR(UnitTestFPArith, fp_arith)
