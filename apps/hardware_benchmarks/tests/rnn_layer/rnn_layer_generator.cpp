#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatrixKernel : public Halide::Generator<MatrixKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Input<Buffer<uint8_t>>  prev_state{"prev_state", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int imgsize = 64;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");

        Func weight_input("weight_input");   // commonly U
        Func weight_state("weight_state");   // commonly W
        Func weight_output("weight_output"); // commonly V
        Func mul_input("mul_input"), mul_state("mul_state"), mul_output ("mul_output");
        Func next_state("next_state");
        RDom r(0, imgsize);

        weight_input(x, y) = 2*x + y;
        weight_state(x, y) = y - x;
        weight_output(x, y) = 2*x*y;
        
        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        mul_input(x, y)  += hw_input(r.x, y) * u16(weight_input(x, r.x));

        Func hw_state("hw_state");
        hw_state(x, y) = u16(prev_state(x, y));
        mul_state(x, y)  += hw_state(r.x, y) * u16(weight_state(x, r.x));

        int bias = 0;
        next_state(x, y) = tanh(mul_input(x, y) + mul_state(x, y) + bias);

        mul_output(x, y) += next_state(r.x, y) * u16(weight_output(x, r.x));

        Func hw_output("hw_output");
        hw_output(x, y) = mul_output(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .reorder(xi,yi, xo,yo)
              .hw_accelerate(xi, xo);

          mul_output.compute_at(hw_output, xo);
          mul_state.compute_at(hw_output, xo);
          mul_input.compute_at(hw_output, xo);

          hw_state.stream_to_accelerator();
          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          hw_output.compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatrixKernel, rnn_layer)
