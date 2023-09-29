#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

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

        RDom r(0, stride,
               0, stride);

        Func input_copy, hw_input("hw_input");
        Func hw_output("hw_output");
        //input_copy(x, y, z) = u16(input(x, y, z));
        //hw_input(x, y, z) = input_copy(x, y, z);
        hw_input(x, y, z) = u16(input(x, y, z));


        /* max pooling 
        Func max_pool("max_pool");
        Func max_pooling[stride][stride];
        
        // max in columns
        for (int i=stride-1; i>=0; --i) {
          for (int j=stride-1; j>=1; --j) {
            if (j==stride-1) {
              max_pooling[i][j](x,y,z) = max(hw_input(x+i, y+j-1, z), hw_input(x+i, y+j, z));
            } else {
              max_pooling[i][j](x,y,z) = max(hw_input(x+i, y+j-1, z), max_pooling[i][(j+1)](x, y, z));
            }
          }
        }

        // take max of each of columns using adjacent rows
        for (int i=stride-1; i>=0; --i) {
          if (i==stride-1) {
            max_pooling[i][0](x,y,z) = max(max_pooling[i-1][1](x,y,z), max_pooling[i][1](x,y,z));
          } else {
            max_pooling[i][0](x,y,z) = max(max_pooling[i+1][0](x,y,z), max_pooling[i-1][1](x,y,z));
          }
        }

        // relu with the final result
        max_pooling[0][0](x,y,z) = max(0, max_pooling[1][0](x,y,z)); // relu
        max_pool(x,y,z) = max_pooling[0][0](x,y,z);
        */
        Func max_pool("max_pool"), maximum_func("maximum_func");
        max_pool(x, y, z) = maximum(hw_input(x * stride + r.x, y * stride + r.y, z), maximum_func);
        maximum_func.update().unroll(r.x).unroll(r.y);
        hw_output(x, y, z) = cast<uint8_t>(max_pool(x, y, z));
        
        /* Average pooling (instead of maximum) works using these lines) */
        //Func avg_pool("avg_pool");
        //avg_pool(x, y, z) = u16(0);
        //avg_pool(x, y, z) += hw_input(x * stride + r.x, y * stride + r.y, z);
        //hw_output(x, y, z) = avg_pool(x, y, z) / 4;

        output(x, y, z) = u8(hw_output(x, y, z));

        output.bound(x, 0, 32);
        output.bound(y, 0, 32);
        output.bound(z, 0, 4);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
            Var xi, yi, xo, yo;
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 64 / stride, 64 / stride)
              .reorder(xi,yi,z,xo,yo)
              .hw_accelerate(xi, xo);

            //max_pool.unroll(x, stride)
            //        .unroll(y, stride);

            //max_pool.linebuffer();
            //avg_pool.compute_at(hw_output, xo);
            maximum_func.compute_at(hw_output, xo);
            maximum_func.update().unroll(r.x).unroll(r.y);

            //hw_input.unroll(x, 4);
            //hw_input.compute_at(hw_output, xo);
            hw_input.stream_to_accelerator();
            //input_copy.compute_root();

        } else { // schedule to CPU
            output.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(MaxPoolKernel, down_sample)
