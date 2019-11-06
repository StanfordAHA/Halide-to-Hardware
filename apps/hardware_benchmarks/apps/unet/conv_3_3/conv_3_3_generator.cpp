#include "Halide.h"

namespace {

using namespace Halide;

class Convolution3x3Kernel : public Halide::Generator<Convolution3x3Kernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
  //Input<Buffer<uint8_t>>  kernel{"kernel", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    int ksize = 3;
    int imgsize = 62;
    int k_z = 2;
    int k_w = 4;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        Expr height = input.dim(0).extent();
        Expr width = input.dim(1).extent();
        //Expr k_z = kernel.dim(2).extent();

        //Expr k_z = 2;
        Func kernel;
        kernel(x,y,z,w) = 0;
        kernel(0,0,0,0) = 11;      kernel(0,1,0,0) = 12;      kernel(0,2,0,0) = 13;
        kernel(1,0,0,0) = 14;      kernel(1,1,0,0) = 0;       kernel(1,2,0,0) = 16;
        kernel(2,0,0,0) = 17;      kernel(2,1,0,0) = 18;      kernel(2,2,0,0) = 19;

        Func conv("conv");
        RDom r(0, ksize,
               0, ksize,
               0, k_z);

        conv(x, y, w) = 0;

        Func hw_input("hw_input");
        Func clamp_input("clamp_input");
        clamp_input(x, y, z) = input(clamp(x, 0, width - 1), clamp(y, 0, height - 1), z);

        Func kernel_copy;
        kernel_copy(x, y, z, w) = kernel(x, y, z, w);
        hw_input(x, y, z) = cast<uint16_t>(clamp_input(x, y, z));
        //conv(x, y, w) += kernel_copy(r.x, r.y, r.z, w) * hw_input(x + r.x, y + r.y, r.z);
        conv(x, y, w) += kernel(r.x, r.y, r.z, w) * hw_input(x + r.x, y + r.y, r.z);

        Func hw_output("hw_output");
        hw_output(x, y, w) = cast<uint8_t>(conv(x, y, w));
        output(x, y, w) = max(0, hw_output(x, y, w));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, k_w);
          hw_output.bound(w, 0, k_w);
          //clamp_input.bound(z, 0, k_z);
          kernel.bound(w, 0, k_w);
          kernel.bound(z, 0, k_z);
          hw_input.bound(z, 0, k_z);
          
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            .reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_w); 
            //   order reorder from inner to outermost
            .reorder(xi,w,yi,xo,yo);
            //.reorder(xi,yi,w,xo,yo);
            //.reorder(xi,w,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);

          hw_input
            .reorder_storage(x,y,z);
          //.unroll(z, k_z);
          
          conv.reorder(w,x,y)
            .reorder_storage(w,x,y);

          conv.update()
            .reorder(r.z,r.x,r.y,w,x,y);
            //.reorder(r.x,r.y,r.z,w,x,y);
            //.reorder(r.x,r.y,r.z,w,x,y);
            //.reorder(w,r.x,r.y,r.z,x,y);
            //.reorder(r.z,w,r.x,r.y,x,y);

          
          conv.update()
            .unroll(r.z, k_z);                       // unroll input channel
            //.unroll(r.x, ksize).unroll(r.y, ksize);  // unroll conv
            //.unroll(r.x, ksize);                     // unroll conv x
            //.unroll(w, k_w);                         // unroll output channel
            //.unroll(w, k_w);                          // unroll for multiple memories?
            
            //.unroll(w, k_w).unroll(r.x, ksize);
            //.unroll(r.y, ksize);


          conv.linebuffer();

          kernel.compute_at(hw_output, xo).
            unroll(x).unroll(y).unroll(z).unroll(w);
          
          //hw_input.unroll(x, 3);
          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          conv.compute_root();
          conv.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Convolution3x3Kernel, conv_3_3)
