#include "Halide.h"

namespace {

using namespace Halide;

class Convolution1x1Kernel : public Halide::Generator<Convolution1x1Kernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Input<Buffer<uint8_t>>  kernel{"kernel", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    int ksize = 1;
    int imgsize = 16-ksize+1;
    int k_z = 2;
    int k_w = 4;
  
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        Expr height = input.dim(0).extent();
        Expr width = input.dim(1).extent();
        //Expr k_z = kernel.dim(2).extent();

        Func conv("conv");
        RDom r(0, 1,
               0, 1,
               0, k_z);

        conv(x, y, w) = 0;

        Func hw_input("hw_input"), hw_kernel("hw_kernel");
        hw_input(z, x, y) = cast<uint16_t>(input(z, x, y));
        hw_kernel(z, w, x, y) = cast<uint16_t>(kernel(z, w, x, y));
        conv(x, y, w)  += hw_kernel(r.z, w, r.x, r.y)  * hw_input(r.z, x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y, w) = cast<uint8_t>(conv(x, y, w));
        output(x, y, w) = max(0, hw_output(x, y, w));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);

          conv.linebuffer();

          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo, w
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, k_w);
          hw_output.bound(w, 0, k_w);
          //clamp_input.bound(z, 0, k_z);
          //kernel.bound(w, 0, k_w);
          hw_kernel.bound(z, 0, k_z);
          hw_input.bound(z, 0, k_z);
          //hw_kernel.bound(w, 0, k_w);
          
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_w); 
            .reorder(xi,yi,w,xo,yo);

          hw_kernel
            .reorder_storage(z,w,x,y)
            .reorder(z,w,x,y);
          
          hw_input
            .reorder_storage(z,x,y)
            .reorder(z,x,y);
          //.unroll(z, k_z);
          
          conv.reorder(w,x,y)
            .reorder_storage(w,x,y);

          conv.update()
            .reorder(x,y,w,r.z,r.x,r.y);
          
          conv.update()
            .unroll(x, ksize).unroll(y, ksize);      // output stationary

          conv.compute_at(hw_output, xo);
          
          hw_input.stream_to_accelerator();
          hw_kernel.stream_to_accelerator();
          
        } else {  // schedule to CPU
          conv.compute_root();
          conv.update()
            .unroll(r.x, 1)
            .unroll(r.y, 1);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Convolution1x1Kernel, conv_1_1)
