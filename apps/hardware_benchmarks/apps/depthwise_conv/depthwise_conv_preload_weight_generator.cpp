#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

const int ksize = 3;

class DepthwiseConv : public Halide::Generator<DepthwiseConv> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    // Input<Buffer<uint8_t>> kernel{"kernel", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 116};    // default: 114
  
    // ksize determines the output stencil size
    // GeneratorParam<int> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // n_ic determines the number of input channels
    GeneratorParam<int> n_ic{"n_ic", 7};    // default: 16

    GeneratorParam<int> unroll{"unroll", 7};    // default: 8


    void generate() {
      
        /* THE ALGORITHM */
        // Define algorithm variables
        Var x("x"), y("y"), c("c");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        // Define hardware input and reduction domain
        input_host(c, x, y) = cast<uint16_t>(input(c, x, y));
        // input_glb(x, y, c) = cast<uint16_t>(input_host(x, y, c));
        // input_cgra(x, y, c) = cast<uint16_t>(input_glb(x, y, c));

        // kernel_host(x, y, c) = cast<uint16_t>(kernel(x, y, c));
        // kernel_glb(x, y, c) = cast<uint16_t>(kernel_host(x, y, c));
        // kernel_cgra(x, y, c) = cast<uint16_t>(kernel_glb(x, y, c));
        RDom r(0, ksize, 0, ksize);

        // create the gaussia nkernel
        Func kernel_f;
        float sigma = 1.5f;
        kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);
        Expr sum_kernel[ksize*ksize];
        for (int idx=0, i=0; i<ksize; ++i) {
          for (int j=0; j<ksize; ++j) {
            if (i==0 && j==0) {
              sum_kernel[idx] = kernel_f(i-ksize/2) * kernel_f(j-ksize/2);
            } else {
              sum_kernel[idx] = kernel_f(i-ksize/2) * kernel_f(j-ksize/2) + sum_kernel[idx-1];
            }
            idx++;
          }
        }
        Func kernel_preload("kernel_preload");
        kernel_preload(c, x, y) = cast<uint16_t>(kernel_f(x-ksize/2) * kernel_f(y-ksize/2) * 256.0f /
                                     sum_kernel[ksize*ksize-1]);

        // MaxPooling Expression
        // Func max_pooling_inner;
        // Func max_pooling;
        // max_pooling(c, x, y) = 
        //     maximum(r, input_host(c, stride * x + r.x, stride * y + r.y), max_pooling_inner);

        // DepthwiseConv Expression
        Func depthwise_conv;
        depthwise_conv(c, x, y) = cast<uint16_t>(0);
        // depthwise_conv(x, y, c) += cast<uint16_t>(kernel_host(r.x, r.y, c) * input_host(stride * x + r.x, stride * y + r.y, c));
        depthwise_conv(c, x, y) += cast<uint16_t>(kernel_preload(c, r.x, r.y) * input_host(c, stride * x + r.x, stride * y + r.y));
        // depthwise_conv(x, y, c) += cast<uint16_t>(input_host(stride * x + r.x, stride * y + r.y, c));

        // Send the Output
        // output_glb(x, y, c) = depthwise_conv(x, y, c);
        // hw_output(x, y, c) = output_glb(x, y, c);
        hw_output(c, x, y) = depthwise_conv(c, x, y);
        output(c, x, y) = cast<uint8_t>(hw_output(c, x, y));

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
            // hw_output.bound(c, 0, n_ic);
            // kernel_host.bound(c, 0, n_ic);
            // kernel_glb.bound(c, 0, n_ic);
            // input_host.bound(c, 0, n_ic);

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
            // input_host.in().store_in(MemoryType::GLB);
            input_host.in().unroll(c, unroll);
            
            input_host.in().in().compute_at(hw_output, c_glb); 
            input_host.in().in().unroll(c, unroll);


            // // Create three-level memory hierarchy for kernels
            // kernel_host.compute_root().accelerator_input(); // host buffer
            
            // kernel_host.in().compute_at(hw_output.in(), c_host); // global buffer
            // kernel_host.in().store_in(MemoryType::GLB);
            // kernel_host.in().unroll(c, unroll);
            
            // kernel_host.in().in().compute_at(hw_output, c_glb); 
            // kernel_host.in().in().unroll(c, unroll);
        }
    }
};
}  // namespace

HALIDE_REGISTER_GENERATOR(DepthwiseConv, depthwise_conv)
