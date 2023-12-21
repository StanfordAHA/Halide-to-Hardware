#include "Halide.h"

namespace {

using namespace Halide;

class ResnetKernel : public Halide::Generator<ResnetKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Input<Buffer<uint8_t>>  kernel{"kernel", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    int ksize = 3;
    int imgsize = 28;
  //int k_z = 8;
    int k_z = 4; //input channel
    int k_w = 3; // should be 8 output channels

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        Expr height = input.dim(0).extent();
        Expr width = input.dim(1).extent();
        //Expr k_z = kernel.dim(2).extent();

        //Func kernel;
        //kernel(x,y,z,w) = 0;
        //kernel(0,0,0,0) = 11;      kernel(0,1,0,0) = 12;      kernel(0,2,0,0) = 13;
        //kernel(1,0,0,0) = 14;      kernel(1,1,0,0) = 0;       kernel(1,2,0,0) = 16;
        //kernel(2,0,0,0) = 17;      kernel(2,1,0,0) = 18;      kernel(2,2,0,0) = 19;

        Func conv("conv");
        RDom r(0, ksize,
               0, ksize,
               0, k_z);

        conv(x, y, w) = cast<uint16_t>(0);

        Func hw_input("hw_input");
        Func clamp_input("clamp_input");
        //clamp_input(x, y, z) = input(clamp(x, 0, width - 1), clamp(y, 0, height - 1), z);
        //clamp_input(z, x, y) = input(z, clamp(x, 0, width - 1), clamp(y, 0, height - 1));
        clamp_input(z, x, y) = input(z, x, y);

        Func hw_kernel;

        Func kernel_copy, input_copy;
        hw_kernel(z, w, x, y) = cast<uint16_t>(kernel(z, w, x, y));
        //kernel_copy(z, w, x, y) = cast<uint16_t>(kernel(z, w, x, y));
        //hw_kernel(z, w, x, y) = kernel_copy(z, w, x, y);
        hw_input(z, x, y) = cast<uint16_t>(clamp_input(z, x, y));
        //input_copy(z, x, y) = cast<uint16_t>(clamp_input(z, x, y));
        //hw_input(z, x, y) = input_copy(z, x, y);

        conv(x, y, w) += hw_kernel(r.z, w, r.x, r.y) * hw_input(r.z, x + r.x, y + r.y);


        Func hw_output("hw_output");
        //hw_output(w, x, y) = cast<uint8_t>(conv(w, x, y));
        hw_output(x, y, w) = conv(x, y, w);
        //output(w, x, y) = max(0, hw_output(w, x, y));
        output(x, y, w) = cast<uint8_t>(max(0, hw_output(x, y, w)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo, w
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, k_w);
          hw_output.bound(w, 0, k_w);
          //clamp_input.bound(z, 0, k_z);
          //kernel.bound(w, 0, k_w);
          hw_kernel.bound(z, 0, k_z);
          hw_input.bound(z, 0, k_z);
          hw_kernel.bound(w, 0, k_w);
          
          Var xi,yi, xo,yo;
          
          hw_input.compute_at(hw_output, xo);
          hw_kernel.compute_at(hw_output, xo);
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_w); 
            //   order reorder from inner to outermost
            //.reorder(xi,w,yi,xo,yo);
            .reorder(xi,yi,w,xo,yo);
            //.reorder(xi,w,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
          //.reorder_storage(x,y,w).reorder(xi,yi,xo,yo,w);
            //.reorder(w,xi,yi,xo,yo);

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
            //.reorder(r.z,r.x,r.y,w,x,y);
            //.reorder(r.x,r.y,r.z,w,x,y);
            //.reorder(w,r.x,r.y,r.z,x,y);
            //.reorder(r.z,w,r.x,r.y,x,y);
            //.reorder(r.z,r.x,r.y,w,x,y);
            //.reorder(r.z,r.x,r.y,x,y,w);
            //.reorder(x,y,r.z,r.x,r.y,w);

            //.reorder(r.x,r.y,r.z,w,x,y);
            .reorder(w,r.z,x,y,r.x,r.y);
            //.reorder(r.y,y,w,r.z,r.x,r.z,y);
            //.reorder(x,y,w,r.z,r.x,r.y);
          
          conv.update()
            //.unroll(r.z, k_z/2);                     // unroll input channel
            //.unroll(r.z);                            // unroll input channel
            //.unroll(r.z).unroll(r.x);                // unroll input channel, x dim
            //.unroll(r.z).unroll(r.x).unroll(r.y);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 2);    // unroll all rdoms, x2
            //.unroll(r.x, ksize).unroll(r.y, ksize);  // unroll conv
            //.unroll(r.x, ksize);                     // unroll conv x
            //.unroll(w, k_w);                         // unroll output channel
            //.unroll(w, ksize).unroll(r.z, ksize);    // unroll channels
            //.unroll(x, ksize).unroll(y, ksize);    // unroll output values
            //.unroll(w, k_w).unroll();                // unroll for multiple memories?
            //.unroll(r.x).unroll(r.y).unroll(r.z);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z, k_z/2);       // unroll all rdoms, partial for r.z
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 4); // unroll all rdoms, x4

            //.unroll(r.x, ksize).unroll(r.y, ksize);  // weight stationary
            .unroll(w, k_w).unroll(r.z, k_z);          // channel weight stationary
            //.unroll(y, ksize).unroll(r.y, ksize);    // row stationary
            //.unroll(x, ksize).unroll(y, ksize);      // output stationary


          conv.linebuffer();

          //kernel.compute_at(hw_output, xo).unroll(x).unroll(y).unroll(z).unroll(w);
          
          //hw_input.unroll(x, 3);
          hw_kernel.stream_to_accelerator();
          hw_input.stream_to_accelerator();
          //hw_kernel.unroll(w, ksize);
          //hw_input.unroll(x, ksize);
          //hw_output.unroll(w, ksize);

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
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
          hw_output.unroll(w);
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_w); 
            //   order reorder from inner to outermost
            //.reorder(xi,w,yi,xo,yo);
            .reorder(xi,yi,w,xo,yo);
            //.reorder(xi,w,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
          //.reorder_storage(x,y,w).reorder(xi,yi,xo,yo,w);
            //.reorder(w,xi,yi,xo,yo);

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
            //.reorder(r.z,r.x,r.y,w,x,y);
            //.reorder(r.x,r.y,r.z,w,x,y);
            //.reorder(w,r.x,r.y,r.z,x,y);
            //.reorder(r.z,w,r.x,r.y,x,y);
            //.reorder(r.z,r.x,r.y,w,x,y);
            //.reorder(r.z,r.x,r.y,x,y,w);
            //.reorder(x,y,r.z,r.x,r.y,w);

            //.reorder(r.x,r.y,r.z,w,x,y);
            .reorder(w,r.z,x,y,r.x,r.y);
            //.reorder(r.y,y,w,r.z,r.x,r.z,y);
            //.reorder(x,y,w,r.z,r.x,r.y);

          conv.unroll(w, k_w);
          conv.update()
            //.unroll(r.z, k_z/2);                     // unroll input channel
            //.unroll(r.z);                            // unroll input channel
            //.unroll(r.z).unroll(r.x);                // unroll input channel, x dim
            //.unroll(r.z).unroll(r.x).unroll(r.y);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 2);    // unroll all rdoms, x2
            //.unroll(r.x, ksize).unroll(r.y, ksize);  // unroll conv
            //.unroll(r.x, ksize);                     // unroll conv x
            //.unroll(w, k_w);                         // unroll output channel
            //.unroll(w, ksize).unroll(r.z, ksize);    // unroll channels
            //.unroll(x, ksize).unroll(y, ksize);    // unroll output values
            //.unroll(w, k_w).unroll();                // unroll for multiple memories?
            //.unroll(r.x).unroll(r.y).unroll(r.z);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z, k_z/2);       // unroll all rdoms, partial for r.z
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 4); // unroll all rdoms, x4

            //.unroll(r.x, ksize).unroll(r.y, ksize);  // weight stationary
            .unroll(w, k_w).unroll(r.z, k_z);          // channel weight stationary
            //.unroll(y, ksize).unroll(r.y, ksize);    // row stationary
            //.unroll(x, ksize).unroll(y, ksize);      // output stationary


          conv.compute_at(hw_output, xo);
          
          //hw_input.compute_at(hw_output, xo);
          //hw_kernel.compute_at(hw_output, xo);
          //input_copy.compute_root();
          //kernel_copy.compute_root();
          //hw_input.unroll(z);
          hw_input.stream_to_accelerator();
          hw_kernel.stream_to_accelerator();
          
        } else {  // schedule to CPU
          conv.compute_root();
          conv.update()
            .unroll(r.x, 3)
            .unroll(r.y, 3);
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetKernel, resnet)
