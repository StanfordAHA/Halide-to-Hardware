#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class E8M0QuantTest : public Halide::Generator<E8M0QuantTest> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Input<Buffer<uint16_t>>  e8m0{"e8m0", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    int imgsize = 64;
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input"), hw_input_bfloat("hw_input_bfloat");
        hw_input(x, y) = bf16(input(x, y));
        hw_input_bfloat(x, y) = hw_input(x, y);

        Func hw_e8m0("hw_e8m0");
        hw_e8m0(x, y) = e8m0(x, y);

        Func quant("quant");
        if (get_target().has_feature(Target::Clockwork)) {
          quant(x, y) = e8m0_quant(hw_input_bfloat(x, y), bf16(hw_e8m0(x, y)));
        } else {
          quant(x, y) = e8m0_quant(hw_input(x, y), hw_e8m0(x, y));
        }

        Func hw_output("hw_output");
        hw_output(x, y) = u16(quant(x, y));
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
          hw_e8m0.stream_to_accelerator();

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

HALIDE_REGISTER_GENERATOR(E8M0QuantTest, fp_e8m0_quant_test)
