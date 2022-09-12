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
    GeneratorParam<int> in_img{"in_img", 28};    // default: 56

    // pad determines the padding to the input image size
    GeneratorParam<int> pad{"pad", 1};    // default: 1
  
    // ksize determines the output stencil size
    GeneratorParam<uint8_t> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // k_ic determines the number of input channels that are unrolled
    GeneratorParam<int> k_ic{"k_ic", 1};    // default: 8
  
    // k_oc determines the number of output channels that are unrolled
    GeneratorParam<int> k_oc{"k_oc", 1};    // default: 8

    // n_ic determines the total number of input channels
    GeneratorParam<int> n_ic{"n_ic", 1};    // default: 64
  
    // n_oc determines the total number of output channels
    GeneratorParam<int> n_oc{"n_oc", 1};    // default: 64
  

    void generate() {
        //int imgsize = (in_img + 0 - ksize + 1) / stride;
        int imgsize = floor( (in_img + 2*pad - ksize) / stride ) + 1;
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w"), zz("zz");

        Expr height = imgsize;
        Expr width = imgsize;
        int ic_outer = (int)n_ic / (int)k_ic;

        Func conv("conv");
        RDom r(0, ksize,
               0, ksize,
               0, n_ic);

        conv(w, x, y) = cast<uint16_t>(0);

        Func hw_input("hw_input"), hw_kernel("hw_kernel");
        hw_input(z, x, y) = i16(input(z, clamp(x-pad, 0, in_img - 1), clamp(y-pad, 0, in_img - 1)));
        hw_kernel(z, w, x, y) = i16(kernel(z, w, x, y));
        
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra"), input_pond("input_pond");
        input_host(z, x, y) = hw_input(z, x, y);
        //input_glb(z, x, y) = input_host(z, x, y);
        //input_cgra(z, x, y) = input_glb(z, x, y);
        input_cgra(z, x, y) = input_host(z, x, y);
        input_pond(z, x, y) = input_cgra(z, x, y);

        Func kernel_host("kernel_host"), kernel_glb("kernel_glb"), kernel_cgra("kernel_cgra"), kernel_pond("kernel_pond");
        kernel_host(z, w, x, y) = hw_kernel(z, w, x, y);
        //kernel_glb(z, w, x, y) = kernel_host(z, w, x, y);
        //kernel_cgra(z, w, x, y) = kernel_glb(z, w, x, y);
        kernel_cgra(z, w, x, y) = kernel_host(z, w, x, y);
        kernel_pond(z, w, x, y) = kernel_cgra(z, w, x, y);

        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
        output_cgra(w, x, y) +=
          kernel_pond(r.z, w, r.x, r.y) *
          input_pond(r.z, stride*x + r.x, stride*y + r.y);

        //output_glb(w, x, y) = output_cgra(w, x, y);
        //hw_output(w, x, y) = output_glb(w, x, y);
        hw_output(w, x, y) = output_cgra(w, x, y);
        output(w, x, y) = max(0, i16(hw_output(w, x, y)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          hw_kernel.bound(z, 0, n_ic);
          kernel_host.bound(z, 0, n_ic);
          input_host.bound(z, 0, n_ic);
          //input_cgra.bound(z, 0, ic_outer);
          //input_cgra.bound(zz, 0, k_ic);
          //kernel_cgra.bound(z, 0, ic_outer);
          //kernel_cgra.bound(zz, 0, k_ic);

          int gbsize = imgsize;
          int tilesize = (stride == 2) ?
            std::min(14, imgsize) : // we want the input to be 30 max
            std::min(28, imgsize);  // min of 28 and output image size
          int tilesize_y = (pad == 3) ?
            16 : // this occurs only for conv1. We need it smaller to fit
            tilesize;

          Var x_host,y_host, x_glb,y_glb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_cgra, w_glb;
          Var z_cgra, z_glb;
          RVar rz_cgra, rz_glb, rz_host;

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          //hw_output.unroll(w, k_oc);
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
            .reorder(w,xi,yi, x_host,y_host)
            .hw_accelerate(xi, x_host);

          //output_glb.compute_at(hw_output, x_host); // global buffer
          //output_glb
          //  .tile(x, y, x_glb,y_glb, x_cgra,y_cgra, tilesize,tilesize_y)
          //  .split(w, w_glb, w_cgra, k_oc)
          //  // reorder from inner to outermost
          //  .reorder(w_cgra, x_cgra, y_cgra,
          //           w_glb, x_glb, y_glb);

          if (imgsize == 7) {
            //output_glb.unroll(w, 2); // unroll cgra->glb channels for small images
          }

          //output_cgra.compute_at(output_glb, w_glb); // memtile
          output_cgra.compute_at(hw_output, x_host); // memtile
          output_cgra
            .reorder(w, x, y);

          //output_cgra.update().compute_with(output_cgra, w_glb);
          output_cgra.update()
            .split(r.z, rz_host, rz_cgra, k_ic)
            //.reorder(rz_cgra, w, x, y, r.x, r.y, rz_host);
            .reorder(rz_cgra, w, r.x, r.y, x, y, rz_host);

          //Func interm_output_cgra = output_cgra.update().rfactor(r.z, z);
          //interm_output_cgra.compute_at(output_glb, x_glb);

          Func output_rf;
          output_cgra
            .unroll(w, k_oc);
          output_cgra.update()
            //.unroll(w_cgra, k_oc)
            .unroll(w, k_oc)
            .unroll(rz_cgra, k_ic); // this is the z reduction

          //interm_output_cgra.update()
          //  //.split(w, w_glb, w_cgra, k_oc)
          //  //.reorder(r.w, w_cgra, x, y, r.x, r.y, w_glb, r.z);
          //  .unroll(w_cgra, k_oc)
          //  .unroll(r.w, k_ic); // this is the z reduction

          
          //output_cgra.unroll(w_cgra, k_oc);

          // Three buffers: one at host,
          //                a copy stage as the global buffer,
          //                another copy stage as the memory tiles
          //hw_input.compute_root();
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          //input_host.stream_to_accelerator();
          //input_glb.compute_at(hw_output, x_host); // global buffer
          input_cgra.compute_at(hw_output, x_host);   // mem tile
          input_pond.store_at(output_cgra, x).compute_at(output_cgra, r.y);   // pond
          //input_cgra.compute_at(output_cgra, Var::outermost());   // mem tile
          //input_cgra.compute_at(interm_output_cgra, w_glb);   // mem tile
          //input_cgra.compute_at(output_cgra, w_glb);   // mem tile
          //input_cgra.compute_at(output_cgra, r.z);   // mem tile

          // kernel buffers
          hw_kernel.compute_root();
          kernel_host.compute_root(); // host buffer
          kernel_host.accelerator_input();
          //kernel_glb.compute_at(hw_output, x_host); // global buffer
          kernel_cgra.compute_at(hw_output, x_host);   // mem tile
          kernel_pond.compute_at(output_cgra, x);   // pond
          //kernel_cgra.compute_at(output_cgra, Var::outermost());   // mem tile
          //kernel_cgra.compute_at(output_cgra, w_glb);   // mem tile
          //kernel_cgra.compute_at(output_cgra, r.z);   // mem tile

          if (imgsize == 7) {
            //input_cgra.compute_at(output_cgra, Var::outermost());   // mem tile
            //kernel_cgra.compute_at(output_cgra, Var::outermost());   // mem tile
            kernel_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
            input_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
          }


          //input_glb.unroll(z, k_ic);
          //input_cgra.unroll(z, k_ic);
          
          //input_cgra
          //  .split(z, z_glb, z_cgra, k_ic)
          //  .reorder(z_cgra, x, y, z_glb);
          //kernel_cgra
          //  .split(z, z_glb, z_cgra, k_ic)
          //  .split(w, w_glb, w_cgra, k_oc)
          //  .reorder(z_cgra, w_cgra, x, y, z_glb, w_glb);
          
        } else {  // schedule to CPU
          output_cgra.compute_root();
          output_cgra.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, resnet_pond_stationary)
