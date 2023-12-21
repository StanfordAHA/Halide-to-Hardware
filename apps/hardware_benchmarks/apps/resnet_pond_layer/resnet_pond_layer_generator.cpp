#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class ResnetKernel : public Halide::Generator<ResnetKernel> {
public:
    Input<Buffer<int16_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  kernel{"kernel", 4};
    Output<Buffer<int16_t>> output{"output", 3};

    // in_img determines the input image size
    GeneratorParam<int> in_img{"in_img", 28};    // default: 28

    // pad determines the padding to the input image size
    GeneratorParam<int> pad{"pad", 1};    // default: 1
  
    // ksize determines the output stencil size
    GeneratorParam<int> ksize{"ksize", 3};    // default: 3
  
    // Stride determines the sampling rate for the down sample
    GeneratorParam<int>  stride{"stride", 1};  // default: 1

    // k_ic determines the number of input channels
    GeneratorParam<int> k_ic{"k_ic", 8};    // default: 8
    //GeneratorParam<int> k_ic{"k_ic", 4};    // default: 4
    //GeneratorParam<int> k_ic{"k_ic", 1};    // default: 1
  
    // k_oc determines the number of channel sizes
    //GeneratorParam<int> k_oc{"k_oc", 6};    // default: 6
    GeneratorParam<int> k_oc{"k_oc", 3};    // default: 3
    //GeneratorParam<int> k_oc{"k_oc", 1};    // default: 1

    void generate() {
        //int imgsize = (in_img + 0 - ksize + 1) / stride;
        int imgsize = floor( (in_img + 2*pad - ksize) / stride ) + 1;
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        //Expr height = input.dim(0).extent();
        //Expr width = input.dim(1).extent();
        //Expr k_z = kernel.dim(2).extent();
        Expr height = imgsize;
        Expr width = imgsize;

        Func conv("conv");
        RDom r(0, ksize,
               0, ksize,
               0, k_ic);

        conv(x, y, w) = i16(0);

        Func hw_input("hw_input");
        Func clamp_input("clamp_input");
        //clamp_input(x, y, z) = input(clamp(x, 0, width - 1), clamp(y, 0, height - 1), z);
        clamp_input(z, x, y) = input(z, clamp(x-pad, 0, in_img - 1), clamp(y-pad, 0, in_img - 1));
        //clamp_input(z, x, y) = input(z, x, y);

        Func hw_kernel;
        Func kernel_copy, input_copy;
        //kernel_copy(z, w, x, y) = cast<uint16_t>(kernel(z, w, x, y));
        //hw_kernel(z, w, x, y) = kernel_copy(z, w, x, y);
        hw_kernel(z, w, x, y) = i16(kernel(z, w, x, y));

        //input_copy(z, x, y) = cast<uint16_t>(clamp_input(z, x, y));
        //hw_input(z, x, y) = input_copy(z, x, y);
        hw_input(z, x, y) = i16(clamp_input(z, x, y));

        conv(x, y, w) += hw_kernel(r.z, w, r.x, r.y) * hw_input(r.z, stride*x + r.x, stride*y + r.y);
        //conv(x, y, w) += hw_input(r.z, stride*x, stride*y);

        Func hw_output("hw_output");
        hw_output(x, y, w) = i16(max(0, conv(x, y, w)));
        //output(x, y, w) = max(0, u8(hw_output(x, y, w)));
        //output(x, y, w) = max(0, i16(hw_output(x, y, w)));
        output(x, y, w) = hw_output(x, y, w);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo, w
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, k_oc);
          hw_output.bound(w, 0, k_oc);
          //clamp_input.bound(z, 0, k_ic);
          //kernel.bound(w, 0, k_oc);
          hw_kernel.bound(z, 0, k_ic);
          hw_input.bound(z, 0, k_ic);
          hw_kernel.bound(w, 0, k_oc);
          
          Var xi,yi, xo,yo;
          
          hw_input.compute_at(hw_output, xo);
          hw_kernel.compute_at(hw_output, xo);
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_oc); 
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
          //.unroll(z, k_ic);
          
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
            //.reorder(w,r.z,r.x,r.y,x,y);
          
          conv.update()
            //.unroll(r.z, k_ic/2);                     // unroll input channel
            //.unroll(r.z);                            // unroll input channel
            //.unroll(r.z).unroll(r.x);                // unroll input channel, x dim
            //.unroll(r.z).unroll(r.x).unroll(r.y);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 2);    // unroll all rdoms, x2
            //.unroll(r.x, ksize).unroll(r.y, ksize);  // unroll conv
            //.unroll(r.x, ksize);                     // unroll conv x
            //.unroll(w, k_oc);                         // unroll output channel
            //.unroll(w, ksize).unroll(r.z, ksize);    // unroll channels
            //.unroll(x, ksize).unroll(y, ksize);    // unroll output values
            //.unroll(w, k_oc).unroll();                // unroll for multiple memories?
            //.unroll(r.x).unroll(r.y).unroll(r.z);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z, k_ic/2);       // unroll all rdoms, partial for r.z
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 4); // unroll all rdoms, x4

            //.unroll(r.x, ksize).unroll(r.y, ksize);  // weight stationary
            .unroll(w, k_oc).unroll(r.z, k_ic);          // channel weight stationary
            //.unroll(y, ksize).unroll(r.y, ksize);    // row stationary
            //.unroll(x, ksize).unroll(y, ksize);      // output stationary
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(w, k_oc); // all


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
          output.bound(w, 0, k_oc);
          hw_output.bound(w, 0, k_oc);
          //clamp_input.bound(z, 0, k_ic);
          //kernel.bound(w, 0, k_oc);
          hw_kernel.bound(z, 0, k_ic);
          hw_input.bound(z, 0, k_ic);
          //hw_kernel.bound(w, 0, k_oc);
          
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();
          //hw_output.unroll(w, 2);
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_oc); 
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
          //.unroll(z, k_ic);
          
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

          conv.unroll(w, k_oc);
          conv.update()
            //.unroll(r.z, k_ic/2);                     // unroll input channel
            //.unroll(r.z);                            // unroll input channel
            //.unroll(r.z).unroll(r.x);                // unroll input channel, x dim
            //.unroll(r.z).unroll(r.x).unroll(r.y);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 2);    // unroll all rdoms, x2
            //.unroll(r.x, ksize).unroll(r.y, ksize);  // unroll conv
            //.unroll(r.x, ksize);                     // unroll conv x
            //.unroll(w, k_oc);                         // unroll output channel
            //.unroll(w, ksize).unroll(r.z, ksize);    // unroll channels
            //.unroll(x, ksize).unroll(y, ksize);    // unroll output values
            //.unroll(w, k_oc).unroll();                // unroll for multiple memories?
            //.unroll(r.x).unroll(r.y).unroll(r.z);    // unroll all rdoms
            //.unroll(r.x).unroll(r.y).unroll(r.z, k_ic/2);       // unroll all rdoms, partial for r.z
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 4); // unroll all rdoms, x4

            //.unroll(r.x, ksize).unroll(r.y, ksize);  // weight stationary
            .unroll(w, k_oc).unroll(r.z, k_ic);          // channel weight stationary
            //.unroll(y, ksize).unroll(r.y, ksize);    // row stationary
            //.unroll(x, ksize).unroll(y, ksize);      // output stationary

          conv.compute_at(hw_output, xo);

          //hw_input.in().unroll(z);
          
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

HALIDE_REGISTER_GENERATOR(ResnetKernel, resnet_pond_layer)
