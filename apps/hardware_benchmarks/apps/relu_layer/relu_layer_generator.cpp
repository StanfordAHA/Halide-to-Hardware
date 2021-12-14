#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class ReluLayer : public Halide::Generator<ReluLayer> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  input_bias{"input_bias", 3};
    Output<Buffer<int16_t>> output{"output", 3};

    GeneratorParam<int> in_img{"in_img", 56};
    GeneratorParam<int> n_oc{"n_oc", 32};
  
    GeneratorParam<int32_t> myunroll{"myunroll", 8};
  
    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y"), w("w");
      Func hw_input("hw_input"), hw_input_bias("hw_input_bias"), relu, hw_output;

      hw_input(w, x, y) = i16(input(w, x, y));
      hw_input_bias(w, x, y) = i16(input_bias(w, x, y));
        
      relu(w, x, y) = max(hw_input(w,x,y) + hw_input_bias(w, x, y), i16(0));
      
      hw_output(w, x, y) = relu(w, x, y);
      output(w, x, y) = i16(hw_output(w, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, in_img);
          output.bound(y, 0, in_img);
          output.bound(w, 0, n_oc);
          
          //hw_input.compute_root();
          hw_output.compute_root();
          hw_output
            .tile(x,y, xo,yo, xi,yi, in_img, in_img)
            .reorder(w, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          hw_output.unroll(w, myunroll);

          relu.compute_at(hw_output, xo)
            .unroll(w, myunroll);
          
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(w, myunroll);

          hw_input_bias.stream_to_accelerator();
          hw_input_bias.in().unroll(w, myunroll);
          
        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ReluLayer, relu_layer)
