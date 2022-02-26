#include "Halide.h"
// in_img=16 pad=1 ksize=3 stride=1 k_ic=2 k_oc=2
namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

int max_rand = pow(2,8) - 1;

class ResnetKernel : public Halide::Generator<ResnetKernel> {
public:
    Input<Buffer<uint16_t>>  input{"input", 3};
    // Input<Buffer<uint16_t>>  kernel{"kernel", 4};
    Output<Buffer<uint16_t>> output{"output", 3};

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
        int imgsize = in_img;
      
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        //Expr height = input.dim(0).extent();
        //Expr width = input.dim(1).extent();
        //Expr k_z = kernel.dim(2).extent();
        Expr height = imgsize;
        Expr width = imgsize;

        Func conv("conv");
        RDom r(0, k_ic);

        conv(x, y, w) = i16(0);

        Func hw_input("hw_input");
        Func clamp_input("clamp_input");
        //clamp_input(x, y, z) = input(clamp(x, 0, width - 1), clamp(y, 0, height - 1), z);
        // clamp_input(z, x, y) = input(z, clamp(x-pad, 0, width - 1), clamp(y-pad, 0, height - 1));
        clamp_input(z, x, y) = input(z, x, y);

        Func kernel;
        kernel(z, w) = 0;

//// BEGIN KERNEL

    // for (int y = 0; y < ksize; y++) {
    //   for (int x = 0; x < ksize; x++) {
        for (int w = 0; w < k_oc; w++) {
          for (int z = 0; z < k_ic; z++) {
            // int rand_i = rand() % 100;
            // kernel(z, w) = rand_i;
            // if (rand_i  < 40) { // 60% zero, else rand
            // } else {
            //   kernel(z, w) = 0;
            // }
            kernel(z, w) = 3;
            
    } } 



//// END KERNEL


        //input_copy(z, x, y) = cast<uint16_t>(clamp_input(z, x, y));
        //hw_input(z, x, y) = input_copy(z, x, y);
        hw_input(z, x, y) = i16(clamp_input(z, x, y));

        conv(x, y, w) += i16(kernel(r.x, w) * hw_input(r.x, x, y));
        //conv(x, y, w) += hw_input(r.z, stride*x, stride*y);

        Func hw_output("hw_output");
        hw_output(x, y, w) = conv(x, y, w);
        //output(x, y, w) = max(0, u8(hw_output(x, y, w)));
        output(x, y, w) = max(0, u16(hw_output(x, y, w)));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
          // loop order: r.z, r.x, r.y, xi, yi, xo, yo, w
          
          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          output.bound(w, 0, k_oc);
          hw_output.bound(w, 0, k_oc);
          hw_input.bound(z, 0, k_ic);
          
          Var xi,yi, xo,yo;
          
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder_storage(w,x,y)
            .hw_accelerate(xi, xo) //.unroll(w, k_oc); 
            //   order reorder from inner to outermost
            //.reorder(xi,w,yi,xo,yo);
            .reorder(xi,yi,w,xo,yo);
          hw_output.unroll(w, k_oc);
            //.reorder(xi,w,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
            //.reorder(w,xi,yi,xo,yo);
          //.reorder_storage(x,y,w).reorder(xi,yi,xo,yo,w);
            //.reorder(w,xi,yi,xo,yo);

          // hw_kernel
          //   .reorder_storage(z,w,x,y)
          //   .reorder(z,w,x,y);
          
          // hw_input
          //   .reorder_storage(z,x,y)
          //   .reorder(z,x,y);
          // //.unroll(z, k_ic);
          kernel.compute_at(hw_output, xo);
          
          // conv.reorder(w,x,y)
          //   .reorder_storage(w,x,y);
          // conv.update()
          //   //.reorder(r.z,r.x,r.y,w,x,y);
          //   //.reorder(r.x,r.y,r.z,w,x,y);
          //   //.reorder(w,r.x,r.y,r.z,x,y);
          //   //.reorder(r.z,w,r.x,r.y,x,y);
          //   //.reorder(r.z,r.x,r.y,w,x,y);
          //   //.reorder(r.z,r.x,r.y,x,y,w);
          //   //.reorder(x,y,r.z,r.x,r.y,w);

          //   //.reorder(r.x,r.y,r.z,w,x,y);
          //   .reorder(w,r.x,x,y);
          //   //.reorder(r.y,y,w,r.z,r.x,r.z,y);
            //.reorder(x,y,w,r.z,r.x,r.y);

          // conv.update().unroll(w, k_oc);
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
            // .unroll(r.x).unroll(r.y).unroll(r.z).unroll(w);    // unroll all rdoms
            // .unroll(r.x).unroll(r.y).unroll(r.z, k_ic/2);       // unroll all rdoms, partial for r.z
            //.unroll(r.x).unroll(r.y).unroll(r.z).unroll(x, 4); // unroll all rdoms, x4
            // .unroll(r.x).unroll(r.y).unroll(r.z);
            //.unroll(r.x, ksize).unroll(r.y, ksize);  // weight stationary
            .unroll(w, k_oc).unroll(r.x, k_ic);          // channel weight stationary
            //.unroll(y, ksize).unroll(r.y, ksize);    // row stationary
            //.unroll(x, ksize).unroll(y, ksize);      // output stationary


          conv.compute_at(hw_output, xo);

          hw_input.in().unroll(z);
          
          hw_input.stream_to_accelerator();
          // hw_kernel.stream_to_accelerator();
          
        } 
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ResnetKernel, resnet_one_input)
