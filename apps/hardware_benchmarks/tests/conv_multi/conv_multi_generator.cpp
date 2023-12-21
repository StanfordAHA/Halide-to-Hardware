#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
  Input<Buffer<uint8_t>>  input{"input", 3};
  Output<Buffer<uint8_t>> output{"output", 2};

  int imgsize = 62;
  int ksize = 3;
  int zsize = 2;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,      0, ksize,      0, zsize);

        kernel(x,y,z) = 0;
        kernel(0,0,0) = 17;     kernel(0,1,0) = 4;       kernel(0,2,0) = 6;
        kernel(1,0,0) = 7;      kernel(1,1,0) = 19;      kernel(1,2,0) = 4;
        kernel(2,0,0) = 5;      kernel(2,1,0) = 21;      kernel(2,2,0) = 15;
        kernel(0,0,1) = 17;     kernel(0,1,1) = 4;       kernel(0,2,1) = 6;
        kernel(1,0,1) = 7;      kernel(1,1,1) = 19;      kernel(1,2,1) = 4;
        kernel(2,0,1) = 5;      kernel(2,1,1) = 21;      kernel(2,2,1) = 15;

        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y, z) = u16(input(x, y, z));
        //conv(x, y)  += kernel(r.x, r.y, r.z) * hw_input_copy(x + r.x, y + r.y, r.z);
        conv(x, y)  += hw_input(x + r.x, y + r.y, r.z);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          hw_input.bound(z, 0, zsize);

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.z, zsize);

          conv.linebuffer();

          //kernel.store_at(hw_output, yi).compute_at(hw_output, yi);

          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          hw_input.bound(z, 0, zsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .reorder(xi,yi, xo,yo)
              .hw_accelerate(xi, xo);

          kernel.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.z, zsize);

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_multi)
