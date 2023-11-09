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

        GeneratorParam<int> out_img{"out_img", 32};
        GeneratorParam<int> tile_size{"tile_size", 32}; 
        GeneratorParam<int32_t> myunroll{"myunroll", 8};
        GeneratorParam<int> io_unroll{"io_unroll", 8};
    
        void generate() {
            Var x("x"), y("y");
            Func matrix_input_host("matrix_input_host"), matrix_input_cgra("matrix_input_cgra"), matrix_input_glb("matrix_input_glb");
            Func vector_input_cgra("vector_input_cgra"), vector_input_host("vector_input_host"), vector_input_glb("vector_input_glb");
            Func multiply("multiply");
            Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

            matrix_input_host(x, y) = u16(matrix(x, y));
            matrix_input_glb(x, y) = matrix_input_host(x, y);
            matrix_input_cgra(x, y) = matrix_input_glb(x, y);

            vector_input_host(y) = u16(vector(y));
            vector_input_glb(y) = vector_input_host(y);
            vector_input_cgra(y) = vector_input_glb(y);

            RDom r(0, out_img);
            output_cgra(y) += matrix_input_cgra(r.x, y) * vector_input_cgra(r.x);  

            output_glb(y) = output_cgra(y);
            hw_output(y) = output_glb(y);
            output(y) = i16(hw_output(y));

            if (get_target().has_feature(Target::Clockwork)) {
                Var yi, y_host, y_glb, y_cgra;

                output.bound(y, 0, out_img);

                hw_output.compute_root();
                hw_output
                    .split(y, y_host, yi, out_img)
                    .reorder(yi, y_host)
                    .hw_accelerate(yi, y_host);
                
                output_glb.compute_at(hw_output, y_host);
                output_glb
                    .split(y, y_glb, y_cgra, tile_size)
                    .reorder(y_cgra, y_glb);    

                output_cgra.compute_at(output_glb, y_glb);
                output_cgra.update().unroll(r.x, myunroll);

                matrix_input_cgra.compute_at(output_glb, y_glb);
                matrix_input_glb.compute_at(hw_output, y_host);
                matrix_input_host.compute_root();
                matrix_input_host.accelerator_input();
              
                matrix_input_glb.unroll(x, io_unroll);
                matrix_input_cgra.unroll(x, io_unroll);

                vector_input_cgra.compute_at(output_glb, y_glb);
                vector_input_glb.compute_at(hw_output, y_host);
                vector_input_host.compute_root();
                vector_input_host.accelerator_input();
            }
        }
    };
}  // namespace

HALIDE_REGISTER_GENERATOR(MatVecMul, mat_vec_mul)
