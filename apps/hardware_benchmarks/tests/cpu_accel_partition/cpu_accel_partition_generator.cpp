#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class CpuAccelPartition : public Halide::Generator<CpuAccelPartition> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

  int ksize = 3;
  int imgsize = 62;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        // stuff that should run on CPU
        Func cpu_stage0;
        Func cpu_stage1;

        cpu_stage0(x, y) = input(x, y) + 1;
        cpu_stage1(x, y) = cpu_stage0(x, y) - 1;

        
        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(cpu_stage1(x, y));

        // stuff that should run on Accel
        Func hw_input_copy("hw_input_copy");
        Func conv("conv");
        Func kernel("kernel");
        RDom r(0, ksize, 0, ksize);
        hw_input_copy(x, y) = hw_input(x, y);

        kernel(x, y) = 0;
        kernel(0,0) = 17;       kernel(1,0) = 4;        kernel(2, 0) = 6;
        kernel(0,1) = 7;        kernel(1,1) = 19;       kernel(2, 1) = 4;
        kernel(0,2) = 5;        kernel(1,2) = 21;       kernel(2, 2) = 15;

        conv(x, y)  += cast<uint16_t>(kernel(r.x, r.y)) * hw_input_copy(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x,y);


        if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);

          //conv.compute_at(hw_output, xo);

          kernel.compute_at(conv, x);
          conv.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          cpu_stage0.compute_root();
          cpu_stage1.compute_root();

          hw_input_copy.compute_at(hw_output, xo);
          hw_input.compute_root();
          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(CpuAccelPartition, cpu_accel_partition)
