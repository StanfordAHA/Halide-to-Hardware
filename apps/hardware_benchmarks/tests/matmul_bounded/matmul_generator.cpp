#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatrixKernel : public Halide::Generator<MatrixKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<int> n{"n", 16};  // matrix size
    GeneratorParam<int> sched{"sched", 0};  // scratch: 0 orig, 1 unroll r.x, 2 mul@xi, 3 mul@xi+unroll4

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");

        Func kernel("kernel");
        Func mul("mul");
        int imgsize = (int)n;
        RDom r(0, imgsize);

        Func hw_input("hw_input");
        // 2-bit inputs: products <= 12, sums of 16 <= 192, exact at any CGRA width >= 8
        hw_input(x, y) = u16(input(x, y) & u8(3));

        // scratch (RV): the second operand comes from the input stream. The
        // original u16(2*x + y) needs the loop coordinates on chip, which
        // clockwork generates with ROM "counter" MEM tiles; the lake spec
        // MemCore cannot be loaded with ROM contents.
        kernel(x, y) = hw_input(x, y) + u16(1);

        mul(x, y) = u16(0);
        mul(x, y)  += hw_input(r.x, y) * u16(kernel(x, r.x));

        Func hw_output("hw_output");
        hw_output(x, y) = mul(x, y);
        output(x, y) = u8(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          
        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .reorder(xi,yi, xo,yo)
              .hw_accelerate(xi, xo);

          kernel.compute_at(hw_output, xo);
          if ((int)sched == 1) {
            mul.update().unroll(r.x);
          } else if ((int)sched == 2) {
            mul.compute_at(hw_output, xi);
          } else if ((int)sched == 3) {
            mul.compute_at(hw_output, xi);
            mul.update().unroll(r.x, 4);
          }

          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          hw_output.compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatrixKernel, matmul)
