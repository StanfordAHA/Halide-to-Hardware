/*
 * An application for applying a Paper blur.
 * It uses a 3x3 stencil with constant weights.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 3;

class Blur : public Halide::Generator<Blur> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
  
    GeneratorParam<uint8_t> schedule{"schedule", 0};    // default: 0

  //Input<int32_t> tilesize{"tilesize", 64, 8, 128}; // default 64. bounded between 8 and 128
  //int tilesize = imgSize / 2;

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // Create a reduction domain of the correct bounds.
        RDom win(0, blockSize, 0, blockSize);

        Func hw_input, input_copy;
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        //input_copy(x, y) = cast<uint16_t>(input(x, y));
        //hw_input(x, y) = input_copy(x, y);

        // create the gaussia nkernel
        Func kernel_f;
        float sigma = 1.5f;
        kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);

        // create a normalized set of 8bit weights
        Func kernel;
        Expr sum_kernel[blockSize*blockSize];
        for (int idx=0, i=0; i<blockSize; ++i) {
          for (int j=0; j<blockSize; ++j) {
            if (i==0 && j==0) {
              sum_kernel[idx] = kernel_f(i-blockSize/2) * kernel_f(j-blockSize/2);
            } else {
              sum_kernel[idx] = kernel_f(i-blockSize/2) * kernel_f(j-blockSize/2) + sum_kernel[idx-1];
            }
            idx++;
          }
        }
        //kernel(x) = cast<uint16_t>(kernel_f(x) * 64 / sum_kernel[blockSize-1]);
        //kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
        //                             sum_kernel[blockSize*blockSize-1]);
        kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
                                     sum_kernel[blockSize*blockSize-1]);

        // Use a 2D filter to blur the input
        Func blur_unnormalized, blur;
        blur_unnormalized(x, y) = cast<uint16_t>(0);
        //blur_unnormalized(x, y) += cast<uint16_t>( kernel(win.x, win.y) * hw_input(x+win.x, y+win.y) );
        blur_unnormalized(x, y) += kernel(win.x, win.y) * hw_input(x+win.x, y+win.y);
        //blur(x, y) = blur_unnormalized(x, y) / 256;
        blur(x, y) += kernel(win.x, win.y) * hw_input(x+win.x, y+win.y);

        Func blur_x, blur_y;
        blur_x(x, y) = (input(x, y) + input(x+1, y) + input(x+2, y))/3;
        blur_y(x, y) = (blur_x(x, y) + blur_x(x, y+1) + blur_x(x, y+2))/3;

        Func hw_output;
        hw_output(x, y) = blur(x, y);
        //hw_output(x, y) = blur_y(x, y);
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

            blur.in().compute_root();

            blur.in()
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            blur.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur.compute_at(blur.in(), xo);

            hw_input.stream_to_accelerator();
          }
          
        } else {    // schedule to CPU
          if (schedule == 1 || schedule == 2 || schedule == 3) {
            output
              .split(y, y, yi, 32)
              .parallel(y)
              .vectorize(x, 16);
            blur
              .compute_at(output, y)
              .split(y, y, yi, 32)
              .vectorize(x, 16);
          }
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Blur, blur)

