/*
 * An application for applying a blur.
 * It uses a separable blur.
 */

#include "Halide.h"

namespace {

using namespace Halide;

class Blur : public Halide::Generator<Blur> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
  
    GeneratorParam<uint8_t> schedule{"schedule", 0};    // default: 0

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        const int blockSize = 3;
        RDom win_x(0, blockSize);
        RDom win_y(0, blockSize);

        Func hw_input;
        hw_input(x, y) = cast<uint16_t>(input(x, y));

        Func blur_x, blur_y;
        //blur_x(x, y) = (hw_input(x, y) + hw_input(x+1, y) + hw_input(x+2, y))/3;
        //blur_y(x, y) = (blur_x(x, y) + blur_x(x, y+1) + blur_x(x, y+2))/3;
        blur_x(x, y) += (hw_input(x + win_x.x, y)) / blockSize;
        blur_y(x, y) += (blur_x(x, y + win_y.x)) / blockSize;

        Func hw_output;
        hw_output(x, y) = blur_y(x, y);
        output(x, y) = cast<uint8_t>( hw_output(x, y) );

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {


        } else if (get_target().has_feature(Target::Clockwork)) {

          if (schedule == 1) {
            
          } else {
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            const int tileSize = outputSize; // single tile
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.compute_root()
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            blur_y.compute_at(hw_output, xo);
            blur_y.update()
              .unroll(win_y.x);

            blur_x.compute_at(hw_output, xo);
            blur_x.update()
              .unroll(win_x.x);

            hw_input.stream_to_accelerator();
          }
          
        } else {    // schedule to CPU
          
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Blur, blur)

