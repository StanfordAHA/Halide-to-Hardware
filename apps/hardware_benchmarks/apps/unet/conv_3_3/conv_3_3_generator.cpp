#include "Halide.h"

namespace {

using namespace Halide;

class Convolution3x3Kernel : public Halide::Generator<Convolution3x3Kernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
  //Input<Buffer<uint8_t>>  kernel{"kernel", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), z("z"), w("w");

        Expr height = input.dim(0).extent();
        Expr width = input.dim(1).extent();
        //Expr k_z = kernel.dim(2).extent();

        Expr k_z = 4;
        Func kernel;
        kernel(x,y,z,w) = 0;
        kernel(0,0,0,0) = 11;      kernel(0,1,0,0) = 12;      kernel(0,2,0,0) = 13;
        kernel(1,0,0,0) = 14;      kernel(1,1,0,0) = 0;       kernel(1,2,0,0) = 16;
        kernel(2,0,0,0) = 17;      kernel(2,1,0,0) = 18;      kernel(2,2,0,0) = 19;

        Func conv("conv");
        RDom r(0, 3,
               0, 3,
               0, k_z);

        conv(x, y, w) = 0;

        Func hw_input("hw_input");
        Func clamp_input("clamp_input");
        clamp_input(x, y, z) = input(clamp(x, 0, width - 1), clamp(y, 0, height - 1), z);

        Func kernel_copy;
        kernel_copy(x, y, z, w) = kernel(x, y, z, w);
        hw_input(x, y, z) = cast<uint16_t>(clamp_input(x, y, z));
        conv(x, y, w) += kernel_copy(r.x, r.y, r.z, w) * hw_input(x + r.x, y + r.y, r.z);

        Func hw_output("hw_output");
        hw_output(x, y, w) = cast<uint8_t>(conv(x, y, w));
        output(x, y, w) = max(0, hw_output(x, y, w));

        output.bound(x, 0, 62);
        output.bound(y, 0, 62);
        output.bound(w, 0, 4);
        //clamp_input.bound(z, 0, 2);
        //kernel_copy.bound(z, 0, 2);
        //kernel_copy.bound(w, 0, 4);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 62, 62)
            .reorder(w,xi,yi,xo,yo) // order from inner to outermost
            .reorder_storage(w,x,y)
            .hw_accelerate(w, xo);

          conv.reorder(w,x,y)
            .reorder_storage(w,x,y);

          conv.update()
            .reorder(w,r.y,r.z,x,y);

          
          conv.update()
            .unroll(r.x, 3)
            //.unroll(r.y, 3);
            //.unroll(r.z, 4);
            .unroll(w, 4);

          conv.linebuffer();

          kernel.compute_at(hw_output, xo);
          
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
