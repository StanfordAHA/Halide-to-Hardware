#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class MatrixKernel : public Halide::Generator<MatrixKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Input<Buffer<uint8_t>>  kernel{"kernel", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    int imgsize = 64;
    int unroll = 8;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z");


        Func mul("mul");
        RDom r(0, imgsize);

        //Func kernel("kernel");
        //kernel(x,y) = u16(2*x + y);
        //kernel(0,0) = 17;     kernel(0,1) = 4;       kernel(0,2) = 6;
        //kernel(1,0) = 7;      kernel(1,1) = 19;      kernel(1,2) = 4;
        //kernel(2,0) = 5;      kernel(2,1) = 21;      kernel(2,2) = 15;
        //kernel(3,0) = 17;     kernel(3,1) = 4;       kernel(3,2) = 6;
        //kernel(4,0) = 7;      kernel(4,1) = 19;      kernel(4,2) = 4;
        //kernel(5,0) = 5;      kernel(5,1) = 21;      kernel(5,2) = 15;

        mul(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        Func hw_kernel("hw_kernel");
        hw_kernel(x, y) = u16(kernel(x, y));
        
        mul(x, y)  += hw_input(r.x, y) * u16(hw_kernel(x, r.x));

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

          //kernel.compute_at(hw_output, xo);
 
          mul.compute_at(hw_output, xo);
          mul.unroll(x, unroll);
          mul.update()
            .reorder(x, r.x, y)
            .unroll(r.x, unroll).unroll(x, unroll);

          hw_input.stream_to_accelerator();
          hw_kernel.stream_to_accelerator();

        } else {  // schedule to CPU
          hw_output.compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MatrixKernel, matmul)
