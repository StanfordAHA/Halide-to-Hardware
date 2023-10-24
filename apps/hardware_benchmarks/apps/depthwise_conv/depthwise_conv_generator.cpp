#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class DepthwiseConv : public Halide::Generator<DepthwiseConv> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    Input<Buffer<uint8_t>> kernel{"kernel", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 9};    // default: 114
  
    // ksize determines the output stencil size
    GeneratorParam<int> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // n_ic determines the number of input channels
    GeneratorParam<int> n_ic{"n_ic", 1};    // default: 16

    GeneratorParam<int> unroll{"unroll", 1};    // default: 8


    void generate() {
      
        /* THE ALGORITHM */
        // Define algorithm variables
        Var x("x"), y("y"), c("c");
        Func hw_input("hw_input"), hw_kernel("hw_kernel");
        Func input_host("input_host"), kernel_host("kernel_host");
        Func hw_output("hw_output");

        // Define hardware input and reduction domain
        hw_input(c, x, y) = cast<uint16_t>(input(c, x, y));
        hw_kernel(c, x, y) = cast<uint16_t>(kernel(c, x, y));
        input_host(c, x, y) = hw_input(c, x, y);
        kernel_host(c, x, y) = hw_kernel(c, x, y);
        RDom r(0, ksize, 0, ksize);
       
        // MaxPooling Expression
        // Func max_pooling_inner;
        // Func max_pooling;
        // max_pooling(c, x, y) = 
        //     maximum(r, input_host(c, stride * x + r.x, stride * y + r.y), max_pooling_inner);

        // DepthwiseConv Expression
        Func depthwise_conv;
        depthwise_conv(c, x, y) = cast<uint16_t>(0);
        depthwise_conv(c, x, y) += kernel_host(c, r.x, r.y) * input_host(c, stride * x + r.x, stride * y + r.y);
        // depthwise_conv(c, x, y) += input_host(c, stride * x + r.x, stride * y + r.y);

        // Send the Output
        hw_output(c, x, y) = cast<uint16_t>(depthwise_conv(c, x, y));
        output(c, x, y) = cast<uint8_t>(hw_output(c, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

            // Define scheduling variables
            int out_img = floor((int(in_img) - int(ksize)) / stride) + 1;
            int tile_size = out_img;
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
                .tile(x, y, x_host, y_host, x_glb, y_glb, out_img, out_img)
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
            depthwise_conv.update().unroll(r.x, ksize).unroll(r.y, ksize);
            depthwise_conv.update().unroll(c, unroll);
            depthwise_conv.unroll(c, unroll);
            depthwise_conv.compute_at(hw_output, x_glb);

            // Unroll input channels along glb; by default using all GLB tiles
            input_host.in().in().compute_at(hw_output, x_glb);
            input_host.in().in().unroll(c, unroll);

            input_host.in().compute_at(hw_output.in(), x_host);
            input_host.in().store_in(MemoryType::GLB);
            input_host.in().unroll(c, unroll);

            // Stream input into the accelerator
            // hw_input.compute_root();
            input_host.compute_root().accelerator_input();
            
            // Unroll kernel channels along glb; by default using all GLB tiles
            kernel_host.in().in().compute_at(hw_output, x_glb);
            kernel_host.in().in().unroll(c, unroll);

            kernel_host.in().compute_at(hw_output.in(), x_host);
            kernel_host.in().store_in(MemoryType::GLB);
            kernel_host.in().unroll(c, unroll);

            // Stream kernel into the accelerator
            hw_kernel.compute_root();
            kernel_host.compute_root().accelerator_input();

        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(DepthwiseConv, depthwise_conv)
