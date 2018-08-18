#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionLayer : public Halide::Generator<ConvolutionLayer> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Input<Buffer<uint8_t>>  filter{"filter", 2};

    Func build() {
        /* THE ALGORITHM */
      
        Func output("output");
        Var x("x"), y("y");

        Func f_conv("conv");
        RDom r(0, 5, 0, 5);

        f_conv(x, y) += filter(r.x, r.y) * input(x + r.x, y + r.y);

        output(x, y) = f_conv(x, y);

        filter.dim(0).set_bounds(0, 5)
              .dim(1).set_bounds(0, 5);

        /* THE SCHEDULE */

           // Blocking spatially with vectorization
        Var xo("xo"), xi("xi"), yo("yo"), yi("yi");
        output.tile(x, y, xo, yo, xi, yi, 64, 64).reorder(xi, yi, xo, yo);
        
        f_conv.compute_at(output, xi);
        f_conv.update().unroll(r.x).unroll(r.y);
      
        // Uncomment the below schedule for the second 
        // level of storage folding  
        //input.in().in().store_at(output, yi)
        //               .compute_at(output, xi);
 
        input.in().store_at(output, xo)
                  .compute_at(output, xi);

        filter.in().compute_root();
       
        output.compile_to_lowered_stmt("conv.html",
               output.infer_arguments(),
               Halide::HTML);
        output.compile_to_c("conv.cpp",
               output.infer_arguments(),
               "conv");
        return output;
   }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionLayer, conv_layer)

