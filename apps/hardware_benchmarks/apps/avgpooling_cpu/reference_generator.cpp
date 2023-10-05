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

    GeneratorParam<int> out_img{"out_img", 512};
    GeneratorParam<int32_t> myunroll{"myunroll", 8};
  
    void generate() {
        /* THE ALGORITHM */

      Var x("x"), y("y");
      Func matrix_input_host("matrix_input_host"), matrix_input_cgra("matrix_input_cgra"), matrix_input_glb("matrix_input_glb"), vector_input_cgra("vector_input_cgra"), vector_input_host("vector_input_host"), vector_input_glb("vector_input_glb"), multiply, hw_output, hw_output_glb;

      matrix_input_host(x, y) = u16(matrix(x, y));
      matrix_input_glb(x, y) = matrix_input_host(x, y);
      matrix_input_cgra(x, y) = matrix_input_glb(x, y);

      vector_input_host(y) = u16(vector(y));
      vector_input_glb(y) = vector_input_host(y);
      vector_input_cgra(y) = vector_input_glb(y);
        
      RDom r(0, out_img);  // input mat dim
      multiply(y) += matrix_input_cgra(r.x, y) * vector_input_cgra(r.x);  
      
      hw_output(y) = multiply(y);
      hw_output_glb(y) = hw_output(y);
      output(y) = i16(hw_output(y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(y, 0, out_img);
          
          hw_output_glb.compute_root();
          

          hw_output.compute_root();

          hw_output
            .split(y,yo,yi,4)
            .reorder(yi, yo)
            .hw_accelerate(yi, yo);

          multiply.compute_at(hw_output, yo);
          // multiply.update().unroll(r.x, myunroll);

          matrix_input_cgra.compute_at(hw_output, yo);
          matrix_input_glb.compute_root();
          matrix_input_host.accelerator_input();
          matrix_input_host.compute_root();
          // matrix_input_glb.compute_at(hw_output, yo);
          // matrix_input.stream_to_accelerator();
          // matrix_input.in().unroll(x, 8);

          vector_input_cgra.compute_at(hw_output, yo);
          vector_input_glb.compute_root();
          vector_input_host.accelerator_input();
          vector_input_host.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatVecMul, mat_vec_mul)