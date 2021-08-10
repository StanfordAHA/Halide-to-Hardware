#include "Halide.h"

namespace {

using namespace Halide;

class NearestNeighborKernel : public Halide::Generator<NearestNeighborKernel> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    GeneratorParam<uint8_t> schedule{"schedule", 0};    // default: 0

    void generate() {
        /* THE ALGORITHM */
        int factor = 2;
        Expr width = input.dim(0).extent();
        Expr height = input.dim(1).extent();

        Var x("x"), y("y"), z("z");

        Func nearest_neighbor("nearest_neighbor");

        Func input_copy, hw_input("hw_input");
        //input_copy(z, x, y) = cast<uint16_t>(input(z, x, y));
        //hw_input(z, x, y) = input_copy(z, x, y);
        hw_input(z, x, y) = cast<uint16_t>(input(x, y, z));

        nearest_neighbor(z, x, y) = hw_input(z, x / factor, y / factor);

        Func hw_output("hw_output");
        hw_output(z, x, y) = nearest_neighbor(z, x, y);
        output(x, y, z) = cast<uint8_t>(hw_output(z, x, y));

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
          Var xi, yi, xo, yo;
          
          if (schedule == 1) { // host and glb tiling
            const int blockSize = 1;
            const int tileSize = 128;
            const int numHostTiles = 5;
            const int numTiles = 2;
            const int glbSize = tileSize * numTiles;
            const int outputSize = numHostTiles * glbSize;
            const int inputSize = outputSize + blockSize-1;

            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);
            output.bound(z, 0, 1);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
              .reorder(xi, yi, z, xo, yo)
              .hw_accelerate(xi, xo);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .reorder(xi, yi, z, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);

            nearest_neighbor.compute_at(hw_output, xo);

            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 2) { // big color parrot
            const int tileWidth = 128;
            const int tileHeight = 208;
            const int numHostTiles = 24;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTiles * glbWidth;
            const int outputHeight = numHostTiles * glbHeight;

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);
            output.bound(z, 0, 3);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .reorder(z, xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(z);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .reorder(z, xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(z);

            nearest_neighbor.compute_at(hw_output, xo);
            nearest_neighbor
              .unroll(z);

            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in()
              .unroll(z); 
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 3) { // big color parrot with unroll
            const int unroll = 2;
            const int tileWidth = 128;
            const int tileHeight = 256;
            const int numHostTilesX = 24;
            const int numHostTilesY = 20;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);
            output.bound(z, 0, 3);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .reorder(z, xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(z)
              .unroll(xi, unroll);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .reorder(z, xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(z)
              .unroll(xi, unroll);

            nearest_neighbor.compute_at(hw_output, xo);
            nearest_neighbor
              .unroll(z)
              .unroll(x, unroll);

            hw_input.in().in().compute_at(hw_output, xo); // represents the mem tile
            hw_input.in().in()
              .unroll(z);
            //.unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in()
              .unroll(z);
            //.unroll(x, unroll); 
            
            hw_input.compute_root()
              .accelerator_input();
            
          } else {
            output.bound(x, 0, 128);
            output.bound(y, 0, 128);
            output.bound(z, 0, 1);

            hw_output.compute_root();

            hw_output.tile(x, y, xo, yo, xi, yi, 128, 128)
              .hw_accelerate(xi, xo)
              .reorder(xi,yi,z,xo,yo);

            nearest_neighbor.compute_at(hw_output, xo);

            hw_input.compute_at(hw_output, xo);
            hw_input.stream_to_accelerator();
            
            input_copy.compute_root();
          }
            
        } else { // schedule to CPU
          if (schedule == 1 || schedule == 2 || schedule == 3) {
            Var yi;
            output
              .split(y, y, yi, 32)
              .parallel(y)
              .vectorize(x, 16);
            nearest_neighbor
              .compute_at(output, y)
              .split(y, y, yi, 32)
              .vectorize(x, 16);
          }
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(NearestNeighborKernel, up_sample)
