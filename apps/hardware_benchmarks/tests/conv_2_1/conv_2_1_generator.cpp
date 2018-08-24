#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, 1,   // 1 column
               0, 2);  // 2 rows

        kernel(x,y) = 0;
        kernel(0,0) = 3;
        kernel(1,0) = 5;
      
        conv(x, y) = 0;

        conv(x, y) += kernel(r.x, r.y) * input(x + r.x, y + r.y);
        output(x, y) = cast<uint16_t>(conv(x, y));

        /* THE SCHEDULE */
        conv.compute_root();
        conv.update()
          .unroll(r.x)
          .unroll(r.y);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_2_1)
