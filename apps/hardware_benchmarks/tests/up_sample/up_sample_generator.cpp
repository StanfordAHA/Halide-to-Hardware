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
        hw_input(x, y) = cast<uint16_t>(input(x, y));

        nearest_neighbor(x, y) = hw_input(x / factor, y / factor);

        Func hw_output("hw_output");
        hw_output(x, y) = nearest_neighbor(x, y);
        output(x, y) = cast<uint8_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

        } else if (get_target().has_feature(Target::Clockwork)) {
            Var xi, yi, xo, yo;

            output.bound(x, 0, 128);
            output.bound(y, 0, 128);
            
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 128, 128)
                .hw_accelerate(xi, xo);

            hw_input.stream_to_accelerator();
        } else if (get_target().has_feature(Target::Clockwork)) {
            Var xi, yi, xo, yo;

            output.bound(x, 0, 128);
            output.bound(y, 0, 128);
            
            hw_input.compute_root();
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 128, 128)
                .hw_accelerate(xi, xo);

            nearest_neighbor.compute_at(hw_output, xo);

            hw_input.stream_to_accelerator();
            
        } else { // schedule to CPU
            nearest_neighbor.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(NearestNeighborKernel, up_sample)
