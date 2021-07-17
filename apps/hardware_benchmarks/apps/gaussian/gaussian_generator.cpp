/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 3;

class GaussianBlur : public Halide::Generator<GaussianBlur> {
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
        blur(x, y) = blur_unnormalized(x, y) / 256;

        Func hw_output;
        hw_output(x, y) = blur(x, y);
        output(x, y) = cast<uint8_t>( hw_output(x, y) );

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {


        } else if (get_target().has_feature(Target::Clockwork)) {

          if (schedule == 1) {
            // use global buffer and large input image
            const int tileSize = 62;
            const int numTiles = 2;
            const int glbSize = tileSize * numTiles;
            const int numHostTiles = 5;
            const int outputSize = numHostTiles * glbSize;
            const int inputSize = outputSize + blockSize-1;
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.in().compute_root();
            hw_output.in().store_in(MemoryType::GLB);

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
              .hw_accelerate(xi, xo);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize);
            hw_output.compute_at(hw_output.in(), xo);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);

            hw_input.in().in().compute_at(hw_output, xo);
            
            hw_input.in().compute_at(hw_output.in(), xo);
            hw_input.in().store_in(MemoryType::GLB);
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 2) {
            // Perform host tiliing
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            const int tileSize = outputSize / 2; // four small tiles
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);
            
            hw_input.stream_to_accelerator();
            
          } else {
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            const int tileSize = outputSize; // single tile
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            //output.bound(x, 0, tileSize);
            //output.bound(y, 0, tileSize);

            hw_output.compute_root();

            hw_output
              //.tile(x, y, xo, yo, xi, yi, outputSize, outputSize)
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);

            hw_input.stream_to_accelerator();//.compute_root();//at(output, xo);
            //hw_input.compute_root();
          
            //hw_input.compute_at(hw_output, xo);
            //input_copy.stream_to_accelerator();
            //input_copy.compute_root();

          }
          
        } else {    // schedule to CPU

          /*output.tile(x, y, xo, yo, xi, yi, outputSize, outputSize)
            .vectorize(xi, 8)
            .fuse(xo, yo, xo)
            .parallel(xo);*/

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GaussianBlur, gaussian)

