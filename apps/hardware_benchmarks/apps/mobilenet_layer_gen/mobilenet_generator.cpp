#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ResnetKernel : public Halide::Generator<ResnetKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Input<Buffer<uint8_t>>  filter_dw{"filter_dw", 3};
    Input<Buffer<uint8_t>>  filter_pw{"filter_pw", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    // in_img determines the input image size
    //GeneratorParam<int> in_img{"in_img", 28};    // default: 28
    GeneratorParam<int> in_img{"in_img", 30};    // default: 30

    // pad determines the padding to the input image size
    GeneratorParam<int> pad{"pad", 1};    // default: 1
  
    // ksize determines the output stencil size
    GeneratorParam<uint8_t> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // k_ic determines the number of input channels
    GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4

    // k_oc determines the number of channel sizes
    GeneratorParam<int> k_oc{"k_oc", 3};    // default: 3

    void generate() {
        //int imgsize = (in_img + 0 - ksize + 1) / stride;
        int imgsize = floor( (in_img + 2*pad - ksize) / stride ) + 1;
      
        /* THE ALGORITHM */
        Var x("x"), y("y"), c("c"), k("k");

        Expr height = imgsize;
        Expr width = imgsize;

        RDom r_dw(0, ksize, 0, ksize);
        RVar pw_c("c");
        RDom r_pw(0, k_ic);

        Func hw_input("hw_input");
        Func clamp_input("clamp_input");

        // add padding to the input using a repeated edge
        clamp_input(c, x, y) = input(c, clamp(x-pad, 0, width - 1), clamp(y-pad, 0, height - 1));
        hw_input(c, x, y) = i16(clamp_input(c, x, y));

        Func hw_filter_dw, hw_filter_pw;
        hw_filter_dw(c, x, y) = i16(filter_dw(c, x, y));
        hw_filter_pw(k, c) = i16(filter_pw(k, c));

        //depthwise ConvolutionLayer
        Func dw_conv("dw_conv");
        dw_conv(c, x, y) = i16(0);//cast<int16_t>(bias_dw(c));
        dw_conv(c, x, y) += i16(hw_filter_dw(c, r_dw.x, r_dw.y) *
                                hw_input(c, stride*x + r_dw.x, stride*y + r_dw.y));

        //pointwise ConvolutionLayer
        Func pw_conv("pw_conv");
        //pw_conv(k, c, x, y) = cast<int16_t>(bias_pw(k));
        pw_conv(k, c, x, y) = i16(hw_filter_pw(k, c) *
                                  dw_conv(c, x, y));

        Func pw_conv_reduction;
        pw_conv_reduction(k, x, y) = i16(0);
        //pw_conv_reduction(k, x, y) += i16(pw_conv(k, r_pw.x, x, y));
        pw_conv_reduction(k,x,y) += i16(hw_filter_pw(k, r_pw.x) *
                                        dw_conv(r_pw.x, x, y));

        Func hw_output("hw_output");
        hw_output(k, x, y) = pw_conv_reduction(k, x, y);

        //hw_output(k, x, y) = max(0, pw_conv_reduction(k, x, y));
        //hw_output(x, y, k) = u8(hw_input(x, y, 0));
        //hw_output(x, y, k) = cast<int8_t>(max(0, dw_conv(x, y, k)));

        output(k, x, y) = u8(max(0, hw_output(k, x, y)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo, w
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          
          output.bound(k, 0, k_oc);
          pw_conv_reduction.bound(k, 0, k_oc);
          
          pw_conv.bound(c, 0, k_ic);
          hw_filter_pw.bound(c, 0, k_ic);
          hw_filter_dw.bound(c, 0, k_ic);
          
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize,imgsize)
            .hw_accelerate(xi, xo);
          hw_output.unroll(k);

          pw_conv_reduction.compute_at(hw_output, xo);
          pw_conv_reduction.unroll(k);
          pw_conv_reduction.update().unroll(r_pw.x).unroll(k);
          
          pw_conv.compute_at(hw_output, xo);
          pw_conv.unroll(c).unroll(k);

          dw_conv.compute_at(hw_output, xo);
          dw_conv.unroll(c);
          dw_conv.update().unroll(r_dw.x).unroll(r_dw.y).unroll(c);

          hw_input.in().unroll(c);
          hw_input.stream_to_accelerator();
          hw_filter_dw.in().unroll(c);
          hw_filter_dw.stream_to_accelerator();
          //hw_filter_pw.in().unroll(c).unroll(k);
          hw_filter_pw.stream_to_accelerator();
          
        } else {  // schedule to CPU

        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetKernel, mobilenet)
