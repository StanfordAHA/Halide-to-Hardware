#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class ReluLayer : public Halide::Generator<ReluLayer> {
public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    Input<Buffer<uint16_t>>  bias{"bias", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    GeneratorParam<int> out_img{"out_img", 56};
    GeneratorParam<int> n_oc{"n_oc", 32};
  
    GeneratorParam<int> unroll{"unroll", 8};

    // pad_o determines the output padding
    GeneratorParam<int> pad_o_left{"pad_o_left", 0};    // default: 0
    GeneratorParam<int> pad_o_right{"pad_o_right", 0};    // default: 0
    // trunc_size determines the number of rows & cols to write zeros
    GeneratorParam<int> trunc_size{"trunc_size", 0};    // default: 0
  
    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y"), w("w");
      Func hw_input("hw_input"), input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
      Func hw_bias("hw_bias"), bias_host("bias_host"), bias_glb("bias_glb"), bias_cgra("bias_cgra");
      Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
      Func relu6;

      hw_input(w, x, y) = cast<bfloat16_t>(input(w, x, y));
      hw_bias(w, x, y) = cast<bfloat16_t>(bias(w, x, y));

      relu6(w, x, y) = min(max(hw_input(w, x, y) + hw_bias(w, x, y), cast<bfloat16_t>(0.0f)), cast<bfloat16_t>(6.0f));
      
      hw_output(w, x, y) = relu6(w, x, y);
      output(w, x, y) = cast<uint16_t>(hw_output(w, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, out_img);
          output.bound(y, 0, out_img);
          output.bound(w, 0, n_oc);
          
          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output
            .tile(x,y, xo,yo, xi,yi, out_img, out_img)
            .reorder(w, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          hw_output.unroll(w, unroll);

          relu6.compute_at(hw_output, xo).unroll(w, unroll);
          
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(w, unroll);

          hw_bias.stream_to_accelerator();
          hw_bias.in().unroll(w, unroll);
          
        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ReluLayer, relu_layer_fp)
