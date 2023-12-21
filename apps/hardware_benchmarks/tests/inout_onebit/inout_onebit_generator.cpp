#include "Halide.h"

namespace {

using namespace Halide;

class UnitTestInoutOnebit : public Halide::Generator<UnitTestInoutOnebit> {
public:
    Input<Buffer<bool>>  input{"input", 2};
    Output<Buffer<bool>> output{"output", 2};

    int imgsize = 64;
  
    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<bool>(input(x, y));

        Func inv;
        inv(x,y) = !( hw_input(x, y) );

        Func hw_output("hw_output");
        hw_output(x, y) = cast<bool>(inv(x, y));
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64-2, 64-2)
            .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, imgsize);
          output.bound(y, 0, imgsize);
          
          hw_input.compute_root();
          hw_output.compute_root();
          
          hw_output.tile(x,y, xo,yo, xi,yi, 64, 64)
            .hw_accelerate(xi, xo);

          inv.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();
          
        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestInoutOnebit, inout_onebit)
