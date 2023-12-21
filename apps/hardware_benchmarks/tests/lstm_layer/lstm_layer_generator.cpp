#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatrixKernel : public Halide::Generator<MatrixKernel> {
public:
    Input<Buffer<int16_t>>  input{"input", 2};
    Input<Buffer<int16_t>>  prev_state{"prev_state", 2};
    Input<Buffer<int16_t>>  prev_cell{"prev_cell", 2};
    Output<Buffer<int16_t>> output{"output", 2};

    int imgsize = 64;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");

        // commonly U for the inputs
        Func wf_input("wf_input"), wi_input("wi_input"), wo_input("wo_input"), wc_input("wc_input");
        // commonly W for the prev state
        Func wf_state("wf_state"), wi_state("wi_state"), wo_state("wo_state"), wc_state("wc_state");

        RDom r(0, imgsize);

        wf_input(x, y) = 2*x + y;      wf_state(x, y) = y - x;
        wi_input(x, y) = 5*x + y;      wi_state(x, y) = 2*y - x;
        wo_input(x, y) = 7*x + y;      wo_state(x, y) = y - 3*x;
        wc_input(x, y) = 8*x + y;      wc_state(x, y) = 5*y - x;
        
        Func hw_input("hw_input");
        hw_input(x, y) = i16(input(x, y));

        Func hw_state("hw_state");
        hw_state(x, y) = i16(prev_state(x, y));

        Func hw_cell("hw_cell");
        hw_cell(x, y) = i16(prev_cell(x, y));

        Func mulf_input("mulf_input"), mulf_state("mulf_state"), forget_gate("forget_gate");
        mulf_input(x, y)  += hw_input(r.x, y) * i16(wf_input(x, r.x));
        mulf_state(x, y)  += hw_state(r.x, y) * i16(wf_state(x, r.x));
        int bias_f = 1;
        forget_gate(x, y) = sigmoid(bias_f + mulf_input(x, y) + mulf_state(x, y));

        Func muli_input("muli_input"), muli_state("muli_state"), input_gate("input_gate");
        muli_input(x, y)  += hw_input(r.x, y) * i16(wi_input(x, r.x));
        muli_state(x, y)  += hw_state(r.x, y) * i16(wi_state(x, r.x));
        int bias_i = 3;
        input_gate(x, y) = sigmoid(bias_i + muli_input(x, y) + muli_state(x, y));

        Func mulo_input("mulo_input"), mulo_state("mulo_state"), output_gate("output_gate");
        mulo_input(x, y)  += hw_input(r.x, y) * i16(wo_input(x, r.x));
        mulo_state(x, y)  += hw_state(r.x, y) * i16(wo_state(x, r.x));
        int bias_o = 5;
        output_gate(x, y) = sigmoid(bias_o + mulo_input(x, y) + mulo_state(x, y));

        Func mulc_input("mulc_input"), mulc_state("mulc_state"), candidate("candidate");
        mulc_input(x, y)  += hw_input(r.x, y) * i16(wc_input(x, r.x));
        mulc_state(x, y)  += hw_state(r.x, y) * i16(wc_state(x, r.x));
        int bias_c = 7;
        candidate(x, y) = tanh(bias_c + mulc_input(x, y) + mulc_state(x, y));

        //Func FCp, IC;
        //FCp(x, y) += forget_gate(r.x, y) * hw_cell(x, r.x);
        //IC(x, y) += hw_input(r.x, y) * candidate(x, r.x);
        Func cell("cell"), mul_output("mul_output");
        cell(x, y) =  forget_gate(x, y) * hw_cell(x, y) + input_gate(x, y) * candidate(x, y);
        mul_output(x, y) = output_gate(x, y) * tanh(cell(x, y)); // this is also the next state

        Func hw_output("hw_output");
        hw_output(x, y) = mul_output(x, y);
        output(x, y) = i16(hw_output(x,y));

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
          
          

          hw_cell.stream_to_accelerator();
          hw_state.stream_to_accelerator();
          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          hw_output.compute_root();
        }

    }

  Expr sigmoid(Expr e) {
    using Halide::_;
    return 1 / (1 + exp(-1*e));
  }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatrixKernel, lstm_layer)
