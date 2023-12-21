#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MemTestHistogram : public Halide::Generator<MemTestHistogram> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 1};

    void generate() {
        int inimgsize = 64;
      
        /* THE ALGORITHM */
        Var x("x"), y("y"), b("b");

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func bin, histogram;
        Expr width = inimgsize;
        Expr height = inimgsize;
        RDom r(0, width, 0, height);
        //RDom r(0, width);
        
        //bin(x, y) = i32(hw_input(x, y));
        //histogram(b) = u16(0);
        //histogram(bin(r.x, r.y)) += u16(1);
        bin(x, y) = clamp(i32(hw_input(x, y) & 0xf), 0, 15); // take the lower 4 bits
        histogram(b) = u16(0);
        histogram(bin(r.x, r.y)) += u16(1);
        
        Func hw_output("hw_output");
        //hw_output(x, y) = histogram(x,y);
        hw_output(b) = histogram(b);
        output(b) = u8(hw_output(b));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {          
          Var xi,yi, xo,yo, bo, bi;

          //histogram.bound(b, 0, 16);
          histogram.bound_extent(b, 16);
          output.bound(b, 0, 16);
          hw_output.compute_root();
          
          hw_output
            //.tile(x,y, xo,yo, xi,yi, outimgsize, outimgsize)
            .split(b, bo, bi, 16)
            .hw_accelerate(bi, bo);
          
          bin.compute_at(hw_output, bo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MemTestHistogram, histogram)
