#include "Halide.h"

namespace {

using namespace Halide;

class Convolution1x1Kernel : public Halide::Generator<Convolution1x1Kernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Input<Buffer<uint8_t>>  kernel{"kernel", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        Expr height = input.dim(0).extent();
        Expr width = input.dim(1).extent();
        Expr k_z = kernel.dim(2).extent();

        Func conv("conv");
        RDom r(0, 1,
               0, 1,
               0, k_z);

        conv(x, y, w) = 0;

        Func hw_input("hw_input");
        hw_input(x, y, z) = cast<uint16_t>(input(x, y, z));
        conv(x, y, w)  += kernel(r.x, r.y, r.z, w)  * hw_input(x + r.x, y + r.y, r.z);

        Func hw_output("hw_output");
        hw_output(x, y, w) = cast<uint8_t>(conv(x, y, w));
        output(x, y, w) = max(0, hw_output(x, y, w));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);

          conv.linebuffer();

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          conv.compute_root();
          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Convolution1x1Kernel, conv_1_1)
