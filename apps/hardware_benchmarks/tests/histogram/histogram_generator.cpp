#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MemTestHistogram : public Halide::Generator<MemTestHistogram> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        int outimgsize = 64;
      
        /* THE ALGORITHM */
        Var x("x"), y("y"), b("b");

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func bin, histogram;
        Expr width = outimgsize;
        Expr height = outimgsize;
        //RDom r(0, width, 0, height);
        RDom r(0, width);
        
        //bin(x, y) = i32(hw_input(x, y));
        //histogram(b) = u16(0);
        //histogram(bin(r.x, r.y)) += u16(1);
        bin(x) = i32(hw_input(x, 0));
        histogram(b) = u16(0);
        histogram(bin(r.x)) += u16(1);
        
        Func hw_output("hw_output");
        //hw_output(x, y) = histogram(x,y);
        hw_output(x, y) = histogram(x);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {          
          Var xi,yi, xo,yo;

          output.bound(x, 0, outimgsize);
          output.bound(y, 0, outimgsize);
          histogram.bound(b, 0, 256);

          hw_output.compute_root();
          
          hw_output
            .tile(x,y, xo,yo, xi,yi, outimgsize, outimgsize)
            .hw_accelerate(xi, xo);
          
          bin.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MemTestHistogram, histogram)
