#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionLayer : public Halide::Generator<ConvolutionLayer> {
public:
    Input<Buffer<int8_t>>  input{"input", 4};
    Input<Buffer<int8_t>>  filter_dw{"filter_dw", 3};
    Input<Buffer<int8_t>>  filter_pw{"filter_pw", 2};
    Input<Buffer<int8_t>>  bias_dw{"bias_dw", 1};
    Input<Buffer<int8_t>>  bias_pw{"bias_pw", 1};
    Output<Buffer<uint8_t>> app_output{"ReLU", 4};

    Func build() {
        /* THE ALGORITHM */

        Func output("output");
        Var x("x"), y("y"), c("c"), n("n");

        Func dw_conv("dw_conv");
        Func pw_conv("pw_conv");
        /*RDom r(filter.dim(0).min(), filter.dim(0).extent(),
               filter.dim(1).min(), filter.dim(1).extent(),
               filter.dim(2).min(), filter.dim(2).extent());
        */
        RDom r_dw(0, 3, 0, 3);
        RDom r_pw(0, 32);

        filter_dw.dim(0).set_bounds(0, 3)
            .dim(1).set_bounds(0, 3)
            .dim(2).set_bounds(0, 32);

        filter_pw.dim(0).set_bounds(0, 32)
            .dim(1).set_bounds(0, 64);

        bias_dw.dim(0).set_bounds(0, 32);
        bias_pw.dim(0).set_bounds(0, 32);

        //depthwise ConvolutionLayer
        dw_conv(x, y, c, n) = cast<int16_t>(bias_dw(c));
        dw_conv(x, y, c, n) += cast<int16_t>(filter_dw(r_dw.x, r_dw.y, c) * input(x + r_dw.x, y + r_dw.y, c, n));

        //pointwise ConvolutionLayer
        pw_conv(x, y, c, n) = cast<int16_t>(bias_pw(c));
        pw_conv(x, y, c, n) += cast<int16_t>(filter_pw(r_pw.x, c) * dw_conv(x, y, r_pw.x, n));
        output(x, y, c, n) = cast<int8_t>(max(0, pw_conv(x, y, c, n)));


        /* THE SCHEDULE */
        // Blocking spatially on X Y dim
        Var xo("xo"), xi("xi"), yo("yo"), yi("yi"), u("u");
        //RVar z_o("z_o"), z_t("z_t");

        output.tile(x, y, xo, yo, xi, yi, 32, 32).reorder(c, xi, yi, xo, yo);

        //schedule dw conv
        Var co("co_dw"), ci("ci_dw");
        dw_conv.compute_at(output, xo);
        dw_conv.update().split(c, co, ci, 4).reorder(ci, x, y, co);
        dw_conv.update().unroll(r_dw.x).unroll(r_dw.y).unroll(ci);

        //add input buffer
        input.in()
            .compute_at(dw_conv, x)
            .store_at(output, xo)
            .reorder(_2, _0, _1)
            .reorder_storage(_2, _0, _1);

        //schedule pw conv
        Var co_("co_pw"), ci_("ci_pw");
        RVar zo("z_o"), zi("z_i");
        pw_conv.compute_at(output, xo);
        pw_conv.update()
            .split(r_pw.x, zo, zi, 4)
            .split(c, co_, ci_, 4).reorder(zi, ci_, zo, x, y, co_);

        //create the function spliting the RDom
        //outter loop zo can be parallelize, but we will use unroll to create a reduction tree of zi
        Func pw_conv_interm = pw_conv.update().rfactor({{zo, u}});
        pw_conv_interm.compute_at(pw_conv, zo);

        //2D unroll input and output channel
        pw_conv_interm.update().unroll(ci_).unroll(zi);
        //unroll the accumulation of partial sum
        pw_conv.update().unroll(ci_);


        output.compile_to_lowered_stmt("conv.html",
               output.infer_arguments(),
               Halide::HTML);
        output.compile_to_c("conv.cpp",
               output.infer_arguments(),
               "conv");
        return output;
   }

  void generate() {
        /* THE ALGORITHM */

        Func hw_output("output");
        Var x("x"), y("y"), c("c"), n("n");

        Func dw_conv("dw_conv");
        Func pw_conv("pw_conv");
        /*RDom r(filter.dim(0).min(), filter.dim(0).extent(),
               filter.dim(1).min(), filter.dim(1).extent(),
               filter.dim(2).min(), filter.dim(2).extent());
        */
        RDom r_dw(0, 3, 0, 3);
        RDom r_pw(0, 32);

        filter_dw.dim(0).set_bounds(0, 3)
            .dim(1).set_bounds(0, 3)
            .dim(2).set_bounds(0, 32);

        filter_pw.dim(0).set_bounds(0, 32)
            .dim(1).set_bounds(0, 64);

        bias_dw.dim(0).set_bounds(0, 32);
        bias_pw.dim(0).set_bounds(0, 32);

        //depthwise ConvolutionLayer
        dw_conv(x, y, c, n) = cast<int16_t>(bias_dw(c));
        dw_conv(x, y, c, n) += cast<int16_t>(filter_dw(r_dw.x, r_dw.y, c) * input(x + r_dw.x, y + r_dw.y, c, n));

        //pointwise ConvolutionLayer
        pw_conv(x, y, c, n) = cast<int16_t>(bias_pw(c));
        pw_conv(x, y, c, n) += cast<int16_t>(filter_pw(r_pw.x, c) * dw_conv(x, y, r_pw.x, n));
        hw_output(x, y, c, n) = cast<int8_t>(max(0, pw_conv(x, y, c, n)));
        app_output(x, y, c, n) = cast<uint8_t>(hw_output(x, y, c, n));


        /* THE SCHEDULE */
        // Blocking spatially on X Y dim
        Var xo("xo"), xi("xi"), yo("yo"), yi("yi"), u("u");
        //RVar z_o("z_o"), z_t("z_t");

        app_output.tile(x, y, xo, yo, xi, yi, 32, 32).reorder(c, xi, yi, xo, yo);

        //schedule dw conv
        Var co("co_dw"), ci("ci_dw");
        dw_conv.compute_at(app_output, xo);
        dw_conv.update().split(c, co, ci, 4).reorder(ci, x, y, co);
        dw_conv.update().unroll(r_dw.x).unroll(r_dw.y).unroll(ci);

        //add input buffer
        input.in()
            .compute_at(dw_conv, x)
            .store_at(app_output, xo)
            .reorder(_2, _0, _1)
            .reorder_storage(_2, _0, _1);

        //schedule pw conv
        Var co_("co_pw"), ci_("ci_pw");
        RVar zo("z_o"), zi("z_i");
        pw_conv.compute_at(app_output, xo);
        pw_conv.update()
            .split(r_pw.x, zo, zi, 4)
            .split(c, co_, ci_, 4).reorder(zi, ci_, zo, x, y, co_);

        //create the function spliting the RDom
        //outter loop zo can be parallelize, but we will use unroll to create a reduction tree of zi
        Func pw_conv_interm = pw_conv.update().rfactor({{zo, u}});
        pw_conv_interm.compute_at(pw_conv, zo);

        //2D unroll input and output channel
        pw_conv_interm.update().unroll(ci_).unroll(zi);
        //unroll the accumulation of partial sum
        pw_conv.update().unroll(ci_);

        if (get_target().has_feature(Target::CoreIR)) {

        }
  }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionLayer, conv_layer)

