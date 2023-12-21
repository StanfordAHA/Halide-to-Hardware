#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

int width = 64;
int height = width;

class UnitTestBoundary : public Halide::Generator<UnitTestBoundary> {
public:
    Input<Buffer<int8_t>>  input{"input", 2};
    Output<Buffer<int8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");

        Func hw_input("hw_input");
        hw_input(x, y) = i16(input(x, y));

        RDom r(-1, 3, -1, 3);
        Func const0("const0"), const5("const5"), conv("conv");
        const0 = Halide::BoundaryConditions::constant_exterior(hw_input, 0, {{0, width},{0,height}});
        const5 = Halide::BoundaryConditions::constant_exterior(hw_input, 5, {{0, width},{0,height}});
        conv(x, y) += const0(x + r.x, y + r.y) + const5(x + r.x, y + r.y);

        Func hw_output("hw_output");
        hw_output(x, y) = conv(x, y);
        output(x, y) = i8(hw_output(x,y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          
        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          Var xi,yi, xo,yo;

          output.bound(x, 0, 64);
          output.bound(y, 0, 64);
          
          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, width,height)
            .hw_accelerate(xi, xo);

          conv.compute_at(hw_output, xo);
          conv.update()
            .unroll(r.x).unroll(r.y);
          
          const0.compute_at(hw_output, xo);
          const5.compute_at(hw_output, xo);
          
          hw_input.stream_to_accelerator();

        } else {  // schedule to CPU
          output.compute_root();
        }
        
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnitTestBoundary, bc_const)
