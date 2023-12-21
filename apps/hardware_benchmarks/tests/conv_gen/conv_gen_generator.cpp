#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

  // Set enviroment variable to set these:
  //  HALIDE_GEN_ARGS="ksize=3 stride=2 is_upsample=true"
  
    // ksize determines the output stencil size
    GeneratorParam<uint8_t> ksize{"ksize", 3};    // default: 3

    // Stride determines the sampling rate, while "is_upsample" chooses
    //  between a down sample vs an up sample.
    //  For example, {3, true} === pixel is reused 3 times
    //               {4, false} === stencil strides by 4 between output values
    GeneratorParam<int>  stride{"stride", 1};  // default: 1
    GeneratorParam<bool> is_upsample{"is_upsample", false};  // default: false

    // Three modes: 0 === fully sequential (one multiplier)
    //              1 === partially sequential, x unrolled
    //              2 === partially sequential, y unrolled
    //              3 === fully unrolled, possibly unrolled further
    //     In mode 3, additionally use "unroll" parameter for further parallelism.
    //     In other modes, ignore the "unroll" parameter.
    GeneratorParam<uint8_t> par_mode{"par_mode", 3};  // default: 3
    GeneratorParam<uint8_t> unroll{"unroll", 1};  // default: 1

    void generate() {
      int imgsize = is_upsample ?
        (64 - ksize + 1)*stride :
        (64 - ksize + 1)/stride;
      
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func kernel("kernel");
        Func conv("conv");
        RDom r(0, ksize,               0, ksize);

        kernel(x,y) = 0;
        kernel(0,0)=17;  kernel(0,1)=4;  kernel(0,2)=6;  kernel(0,3)=7;  kernel(0,4)=7;  kernel(0,5)=7;  kernel(0,6)=7;  kernel(0,7)=7;  kernel(0,8)=7;  kernel(0,9)=7;
        kernel(1,0)=7;   kernel(1,1)=19; kernel(1,2)=4;  kernel(1,3)=7;  kernel(1,4)=7;  kernel(1,5)=7;  kernel(1,6)=7;  kernel(1,7)=7;  kernel(1,8)=7;  kernel(1,9)=7;
        kernel(2,0)=5;   kernel(2,1)=21; kernel(2,2)=15; kernel(2,3)=7;  kernel(2,4)=7;  kernel(2,5)=7;  kernel(2,6)=7;  kernel(2,7)=7;  kernel(2,8)=7;  kernel(2,9)=7;
        kernel(3,0)=7;   kernel(3,1)=7;  kernel(3,2)=7;  kernel(3,3)=7;  kernel(3,4)=7;  kernel(3,5)=7;  kernel(3,6)=7;  kernel(3,7)=7;  kernel(3,8)=7;  kernel(3,9)=7;
        kernel(4,0)=7;   kernel(4,1)=7;  kernel(4,2)=7;  kernel(4,3)=7;  kernel(4,4)=7;  kernel(4,5)=7;  kernel(4,6)=7;  kernel(4,7)=7;  kernel(4,8)=7;  kernel(4,9)=7;
        kernel(5,0)=7;   kernel(5,1)=7;  kernel(5,2)=7;  kernel(5,3)=7;  kernel(5,4)=7;  kernel(5,5)=7;  kernel(5,6)=7;  kernel(5,7)=7;  kernel(5,8)=7;  kernel(5,9)=7;
        kernel(6,0)=7;   kernel(6,1)=7;  kernel(6,2)=7;  kernel(6,3)=7;  kernel(6,4)=7;  kernel(6,5)=7;  kernel(6,6)=7;  kernel(6,7)=7;  kernel(6,8)=7;  kernel(6,9)=7;
        kernel(7,0)=7;   kernel(7,1)=7;  kernel(7,2)=7;  kernel(7,3)=7;  kernel(7,4)=7;  kernel(7,5)=7;  kernel(7,6)=7;  kernel(7,7)=7;  kernel(7,8)=7;  kernel(7,9)=7;
        kernel(8,0)=7;   kernel(8,1)=7;  kernel(8,2)=7;  kernel(8,3)=7;  kernel(8,4)=7;  kernel(8,5)=7;  kernel(8,6)=7;  kernel(8,7)=7;  kernel(8,8)=7;  kernel(8,9)=7;
        kernel(9,0)=7;   kernel(9,1)=7;  kernel(9,2)=7;  kernel(9,3)=7;  kernel(9,4)=7;  kernel(9,5)=7;  kernel(9,6)=7;  kernel(9,7)=7;  kernel(9,8)=7;  kernel(9,9)=7;
        
        conv(x, y) = u16(0);

        Func hw_input("hw_input");
        hw_input(x, y) = u16(input(x, y));

        if (is_upsample) {
          conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x/stride + r.x, y/stride + r.y);
        } else {
          conv(x, y)  += u16(kernel(r.x, r.y)) * hw_input(x*stride + r.x, y*stride + r.y);
        }

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = u8(hw_output(x,y));

        //output.bound(x, 0, imgsize);
        //output.bound(y, 0, imgsize);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

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

          conv.linebuffer();
          
          if (par_mode == 0) {
            // fully sequential, so no unroll needed
          } else if (par_mode == 1) {
            conv.update()
              .unroll(r.x);
          } else if (par_mode == 2) {
            conv.update()
              .unroll(r.y);
          } else if (par_mode == 3) {
            conv.update()
              .unroll(r.x)
              .unroll(r.y)
              .unroll(x, stride);
          } else {
            conv.update()
              .unroll(r.x)
              .unroll(r.y);
          }

          //hw_input.linebuffer();
          hw_input.stream_to_accelerator();
          kernel.compute_at(hw_output, yi);
          //kernel.compute_root();

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);
            
          kernel.compute_at(conv, x);
          if (par_mode == 0) {
            // fully sequential, so no unroll needed
          } else if (par_mode == 1) {
            conv.update()
              .unroll(r.x);
          } else if (par_mode == 2) {
            conv.update()
              .unroll(r.y);
          } else if (par_mode == 3) {
            conv.update()
              .unroll(r.x)
              .unroll(r.y)
              .unroll(x, stride);
          } else {
            conv.update()
              .unroll(r.x)
              .unroll(r.y);
          }

          hw_input.stream_to_accelerator();

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
          //hw_input.in().store_root().compute_at(output,x);
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

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_gen)
