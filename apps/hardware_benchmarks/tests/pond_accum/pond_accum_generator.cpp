#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class AccumulationKernel : public Halide::Generator<AccumulationKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int ksize = 8;
    int imgsize = 64;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func accum("conv");
        RDom r(0, ksize);

        
        accum(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        accum(x, y)  += hw_input(ksize*x + r.x, y);

        Func hw_output("hw_output");
        hw_output(x, y) = accum(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize / ksize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize / ksize, imgsize)
              .hw_accelerate(xi, xo);

          accum.compute_at(hw_output, xo);
          
          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
          
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(AccumulationKernel, pond_accum)
