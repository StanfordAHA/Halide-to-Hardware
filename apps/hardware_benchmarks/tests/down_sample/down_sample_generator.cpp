#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MaxPoolKernel : public Halide::Generator<MaxPoolKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */
        int stride = 2;

        Var x("x"), y("y");

        Func max_pool("max_pool");
        RDom r(0, stride,
               0, stride);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));;

        max_pool(x, y) = maximum(hw_input(x * stride + r.x, y * stride + r.y));

        Func hw_output("hw_output");
        hw_output(x, y) = max_pool(x, y);
        output(x ,y) = u8(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
            Var xi, yi, xo, yo;

            output.bound(x, 0, 64);
            output.bound(y, 0, 64);
            
            hw_input.compute_root();
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 32, 32)
                .hw_accelerate(xi, xo);

            max_pool.unroll(x, stride)
                    .unroll(y, stride);

            max_pool.linebuffer();

            hw_input.stream_to_accelerator();
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 32);
          output.bound(y, 0, 32);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, 32, 32)
            .hw_accelerate(xi, xo);

          max_pool.compute_at(hw_output, xo);

          // there is no handle to perform below unroll
          //max_pool.unroll(r.x).unroll(r.y);

          hw_input.stream_to_accelerator();

        } else { // schedule to CPU
            max_pool.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MaxPoolKernel, down_sample)
