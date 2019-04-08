#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionLayer : public Halide::Generator<ConvolutionLayer> {
public:
    Input<Buffer<int8_t>>  input{"input", 4};
    Input<Buffer<int8_t>>  filter{"filter", 4};
    Input<Buffer<int8_t>>  bias{"bias", 1};
    //Output<Buffer<uint8_t>> f_ReLU{"ReLU", 4};

    Func build() {
        /* THE ALGORITHM */

        Func output("output");
        Var x("x"), y("y"), c("c"), n("n");

        Func f_conv("conv");
        /*RDom r(filter.dim(0).min(), filter.dim(0).extent(),
               filter.dim(1).min(), filter.dim(1).extent(),
               filter.dim(2).min(), filter.dim(2).extent());
*/
        RDom r(0, 3, 0, 3, 0, 32);

        filter.dim(0).set_bounds(0, 3)
            .dim(1).set_bounds(0, 3)
            .dim(2).set_bounds(0, 32)
            .dim(3).set_bounds(0, 32);

        bias.dim(0).set_bounds(0, 32);

        f_conv(x, y, c, n) = 0;//cast<int16_t>(bias(c));

        f_conv(x, y, c, n) += cast<int16_t>(filter(r.x, r.y, r.z, c) *
                                            input(x + r.x, y + r.y, r.z, n));

        output(x, y, c, n) = cast<int8_t>(max(0, bias(c) + f_conv(x, y, c, n)));


        /* THE SCHEDULE */

        // Blocking spatially with vectorization
        Var xo("xo"), xi("xi"), yo("yo"), yi("yi"), c_t("c_t"), c_o("c_o"), u("u");
        RVar z_o("z_o"), z_t("z_t");

        output.tile(x, y, xo, yo, xi, yi, 32, 32).reorder(xi, yi, xo, yo);
        output.split(c, c_o, c_t, 8)
            .reorder(c_t, xi, yi, c_o, xo, yo);

        //create the output buffer to hold psum
        f_conv.compute_at(output, c_o);
        f_conv.update()
            .split(r.z, z_o, z_t, 8);

        //split the input channel dim, which is a RDom. Use rfactor to create a separate function
        Func interm = f_conv.update().rfactor({{z_o, u}});

        //move the input channel tiling to the out
        f_conv.update().reorder(c, x, y, z_o);

        // unroll some
        interm.compute_at(f_conv, x);
        interm.update().reorder(c, r.x).unroll(c);
        f_conv.update().unroll(c);
        
        //input buffer
        input.in().compute_at(f_conv, z_o)
        .reorder_storage(_2,_0,_1);

        //weight buffer
        filter.in().compute_at(f_conv, z_o);

        //output.accelerate({input}, xi, xo);
        output.compile_to_lowered_stmt("conv.html",
               output.infer_arguments(),
               Halide::HTML);
        /*output.compile_to_c("conv.cpp",
               output.infer_arguments(),
               "conv");*/
        return output;
   }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionLayer, conv_layer)

