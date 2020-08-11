#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

  int ksize = 3;
  int imgsize = 62;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        kernel(x,y) = 0;
//        kernel(0,0) = 17;     kernel(0,1) = 4;       kernel(0,2) = 6;
//        kernel(1,0) = 7;      kernel(1,1) = 19;      kernel(1,2) = 4;
//        kernel(2,0) = 5;      kernel(2,1) = 21;      kernel(2,2) = 15;
        kernel(0,0) = 17;      kernel(1,0) = 4;        kernel(2,0) = 6;
        kernel(0,1) = 7;       kernel(1,1) = 19;       kernel(2,1) = 4;
        kernel(0,2) = 5;       kernel(1,2) = 21;       kernel(2,2) = 15;

        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));
        conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x + r.x, y + r.y);

        Func hw_output("hw_output");

        hw_output(x, y) = conv(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x,y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          Var xi,yi, xo,yo;
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          conv.bound(x, 0, imgsize);
          conv.bound(y, 0, imgsize);
//          Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
//          // Produce loop levels: host, global buffer, cgra
//          output.tile(x, y, x_host,y_host, xi,yi, 256,256);
//          output.tile(xi, yi, x_gb,y_gb, x_cgra,y_cgra, 64-2,64-2);

//          hw_input.store_root().compute_root();
//          hw_input.in().store_at(output, x_host).compute_at(output,x_gb);
//          hw_input.in().in().store_at(output, x_gb).compute_at(output,x_cgra);
          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);
          conv.update()
            .unroll(r.y, ksize)
            .unroll(r.x, ksize);
          conv.linebuffer();
          //kernel.store_at(hw_output, yi).compute_at(hw_output, yi);
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);

          kernel.compute_at(conv, x);
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

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_3)
