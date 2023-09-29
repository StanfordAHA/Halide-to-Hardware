#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MobilenetLayer : public Halide::Generator<MobilenetLayer> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  filter_dw{"filter_dw", 3};
    Input<Buffer<int16_t>>  filter_pw{"filter_pw", 2};
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
    GeneratorParam<int> k_ic{"k_ic", 8};    // default: 8
  
    // k_oc determines the number of output channels
    GeneratorParam<int> k_oc{"k_oc", 8};    // default: 8

    // n_ic determines the total number of input channels
    GeneratorParam<int> n_ic{"n_ic", 32};    // default: 32
  
    // n_oc determines the total number of output channels
    GeneratorParam<int> n_oc{"n_oc", 32};    // default: 32
  

    void generate() {
        //int imgsize = (in_img + 0 - ksize + 1) / stride;
        int imgsize = floor( (in_img + 2*pad - ksize) / stride ) + 1;
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w"), zz("zz");

        Expr height = imgsize;
        Expr width = imgsize;
        int ic_outer = (int)n_ic / (int)k_ic;

        Func hw_input("hw_input"), hw_filter_dw("hw_filter_dw"), hw_filter_pw("hw_filter_pw");
        hw_input(z, x, y) = i16(input(z, clamp(x-pad, 0, width - 1), clamp(y-pad, 0, height - 1)));
        hw_filter_dw(z, x, y) = i16(filter_dw(z, x, y));
        hw_filter_pw(w, z) = i16(filter_pw(w, z));
        
        Func input_host("input_host"), input_glb("input_glb"), input_cgra("input_cgra");
        input_host(z, x, y) = hw_input(z, x, y);
        input_glb(z, x, y) = input_host(z, x, y);
        input_cgra(z, x, y) = input_glb(z, x, y);

        Func filter_dw_host("filter_dw_host"), filter_dw_glb("filter_dw_glb"), filter_dw_cgra("filter_dw_cgra");
        filter_dw_host(z, x, y) = hw_filter_dw(z, x, y);
        filter_dw_glb(z, x, y) = filter_dw_host(z, x, y);
        filter_dw_cgra(z, x, y) = filter_dw_glb(z, x, y);

        Func filter_pw_host("filter_pw_host"), filter_pw_glb("filter_pw_glb"), filter_pw_cgra("filter_pw_cgra");
        filter_pw_host(w, z) = hw_filter_pw(w, z);
        filter_pw_glb(w, z) = filter_pw_host(w, z);
        filter_pw_cgra(w, z) = filter_pw_glb(w, z);
        
        Func dw_conv("dw_conv"), pw_conv("pw_conv");
        //depthwise ConvolutionLayer
        RDom r_dw(0, ksize, 0, ksize);
        dw_conv(z, x, y) = i16(0); //cast<int16_t>(bias_dw(c));
        dw_conv(z, x, y) += i16(filter_dw_cgra(z, r_dw.x, r_dw.y) *
                                input_cgra(z, stride*x + r_dw.x, stride*y + r_dw.y));

        //pointwise ConvolutionLayer
        //RVar pw_c("c");
        //pw_conv(w, z, x, y) = cast<int16_t>(bias_pw(k));
        //pw_conv(w, z, x, y) = i16(hw_filter_pw(w, z) * dw_conv(z, x, y));

        Func pw_conv_reduction("pw_conv_reduction");
        //RDom r_pw(0, k_ic);
        RDom r_pw(0, n_ic);
        pw_conv_reduction(w, x, y) = i16(0);
        //pw_conv_reduction(w, x, y) += i16(pw_conv(w, r_pw.x, x, y));
        pw_conv_reduction(w, x, y) += i16(filter_pw_cgra(w, r_pw.x) *
                                          dw_conv(r_pw.x, x, y));

        
        Func hw_output("hw_output"), output_glb("output_glb"), output_cgra("output_cgra");
        output_cgra(w, x, y) = pw_conv_reduction(w, x, y); 
        output_glb(w, x, y) = output_cgra(w, x, y);
        hw_output(w, x, y) = output_glb(w, x, y);
        output(w, x, y) = max(0, i16(hw_output(w, x, y)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, n_oc);
          hw_output.bound(w, 0, n_oc);

          hw_filter_dw.bound(z, 0, n_ic);
          hw_filter_pw.bound(z, 0, n_ic);
          input_host.bound(z, 0, n_ic);
          filter_pw_glb.bound(w, 0, n_oc);

          int gbsize = imgsize;
          int tilesize = ((int)stride == 2) ?
            std::min(14, imgsize) : // we want the input to be 30 max
            std::min(28, imgsize);  // min of 28 and output image size
          int tilesize_y = (pad == 3) ?
            16 : // this occurs only for conv1. We need it smaller to fit
            tilesize;

          Var x_host,y_host, x_glb,y_glb, x_cgra,y_cgra;
          Var xi,yi;
          Var w_cgra, w_glb;
          Var z_cgra, z_glb;
          RVar rz_cgra, rz_glb;

          // Produce loop levels: host, global buffer, cgra
          hw_output.compute_root();
          //hw_output.unroll(w, k_oc);
          hw_output
            .tile(x, y, x_host,y_host, xi,yi, gbsize,gbsize)
            .reorder(w,xi,yi, x_host,y_host)
            .hw_accelerate(xi, x_host);
          hw_output.unroll(w, k_oc);

          output_glb.compute_at(hw_output, x_host); // global buffer
          output_glb
            .tile(x, y, x_glb,y_glb, x_cgra,y_cgra, tilesize,tilesize_y)
            .split(w, w_glb, w_cgra, k_oc)
            // reorder from inner to outermost
            .reorder(w_cgra, x_cgra, y_cgra,
                     w_glb, x_glb, y_glb)
            .unroll(w_cgra, k_oc);


          int glb_o = 1;
          if (glb_o > 1) {
            hw_output.unroll(w, glb_o);
            output_glb.unroll(w_cgra, glb_o); // unroll cgra->glb channels for small images
          }
          
          //output_cgra.compute_at(output_glb, w_glb); // memtile
          //output_cgra
          //  .reorder(w, x, y);
          //output_cgra.update()
          //  .split(r.z, rz_glb, rz_cgra, k_ic)
          //  .reorder(rz_cgra, w, x, y, r.x, r.y, rz_glb);

          pw_conv_reduction.compute_at(output_glb, w_glb)
            .unroll(w, k_oc);
          pw_conv_reduction.update()
            .split(r_pw.x, rz_glb, rz_cgra, k_ic)
            .reorder(rz_cgra, w, x, y, rz_glb)
            .unroll(rz_cgra).unroll(w, k_oc);

          dw_conv.compute_at(pw_conv_reduction, rz_glb)
            .unroll(z, k_ic);
          dw_conv.update()
            .split(z, z_glb, z_cgra, k_ic)
            .reorder(z_cgra, r_dw.x, r_dw.y, x, y, z_glb)
            .unroll(r_dw.x).unroll(r_dw.y).unroll(z_cgra, k_ic);

          //Func output_rf;
          //output_cgra
          //  .unroll(w, k_oc);
          //output_cgra.update()
          //  //.unroll(w_cgra, k_oc)
          //  .unroll(w, k_oc)
          //  .unroll(rz_cgra, k_ic); // this is the z reduction


          // Three buffers: one at host,
          //                a copy stage as the global buffer,
          //                another copy stage as the memory tiles
          //hw_input.compute_root();
          input_host.compute_root(); // host buffer
          input_host.accelerator_input();
          //input_host.stream_to_accelerator();
          input_glb.compute_at(hw_output, x_host); // global buffer
          input_cgra.compute_at(pw_conv_reduction, rz_glb);   // mem tile
          //input_cgra.compute_at(, Var::outermost());   // mem tile
          input_cgra.unroll(z, k_ic);   // mem tile
          
          // filter_dw buffers
          hw_filter_dw.compute_root();
          filter_dw_host.compute_root(); // host buffer
          filter_dw_host.accelerator_input();
          filter_dw_glb.compute_at(hw_output, x_host); // global buffer
          filter_dw_cgra.compute_at(pw_conv_reduction, rz_glb);   // mem tile

          // filter_pw buffers
          hw_filter_pw.compute_root();
          filter_pw_host.compute_root(); // host buffer
          filter_pw_host.accelerator_input();
          filter_pw_glb.compute_at(hw_output, x_host); // global buffer
          filter_pw_cgra.compute_at(pw_conv_reduction, rz_glb);   // mem tile
          
          if (imgsize == 7) {
            filter_dw_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
            filter_pw_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
            input_cgra.unroll(z, 2); // unroll glb->cgra channels for small images
          }

          //input_glb.unroll(z, k_ic);
          //input_cgra.unroll(z, k_ic);
          //input_cgra
          //  .split(z, z_glb, z_cgra, k_ic)
          //  .reorder(z_cgra, x, y, z_glb);
          //.reorder(zz, x, y, z);
          filter_dw_cgra
            .split(z, z_glb, z_cgra, k_ic)
            .reorder(z_cgra, x, y, z_glb);
            //.reorder(zz, w_cgra, x, y, z, w_glb);
          filter_pw_cgra
            .split(z, z_glb, z_cgra, k_ic)
            .split(w, w_glb, w_cgra, k_oc)
            .reorder(z_cgra, w_cgra, z_glb, w_glb);
            //.reorder(zz, w_cgra, x, y, z, w_glb);

          int glb_k = 1;
          int glb_i = 1;

          if (glb_k > 1) {
            filter_dw_glb.unroll(z, glb_k); // unroll glb input for small images
            filter_dw_cgra.unroll(z_cgra, glb_k); // unroll glb input for small images
            filter_pw_glb.unroll(z, glb_k); // unroll glb input for small images
            filter_pw_cgra.unroll(z_cgra, glb_k); // unroll glb input for small images
          }
          if (glb_i > 1) {
            input_glb.unroll(z, glb_i); // unroll glb input for small images
            input_cgra.unroll(z_cgra, glb_i); // unroll glb->cgra channels for small images
          }

          
        } else {  // schedule to CPU
          output_cgra.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MobilenetLayer, mobilenet_output_stationary)
