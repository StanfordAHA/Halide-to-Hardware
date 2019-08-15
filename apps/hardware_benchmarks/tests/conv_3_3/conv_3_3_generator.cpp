#include "Halide.h"

namespace {

using namespace Halide;

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
        kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
        kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
        kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

        //conv(x, y) = 0;

        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        //conv(x,y) =
        //  kernel(0,0)*hw_input(x,y) +
        //  kernel(1,0)*hw_input(x+1,y) +
        //  kernel(2,0)*hw_input(x+2,y) +
        //  kernel(0,1)*hw_input(x,y+1) +
        //  kernel(1,1)*hw_input(x+1,y+1) +
        //  kernel(2,1)*hw_input(x+2,y+1) +
        //  kernel(0,2)*hw_input(x,y+2) +
        //  kernel(1,2)*hw_input(x+1,y+2) +
        //  kernel(2,2)*hw_input(x+2,y+2);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv(x, y));
        output(x, y) = hw_output(x,y);

        output.bound(x, 0, imgsize);
        output.bound(y, 0, imgsize);

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
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, ksize)
            .unroll(r.y, ksize);

          conv.linebuffer();
          //hw_input.linebuffer();

          hw_input.stream_to_accelerator();
          kernel.compute_at(hw_output, xo);
          
        } else {  // schedule to CPU
//          kernel.compute_root();
//          conv.compute_root();
//          conv.update()
//            .unroll(r.x, ksize)
//            .unroll(r.y, ksize);
          
//          Var xi,yi, xo,yo;
//          output.tile(x, y, xo,yo, xi,yi, 64,64);
//          hw_input.in().store_at(output, xo).compute_at(output, xi);
//          //hw_input.in().store_root().compute_at(output, x);
          hw_input.in().store_root().compute_at(output,x);
          conv.update()
            .unroll(r.x)
            .unroll(r.y);
//          //output.compute_root();
//          
//          kernel.compute_at(output, xo);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_3_3)
