#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ResnetLayer : public Halide::Generator<ResnetLayer> {
public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    Input<Buffer<uint16_t>>  kernel{"kernel", 4};
    Input<Buffer<uint16_t>>  bias{"bias", 1};
    Output<Buffer<uint16_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 161};    // default: 161
  
    // ksize determines the output stencil size
    GeneratorParam<int> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 2};  // default: 2

    // k_ic determines the number of input channels
    GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
  
    // k_oc determines the number of output channels
    GeneratorParam<int> k_oc{"k_oc", 32};    // default: 32

    // m_ic determines the multiples of input channels in memory (m_ic * k_ic total)
    GeneratorParam<int> m_ic{"m_ic", 1};    // default: 1
  
    // m_oc determines the multiples of output channels in memory (m_oc * k_oc total)
    GeneratorParam<int> m_oc{"m_oc", 1};    // default: 1

    // n_ic determines the total number of input channels
    GeneratorParam<int> n_ic{"n_ic", 4};    // default: 4
  
    // n_oc determines the total number of output channels
    GeneratorParam<int> n_oc{"n_oc", 32};    // default: 32

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{"glb_i", 4};    // default: 4

    // glb_k determines the kernel glb unrolling
    GeneratorParam<int> glb_k{"glb_k", 4};    // default: 4

    // glb_b determines the bias glb unrolling
    GeneratorParam<int> glb_b{"glb_b", 1};    // default: 1

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{"glb_o", 8};    // default: 4; Opal uses 8 due to half glb size
  
    // schedule to be used
    GeneratorParam<int> schedule{"schedule", 0};    // default: 0

    void generate() {
        assert((int)n_ic >= (int)k_ic * (int)m_ic); // the number of total input channels must exceed unrolled channels
        assert((int)n_oc >= (int)k_oc * (int)m_oc); // the number of total output channels must exceed unrolled channels
      
        //int imgsize = (in_img + 0 - ksize + 1) / stride;
        int imgsize = floor( (int(in_img) - int(ksize)) / stride ) + 1;
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w"), zz("zz");

        Expr height = imgsize;
        Expr width = imgsize;
        
        RDom r(0, ksize,
               0, ksize,
               0, n_ic);

        Func hw_input("hw_input"), hw_kernel("hw_kernel"), hw_bias("hw_bias");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");        
        Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
        Func bias_host("bias_host"), bias_glb("bias_glb"), bias_cgra("bias_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        //Func hw_input("hw_input"), hw_kernel("hw_kernel");
        hw_input(z, x, y) = cast<bfloat16_t>(input(z, x, y));
        hw_kernel(w, z, x, y) = cast<bfloat16_t>(kernel(w, z, x, y));
        hw_bias(w) = cast<bfloat16_t>(bias(w));

        //Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        input_host(z, x, y) = hw_input(z, x, y);
        input_glb(z, x, y) = input_host(z, x, y);
        input_cgra(z, x, y) = input_glb(z, x, y);
      
        //Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
        kernel_host(w, z, x, y) = hw_kernel(w, z, x, y);
        kernel_glb(w, z, x, y) = kernel_host(w, z, x, y);
        kernel_cgra(w, z, x, y) = kernel_glb(w, z, x, y);

        //Func bias_host("bias_host"), bias_glb("bias_glb"), bias_cgra("bias_cgra");
        bias_host(w) = hw_bias(w);
        bias_glb(w) = bias_host(w);
        bias_cgra(w, x, y) = bias_glb(w);
      
        //Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
        output_cgra(w, x, y) = cast<bfloat16_t>(bias_cgra(w, x, y));
        output_cgra(w, x, y) +=
          cast<bfloat16_t>(kernel_cgra(w, r.z, r.x, r.y)) *
          cast<bfloat16_t>(input_cgra(r.z, stride*x + r.x, stride*y + r.y));
      
        output_glb(w, x, y) = min(max(cast<bfloat16_t>(output_cgra(w, x, y)), bfloat16_t(0.0f)), bfloat16_t(6.0f));

        // hw_output(w, x, y) = max(cast<bfloat16_t>(output_glb(w, x, y)), Expr(bfloat16_t(0.0f)));
        // hw_output(w, x, y) = clamp(cast<bfloat16_t>(output_glb(w, x, y)), Expr(bfloat16_t(0.0f)), Expr(bfloat16_t(6.0f)));
        hw_output(w, x, y) = cast<bfloat16_t>(output_glb(w, x, y));

        output(w, x, y) = cast<uint16_t>(hw_output(w, x, y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) && schedule == 0) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          int mem_oc = (int)k_oc * (int)m_oc;
          int mem_ic = (int)k_ic * (int)m_ic;

          input_host.bound(z, 0, n_ic);
          input_glb.bound(z, 0, n_ic);
          input_cgra.bound_extent(z, mem_ic);
          input_cgra.bound_extent(z, k_ic);

          hw_kernel.bound(z, 0, n_ic);
          kernel_glb.bound(z, 0, n_ic);
          kernel_glb.bound(w, 0, n_oc);
          kernel_cgra.bound_extent(w, mem_oc);
          kernel_cgra.bound_extent(z, mem_ic);
          kernel_cgra.bound_extent(z, k_ic);
          kernel_cgra.bound_extent(w, k_oc);

          bias_host.bound(w, 0, n_oc);
          bias_glb.bound(w, 0, n_oc);
          bias_cgra.bound_extent(w, mem_oc);
          bias_cgra.bound_extent(w, k_oc);

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);
          output_cgra.bound_extent(w, mem_oc);
          
          int gbsize = imgsize;
          // we want the input to be 30 max
          int maxTileSize = (stride == 2) ? 14 : 28; 
          int tilesize = maxTileSize;
          for (int i = 1; i <= maxTileSize; ++i) {
              if (imgsize % i == 0) {
                  tilesize = i;
              }
          }
          int tilesize_y = tilesize;

          Var x_host, y_host, x_glb, y_glb, x_cgra, y_cgra;
          Var xi, yi;
          Var w_cgra, w_glb;
          Var z_cgra, z_glb;
          RVar rz_unroll("rz_unroll"), rz_cgra("rz_cgra"), rz_glb("rz_glb");
          RVar rz_split("rz_split");

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          hw_output
            .tile(x, y, x_host, y_host, xi, yi, gbsize, gbsize)
            .reorder(w, xi, yi, x_host, y_host)
            .hw_accelerate(xi, x_host);

          output_glb.compute_at(hw_output, x_host); // global buffer
          output_glb
            .tile(x, y, x_glb,y_glb, x_cgra,y_cgra, tilesize,tilesize_y)
            .split(w, w_glb, w_cgra, mem_oc)
            // reorder from inner to outermost
            .reorder(w_cgra, x_cgra, y_cgra,
                     w_glb, x_glb, y_glb);

          // Unroll output over glb (default 1)
          hw_output.unroll(w, glb_o);
          output_glb.unroll(w_cgra, glb_o); // unroll cgra->glb channels for small images

          output_cgra.compute_at(output_glb, w_glb); // memtile
          output_cgra
            .reorder(w, x, y);

          output_cgra.update()
            .split(r.z, rz_glb, rz_cgra, mem_ic)
            .split(rz_cgra, rz_cgra, rz_unroll, k_ic)
            .reorder(rz_unroll, w, rz_cgra, x, y, r.x, r.y, rz_glb);

          output_cgra
            .unroll(w, k_oc);
          output_cgra.update()
            //.unroll(w_cgra, k_oc)
            .unroll(w, k_oc)
            .unroll(rz_unroll, k_ic); // this is the z reduction
          
          // Three buffers: one at host,
          //                a copy stage as the global buffer,
          //                another copy stage as the memory tiles

          //hw_input.compute_root();
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          input_glb.compute_at(hw_output, x_host); // global buffer
          input_cgra.compute_at(output_cgra, rz_glb);   // mem tile

          // kernel buffers
          hw_kernel.compute_root();
          kernel_host.compute_root(); // host buffer
          kernel_host.accelerator_input();
          kernel_glb.compute_at(hw_output, x_host); // global buffer
          kernel_cgra.compute_at(output_cgra, rz_glb);   // mem tile

          // bias buffer
          // hw_bias.compute_root();
          bias_host.compute_root(); // host buffer
          bias_host.accelerator_input();
          bias_glb.compute_at(hw_output, x_host); // global buffer
          bias_cgra.compute_at(output_glb, w_glb);   // mem tile

          input_cgra
            .split(z, z_glb, z_cgra, mem_ic)
            .reorder(z_cgra, x, y, z_glb);

          kernel_cgra
            .split(z, z_glb, z_cgra, mem_ic)
            .split(w, w_glb, w_cgra, mem_oc)
            .reorder(w_cgra, z_cgra, x, y, w_glb, z_glb);
          
          bias_cgra
            .split(w, w_glb, w_cgra, mem_oc)
            .reorder(w_cgra, x, y, w_glb);

          // Unroll input and kernel over glb (default 1)
          kernel_glb.unroll(w, glb_k); // unroll glb input for small images
          kernel_cgra.unroll(w_cgra, glb_k); // unroll glb->cgra channels for small images
          input_glb.unroll(z, glb_i); // unroll glb input for small images
          input_cgra.unroll(z_cgra, glb_i); // unroll glb->cgra channels for small images
          bias_glb.unroll(w, glb_b); // unroll glb bias for small images
          bias_cgra.unroll(w_cgra, glb_b); // unroll glb->cgra bias for small images

        } else {  // schedule to CPU
          output_cgra.compute_root();
          output_cgra.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, conv2D_fp)