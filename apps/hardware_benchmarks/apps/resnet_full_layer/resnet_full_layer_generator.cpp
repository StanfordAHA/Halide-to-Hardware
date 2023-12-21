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
        hw_input(z, x, y) = u16(input(z, clamp(x-pad, 0, in_img - 1), clamp(y-pad, 0, in_img - 1)));
        hw_kernel(z, w, x, y) = u16(kernel(z, w, x, y));
        
        Func input_host("input_host"), input_gb("input_gb"), input_cgra("input_cgra");
        input_host(z, x, y) = hw_input(z, x, y);
        input_gb(z, x, y)   = input_host(z, x, y);
        input_cgra(zz, z, x, y) = input_gb(k_ic*z + zz, x, y);

        Func kernel_host("kernel_host"), kernel_gb("kernel_gb"), kernel_cgra("kernel_cgra");
        kernel_host(z, w, x, y) = hw_kernel(z, w, x, y);
        kernel_gb(z, w, x, y)   = kernel_host(z, w, x, y);
        kernel_cgra(zz, z, w, x, y) = kernel_gb(k_ic*z + zz, w, x, y);

        Func hw_output("hw_output"), output_gb("output_gb"), output_cgra("output_cgra");
        output_cgra(x, y, w) +=
          kernel_cgra(r.w, r.z, w, r.x, r.y) *
          input_cgra(r.w, r.z, stride*x + r.x, stride*y + r.y);

        output_gb(x, y, w) = output_cgra(x, y, w);
        hw_output(x, y, w) = output_gb(x, y, w);
        output(x, y, w) = max(0, u8(hw_output(x, y, w)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          //hw_kernel.bound(z, 0, n_ic);
          //hw_input.bound(z, 0, n_ic);
          input_cgra.bound(z, 0, ic_outer);
          input_cgra.bound(zz, 0, k_ic);
          kernel_cgra.bound(z, 0, ic_outer);
          kernel_cgra.bound(zz, 0, k_ic);
          //kernel_gb.bound(w, 0, n_oc);

          int gbsize = imgsize;
          int tilesize = std::min(imgsize, 28);

          Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_cgra, w_gb;
          Var z_cgra, z_gb;
          RVar rz_cgra, rz_gb;

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          //hw_output.unroll(w, k_oc);
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
            .reorder(xi,yi,w, x_host,y_host)
            .hw_accelerate(xi, x_host);

          output_gb.compute_at(hw_output, x_host); // global buffer
          output_gb
            .tile(x, y, x_gb,y_gb, x_cgra,y_cgra, tilesize,tilesize)
            .split(w, w_gb, w_cgra, k_oc)
            // reorder from inner to outermost
            .reorder(w_cgra, x_cgra, y_cgra,
                     x_gb, y_gb, w_gb);

          output_cgra.compute_at(output_gb, x_gb); // memtile
          output_cgra
            .split(w, w_gb, w_cgra, k_oc)
            .reorder(w_cgra, x, y, w_gb);
          output_cgra.update()
            //.split(r.z, rz_gb, rz_cgra, k_ic)
            .split(w, w_gb, w_cgra, k_oc)
            .reorder(r.w, w_cgra, x, y, r.x, r.y, w_gb, r.z);

          Func output_rf;
          output_cgra.update()
            .unroll(w_cgra, k_oc)
            .unroll(r.w, k_ic); // this is the z reduction

          //output_cgra.unroll(w_cgra, k_oc);

          // Three buffers: one at host,
          //                a copy stage as the global buffer,
          //                another copy stage as the memory tiles
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          input_gb.compute_at(hw_output, x_host); // global buffer
          //input_cgra.compute_at(output_gb, x_gb);   // mem tile
          input_cgra.compute_at(output_cgra, w_gb);   // mem tile

          // kernel buffers
          kernel_host.compute_root(); // host buffer
          kernel_host.accelerator_input();
          kernel_gb.compute_at(hw_output, x_host); // global buffer
          //kernel_cgra.compute_at(output_gb, x_gb);   // mem tile
          kernel_cgra.compute_at(output_cgra, w_gb);   // mem tile

          //input_gb.unroll(z, k_ic);
          //input_cgra.unroll(z, k_ic);
          input_cgra
            //.split(z, z_gb, z_cgra, k_ic)
            //.reorder(z_cgra, x, y, z_gb);
            .reorder(zz, x, y, z);
          kernel_cgra
            //.split(z, z_gb, z_cgra, k_ic)
            //.reorder(z_cgra, w_cgra, x, y, z_gb, w_gb);
            .split(w, w_gb, w_cgra, k_oc)
            .reorder(zz, w_cgra, x, y, z, w_gb);
          
        } else {  // schedule to CPU
          output_cgra.compute_root();
          output_cgra.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetLayer, resnet_full_layer)
