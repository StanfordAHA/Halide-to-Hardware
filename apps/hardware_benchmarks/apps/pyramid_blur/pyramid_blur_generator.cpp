/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 * Applies blur using a pyramid.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 3;
int num_levels = 4;
int imgSize = 4;//64-(blockSize-1)*num_levels;

class PyramidGaussianBlur : public Halide::Generator<PyramidGaussianBlur> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // Create a reduction domain of the correct bounds.
        RDom win(0, blockSize, 0, blockSize);

        Func hw_input, input_copy;
        input_copy(x, y) = cast<uint16_t>(input(x, y));
        hw_input(x, y) = input_copy(x, y);

        // create the gaussian kernel
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
        // normalizes constants so that the sum is 256
        kernel(x,y) = cast<uint32_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
                                     sum_kernel[blockSize*blockSize-1]);

        // Use a 2D filter to blur the input
        //Func blur[num_levels];
        //
        //for (int level=0; level<num_levels; ++level) {
        //  blur[level](x, y) = 0;
        //  if (level == 0) {
        //    blur[level](x, y) += kernel(win.x, win.y) * hw_input(2*x+win.x, 2*y+win.y);
        //  } else {
        //    blur[level](x, y) += kernel(win.x, win.y) * blur[level-1](2*x+win.x, 2*y+win.y);
        //  }
        //  //blur[level](x, y) = blur[level](x, y) / 256;
        //}

        Func blur0, blur1, blur2, blur3;
        blur0(x, y) = kernel(0,0) * hw_input(2*x,2*y) + kernel(1,0) * hw_input(2*x+1,2*y) + kernel(0,1) * hw_input(2*x,2*y+1) + kernel(1,1) * hw_input(2*x+1,2*y+1);
        blur1(x, y) = kernel(0,0) * blur0(2*x,2*y) + kernel(1,0) * blur0(2*x+1,2*y) + kernel(0,1) * blur0(2*x,2*y+1) + kernel(1,1) * blur0(2*x+1,2*y+1);
        blur2(x, y) = kernel(0,0) * blur1(2*x,2*y) + kernel(1,0) * blur1(2*x+1,2*y) + kernel(0,1) * blur1(2*x,2*y+1) + kernel(1,1) * blur1(2*x+1,2*y+1);
        blur3(x, y) = kernel(0,0) * blur2(2*x,2*y) + kernel(1,0) * blur2(2*x+1,2*y) + kernel(0,1) * blur2(2*x,2*y+1) + kernel(1,1) * blur2(2*x+1,2*y+1);

        Func hw_output;
        //hw_output(x, y) = cast<uint8_t>( blur[num_levels-1](x, y) );
        hw_output(x, y) = cast<uint8_t>( blur3(x, y) );
        output(x, y) = hw_output(x, y);

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {


        } else if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, imgSize);
          output.bound(y, 0, imgSize);

          hw_output.compute_root();

          hw_output
            .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .hw_accelerate(xi, xo);

          //for (int level=0; level<num_levels; ++level) {
          //  blur[level].update(0)
          //    .unroll(win.x, blockSize)
          //    .unroll(win.y, blockSize);
          //
          //  //blur[level].compute_at(hw_output, xo);
          //}
          
          blur3.compute_at(hw_output, xo);
          blur3.compute_share_root(blur3, x);
          blur2.compute_share(blur3);
          blur1.compute_share(blur3);
          
          blur0.compute_at(hw_output, xo);

          hw_input.compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          input_copy.compute_root();
          
        } else {    // schedule to CPU

          /*output.tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .vectorize(xi, 8)
            .fuse(xo, yo, xo)
            .parallel(xo);*/

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PyramidGaussianBlur, pyramid_blur)

