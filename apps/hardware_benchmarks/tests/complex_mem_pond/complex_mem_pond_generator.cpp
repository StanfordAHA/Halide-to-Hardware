#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ResnetLayer : public Halide::Generator<ResnetLayer> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Input<Buffer<uint8_t>>  kernel{"kernel", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 28};    // default: 28

    // pad determines the padding to the input image size
    GeneratorParam<int> pad{"pad", 1};    // default: 1
  
    // ksize determines the output stencil size
    GeneratorParam<uint8_t> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // k_ic determines the number of input channels
    //GeneratorParam<int> k_ic{"k_ic", 8};    // default: 8
    GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
  
    // k_oc determines the number of output channels
    //GeneratorParam<int> k_oc{"k_oc", 6};    // default: 6
    GeneratorParam<int> k_oc{"k_oc", 3};    // default: 3

    // n_ic determines the total number of input channels
    GeneratorParam<int> n_ic{"n_ic", 64};    // default: 64
  
    // n_oc determines the total number of output channels
    GeneratorParam<int> n_oc{"n_oc", 48};    // default: 48
  

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
               0, ic_outer,
               0, k_ic);

        conv(x, y, w) = cast<uint16_t>(0);


        Func hw_input("clamp_input"), hw_kernel("hw_kernel");
        hw_input(z, x, y) = u16(input(z, clamp(x-pad, 0, width - 1), clamp(y-pad, 0, height - 1)));
        hw_kernel(z, w, x, y) = u16(kernel(z, w, x, y));
        
        Func input_host("input_host"), input_cgra("input_cgra"), input_pond("input_pond");
        input_host(z, x, y) = hw_input(z, x, y);
        input_cgra(zz, z, x, y) = input_host(k_ic*z + zz, x, y);
        input_pond(zz, z, x, y) = input_cgra(zz, z, x, y);

        Func kernel_host("kernel_host"), kernel_cgra("kernel_cgra"), kernel_pond("kernel_pond");
        kernel_host(z, w, x, y) = hw_kernel(z, w, x, y);
        kernel_cgra(z, w, x, y)   = kernel_host(z, w, x, y);
        kernel_pond(zz, z, w, x, y) = kernel_cgra(k_ic*z + zz, w, x, y);

        Func hw_output("hw_output"), output_cgra("output_cgra"), output_pond("output_pond");
        output_pond(x, y, w) +=
          kernel_pond(r.w, r.z, w, r.x, r.y) *
          input_pond(r.w, r.z, stride*x + r.x, stride*y + r.y);

        output_cgra(x, y, w) = output_pond(x, y, w);
        hw_output(x, y, w) = output_cgra(x, y, w);
        output(x, y, w) = max(0, u8(hw_output(x, y, w)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          //hw_kernel.bound(z, 0, n_ic);
          //hw_input.bound(z, 0, n_ic);
          input_pond.bound(z, 0, ic_outer);
          input_pond.bound(zz, 0, k_ic);
          kernel_pond.bound(z, 0, ic_outer);
          kernel_pond.bound(zz, 0, k_ic);

          //int tilesize = std::min(imgsize, 28);
          int tilesize = imgsize;

          Var x_host,y_host, x_cgra,y_cgra, x_pond,y_pond;
          Var xi,yi;
          Var w_mem, w_cgra, w_outer, w_unroll, w_iter, w_pond;
          Var z_cgra;
          RVar rz_cgra, rx_pond, rx_cgra;

          int schedule = 1;
          if (schedule == 1) {
            // Produce loop levels: host, global buffer, cgra
            hw_output.compute_root();
            //hw_output.unroll(w, k_oc);
            hw_output
              .tile(x, y, x_host,y_host, xi,yi, imgsize,imgsize)
              .reorder(xi,yi,w, x_host,y_host)
              .hw_accelerate(xi, x_host);

            output_cgra.compute_at(hw_output, x_host); // memtile
            output_cgra
              //.split(w, w_mem, w_cgra, k_oc*2) // how many oc loops in the ponds
              //.reorder(w_cgra, x, y, w_mem);
              .reorder(w, x, y);

            output_pond.compute_at(output_cgra, w); // pond
            output_pond
              .split(w, w_iter, w_unroll, k_oc)
              .reorder(w_unroll, w_iter, x, y);
            
            output_pond.update()
              //.split(w, w_iter, w_unroll, k_oc)
              //.split(w, w_outer, w_cgra, k_oc*2)
              //.split(w_cgra, w_iter, w_unroll, k_oc)
              .split(w, w_iter, w_unroll, k_oc)
              .reorder(r.w, w_unroll, r.x, r.y, w_iter, x, y, r.z);

            //output_pond.update()
            //  .unroll(w_unroll, k_oc) // this is the w/oc reduction
            //  .unroll(r.w, k_ic); // this is the z/ic reduction

            //output_cgra.unroll(w, k_oc);

            // Three buffers: one at host,
            //                a copy stage as the global buffer,
            //                another copy stage as the memory tiles
            input_host.compute_root(); // host buffer
            input_host.accelerator_input();
            input_cgra.compute_at(output_cgra, Var::outermost());   // mem tile
            input_pond.compute_at(output_pond, Var::outermost());   // pond
            //input_pond.reorder(z, x, y);

            // kernel buffers
            kernel_host.compute_root(); // host buffer
            kernel_host.accelerator_input();
            kernel_cgra.compute_at(output_cgra, Var::outermost());   // mem tile
            kernel_pond.compute_at(output_pond, Var::outermost());   // pond

            //input_cgra.unroll(z, k_ic);
            input_pond
              //.split(z, z_gb, z_cgra, k_ic)
              //.reorder(z_cgra, x, y, z_gb);
              .reorder(zz, x, y, z);
            kernel_pond
              //.split(z, z_gb, z_cgra, k_ic)
              //.reorder(z_cgra, w_cgra, x, y, z_gb, w_gb);
              .split(w, w_mem, w_cgra, k_oc)
              .reorder(zz, w_cgra, x, y, z, w_mem);

          }
          
        } else {  // schedule to CPU
          output_cgra.compute_root();
          output_cgra.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, complex_mem_pond)
