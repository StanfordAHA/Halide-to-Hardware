#include "Halide.h"

namespace {

using namespace Halide;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel"), fp_kernel("fp_kernel");
        Func conv("conv");
        RDom r(0, 7,
               0, 7);

        kernel(x,y) = 0.f;
        kernel(0,0) = 11.f;      kernel(0,1) = 12.f;      kernel(0,2) = 13.f;      kernel(0,3) = 23.f;      kernel(0,4) = 33.f;      kernel(0,5) = 43.f;      kernel(0,6) = 53.f;
        kernel(1,0) = 14.f;      kernel(1,1) = 0.f;       kernel(1,2) = 16.f;      kernel(1,3) = 21.f;      kernel(1,4) = 31.f;      kernel(1,5) = 41.f;      kernel(1,6) = 51.f;
        kernel(2,0) = 17.f;      kernel(2,1) = 18.f;      kernel(2,2) = 19.f;      kernel(2,3) = 26.f;      kernel(2,4) = 32.f;      kernel(2,5) = 42.f;      kernel(2,6) = 52.f;
        kernel(3,0) = 20.f;      kernel(3,1) = 29.f;      kernel(3,2) = 22.f;      kernel(3,3) = 24.f;      kernel(3,4) = 34.f;      kernel(3,5) = 44.f;      kernel(3,6) = 54.f;
        kernel(4,0) = 30.f;      kernel(4,1) = 39.f;      kernel(4,2) = 32.f;      kernel(4,3) = 34.f;      kernel(4,4) = 37.f;      kernel(4,5) = 47.f;      kernel(4,6) = 57.f;
        kernel(5,0) = 40.f;      kernel(5,1) = 49.f;      kernel(5,2) = 42.f;      kernel(5,3) = 44.f;      kernel(5,4) = 47.f;      kernel(5,5) = 48.f;      kernel(5,6) = 58.f;
        kernel(6,0) = 50.f;      kernel(6,1) = 59.f;      kernel(6,2) = 52.f;      kernel(6,3) = 54.f;      kernel(6,4) = 57.f;      kernel(6,5) = 58.f;      kernel(6,6) = 54.f;
        fp_kernel(x, y) = cast<bfloat16_t>(kernel(x, y));

        conv(x, y) = cast<bfloat16_t>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bfloat16_t>(input(x, y));
        conv(x, y)  += fp_kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y) % 256);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();

          output.bound(x, 0, 64-6);
          output.bound(y, 0, 64-6);
          conv.bound(x, 0, 64-6);
          conv.bound(y, 0, 64-6);
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-6, 64-6)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 7)
            .unroll(r.y, 7);

          conv.linebuffer();

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, 7)
            .unroll(r.y, 7);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, fp_conv_7_7)
