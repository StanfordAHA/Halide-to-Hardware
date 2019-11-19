#include "Halide.h"

namespace {

using namespace Halide;

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

        Func conv1 = Func("conv1");
        Func conv2 = Func("conv2");

        Func hw_input("hw_input");
        //hw_input(x, y) = cast<uint16_t>(input(x, y));
        hw_input(x, y) = x + y;
        conv1(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
        
        conv2(x, y)  += kernel(r.x, r.y) * conv1(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = cast<uint8_t>(conv2(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          hw_output.bound(x, 0, 64);
          hw_output.bound(y, 0, 64);
          output.bound(x, 0, 60);
          output.bound(y, 0, 60);
          

          hw_output.compute_root();
          hw_output.tile(x,y, xo,yo, xi,yi, 64-4, 64-4)
          //hw_output.tile(x,y, xo,yo, xi,yi, 32, 32)
            .hw_accelerate(xi, xo);

          //hw_input.compute_root();
          hw_input.store_at(hw_output, xo).compute_at(conv1, x);
          
          kernel.compute_at(hw_output, xo);


          conv1.store_at(hw_output, xo).compute_at(hw_output, xi);
          conv1.update()
            .unroll(r.x)
            .unroll(r.y);
          //conv1.linebuffer();


          conv2.update()
            .unroll(r.x)
            .unroll(r.y);
          //conv2.linebuffer();
          conv2.store_at(hw_output, xo).compute_at(hw_output, xi);
          
          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          kernel.compute_root();
          conv2.compute_root();
          conv2.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ConvolutionKernel, cascade)
