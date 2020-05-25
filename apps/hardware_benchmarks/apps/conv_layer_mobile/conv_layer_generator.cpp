#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionLayer : public Halide::Generator<ConvolutionLayer> {
public:
    Input<Buffer<int8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

  void generate() {
        /* THE ALGORITHM */

        Func input_copy("input_copy");
        Func hw_input("hw_input");
        Func hw_output("hw_output");
        Var x("x"), y("y"), c("c"), k("k");

        Func filter_dw, filter_pw;
        Func bias_dw, bias_pw;

        filter_dw(x, y, c) = 1;
        filter_pw(c, k) = 3;
        bias_dw(c) = 0;
        bias_pw(k) = 0;

        Func dw_conv("dw_conv");
        Func pw_conv("pw_conv");
        Func pw_conv_reduction;

        RDom r_dw(0, 3, 0, 3);
        RVar pw_c("c");
        RDom r_pw(0, 4);

        input_copy(x, y, c) = input(x, y, c);
        hw_input(x, y, c) = input_copy(x, y, c);

        //depthwise ConvolutionLayer
        dw_conv(x, y, c) = 0;//cast<int16_t>(bias_dw(c));
        dw_conv(x, y, c) += cast<int16_t>(filter_dw(r_dw.x, r_dw.y, c) *
                                          hw_input(x + r_dw.x, y + r_dw.y, c));

        //pointwise ConvolutionLayer
        pw_conv(k, x, y, c) = 0;//cast<int16_t>(bias_pw(k));
        pw_conv(k, x, y, c) += cast<int16_t>(filter_pw(k, c) * dw_conv(x, y, c));
        pw_conv_reduction(k, x, y) = 0;
        pw_conv_reduction(k, x, y) += cast<int16_t>(pw_conv(k, x, y, r_pw.x));
        hw_output(k, x, y) = cast<int8_t>(max(0, pw_conv_reduction(k, x, y)));
        //hw_output(x, y, k) = cast<int8_t>(max(0, dw_conv(x, y, k)));

        output(k, x, y) = cast<uint8_t>(hw_output(k, x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          output.bound(x, 0, 16);
          output.bound(y, 0, 16);
          output.bound(k, 0, 4);

          hw_output.bound(x, 0, 16);
          hw_output.bound(y, 0, 16);
          hw_output.bound(k, 0, 4);

          pw_conv_reduction.bound(x, 0, 16);
          pw_conv_reduction.bound(y, 0, 16);
          pw_conv_reduction.bound(k, 0, 4);

          //bias_dw.bound(c, 0, 4);
          //bias_pw.bound(k, 0, 4);
          //
          //filter_dw.bound(c, 0, 4);
          //filter_pw.bound(c, 0, 4);
          //filter_pw.bound(k, 0, 4);
          //
          //pw_conv.bound(x, 0, 16);
          //pw_conv.bound(y, 0, 16);
          //pw_conv.bound(k, 0, 4);
          
          // Blocking spatially on X Y dim
          Var xo("xo"), xi("xi"), yo("yo"), yi("yi");
          
          //hw_input.compute_root();
          //hw_input.store_at(hw_output, xo).compute_at(dw_conv, x);
          hw_output.compute_root();

          hw_output.tile(x, y, xo, yo, xi, yi, 14, 14)
            .reorder(k, xi, yi, xo, yo)
            .reorder_storage(k, x, y)
            .hw_accelerate(xi, xo);
            //.accelerate({hw_input}, xi, xo);

          filter_dw.compute_at(hw_output, yo)
            .unroll(x).unroll(y).unroll(c);
          filter_pw.compute_at(hw_output, yo)
            .unroll(c).unroll(k);
          bias_dw.compute_at(hw_output, yo)
            .unroll(c);
          bias_pw.compute_at(hw_output, yo)
            .unroll(k);


          //schedule pw conv reduction
          pw_conv_reduction.update()
            .reorder(k, x, y, r_pw.x)
            .unroll(k);

          pw_conv_reduction.compute_at(hw_output, xo).store_at(hw_output, xo);

          //schedule pw conv
          //pw_conv.compute_at(pw_conv_reduction, y).store_at(hw_output, xo)
          pw_conv.compute_at(hw_output, xo).store_at(hw_output, xo)
            .reorder(c, x, y, k);
          
          //schedule dw conv
          dw_conv.compute_at(pw_conv, x).store_at(hw_output, xo)
            .reorder(x, y, c);
          
          //dw_conv.compute_at(pw_conv, x).store_at(hw_output, xo)
          //dw_conv.compute_at(hw_output, xi).store_at(hw_output, xo)
          //dw_conv.linebuffer()

          dw_conv.update()
            .reorder(r_dw.x, r_dw.y, x, y, c)
            .unroll(r_dw.x)
            .unroll(r_dw.y);
          //.unroll(c);
          

          //add input stream
          hw_input.stream_to_accelerator();//.reorder_storage(c, x, y);
          hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          //hw_input.store_root().compute_at(pw_conv, x);
          //hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          //hw_input.store_at(hw_output, xo).compute_at(pw_conv, x);
        } else if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, 16);
          output.bound(y, 0, 16);
          output.bound(k, 0, 4);

          pw_conv_reduction.bound(x, 0, 16);
          pw_conv_reduction.bound(y, 0, 16);
          pw_conv_reduction.bound(k, 0, 4);

          // Blocking spatially on X Y dim
          Var xo("xo"), xi("xi"), yo("yo"), yi("yi");
          
          hw_output.compute_root();

          hw_output.tile(x, y, xo, yo, xi, yi, 14, 14)
            .reorder(k, xi, yi, xo, yo)
            .reorder_storage(k, x, y);

          filter_dw.compute_at(hw_output, yo)
            .unroll(x).unroll(y).unroll(c);
          filter_pw.compute_at(hw_output, yo)
            .unroll(c).unroll(k);
          bias_dw.compute_at(hw_output, yo)
            .unroll(c);
          bias_pw.compute_at(hw_output, yo)
            .unroll(k);

          //schedule pw conv reduction
          pw_conv_reduction.update()
            .reorder(k, x, y, r_pw.x)
            .unroll(k);

          pw_conv_reduction.compute_at(hw_output, xo).store_at(hw_output, xo);

          //schedule pw conv
          pw_conv.compute_at(hw_output, xo).store_at(hw_output, xo)
            .reorder(c, x, y, k);
          
          //schedule dw conv
          dw_conv.compute_at(pw_conv, x).store_at(hw_output, xo)
            .reorder(x, y, c);
          
          dw_conv.update()
            .reorder(r_dw.x, r_dw.y, x, y, c)
            .unroll(r_dw.x)
            .unroll(r_dw.y);

          //add input stream
          hw_input.store_at(hw_output, xo).compute_at(hw_output, xo);
          input_copy.compute_root();
        }


  }
  
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionLayer, conv_layer)

