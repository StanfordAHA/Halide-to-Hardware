#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class BitCastingKernel : public Halide::Generator<BitCastingKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int imgsize = 62;
    int ksize = 3;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func cast_u8, cast_i8, cast_u16, cast_i16, cast_u32, cast_i32;
        RDom r(0, ksize,               0, ksize);

        kernel(x,y) = 0;
        kernel(0,0) = 17;      kernel(1,0) = 4;        kernel(2,0) = 6;
        kernel(0,1) = 7;       kernel(1,1) = 19;       kernel(2,1) = 4;
        kernel(0,2) = 5;       kernel(1,2) = 21;       kernel(2,2) = 15;

        cast_u32(x, y) = cast<uint32_t>(0);

        Func hw_input("hw_input");
        hw_input(x, y) = cast<int16_t>(input(x, y));

        cast_u32(x, y)  += cast<uint32_t>(kernel(r.x, r.y)) * cast<uint32_t>(hw_input(x + r.x, y + r.y));
        cast_i16(x, y)  = cast<int16_t>(cast_u32(x, y)) * cast<int16_t>(hw_input(x, y));
        cast_u16(x, y)  = cast<uint16_t>(cast_i16(x, y)) + cast<uint16_t>(100);
        cast_i8(x, y)   = cast<int8_t>(cast_u16(x, y)) + cast<int8_t>(cast_u16(x, y));
        cast_i32(x, y)  = cast<int32_t>(cast_i8(x, y)) * cast<int32_t>(333) * cast<int32_t>(cast_u32(x, y));
        cast_u8(x, y)   = cast<uint8_t>(cast_u16(x, y)) - cast<uint8_t>(cast_i16(x, y));

        Func hw_output("hw_output");
        hw_output(x, y) = cast<int16_t>(cast_u8(x, y));
        output(x, y) = cast<uint8_t>(hw_output(x,y));


        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_input.stream_to_accelerator();
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);

          cast_u8.compute_at(hw_output, xo);
          cast_i8.compute_at(hw_output, xo);
          cast_u16.compute_at(hw_output, xo);
          cast_i16.compute_at(hw_output, xo);
          cast_u32.compute_at(hw_output, xo);
          cast_i32.compute_at(hw_output, xo);
          
          kernel.compute_at(hw_output, xo);
          cast_u32.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU

        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BitCastingKernel, casts)
