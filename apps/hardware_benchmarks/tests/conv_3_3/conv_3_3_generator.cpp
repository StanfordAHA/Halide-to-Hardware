#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<int16_t>>  input{"input", 2};
    Output<Buffer<int16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, 3,
               0, 3);

        kernel(x,y) = 0;
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

        conv(x, y) = 0;

        conv(x, y) += kernel(r.x, r.y) * input(x + r.x, y + r.y);
        output(x, y) = cast<int16_t>(conv(x, y));

        /* THE SCHEDULE */
        conv.compute_root();
        conv.update()
          .unroll(r.x, 3)
          .unroll(r.y, 3);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_3)
