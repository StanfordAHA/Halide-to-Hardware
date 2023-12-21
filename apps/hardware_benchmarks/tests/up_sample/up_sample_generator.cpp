#include "Halide.h"

namespace {

using namespace Halide;

class NearestNeighborKernel : public Halide::Generator<NearestNeighborKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */
        int factor = 2;

        Var x("x"), y("y");

        Func nearest_neighbor("nearest_neighbor");

        Func hw_input("hw_input");
        hw_input(x, y) = cast<uint16_t>(input(x+1, y+1));

        //nearest_neighbor(x, y) = hw_input(x / factor, y / factor);
        Func upx, upy;
        upx(x, y, _) = hw_input((x/2) - 1 + 2*(x % 2), y, _) / 4 + 3*hw_input(x/2, y, _) / 4;
        nearest_neighbor(x, y, _) = upx(x, (y/2) - 1 + 2*(y % 2), _) / 4 + 3*upx(x, y/2, _) / 4;

        Func hw_output("hw_output");
        hw_output(x, y) = nearest_neighbor(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
            Var xi, yi, xo, yo;

            int osize = 124;

            output.bound(x, 0, osize);
            output.bound(y, 0, osize);
            
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, osize, osize)
                .hw_accelerate(xi, xo);

            hw_input.stream_to_accelerator();
            
        } else { // schedule to CPU
            nearest_neighbor.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(NearestNeighborKernel, up_sample)
