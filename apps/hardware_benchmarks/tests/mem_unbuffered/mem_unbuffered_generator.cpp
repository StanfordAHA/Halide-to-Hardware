#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
  
  int ksize = 3;  
  //Input<int32_t> tilesize_x{"tilesize_x", 64, 8, 128}; // default 64. bounded between 8 and 128
  //Input<int32_t> tilesize_y{"tilesize_y", 64, 8, 128}; // default 64. bounded between 8 and 128
  int tilesize_x = 64-ksize+1;
  int tilesize_y = 64-ksize+1;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        kernel(x,y) = 0;
//        kernel(0,0) = 17;      kernel(1,0) = 4;        kernel(2,0) = 6;
//        kernel(0,1) = 7;       kernel(1,1) = 19;       kernel(2,1) = 4;
//        kernel(0,2) = 5;       kernel(1,2) = 21;       kernel(2,2) = 15;
//        kernel(0,0) = 1;      kernel(1,0) = 1;       kernel(2,0) = 1;
//        kernel(0,1) = 1;      kernel(1,1) = 1;       kernel(2,1) = 1;
//        kernel(0,2) = 1;      kernel(1,2) = 1;       kernel(2,2) = 1;
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        kernel(1,0) = 14;      kernel(1,1) = 255;       kernel(1,2) = 16;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;
        
        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        //conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");

        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, tilesize_x);
          output.bound(y, 0, tilesize_y);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, tilesize_x, tilesize_y)
              .hw_accelerate(xi, xo);

          //kernel.compute_at(conv, x);
          kernel.compute_at(hw_output, xo);

          // conv is purposely not buffered in this test
          //conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);
          
          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
          kernel.compute_at(output, x);
          
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, mem_unbuffered)
