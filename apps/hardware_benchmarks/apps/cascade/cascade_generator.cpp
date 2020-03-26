#include "Halide.h"

namespace {

using namespace Halide;

const int inImgSize = 64;
//const int outImgSize = inImgSize - 2;
const int outImgSize = inImgSize - 4;

class ConvolutionKernel : public Halide::Generator<ConvolutionKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */
      
        Var x("x"), y("y");

        Func kernel("kernel");
        //Func conv1("conv1"), conv2("conv2");
        RDom r(0, 3,
               0, 3);

        kernel(x,y) = 0;
        kernel(0,0) = 1;      kernel(0,1) = 2;      kernel(0,2) = 1;
        kernel(1,0) = 2;      kernel(1,1) = 4;      kernel(1,2) = 2;
        kernel(2,0) = 1;      kernel(2,1) = 2;      kernel(2,2) = 1;

        Func conv1("conv1");
        Func conv2("conv2");

        conv1(x, y) = 0;
        conv2(x, y) = 0;

        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        conv1(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        
        conv2(x, y)  += kernel(r.x, r.y) * conv1(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv2(x, y));
        //hw_output(x, y) = cast<uint8_t>(conv1(x, y));
        output(x, y) = hw_output(x,y);

        hw_output.bound(x, 0, outImgSize);
        hw_output.bound(y, 0, outImgSize);
        output.bound(x, 0, outImgSize);
        output.bound(y, 0, outImgSize);
        conv2.bound(x, 0, outImgSize);
        conv2.bound(y, 0, outImgSize);
        conv1.bound(x, 0, outImgSize-2);
        conv1.bound(y, 0, outImgSize-2);
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, outImgSize, outImgSize)
            .hw_accelerate(xi, xo);

          //hw_input.compute_root();
          //hw_input.store_at(hw_output, xo).compute_at(conv1, x);
          
          kernel.compute_at(hw_output, yi);

          conv2.store_at(hw_output, xo).compute_at(hw_output, xi);
          conv2.update()
            .unroll(r.x)
            .unroll(r.y);
          //conv2.linebuffer();
          
          //conv1.linebuffer();
          conv1.store_at(hw_output, xo).compute_at(hw_output, xi);
          conv1.update()
            .unroll(r.x)
            .unroll(r.y);
          
          hw_input.stream_to_accelerator();
          //hw_input.compute_root();
          hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);
          
        } else {  // schedule to CPU
          conv1.update()
            .unroll(r.x)
            .unroll(r.y);
          //kernel.compute_root();
          //conv2.compute_root();
          //conv2.update()
            //.unroll(r.x, 3)
            //.unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, cascade)
