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

    GeneratorParam<int> out_img{"out_img", 8};
    GeneratorParam<int32_t> myunroll{"myunroll", 8};
  
    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y");
      Func matrix_input("matrix_input"), vector_input("vector_input"), multiply, hw_output;

      matrix_input(x, y) = u16(matrix(x, y));
      vector_input(y) = u16(vector(y));
        
      RDom r(0, out_img);  // input mat dim
      multiply(y) += matrix_input(r.x, y) * vector_input(r.x);  
      
      hw_output(y) = multiply(y);
      output(y) = i16(hw_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(y, 0, out_img);
          
          hw_output.compute_root();

          hw_output
            .split(y,yo,yi,out_img)
            .reorder(yi, yo)
            .hw_accelerate(yi, yo);

          multiply.compute_at(hw_output, yo);
          multiply.update().unroll(r.x, myunroll);

          matrix_input.stream_to_accelerator();
          // matrix_input.in().unroll(x, myunroll);

          vector_input.stream_to_accelerator();
          // vector_input.in().unroll(y, myunroll);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatVecMul, mat_vec_mul)
