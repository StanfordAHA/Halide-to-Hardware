#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class DepthwiseConv : public Halide::Generator<DepthwiseConv> {
public:
    Input<Buffer<uint16_t>> input{"input", 3};
    // Input<Buffer<uint16_t>> kernel{"kernel", 3};
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
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        // Define hardware input and reduction domain
        input_host(c, x, y) = cast<bfloat16_t>(input(c, x, y));

        RDom r(0, ksize, 0, ksize);

        // create preload kernel
        const int block_size = int(ksize);
        Func kernel_preload("kernel_preload");
        float total_range = 2.0f; // From -1 to 1
        int total_elements = block_size * block_size;
        float step = total_range / (total_elements - 1);
        Expr value = cast<bfloat16_t>(0.0f + step * (y * block_size + x));
        kernel_preload(c, x, y) = cast<bfloat16_t>(value);

        // DepthwiseConv Expression
        Func depthwise_conv;
        depthwise_conv(c, x, y) = cast<bfloat16_t>(0);
        depthwise_conv(c, x, y) += cast<bfloat16_t>(kernel_preload(c, r.x, r.y) * input_host(c, stride * x + r.x, stride * y + r.y));

        // Send the Output
        hw_output(c, x, y) = depthwise_conv(c, x, y);
        output(c, x, y) = cast<uint16_t>(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define scheduling variables
            int out_img = floor((int(in_img) - int(ksize)) / stride) + 1;
            int glb_tile_x = out_img;
            int glb_tile_y = out_img;
            int glb_tile_c = n_ic;
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

            // Tile the image at host level, by default the tile size is the whole output image
            // Reorder channel dimension to be the innermost to get the clockwork pass
            hw_output.in().compute_root(); // global buffer
            hw_output.in()
                .tile(x, y, x_host, y_host, x_glb, y_glb, glb_tile_x, glb_tile_y)
                .split(c, c_host, c_glb, glb_tile_c)
                .reorder(c_glb, x_glb, y_glb, c_host, x_host, y_host)
                .hw_accelerate(c_glb, c_host);
            hw_output.in().unroll(c_glb, unroll);
            hw_output.in().store_in(MemoryType::GLB);

            // Tile the image at glb level, by default the cgra tile size is the glb tile size
            hw_output
                .tile(x, y, x_glb, y_glb, x_cgra, y_cgra, cgra_tile_x, cgra_tile_y)
                .split(c, c_glb, c_cgra, cgra_tile_c)
                .reorder(c_cgra, x_cgra, y_cgra, c_glb, x_glb, y_glb);
            hw_output.compute_at(hw_output.in(), c_host);
            hw_output.unroll(c_cgra, unroll);

            depthwise_conv.update().unroll(r.x, ksize).unroll(r.y, ksize);
            depthwise_conv.update().unroll(c, unroll);
            depthwise_conv.unroll(c, unroll);
            depthwise_conv.compute_at(hw_output, c_glb); // mem tile

            // Create three-level memory hierarchy for inputs
            input_host.compute_root().accelerator_input(); // host buffer
            input_host.in().compute_at(hw_output.in(), c_host); // global buffer
            input_host.in().unroll(c, unroll);

            input_host.in().in().compute_at(hw_output, c_glb); 
            input_host.in().in().unroll(c, unroll);
        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(DepthwiseConv, depthwise_conv)
