/*
 * An application for sharpening an image. A blurred version
 * is subtracted from the original image.
 */

#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// Size of blur for gradients.
const int blockSize = 5;
  
class UnsharpFilter : public Halide::Generator<UnsharpFilter> {
public:
    Input<Buffer<uint8_t>>  input{"input", 3};
  Output<Buffer<uint8_t>> output{"output", 3};
  //Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<uint8_t> schedule{"schedule", 0};    // default: 0

    void generate() {
        /* THE ALGORITHM */
        Var c("c"), x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // create the gaussian kernel
        Func kernel_f;
        float sigma = 1.5f;
        kernel_f(x) = exp(-1.f*x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);

        // create a normalized set of 8bit weights
        Func kernel;
        //Expr sum_kernel[blockSize];
        //for (int i=0; i<blockSize; ++i) {
        //  if (i==0) {
        //    sum_kernel[i] = kernel_f(i-blockSize/2);
        //  } else {
        //    sum_kernel[i] = kernel_f(i-blockSize/2) + sum_kernel[i-1];
        //  }
        //}
        //kernel(x) = cast<uint16_t>(kernel_f(x) * 256 / sum_kernel[blockSize-1]);
        
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
        kernel(x,y) = cast<uint16_t>(kernel_f(x) * kernel_f(y) * 256.0f /
                                     sum_kernel[blockSize*blockSize-1]);
        
        //RDom sum_win(-blockSize/2, blockSize, -blockSize/2, blockSize);
        //Expr kernel_sum;
        //kernel_sum = 0.f;
        //kernel_sum += kernel_f(sum_win.x) * kernel_f(sum_win.y);
        //kernel(x,y) = cast<uint16_t>(kernel_f(x) * kernel_f(y) * 256.0f /
        //                             kernel_sum);
                                     
                             
        // create the input
        Func hw_input, input_copy;
        //hw_input(c, x, y) = cast<uint16_t>(input(x, y, c));
        hw_input(c, x, y) = cast<uint16_t>(input(x + blockSize/2, y + blockSize/2, c));

        // create a grayscale image
        Func gray;
        gray(x, y) = cast<uint16_t>((77 * cast<uint16_t>(hw_input(0, x, y))
                                     + 150 * cast<uint16_t>(hw_input(1, x, y))
                                     + 29 * cast<uint16_t>(hw_input(2, x, y))) >> 8);
        
        // Use a 2D filter to blur the input
        Func blur_unnormalized, blur;
        //RDom win(0, blockSize, 0, blockSize);
        RDom win(-blockSize/2, blockSize, -blockSize/2, blockSize);

        //blur_unnormalized(x, y) += cast<uint16_t>( kernel(win.x) * gray(x+win.x, y+win.y) );
        //blur_unnormalized(x, y) += i16( kernel(win.x+blockSize/2, win.y+blockSize/2) * gray(x+win.x, y+win.y) );
        blur_unnormalized(x, y) += u16( kernel(win.x, win.y) * gray(x+win.x, y+win.y) );
        //blur(x, y) = u16( clamp(blur_unnormalized(x, y) / 256, 0, 255) );
        blur(x, y) = u16( clamp(u8(blur_unnormalized(x, y) / 256), 0, 255) );

        // sharpen the image by subtracting the blurred image
        Func sharpen;
        //sharpen(x, y) = u16( clamp(i16(2 * i16(gray(x, y)) - blur(x, y)), i16(0), i16(255)) );
        sharpen(x, y) = u16(clamp(2 * i16(gray(x, y)) - i16(blur(x, y)), 0, 255));

        // find the ratio of sharpened and original image. Leave it in 8.8 format
        Func ratio;
        Func rom_div_lookup;
        rom_div_lookup(x) = u16( u32(1 << 8) / u32(x));

        Func divisor;
        //ratio(x, y) = cast<uint16_t>(clamp(cast<uint16_t>(sharpen(x, y)) * 32 / max(gray(x, y), 1), 0, 255));
        divisor(x,y) = max(gray(x, y), u16(1)); // take max so no div by 0

        Func reciprocal;
        reciprocal(x, y) = rom_div_lookup(divisor(x, y));
        //Expr divide = div_using_lookup(sharpen(x, y), divisor(x,y), rom_div_lookup);
        Expr divide = sharpen(x, y) * reciprocal(x, y);
        
        ratio(x, y) = divide;//clamp(divide, u16(0), u16(255));

        // Use the ratio to sharpen the input image.
        Func hw_output;
        hw_output(c, x, y) = mul1(ratio(x, y), hw_input(c, x, y));
        //hw_output(c, x, y) = divisor(x, y);
        
        output(x, y, c) = u8(hw_output(c, x, y));

        output.bound(c, 0, 3);
        output.bound(x, 0, 64-blockSize+1);
        output.bound(y, 0, 64-blockSize+1);
        
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

          kernel.compute_at(hw_output, xo).unroll(x).unroll(y);
          //kernel.bound(x, -blockSize/2, blockSize);


        } else if (get_target().has_feature(Target::Clockwork)) {
          if (schedule == 1) { // single buffer
            hw_output.compute_root();
            hw_output
              .tile(x, y, xo, yo, xi, yi, 60, 60).reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);

            rom_div_lookup.compute_at(hw_output, xo).unroll(x); // synthesize lookup to a ROM (8.8 output)
            
            kernel.compute_at(blur_unnormalized, x).unroll(x);
            hw_input.stream_to_accelerator();

          } else if (schedule == 2) { // all buffers
            hw_output.compute_root();
            hw_output
              .tile(x, y, xo, yo, xi, yi, 60, 60).reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            
            blur_unnormalized.compute_at(hw_output, xo);
            ratio.compute_at(hw_output, xo);
            gray.compute_at(hw_output, xo);
            kernel.compute_at(blur_unnormalized, x).unroll(x);
            hw_input.stream_to_accelerator();
            
          } else {
            hw_output.compute_root();
          
            hw_output
              .tile(x, y, xo, yo, xi, yi, 60, 60).reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);

            hw_output.unroll(c);  // hw output bound
            hw_input.in().unroll(c);  // hw input bound
            //gray.linebuffer().fifo_depth(ratio, 20);
            //blur_y.linebuffer();
            ratio.compute_at(hw_output, xo);

            reciprocal.compute_at(hw_output, xo); // we don't want this memory
            rom_div_lookup.compute_at(hw_output, xo).unroll(x); // synthesize lookup to a ROM (8.8 output)

            sharpen.compute_at(hw_output, xo);

            blur_unnormalized.compute_at(hw_output, xo);
            blur_unnormalized.update()
              .unroll(win.x).unroll(win.y);
            rom_div_lookup.compute_at(hw_output, xo).unroll(x); // synthesize lookup to a ROM (8.8 output)
            
            hw_output.unroll(c);  // hw output bound
            hw_input.in().unroll(c);  // hw input bound
            //hw_input.fifo_depth(hw_output, 480*9); // hw input bounds
            
            gray.fifo_depth(hw_output, 60*9); // hw input bounds

            kernel.compute_at(hw_output, xo).unroll(x).unroll(y);
            //kernel.compute_at(blur_unnormalized, x).unroll(x).unroll(y);

            gray.compute_at(hw_output, xo);

            //hw_input.fifo_depth(hw_output, 480*9); // hw input bounds
            //hw_input.compute_at(hw_output, xo);
            hw_input.stream_to_accelerator();
            //input_copy.compute_root();
          }

        } else {    // schedule to CPU
          output.compute_root();
        }
    }

  Expr mul1(Expr a, Expr b) {
    return i16(( (i32(a)) * (i32(b)) ) >> 8);
  }
  Expr mul2(Expr a, Expr b) {
    return u16(( (u32(a)) * (u32(b)) ) >> 16);
  }

  // We convert a/b into a*(1/b) where 1/b is found in a lookup table.
  // b is assumed to be in range [0, 255]
  // return value is in 8.8 format
  Expr div_using_lookup(Expr a, Expr b, Func lookup) {    
    return a * lookup(b);
  }
};


}  // namespace

HALIDE_REGISTER_GENERATOR(UnsharpFilter, unsharp)

