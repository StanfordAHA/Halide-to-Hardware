/*
 * An application for sharpening an image. A blurred version
 * is subtracted from the original image.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
int blockSize = 5;
  
class UnsharpFilter : public Halide::Generator<UnsharpFilter> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        /* THE ALGORITHM */
        Var c("c"), x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // create the gaussian kernel
        Func kernel_f;
        float sigma = 1.5f;
        kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);

        // create a normalized set of 8bit weights
        Func kernel;
        Expr sum_kernel[blockSize];
        for (int i=0; i<blockSize; ++i) {
          if (i==0) {
            sum_kernel[i] = kernel_f(i-blockSize/2);
          } else {
            sum_kernel[i] = kernel_f(i-blockSize/2) + sum_kernel[i-1];
          }
        }
        kernel(x) = cast<uint16_t>(kernel_f(x) * 255 / sum_kernel[blockSize-1]);

        // create the input
        Func hw_input;
        hw_input(c, x, y) = cast<uint16_t>(input(x+blockSize/2, y+blockSize/2, c));

        // create a grayscale image
        Func gray;
        gray(x, y) = cast<uint8_t>((77 * cast<uint16_t>(hw_input(0, x, y))
                                    + 150 * cast<uint16_t>(hw_input(1, x, y))
                                    + 29 * cast<uint16_t>(hw_input(2, x, y))) >> 8);
        
        // Use a 2D filter to blur the input
        Func blur_unnormalized, blur;
        RDom win(-blockSize/2, blockSize, -blockSize/2, blockSize);
        blur_unnormalized(x, y) += cast<uint16_t>( kernel(win.x) * gray(x+win.x, y+win.y) );
        blur(x, y) = blur_unnormalized(x, y) / 256 / 256;

        // sharpen the image by subtracting the blurred image
        Func sharpen;
        sharpen(x, y) = cast<uint8_t>(clamp(2 * cast<uint16_t>(gray(x, y)) - blur(x, y), 0, 255));

        // find the ratio of sharpened and original image
        Func ratio;
        ratio(x, y) = cast<uint8_t>(clamp(cast<uint16_t>(sharpen(x, y)) * 32 / max(gray(x, y), 1), 0, 255));

        // Use the ratio to sharpen the input image.
        Func hw_output;
        hw_output(c, x, y) = cast<uint8_t>(clamp(cast<uint16_t>(ratio(x, y)) * hw_input(c, x, y) / 32, 0, 255));
                
        output(c, x, y) = hw_output(c, x, y);
        output.bound(c, 0, 3);
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          hw_input.compute_root();
          hw_output.compute_root();
          
          output.tile(x, y, xo, yo, xi, yi, 480, 640).reorder(c, xi, yi, xo, yo);

          hw_output.tile(x, y, xo, yo, xi, yi, 480, 640).reorder(c, xi, yi, xo, yo);
          blur_unnormalized.update().unroll(win.x).unroll(win.y);

          hw_output.accelerate({hw_input}, xi, xo);
          gray.linebuffer().fifo_depth(ratio, 20);
          //blur_y.linebuffer();
          ratio.linebuffer();
          hw_output.unroll(c);  // hw output bound
          hw_input.fifo_depth(hw_output, 480*9); // hw input bounds

        } else {    // schedule to CPU
          output.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnsharpFilter, unsharp)

