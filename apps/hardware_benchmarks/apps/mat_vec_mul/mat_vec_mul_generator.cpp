#include "Halide.h"

namespace {

using namespace std;

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatVecMul : public Halide::Generator<MatVecMul> {
public:
    Input<Buffer<int16_t>>  matrix{"matrix", 2};
    Input<Buffer<int16_t>>  vector{"vector", 1};
    Output<Buffer<int16_t>> output{"output", 1};

    GeneratorParam<int> out_X{"out_img", 10};
    GeneratorParam<int32_t> myunroll{"myunroll", 5};
  
    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y");
      Func hw_input("hw_input"), vector_input("vector_input"), multiply, hw_output;

      hw_input(x, y) = i16(matrix(x, y));
      vector_input(x) = i16(vector(x));
        
      RDom r(0, out_X);  // input mat dim

      multiply(x) += hw_input(x,r) * vector_input(r);  
      // max(hw_input(x,y) + hw_input_bias(x, y), i16(0));
      
      hw_output(x) = multiply(x);
      output(x) = i16(hw_output(x));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,xo;
          // RVar rx_unroll("rx_unroll");

          output.bound(x, 0, out_X);
          
          // hw_input.compute_root();
          hw_output.compute_root();

          hw_output
            .split(x,xo,xi,out_X)
            .reorder(xi,xo)
            .hw_accelerate(xi,xo);

          multiply.compute_at(hw_output, xo)
            .unroll(x, myunroll);

          // multiply.compute_root();
          
          hw_input.stream_to_accelerator();
          // hw_input.in().unroll(x, myunroll);

          vector_input.stream_to_accelerator();
          // vector_input.in().unroll(x, myunroll);

        } else {

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatVecMul, mat_vec_mul)
