#include "Halide.h"

namespace {

using namespace Halide;

class NearestNeighborKernel : public Halide::Generator<NearestNeighborKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */
        int factor = 2;
        Expr width = input.dim(0).extent();
        Expr height = input.dim(1).extent();

        Var x("x"), y("y"), z("z");

        Func nearest_neighbor("nearest_neighbor");

        Func input_copy, hw_input("hw_input");
        //input_copy(x, y, z) = cast<uint16_t>(input(x, y, z));
        //hw_input(x, y, z) = input_copy(x, y, z);
        hw_input(x, y, z) = cast<uint16_t>(input(x, y, z));

        nearest_neighbor(x, y, z) = hw_input(x / factor, y / factor, z);

        Func hw_output("hw_output");
        hw_output(x, y, z) = nearest_neighbor(x, y, z);
        output(x, y, z) = cast<uint8_t>(hw_output(x, y, z));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
            nearest_neighbor(x, 0, 64);
            nearest_neighbor(y, 0, 64);
            nearest_neighbor(z, 0, 4);
            output.bound(x, 0, 64);
            output.bound(y, 0, 64);
            output.bound(z, 0, 4);
            
            Var xi, yi, xo, yo;
            hw_input.compute_root();
            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 64, 64)
              .reorder(xi,yi,z,xo,yo)
              .hw_accelerate(xi, xo);

            nearest_neighbor.linebuffer();

            hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
            output.bound(x, 0, 128);
            output.bound(y, 0, 128);
            output.bound(z, 0, 1);
            
            Var xi, yi, xo, yo;

            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 128, 128)
              .hw_accelerate(xi, xo)
              .reorder(xi,yi,z,xo,yo);

            nearest_neighbor.compute_at(hw_output, xo);

            hw_input.compute_at(hw_output, xo);
            hw_input.stream_to_accelerator();
            input_copy.compute_root();
            
        } else { // schedule to CPU
            nearest_neighbor.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(NearestNeighborKernel, up_sample)
