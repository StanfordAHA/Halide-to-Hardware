#include "Halide.h"

namespace {
    using namespace std;

    using namespace Halide;
    using namespace Halide::ConciseCasts;

    class MatMatMul : public Halide::Generator<MatMatMul> {
    public:
        Input<Buffer<int16_t>>  matrix0{"matrix0", 2};
        Input<Buffer<int16_t>>  matrix1{"matrix1", 2};
        Output<Buffer<int16_t>> output{"output", 2};

        GeneratorParam<int> in_img_width{"in_img_width", 8};
        GeneratorParam<int> in_img_height{"in_img_height", 8};
        GeneratorParam<int> tile_size{"tile_size", 8}; 
        GeneratorParam<int32_t> myunroll{"myunroll", 1};
        GeneratorParam<int> io_unroll{"io_unroll", 1};
    
        void generate() {
            Var x("x"), y("y");
            Func matrix0_input_host("matrix0_input_host"), matrix0_input_cgra("matrix0_input_cgra"), matrix0_input_glb("matrix0_input_glb");
            Func matrix1_input_cgra("matrix1_input_cgra"), matrix1_input_host("matrix1_input_host"), matrix1_input_glb("matrix1_input_glb");
            Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

            matrix0_input_host(x, y) = u16(matrix0(x, y));
            matrix0_input_glb(x, y) = matrix0_input_host(x, y);
            matrix0_input_cgra(x, y) = matrix0_input_glb(x, y);

            matrix1_input_host(x, y) = u16(matrix1(x, y));
            matrix1_input_glb(x, y) = matrix1_input_host(x, y);
            matrix1_input_cgra(x, y) = matrix1_input_glb(x, y);

            RDom r(0, tile_size);
            output_cgra(x, y) = i16(0);
            output_cgra(x, y) += matrix0_input_cgra(r.x, y) * matrix1_input_cgra(r.x, x);

            output_glb(x, y) = output_cgra(x, y);
            hw_output(x, y) = output_glb(x, y);
            output(x, y) = i16(hw_output(x, y));

            if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono) || get_target().has_feature(Target::Pono)) {
                Var yi, y_host, y_glb, y_cgra;
                Var xi, x_host, x_glb, x_cgra;

                output.bound(x, 0, in_img_height);
                output.bound(y, 0, in_img_height);

                hw_output.compute_root();
                hw_output
                    .tile(x,y, x_host,y_host, xi,yi, tile_size, tile_size)
                    .reorder(xi,yi, x_host,y_host)
                    .hw_accelerate(xi, x_host);
                
                output_glb.compute_at(hw_output, x_host);
                output_glb
                    .tile(x,y, x_glb,y_glb, x_cgra,y_cgra, tile_size,tile_size)
                    .reorder(x_cgra,y_cgra, x_glb,y_glb);    

                // output_cgra.store_in(MemoryType::MemoryTile);
                output_cgra.compute_at(output_glb, x_glb);
                // output_cgra.update().reorder(x, r.x);
                output_cgra.update().unroll(r.x, myunroll);

                matrix0_input_cgra.compute_at(output_glb, x_glb);
                matrix0_input_glb.compute_at(hw_output, x_host);
                matrix0_input_host.compute_root();
                matrix0_input_host.accelerator_input();
              
                matrix0_input_glb.unroll(x, io_unroll);
                matrix0_input_cgra.unroll(x, io_unroll);

                matrix1_input_cgra.compute_at(output_glb, x_glb);
                matrix1_input_glb.compute_at(hw_output, x_host);
                matrix1_input_host.compute_root();
                matrix1_input_host.accelerator_input();

                matrix1_input_glb.unroll(x, io_unroll);
                matrix1_input_cgra.unroll(x, io_unroll);
            }
        }
    };
}  // namespace

HALIDE_REGISTER_GENERATOR(MatMatMul, mat_mat_mul)
