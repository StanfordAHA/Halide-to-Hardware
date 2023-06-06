#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ResnetLayer : public Halide::Generator<ResnetLayer> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  kernel{"kernel", 4};
    Output<Buffer<int16_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 56};    // default: 56

    // pad determines the padding to the input image size
    GeneratorParam<int> pad{"pad", 1};    // default: 1
  
    // ksize determines the output stencil size
    GeneratorParam<uint8_t> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // k_ic determines the number of input channels
    GeneratorParam<int> k_ic{"k_ic", 16};    // default: 16
  
    // k_oc determines the number of output channels
    GeneratorParam<int> k_oc{"k_oc", 8};    // default: 8

    // m_ic determines the multiples of input channels in memory (m_ic * k_ic total)
    GeneratorParam<int> m_ic{"m_ic", 1};    // default: 1
  
    // m_oc determines the multiples of output channels in memory (m_oc * k_oc total)
    GeneratorParam<int> m_oc{"m_oc", 1};    // default: 1

    // n_ic determines the total number of input channels
    GeneratorParam<int> n_ic{"n_ic", 32};    // default: 32
  
    // n_oc determines the total number of output channels
    GeneratorParam<int> n_oc{"n_oc", 32};    // default: 32

    // glb_i determines the input glb unrolling
    GeneratorParam<int> glb_i{"glb_i", 1};    // default: 32

    // glb_k determines the kernel glb unrolling
    GeneratorParam<int> glb_k{"glb_k", 1};    // default: 32

    // glb_o determines the output glb unrolling
    GeneratorParam<int> glb_o{"glb_o", 1};    // default: 32
  
    // schedule to be used
    GeneratorParam<int> schedule{"schedule", 0};    // default: 0

    void generate() {
        assert((int)n_ic >= (int)k_ic * (int)m_ic); // the number of total input channels must exceed unrolled channels
        assert((int)n_oc >= (int)k_oc * (int)m_oc); // the number of total output channels must exceed unrolled channels
      
        //int imgsize = (in_img + 0 - ksize + 1) / stride;
        int imgsize = floor( (in_img + 2*pad - ksize) / stride ) + 1;
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w"), zz("zz");

        Expr height = imgsize;
        Expr width = imgsize;
        int ic_outer = (int)n_ic / (int)k_ic;
        int oc_outer = (int)n_oc / (int)k_oc;

        Func conv("conv");
        RDom r(0, ksize,
               0, ksize,
               0, n_ic);

        conv(w, x, y) = cast<uint16_t>(0);

        Func hw_input("hw_input"), hw_kernel("hw_kernel");
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");        
        Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");

        if (schedule == 11) {
          hw_input(z, x, y) = i16(input(z, clamp(x-pad, 0, in_img - 1), clamp(y-pad, 0, in_img - 1)));
          hw_kernel(z, w, x, y) = i16(kernel(z, w, x, y));
          hw_output(w, x, y) = i16(0);
          hw_output(w, x, y) +=
            hw_kernel(r.z, w, r.x, r.y) *
            hw_input(r.z, stride*x + r.x, stride*y + r.y);

        } else {
          //Func hw_input("hw_input"), hw_kernel("hw_kernel");
          hw_input(z, x, y) = i16(input(z, clamp(x-pad, 0, in_img - 1), clamp(y-pad, 0, in_img - 1)));
          hw_kernel(w, z, x, y) = i16(kernel(w, z, x, y));
        
          //Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
          input_host(z, x, y) = hw_input(z, x, y);
          input_glb(z, x, y) = input_host(z, x, y);
          input_cgra(z, x, y) = input_glb(z, x, y);
        
          //Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra");
          kernel_host(w, z, x, y) = hw_kernel(w, z, x, y);
          kernel_glb(w, z, x, y) = kernel_host(w, z, x, y);
          kernel_cgra(w, z, x, y) = kernel_glb(w, z, x, y);
        
          //Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
          output_cgra(w, x, y) +=
            kernel_cgra(w, r.z, r.x, r.y) *
            input_cgra(r.z, stride*x + r.x, stride*y + r.y);
        
          output_glb(w, x, y) = output_cgra(w, x, y);
          hw_output(w, x, y) = output_glb(w, x, y);
        }

        output(w, x, y) = max(0, i16(hw_output(w, x, y)));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) && schedule == 0) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          hw_kernel.bound(z, 0, n_ic);
          kernel_glb.bound(z, 0, n_ic);
          input_host.bound(z, 0, n_ic);

          int mem_oc = (int)k_oc * (int)m_oc;
          int mem_ic = (int)k_ic * (int)m_ic;
          
          output_cgra.bound_extent(w, mem_oc);
          kernel_cgra.bound_extent(w, mem_oc);
          input_cgra.bound_extent(z, mem_ic);
          kernel_cgra.bound_extent(z, mem_ic);
          
          kernel_glb.bound(w, 0, n_oc);
          input_glb.bound(z, 0, n_ic);
          input_cgra.bound_extent(z, k_ic);
          kernel_cgra.bound_extent(z, k_ic);
          kernel_cgra.bound_extent(w, k_oc);
          
          int gbsize = imgsize;
          int tilesize = ((int)stride == 2) ?
            std::min(14, imgsize) : // we want the input to be 30 max
            std::min(28, imgsize);  // min of 28 and output image size
          tilesize = (pad == 3) ?
            16 : // this occurs only for conv1
            tilesize;
          int tilesize_y = (pad == 3) ?
            8 : // this occurs only for conv1. We need it smaller to fit
            tilesize;


          Var x_host,y_host, x_glb,y_glb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_cgra, w_glb;
          Var z_cgra, z_glb;
          RVar rz_unroll("rz_unroll"), rz_cgra("rz_cgra"), rz_glb("rz_glb");
          RVar rz_split("rz_split");

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          //hw_output.unroll(w, k_oc);
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
            .reorder(w,xi,yi, x_host,y_host)
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

          //Func interm_output_cgra = output_cgra.update().rfactor(r.z, z);
          //interm_output_cgra.compute_at(output_glb, x_glb);

          Func output_rf;
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
          //input_host.stream_to_accelerator();
          input_glb.compute_at(hw_output, x_host); // global buffer
          input_cgra.compute_at(output_cgra, rz_glb);   // mem tile

          // kernel buffers
          hw_kernel.compute_root();
          kernel_host.compute_root(); // host buffer
          kernel_host.accelerator_input();
          kernel_glb.compute_at(hw_output, x_host); // global buffer
          kernel_cgra.compute_at(output_cgra, rz_glb);   // mem tile

          //input_glb.unroll(z, k_ic);
          //input_cgra.unroll(z, k_ic);
          input_cgra
            .split(z, z_glb, z_cgra, mem_ic)
            .reorder(z_cgra, x, y, z_glb);
          //.reorder(zz, x, y, z);
          kernel_cgra
            .split(z, z_glb, z_cgra, mem_ic)
            .split(w, w_glb, w_cgra, mem_oc)
            .reorder(w_cgra, z_cgra, x, y, w_glb, z_glb);
            //.reorder(zz, w_cgra, x, y, z, w_glb);

          // Unroll input and kernel over glb (default 1)
          kernel_glb.unroll(w, glb_k); // unroll glb input for small images
          kernel_cgra.unroll(w_cgra, glb_k); // unroll glb->cgra channels for small images
          input_glb.unroll(z, glb_i); // unroll glb input for small images
          input_cgra.unroll(z_cgra, glb_i); // unroll glb->cgra channels for small images

        } else if (get_target().has_feature(Target::Clockwork) && schedule == 11) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          hw_kernel.bound(z, 0, n_ic);

          int gbsize = imgsize;
          int tilesize = ((int)stride == 2) ?
            std::min(14, imgsize) : // we want the input to be 30 max
            std::min(28, imgsize);  // min of 28 and output image size
          int tilesize_y = (pad == 3) ?
            16 : // this occurs only for conv1. We need it smaller to fit
            tilesize;


          // Create three level memory hierarchy with iteration_order (and set rate)
          auto level1 = IterLevel("CGRA", {{r.z, k_ic}, {w, k_oc}, {x, tilesize}, {y, tilesize_y}, {r.x, ksize}, {r.y, ksize}, {r.z, 2}});
          auto level2 = IterLevel("GLB",  {{w, 4}, {x, 2}, {y, 2}});
          auto level3 = IterLevel("host", {{x, 1}, {y, 1}});
          hw_output
            .iteration_order({level1, level2, level3});

          // Specify compute variables for memory hierarchy
          auto compute_cgra = LoopLevel(hw_output, RVar("r$z0"));
          auto compute_glb = hw_output.get_looplevel("GLB", Var("x0"));
          auto compute_host = LoopLevel::root();

          // Create hardware accelerator
          hw_output.get_memory_level("GLB")
            .hw_accelerate(Var("x1"), Var("x0"));

          // Stream input to accelerator (and set rate)
          hw_input
            .stream_to_accelerator({"CGRA", "GLB", "host"},
                                   {compute_cgra, compute_glb, compute_host});
          hw_kernel
            .stream_to_accelerator({"CGRA", "GLB", "host"},
                                   {compute_cgra, compute_glb, compute_host});





          
          //Var x_host,y_host, x_glb,y_glb, x_cgra,y_cgra;
          //Var xi,yi;
          //Var w_cgra, w_glb;
          //Var z_cgra, z_glb;
          //RVar rz_cgra, rz_glb;
          //
          //// Produce loop levels: host, global buffer, cgra
          //hw_output.compute_root();
          //hw_output
          //  .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
          //  .reorder(w,xi,yi, x_host,y_host)
          //  .hw_accelerate(xi, x_host);
          //
          //output_glb.compute_at(hw_output, x_host); // global buffer
          //output_glb
          //  .tile(x, y, x_glb,y_glb, x_cgra,y_cgra, tilesize,tilesize_y)
          //  .split(w, w_glb, w_cgra, k_oc)
          //  // reorder from inner to outermost
          //  .reorder(w_cgra, x_cgra, y_cgra,
          //           w_glb, x_glb, y_glb);
          //
          //output_cgra.compute_at(output_glb, w_glb); // memtile
          //output_cgra
          //  .reorder(w, x, y);
          //
          //output_cgra.update()
          //  .split(r.z, rz_glb, rz_cgra, k_ic)
          //  .reorder(rz_cgra, w, x, y, r.x, r.y, rz_glb);
          //
          ////Func interm_output_cgra = output_cgra.update().rfactor(r.z, z);
          ////interm_output_cgra.compute_at(output_glb, x_glb);
          //
          //Func output_rf;
          //output_cgra
          //  .unroll(w, k_oc);
          //output_cgra.update()
          //  //.unroll(w_cgra, k_oc)
          //  .unroll(w, k_oc)
          //  .unroll(rz_cgra, k_ic); // this is the z reduction
          //
          //// Three buffers: one at host,
          ////                a copy stage as the global buffer,
          ////                another copy stage as the memory tiles
          ////hw_input.compute_root();
          //input_host.compute_root(); // host buffer
          //input_host.accelerator_input();
          ////input_host.stream_to_accelerator();
          //input_glb.compute_at(hw_output, x_host); // global buffer
          //input_cgra.compute_at(output_cgra, rz_glb);   // mem tile
          //
          //// kernel buffers
          //hw_kernel.compute_root();
          //kernel_host.compute_root(); // host buffer
          //kernel_host.accelerator_input();
          //kernel_glb.compute_at(hw_output, x_host); // global buffer
          //kernel_cgra.compute_at(output_cgra, rz_glb);   // mem tile
          //
          //if (imgsize == 7) {
          //  kernel_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
          //  input_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
          //}
          //
          //
          ////input_glb.unroll(z, k_ic);
          ////input_cgra.unroll(z, k_ic);
          //input_cgra
          //  .split(z, z_glb, z_cgra, k_ic)
          //  .reorder(z_cgra, x, y, z_glb);
          ////.reorder(zz, x, y, z);
          //kernel_cgra
          //  .split(z, z_glb, z_cgra, k_ic)
          //  .split(w, w_glb, w_cgra, k_oc)
          //  .reorder(z_cgra, w_cgra, x, y, z_glb, w_glb);
          //  //.reorder(zz, w_cgra, x, y, z, w_glb);



          

        } else if (get_target().has_feature(Target::Clockwork) && schedule == 1) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          hw_kernel.bound(z, 0, n_ic);
          kernel_glb.bound(z, 0, n_ic);
          input_host.bound(z, 0, n_ic);

          //output_cgra.bound(w, 0, k_oc*8);
          output_cgra.bound_extent(w, k_oc*8);
          kernel_glb.bound(w, 0, n_oc);
          //output_glb.bound(w, 0, n_oc);


          //kernel_cgra.bound_extent(z, ic_outer);
          //output_cgra.bound_extent(w, oc_outer);
          //std::cout << "set extent to " << (int)k_ic << " and " << (int)k_oc << std::endl;

          int gbsize = imgsize;
          int tilesize = ((int)stride == 2) ?
            std::min(14, imgsize) : // we want the input to be 30 max
            std::min(28, imgsize);  // min of 28 and output image size
          int tilesize_y = (pad == 3) ?
            16 : // this occurs only for conv1. We need it smaller to fit
            tilesize;

          Var x_host,y_host, x_glb,y_glb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_iter("w_iter"), w_unroll("w_unroll"), w_cgra, w_glb("w_glb");
          Var z_cgra, z_glb;
          RVar rz_cgra, rz_glb;

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          //hw_output.unroll(w, k_oc);
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
            .reorder(w,xi,yi, x_host,y_host)
            .hw_accelerate(xi, x_host);

          output_glb.compute_at(hw_output, x_host); // global buffer
          output_glb
            .tile(x, y, x_glb,y_glb, x_cgra,y_cgra, tilesize,tilesize_y)
            .split(w, w_glb, w_cgra, k_oc * 8)
            // reorder from inner to outermost
            .reorder(w_cgra, x_cgra, y_cgra,
                     w_glb, x_glb, y_glb);

          if (imgsize == 7) {
            //output_glb.unroll(w, 2); // unroll cgra->glb channels for small images
          }

          output_cgra.compute_at(output_glb, w_glb); // memtile
          output_cgra
            .split(w, w_iter, w_unroll, k_oc)
            .reorder(w_unroll, w_iter, x, y);

          output_cgra.update()
            .split(r.z, rz_glb, rz_cgra, k_ic)
            .split(w, w_iter, w_unroll, k_oc)
            .reorder(rz_cgra, w_unroll, w_iter, x, y, r.x, r.y, rz_glb);

          //Func interm_output_cgra = output_cgra.update().rfactor(r.z, z);
          //interm_output_cgra.compute_at(output_glb, x_glb);

          Func output_rf;
          output_cgra
            .unroll(w_unroll, k_oc);
          output_cgra.update()
            //.unroll(w_cgra, k_oc)
            .unroll(w_unroll, k_oc)
            .unroll(rz_cgra, k_ic); // this is the z reduction

          // Three buffers: one at host,
          //                a copy stage as the global buffer,
          //                another copy stage as the memory tiles
          //hw_input.compute_root();
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          //input_host.stream_to_accelerator();
          input_glb.compute_at(hw_output, x_host); // global buffer
          input_cgra.compute_at(output_cgra, rz_glb);   // mem tile
          //input_cgra.compute_at(output_cgra, w_iter);   // mem tile

          // kernel buffers
          hw_kernel.compute_root();
          kernel_host.compute_root(); // host buffer
          kernel_host.accelerator_input();
          kernel_glb.compute_at(hw_output, x_host); // global buffer
          kernel_cgra.compute_at(output_cgra, rz_glb);   // mem tile
          //kernel_cgra.compute_at(output_cgra, w_iter);   // mem tile

          if (imgsize == 7) {
            kernel_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
            input_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
          }


          //input_glb.unroll(z, k_ic);
          //input_cgra.unroll(z, k_ic);
          input_cgra
            .split(z, z_glb, z_cgra, k_ic)
            .reorder(z_cgra, x, y, z_glb);
          //.reorder(zz, x, y, z);
          kernel_cgra
            .split(z, z_glb, z_cgra, k_ic)
            .split(w, w_glb, w_cgra, k_oc * 8)
            .reorder(z_cgra, w_cgra, x, y, z_glb, w_glb);
            //.reorder(zz, w_cgra, x, y, z, w_glb);
          
        } else {  // schedule to CPU
          output_cgra.compute_root();
          output_cgra.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, resnet_output_stationary)
