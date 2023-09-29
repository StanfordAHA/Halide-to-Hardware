#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatrixKernel : public Halide::Generator<MatrixKernel> {
public:
    Input<Buffer<int16_t>>  input{"input", 2};
    Input<Buffer<int16_t>>  kernel{"kernel", 2};
    Output<Buffer<int16_t>> output{"output", 2};

    GeneratorParam<int> imgsize{"imgsize", 64};      // default: 64
    GeneratorParam<int> myunroll{"myunroll", 8};     // default: 8
    GeneratorParam<int> schedule{"schedule", 0};     // default: 0

    // io_unroll determines the input and output unrolling
    GeneratorParam<int> io_unroll{"io_unroll", 1};    // default: 1


    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");


        Func mul("mul");
        RDom r(0, imgsize);

        //Func kernel("kernel");
        //kernel(x,y) = i16(2*x + y);
        //kernel(0,0) = 17;     kernel(0,1) = 4;       kernel(0,2) = 6;
        //kernel(1,0) = 7;      kernel(1,1) = 19;      kernel(1,2) = 4;
        //kernel(2,0) = 5;      kernel(2,1) = 21;      kernel(2,2) = 15;
        //kernel(3,0) = 17;     kernel(3,1) = 4;       kernel(3,2) = 6;
        //kernel(4,0) = 7;      kernel(4,1) = 19;      kernel(4,2) = 4;
        //kernel(5,0) = 5;      kernel(5,1) = 21;      kernel(5,2) = 15;


        Func hw_input("hw_input");
        hw_input(x, y) = i16(input(x, y));

        Func hw_kernel("hw_kernel");
        hw_kernel(x, y) = i16(kernel(x, y));

        mul(x, y) = i16(0);
        mul(x, y)  += hw_input(r.x, y) * i16(hw_kernel(x, r.x));

        Func hw_output("hw_output");
        hw_output(x, y) = mul(x, y);
        output(x, y) = i16(hw_output(x,y));

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
              .unroll(xi, io_unroll)
              .hw_accelerate(xi, xo);

          //kernel.compute_at(hw_output, xo);

          RVar rxi, rxo;
          mul.compute_at(hw_output, xo);
          mul.unroll(x, myunroll);
          mul.update()
            .split(r.x, rxo, rxi, myunroll)
            .split(x, xo, xi, myunroll)
            .reorder(xi, rxi, xo, rxo, y)
            //.reorder(rxi, xi, rxo, xo, y)
            //.reorder(xi, rxi, rxo, xo, y)
            .unroll(rxi, myunroll).unroll(xi, myunroll);
            //.reorder(x, r.x, y)
            //.unroll(x, unroll).unroll(r.x, unroll);
            //.unroll(r.x, unroll).unroll(x, unroll);

          //hw_input.unroll(x, io_unroll).stream_to_accelerator();
          hw_input.stream_to_accelerator();
          hw_input.in().unroll(x, io_unroll);
          //hw_kernel.unroll(x, io_unroll).stream_to_accelerator();
          hw_kernel.stream_to_accelerator();
          hw_kernel.in().unroll(x, io_unroll);

        } else {  // schedule to CPU
          hw_output.compute_root();

          if (schedule == 0) {
            mul.update().unroll(r.x, 8).parallel(x, 4);
          } else if (schedule == 1) {
            mul.update().vectorize(x, 8);
          } else if (schedule == 2) {
            mul.update().vectorize(x, 8).parallel(y, 4);
          } else if (schedule == 3) {
            mul.update().reorder(x, y, r.x).vectorize(x, 8).parallel(y, 4);
          } else {

          }
          

        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatrixKernel, gemm)
