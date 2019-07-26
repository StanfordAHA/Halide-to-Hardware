#include "Halide.h"

namespace {

using namespace Halide;

class MaxPoolKernel : public Halide::Generator<MaxPoolKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */
        int stride = 2;
        Expr width = input.dim(0).extent();
        Expr height = input.dim(1).extent();

        Var x("x"), y("y"), z("z");

        Func max_pool("max_pool");
        RDom r(0, stride,
               0, stride);

        Func hw_input("hw_input");
        hw_input(x, y, z) = cast<uint16_t>(input(x, y, z));

        max_pool(x, y, z) = maximum(hw_input(x * stride + r.x, y * stride + r.y, z));

        Func hw_output("hw_output");
        hw_output(x, y, z) = cast<uint8_t>(max_pool(x, y, z));
        output(x, y, z) = hw_output(x, y, z);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
            Var xi, yi, xo, yo;
            hw_input.compute_root();
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, width / stride, height / stride)
                .hw_accelerate(xi, xo);

            //max_pool.unroll(x, stride)
            //        .unroll(y, stride);

            max_pool.linebuffer();

            hw_input.stream_to_accelerator();
        } else { // schedule to CPU
            max_pool.compute_root();
            max_pool.unroll(x, stride)
                    .unroll(y, stride);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MaxPoolKernel, down_sample)
