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
               0, 3);  // 3 row

        kernel(x,y) = 0;
        kernel(0,0) = 3;
        kernel(1,0) = 5;
        kernel(2,0) = 7;

        Func hw_input, hw_output;
        hw_input(x, y) = input(x, y);
        conv(x, y) = 0;

        conv(x, y) += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        hw_output(x, y) = cast<uint16_t>(conv(x, y));
        output(x, y) = hw_output(x, y);
 
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          std::cout << "compiling the coreir schedule" << std::endl;
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            .accelerate({hw_input}, xi, xo, {});

          conv.update()
            .unroll(r.x)
            .unroll(r.y);

          conv.linebuffer();

        } else if (get_target().has_feature(Target::HLS)) {
          std::cout << "compiling the hls schedule" << std::endl;
          get_target().set_feature(Target::LegacyBufferWrappers);
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64-2)
            .accelerate({hw_input}, xi, xo, {});

          conv.update()
            .unroll(r.x)
            .unroll(r.y);

          conv.linebuffer();

        } else {  // schedule to CPU
          std::cout << "compiling the cpu schedule" << std::endl;
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_1)
