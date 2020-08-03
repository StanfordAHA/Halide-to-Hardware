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
        kernel(0,0) = 17;      kernel(0,1) = 4;      kernel(0,2) = 6;
        kernel(1,0) = 7;      kernel(1,1) = 19;       kernel(1,2) = 4;
        kernel(2,0) = 5;      kernel(2,1) = 21;      kernel(2,2) = 15;

        conv(x, y) = 0;

        Func hw_input("hw_input");
        Func hw_input_copy("hw_input_copy");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        hw_input_copy(x, y) = hw_input(x, y);
        conv(x, y)  += cast<uint16_t>(kernel(r.x, r.y)) * hw_input_copy(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x,y);

        output.bound(x, 0, imgsize);
        output.bound(y, 0, imgsize);
        //hw_output.bound(x, 0, imgsize);
        //hw_output.bound(y, 0, imgsize);
        conv.bound(x, 0, imgsize);
        conv.bound(y, 0, imgsize);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

//          Var x_host,y_host, x_gb,y_gb, x_cgra,y_cgra;
//          // Produce loop levels: host, global buffer, cgra
//          output.tile(x, y, x_host,y_host, xi,yi, 256,256);
//          output.tile(xi, yi, x_gb,y_gb, x_cgra,y_cgra, 64-2,64-2);
//
//          hw_input.store_root().compute_root();
//          hw_input.in().store_at(output, x_host).compute_at(output,x_gb);
//          hw_input.in().in().store_at(output, x_gb).compute_at(output,x_cgra);

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          conv.linebuffer();
          //hw_input.linebuffer();

          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);

          hw_input.stream_to_accelerator();
          kernel.compute_at(hw_output, yi);
          //kernel.compute_root();

        } else if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();

          hw_output
              .tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
              .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);
          kernel.compute_at(conv, x);

          hw_input_copy.compute_at(hw_output, xo);
          hw_input.compute_root();
          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
            output.compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_rolled)
