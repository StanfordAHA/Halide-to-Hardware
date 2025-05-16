#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MEMReshapeTest : public Halide::Generator<MEMReshapeTest> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    GeneratorParam<int> in_img{"in_img", 64};    // default: 64

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, 1,
               0, 2);

        kernel(x,y) = 0;
        kernel(0,0) = 1;
        kernel(0,1) = 1;

        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");

        hw_output(x, y) = conv(x, y);
        output(x, y) = u16(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();

          output.bound(x, 0, in_img);
          output.bound(y, 0, in_img-1);
          conv.bound(x, 0, in_img);
          conv.bound(y, 0, in_img-1);

          hw_output.tile(x,y, xo,yo, xi,yi, in_img, in_img-1)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 2);

          conv.linebuffer();

          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, in_img);
          output.bound(y, 0, in_img-1);

          hw_output.compute_root();
          hw_output
              .tile(x,y, xo,yo, xi,yi, in_img, in_img-1)
              .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 2);

          conv.compute_at(hw_output, xo);

          kernel.compute_at(conv, x);

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          kernel.compute_root();
          conv.compute_root();
          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 2);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MEMReshapeTest, mem_reshape_test)
