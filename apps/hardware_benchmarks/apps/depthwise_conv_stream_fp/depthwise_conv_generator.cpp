#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class DepthwiseConv : public Halide::Generator<DepthwiseConv> {
public:
    Input<Buffer<uint16_t>> input{"input", 3};
    Input<Buffer<uint16_t>> kernel{"kernel", 3};
    Output<Buffer<uint16_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 40};    // default: 114
  
    // ksize determines the output stencil size
    GeneratorParam<int> ksize{"ksize", 5};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 2};  // default: 1

    // n_ic determines the number of input channels
    GeneratorParam<int> n_ic{"n_ic", 24};    // default: 16

    GeneratorParam<int> unroll{"unroll", 6};    // default: 8


    void generate() {
      
        /* THE ALGORITHM */
        // Define algorithm variables
        Var x("x"), y("y"), c("c");
        Func input_host("input_host"), kernel_host("kernel_host");
        Func hw_output("hw_output");

        // Define hardware input and reduction domain
        input_host(c, x, y) = cast<bfloat16_t>(input(c, x, y));
        kernel_host(c, x, y) = cast<bfloat16_t>(kernel(c, x, y));
        RDom r(0, ksize, 0, ksize);

        // DepthwiseConv Expression
        Func depthwise_conv;
        depthwise_conv(c, x, y) = cast<bfloat16_t>(0);
        depthwise_conv(c, x, y) += cast<bfloat16_t>(kernel_host(c, r.x, r.y) * input_host(c, stride * x + r.x, stride * y + r.y));

        // Send the Output
        hw_output(c, x, y) = cast<bfloat16_t>(depthwise_conv(c, x, y));
        output(c, x, y) = cast<uint16_t>(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define scheduling variables
            int out_img = floor((int(in_img) - int(ksize)) / stride) + 1;
            int gbsize_x = out_img;
            int gbsize_y = out_img;
            int gbsize_c = n_ic;
            int tile_size_x = gbsize_x;
            int tile_size_y = gbsize_y;
            Var x_host, x_glb, x_cgra;
            Var y_host, y_glb, y_cgra;
            Var c_glb, c_cgra;
            Var c_accel, c_host;
    
            // Create output boundaries
            output.bound(x, 0, out_img);
            output.bound(y, 0, out_img);
            output.bound(c, 0, n_ic);
            // kernel_host.bound(c, 0, n_ic);

            // Tile the image at host level, by default the tile size is the whole output image
            // Reorder channel dimension to be the innermost to get the clockwork pass
            hw_output.compute_root();
            hw_output.tile(x, y, x_host, y_host, x_glb, y_glb, gbsize_x, gbsize_y)
                .split(c, c_host, c_glb, gbsize_c)
                .reorder(c_glb, x_glb, y_glb, c_host, x_host, y_host)
                .hw_accelerate(c_glb, c_host);
            hw_output.unroll(c_glb, unroll);

            // Fully unroll the reduction domain to create line buffer scheduling
            // Unroll channel dimension for hardware compute unrolling
            // .update() is for initialization
            depthwise_conv.compute_at(hw_output, c_host);
            depthwise_conv.unroll(c, unroll);
            depthwise_conv.update().unroll(r.x).unroll(r.y).unroll(c, unroll);

            // Unroll input channels along glb; by default using all GLB tile
            input_host.in().compute_at(hw_output, c_host);
            // input_host.in().store_in(MemoryType::GLB);
            input_host.in().unroll(c, unroll);
            input_host.compute_root().accelerator_input();
            
            // Unroll kernel channels along glb; by default using all GLB tiles
            kernel_host.in().compute_at(hw_output, c_host);
            kernel_host.in().unroll(c, unroll);
            kernel_host.compute_root().accelerator_input();

        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(DepthwiseConv, depthwise_conv)
