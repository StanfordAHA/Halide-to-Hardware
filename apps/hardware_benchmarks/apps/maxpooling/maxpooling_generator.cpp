#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MaxPooling : public Halide::Generator<MaxPooling> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};

    Output<Buffer<uint8_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 114};    // default: 114
  
    // ksize determines the output stencil size
    GeneratorParam<int> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 2};  // default: 1

    // n_ic determines the number of input channels
    GeneratorParam<int> n_ic{"n_ic", 64};    // default: 16

    GeneratorParam<int> unroll{"unroll", 8};    // default: 16


    void generate() {
      
        /* THE ALGORITHM */
        // Define algorithm variables
        Var x("x"), y("y"), c("c");
        Func input_host("input_host");
        Func hw_output("hw_output");

        // Define hardware input and reduction domain
        input_host(c, x, y) = cast<uint16_t>(input(c, x, y));
        RDom r(0, ksize, 0, ksize);
       
        // MaxPooling Expression
        Func max_pooling_inner;
        Func max_pooling;
        max_pooling(c, x, y) = 
            maximum(r, input_host(c, stride * x + r.x, stride * y + r.y), max_pooling_inner);

        // Send the Output
        hw_output(c, x, y) = cast<uint16_t>(max_pooling(c, x, y));
        output(c, x, y) = cast<uint8_t>(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define scheduling variables
            int out_img = floor((int(in_img) - int(ksize)) / stride) + 1;
            int gbsize = out_img / 4;
            int tile_size = gbsize;
            Var x_host, x_glb, x_cgra;
            Var y_host, y_glb, y_cgra;
            Var c_glb, c_cgra;
    
            // Create output boundaries
            output.bound(x, 0, out_img);
            output.bound(y, 0, out_img);
            output.bound(c, 0, n_ic);

            // Tile the image at host level, by default the tile size is the whole output image
            // Reorder channel dimension to be the innermost to get the clockwork pass
            hw_output.in().compute_root();
            hw_output.in()
                .tile(x, y, x_host, y_host, x_glb, y_glb, gbsize, gbsize)
                .reorder(c, x_glb, y_glb, x_host, y_host)
                .hw_accelerate(x_glb, x_host);
            
            // Unroll output channels along glb; by default using all GLB tiles
            hw_output.in().unroll(c, unroll);
            hw_output.in().store_in(MemoryType::GLB);

            // Tile the image at glb level, by default the tile size is the whole output image
            // Split channel dimension for hardware unrolling
            // Reorder c_glb to be inner level to get clockwork pass
            hw_output
                .tile(x, y, x_glb, y_glb, x_cgra, y_cgra, tile_size, tile_size)
                .split(c, c_glb, c_cgra, unroll)
                .reorder(c_cgra, c_glb, x_cgra, y_cgra, x_glb, y_glb);
            hw_output.compute_at(hw_output.in(), x_host);
            hw_output.unroll(c_cgra, unroll);

            // Fully unroll the reduction domain to create line buffer scheduling
            // Unroll channel dimension for hardware compute unrolling
            // .update() is for initialization
            max_pooling_inner.update().unroll(r.x, ksize).unroll(r.y, ksize);
            max_pooling_inner.update().unroll(c, unroll);
            max_pooling_inner.unroll(c, unroll);
            max_pooling_inner.compute_at(hw_output, x_glb);

            // Unroll input channels along glb; by default using all GLB tiles
            input_host.in().in().compute_at(hw_output, x_glb);
            input_host.in().in().unroll(c, unroll);

            input_host.in().compute_at(hw_output.in(), x_host);
            input_host.in().store_in(MemoryType::GLB);
            input_host.in().unroll(c, unroll);

            // Stream input into the accelerator
            input_host.compute_root().accelerator_input();

        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(MaxPooling, maxpooling)
