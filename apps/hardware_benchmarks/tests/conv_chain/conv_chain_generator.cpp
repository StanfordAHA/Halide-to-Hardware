#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
  Input<Buffer<uint8_t>>  input{"input", 2};
  Output<Buffer<uint8_t>> output{"output", 2};

  // Set enviroment variable to set these:
  //  HALIDE_GEN_ARGS="ksize=3 nconv=2 stride=2 is_upsample=true"
  
  // ksize determines the output stencil size
  GeneratorParam<uint8_t> ksize{"ksize", 3};    // default: 3

  // nconv determines the number of convolutions chained together
  GeneratorParam<uint8_t> ncon{"nconv", 3};    // default: 3
  
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
    int nconv = ncon;
    int input_size = 64;
    int imgsize = input_size;
    for (int i=0; i<nconv; ++i) {
      imgsize = is_upsample ?
        (imgsize - ksize + 1)*stride :
        (imgsize - ksize + 1)/stride;
    }
      
    /* THE ALGORITHM */

    Var x("x"), y("y");

    Func kernel("kernel");
    Func conv[nconv];
    RDom r(0, ksize,               0, ksize);

    kernel(x,y) = 0;
    kernel(0,0)=17;  kernel(0,1)=4;  kernel(0,2)=6;  //kernel(0,3)=19; kernel(0,4)=20; kernel(0,5)=14; kernel(0,6)=13; kernel(0,7)=5;  kernel(0,8)=12; kernel(0,9)=9;
    kernel(1,0)=7;   kernel(1,1)=19; kernel(1,2)=4;  //kernel(1,3)=10; kernel(1,4)=3;  kernel(1,5)=15; kernel(1,6)=19; kernel(1,7)=13; kernel(1,8)=15; kernel(1,9)=17;
    kernel(2,0)=5;   kernel(2,1)=21; kernel(2,2)=15; //kernel(2,3)=5;  kernel(2,4)=18; kernel(2,5)=11; kernel(2,6)=6;  kernel(2,7)=17; kernel(2,8)=16; kernel(2,9)=2;
    kernel(3,0)=16;  kernel(3,1)=13; kernel(3,2)=12; //kernel(3,3)=7;  kernel(3,4)=13; kernel(3,5)=16; kernel(3,6)=2;  kernel(3,7)=0;  kernel(3,8)=3;  kernel(3,9)=12;
    kernel(4,0)=17;  kernel(4,1)=6;  kernel(4,2)=8;  //kernel(4,3)=12; kernel(4,4)=8;  kernel(4,5)=9;  kernel(4,6)=20; kernel(4,7)=14; kernel(4,8)=4;  kernel(4,9)=19;
    kernel(5,0)=13;  kernel(5,1)=17; kernel(5,2)=19; //kernel(5,3)=4;  kernel(5,4)=15; kernel(5,5)=18; kernel(5,6)=12; kernel(5,7)=1;  kernel(5,8)=20; kernel(5,9)=18;
    kernel(6,0)=14;  kernel(6,1)=0;  kernel(6,2)=2;  //kernel(6,3)=16; kernel(6,4)=2;  kernel(6,5)=1;  kernel(6,6)=10; kernel(6,7)=18; kernel(6,8)=10; kernel(6,9)=1;
    kernel(7,0)=20;  kernel(7,1)=20; kernel(7,2)=16; //kernel(7,3)=6;  kernel(7,4)=1;  kernel(7,5)=0;  kernel(7,6)=17; kernel(7,7)=7;  kernel(7,8)=19; kernel(7,9)=16;
    kernel(8,0)=15;  kernel(8,1)=15; kernel(8,2)=13; //kernel(8,3)=14; kernel(8,4)=0;  kernel(8,5)=3;  kernel(8,6)=7;  kernel(8,7)=11; kernel(8,8)=9;  kernel(8,9)=15;
    kernel(9,0)=7;   kernel(9,1)=0;  kernel(9,2)=0;  //kernel(9,3)=11; kernel(9,4)=17; kernel(9,5)=5;  kernel(9,6)=4;  kernel(9,7)=6;  kernel(9,8)=2;  kernel(9,9)=11;

    Func hw_input("hw_input");
    hw_input(x, y) = u16(input(x, y));
        
    for (int i=0; i<nconv; ++i) {
      conv[i] = Func("conv" + std::to_string(i));
      //conv[i](x, y) = u16(0);

      Func& conv_in = i==0 ? hw_input : conv[i-1];
      if (is_upsample) {
        conv[i](x, y)  += u16(kernel(r.x, r.y)) * conv_in(x/stride + r.x, y/stride + r.y);
      } else {
        conv[i](x, y)  += u16(kernel(r.x, r.y)) * conv_in(x*stride + r.x, y*stride + r.y);
      }

      //if (i == 0) {
      //  if (is_upsample) {
      //    conv[i](x, y)  += u16(kernel(r.x, r.y)) * hw_input(x/stride + r.x, y/stride + r.y);
      //  } else {
      //    conv[i](x, y)  += u16(kernel(r.x, r.y)) * hw_input(x*stride + r.x, y*stride + r.y);
      //  }
      //} else {
      //  if (is_upsample) {
      //    conv[i](x, y)  += u16(kernel(r.x, r.y)) * conv[i-1](x/stride + r.x, y/stride + r.y);
      //  } else {
      //    conv[i](x, y)  += u16(kernel(r.x, r.y)) * conv[i-1](x*stride + r.x, y*stride + r.y);
      //  }
      //}
    }

    Func hw_output("hw_output");
    hw_output(x, y) = conv[nconv-1](x, y);
    output(x, y) = u8(hw_output(x,y));

    //output.bound(x, 0, imgsize);
    //output.bound(y, 0, imgsize);

    /* THE SCHEDULE */
    if (get_target().has_feature(Target::CoreIR)) {

    } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
      Var xi,yi, xo,yo;

      output.bound(x, 0, imgsize);
      output.bound(y, 0, imgsize);
          
      hw_output.compute_root();

      hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
        .hw_accelerate(xi, xo);

      kernel.compute_at(hw_output, xo);
      for (int i=0; i<nconv; ++i) {
        conv[i].compute_at(hw_output, xo);

        if (par_mode == 0) {
          // fully sequential, so no unroll needed
        } else if (par_mode == 1) {
          conv[i].update()
            .unroll(r.x);
        } else if (par_mode == 2) {
          conv[i].update()
            .unroll(r.y);
        } else if (par_mode == 3) {
          conv[i].update()
            .unroll(r.x)
            .unroll(r.y)
            .unroll(x, stride);
        } else {
          conv[i].update()
            .unroll(r.x)
            .unroll(r.y);
        }
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
      //for (int i=0; i<nconv; ++i) {
      //  conv[i].update()
      //    .unroll(r.x)
      //    .unroll(r.y);
      //}
//          //output.compute_root();
//
//          kernel.compute_at(output, xo);
    }

  }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, conv_chain)
