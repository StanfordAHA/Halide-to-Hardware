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
    GeneratorParam<int>  stride{"stride", 2};  // default: 2

    // n_ic determines the number of input channels
    GeneratorParam<int> n_ic{"n_ic", 112};    // default: 112

    GeneratorParam<int> unroll{"unroll", 14};    // default: 14


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
            int glb_tile_x = out_img / 2; // Tile image into 2x1 tiles at host level
            int glb_tile_y = out_img;
            int glb_tile_c = n_ic / 2; // Tile channels into 2 chunks at host level
            int cgra_tile_x = glb_tile_x;
            int cgra_tile_y = glb_tile_y;
            int cgra_tile_c = glb_tile_c;
            Var x_host, x_glb, x_cgra;
            Var y_host, y_glb, y_cgra;
            Var c_host, c_glb, c_cgra;
    
            // Create output boundaries
            output.bound(x, 0, out_img);
            output.bound(y, 0, out_img);
            output.bound(c, 0, n_ic);

            // Tile the image and channel at host level
            // Tile channel to make it multiples of MEM fetch width since input size is odd
            // when stride=2 and ksize=3 due to floor rounding
            hw_output.in().compute_root();
            hw_output.in()
                .tile(x, y, x_host, y_host, x_glb, y_glb, glb_tile_x, glb_tile_y)
                .split(c, c_host, c_glb, glb_tile_c)
                .reorder(c_glb, x_glb, y_glb, c_host, x_host, y_host)
                .hw_accelerate(c_glb, c_host);
            
            // Unroll output channels along glb; by default using all GLB tiles
            hw_output.in().unroll(c_glb, unroll);
            hw_output.in().store_in(MemoryType::GLB);

            // Tile the image at glb level, by default the cgra tile size is the glb tile size
            hw_output
                .tile(x, y, x_glb, y_glb, x_cgra, y_cgra, cgra_tile_x, cgra_tile_y)
                .split(c, c_glb, c_cgra, cgra_tile_c)
                .reorder(c_cgra, x_cgra, y_cgra, c_glb, x_glb, y_glb);
            hw_output.compute_at(hw_output.in(), c_host);
            hw_output.unroll(c_cgra, unroll);

            // Fully unroll the reduction domain to create line buffer schedules
            // Unroll channel dimension for hardware compute unrolling
            // .update() is for initialization
            max_pooling_inner.update().unroll(r.x, ksize).unroll(r.y, ksize);
            max_pooling_inner.update().unroll(c, unroll);
            max_pooling_inner.unroll(c, unroll);
            max_pooling_inner.compute_at(hw_output, c_glb);

            // Unroll input channels for glb->cgra; by default using all GLB tiles
            input_host.in().in().compute_at(hw_output, c_glb);
            input_host.in().in().unroll(c, unroll);

            // Unroll input channels for host->glb; by default using all GLB tiles
            input_host.in().compute_at(hw_output.in(), c_host);
            input_host.in().store_in(MemoryType::GLB);
            input_host.in().unroll(c, unroll);

            // Stream input into the accelerator
            input_host.compute_root().accelerator_input();

        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(MaxPooling, maxpooling)