#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

class Addone : public Halide::Generator<Addone> {
public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        Func hw_output("hw_output");

        hw_input(x, y) = cast<uint16_t>(input(x, y));
        hw_output(x, y) = hw_input(x, y) + 1;
        output(x, y) = hw_output(x,y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 256);
          output.bound(y, 0, 256);

          hw_output.compute_root();

          hw_output
              .tile(x, y, xo, yo, xi, yi, 256, 256)
              .hw_accelerate(xi, xo);

          hw_input.stream_to_accelerator();
            
        } else {  // schedule to CPU
            output.compute_root();
        }

    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Addone, addone)
