/*
 * An application for sharpening an image. A blurred version
 * is subtracted from the original image.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 5;
  
class UnsharpFilter : public Halide::Generator<UnsharpFilter> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
  //Output<Buffer<uint8_t>> output{"output", 3};
    Output<Buffer<uint8_t>> output{"output", 2};

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
        Func hw_input, input_copy;
        //hw_input(c, x, y) = cast<uint16_t>(input(x+blockSize/2, y+blockSize/2, c));
        input_copy(x, y, c) = cast<uint16_t>(input(x, y, c));
        hw_input(x, y, c) = input_copy(x, y, c);

        // create a grayscale image
        Func gray;
        gray(x, y) = cast<uint16_t>((77 * cast<uint16_t>(hw_input(x, y, 0))
                                    + 150 * cast<uint16_t>(hw_input(x, y, 1))
                                    + 29 * cast<uint16_t>(hw_input(x, y, 2))) >> 8);
        
        // Use a 2D filter to blur the input
        Func blur_unnormalized, blur;
        //RDom win(-blockSize/2, blockSize, -blockSize/2, blockSize);
        RDom win(0, blockSize, 0, blockSize);
        blur_unnormalized(x, y) += cast<uint16_t>( kernel(win.x) * gray(x+win.x, y+win.y) );
        blur(x, y) = blur_unnormalized(x, y) / 256;

        // sharpen the image by subtracting the blurred image
        Func sharpen;
        sharpen(x, y) = cast<uint16_t>(clamp(2 * cast<uint16_t>(gray(x, y)) - blur(x, y), 0, 255));

        // find the ratio of sharpened and original image
        Func ratio;
        ratio(x, y) = cast<uint16_t>(clamp(cast<uint16_t>(sharpen(x, y)) * 32 / max(gray(x, y), 1), 0, 255));

        // Use the ratio to sharpen the input image.
        Func hw_output;
        //hw_output(c, x, y) = cast<uint8_t>(clamp(cast<uint16_t>(ratio(x, y)) * hw_input(c, x, y) / 32, 0, 255));
        hw_output(x, y) = cast<uint8_t>(clamp(cast<uint16_t>(ratio(x, y)) * gray(x, y) / 32, 0, 255));
        //hw_output(x, y) = cast<uint8_t>(blur(x, y));

        output(x, y) = hw_output(x, y);
        //output(c, x, y) = hw_output(c, x, y);
        
        //output(x, y, c) = hw_output(x, y);
        //output.bound(c, 0, 3);
        output.bound(x, 0, 60);
        output.bound(y, 0, 60);
        
        //hw_output.bound(c, 0, 1);
        //hw_output.bound(x, 0, 60);
        //hw_output.bound(y, 0, 60);
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) ||
            get_target().has_feature(Target::HLS)) {
          hw_input.compute_root();
          hw_output.compute_root();
          
          //output.tile(x, y, xo, yo, xi, yi, 64, 64).reorder(c, xi, yi, xo, yo);

          hw_output.tile(x, y, xo, yo, xi, yi, 60, 60).reorder(xi, yi, xo, yo);
          blur_unnormalized.linebuffer();
          blur_unnormalized.update()
            .unroll(win.x).unroll(win.y);

          hw_output.accelerate({gray}, xi, xo);
          //gray.linebuffer().fifo_depth(ratio, 20);
          //blur_y.linebuffer();
          ratio.linebuffer();
          //hw_output.unroll(c);  // hw output bound
          //hw_input.unroll(c);  // hw input bound
          //hw_input.fifo_depth(hw_output, 480*9); // hw input bounds
          gray.fifo_depth(hw_output, 60*9); // hw input bounds
          gray.stream_to_accelerator();

          kernel.compute_at(hw_output, xo).unroll(x);
          //kernel.bound(x, -blockSize/2, blockSize);


        } else if (get_target().has_feature(Target::Clockwork)) {

          hw_output.compute_root();
          
          //output.tile(x, y, xo, yo, xi, yi, 64, 64).reorder(c, xi, yi, xo, yo);

          hw_output
              .tile(x, y, xo, yo, xi, yi, 60, 60).reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);

          blur_unnormalized.compute_at(hw_output, xo);
          blur_unnormalized.update()
            .unroll(win.x).unroll(win.y);

          //gray.linebuffer().fifo_depth(ratio, 20);
          //blur_y.linebuffer();
          ratio.compute_at(hw_output, xo);
          //hw_output.unroll(c);  // hw output bound
          //hw_input.unroll(c);  // hw input bound
          //hw_input.fifo_depth(hw_output, 480*9); // hw input bounds
          gray.fifo_depth(hw_output, 60*9); // hw input bounds

          //kernel.compute_at(hw_output, xo).unroll(x);
          //kernel.compute_at(blur_unnormalized, x).unroll(x);
          kernel.compute_at(blur_unnormalized, x).unroll(x);

          gray.compute_at(hw_output, xo);

          //hw_input.compute_root();
          hw_input.compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();

          input_copy.compute_root();

        } else {    // schedule to CPU
          output.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(UnsharpFilter, unsharp)

