#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

int ksize = 3;

class MemTestAccumulation : public Halide::Generator<MemTestAccumulation> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        //kernel(x,y) = 0;
        //kernel(0,0) = 1;      kernel(0,1) = 4;       kernel(0,2) = 6;
        //kernel(1,0) = 7;      kernel(1,1) = 3;       kernel(1,2) = 2;
        //kernel(2,0) = 5;      kernel(2,1) = 1;       kernel(2,2) = 1;
        kernel(x) = x;
        //kernel.unroll(x, 9);

        conv(x, y) = u16(0);

        Func input_copy, hw_input("hw_input");

        //input_copy(x, y) = u16(input(x, y));
        hw_input(x, y) = u16(input(x, y));
        //conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        conv(x, y)  += u16(kernel(r.x + 3*r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = u8(conv(x, y));
        output(x, y) = u8(hw_output(x,y));

      
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-ksize);
          output.bound(y, 0, 64-ksize);

          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          // conv is not unrolled
          conv.update().unroll(r.y);
          conv.store_at(hw_output, xo).compute_at(hw_output, xi);

          kernel.store_at(hw_output, yi).compute_at(hw_output, yi);
          
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64-ksize+1);
          output.bound(y, 0, 64-ksize+1);

          hw_output.compute_root();
          
          hw_output
              .tile(x,y, xo,yo, xi,yi, 64-ksize+1, 64-ksize+1)
              .hw_accelerate(xi, xo);

          // conv is not unrolled
          //conv.update().unroll(r.y);
          conv.store_at(hw_output, xo).compute_at(hw_output, xo);

          kernel.compute_at(conv, x);
          kernel.unroll(x, 9);
          
          //hw_input.compute_at(hw_output, xo).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          //input_copy.compute_root();

        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MemTestAccumulation, accumulation)
