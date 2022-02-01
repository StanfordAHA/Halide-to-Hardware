#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    const int ksize = 2;
    const int numchannels = 8;
  
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), c("c");

        Func conv("conv");
        RDom r(0, 1,    0, ksize,    0, numchannels);

        conv(x, y) = u16(0);

        Func hw_input("hw_input"), input_broadcast("input_broadcast");
        hw_input(x, y) = u16(input(x, y));
        input_broadcast(x, y, c) = hw_input(x, y) * u16(c);

        conv(x, y)  += u16(input_broadcast(x + r.x, y + r.y, r.z));

        Func hw_output("hw_output");

        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          int tilesize_x = 16;
          int tilesize_y = 64 - ksize+1;
          output.bound(x, 0, tilesize_x);
          output.bound(y, 0, tilesize_y);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, tilesize_x, tilesize_y)
              .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x)
            .unroll(r.y, ksize)
            .unroll(r.z, numchannels);

          input_broadcast
            .compute_at(hw_output, xo)
            .reorder(c, x, y)
            .unroll(c);
          
          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, pond_benchmark)
