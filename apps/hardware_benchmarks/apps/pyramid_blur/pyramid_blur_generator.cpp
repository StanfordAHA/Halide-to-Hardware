/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 * Applies blur using a pyramid.
 */

#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// Size of blur for gradients.
const int blockSize = 2; // this is controlled below, not here
int num_levels = 4;
//int imgSize = 4;//64-(blockSize-1)*num_levels;
int imgSize = 8;

class PyramidGaussianBlur : public Halide::Generator<PyramidGaussianBlur> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<uint8_t> schedule{"schedule", 0};    // default: 0

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // Create a reduction domain of the correct bounds.
        RDom win(0, blockSize, 0, blockSize);

        Func hw_in, input_copy;
        input_copy(x, y) = cast<uint16_t>(input(x, y));
        hw_in(x, y) = input_copy(x, y);

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
        kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
                                     sum_kernel[blockSize*blockSize-1]);

        // Use a 2D filter to blur the input
        Func blur[num_levels];
        for (int i=0; i<num_levels; ++i) {
          blur[i] = Func("blur" + std::to_string(i));
        }
        
        for (int level=0; level<num_levels; ++level) {
          blur[level](x, y) = 0;
          if (level == 0) {
            blur[level](x, y) += kernel(win.x, win.y) * hw_in(2*x+win.x, 2*y+win.y);
          } else {
            blur[level](x, y) += kernel(win.x, win.y) * blur[level-1](2*x+win.x, 2*y+win.y);
          }
          //blur[level](x, y) = blur[level](x, y) / 256;
        }

        Func ha_in;
        ha_in(x, y) = u16(hw_in(x, y));

        Func k;
        k(x, y) = 0;
        k(0, 0) = 1;
        k(0, 1) = 1;
        k(1, 0) = 1;
        k(1, 1) = 1;
        
        Func blur0, blur1, blur2, blur3;
        // 2x2 conv
        //blur0(x, y) = (ha_in(2*x,2*y) + hw_in(2*x+1,2*y) + hw_in(2*x,2*y+1) + hw_in(2*x+1,2*y+1)) / 4;
        //blur1(x, y) = (blur0(2*x,2*y) + blur0(2*x+1,2*y) + blur0(2*x,2*y+1) + blur0(2*x+1,2*y+1)) / 4;
        //blur2(x, y) = (blur1(2*x,2*y) + blur1(2*x+1,2*y) + blur1(2*x,2*y+1) + blur1(2*x+1,2*y+1)) / 4;
        //blur3(x, y) = (blur2(2*x,2*y) + blur2(2*x+1,2*y) + blur2(2*x,2*y+1) + blur2(2*x+1,2*y+1)) / 4;

        // 2x2 conv
        blur0(x, y) = (kernel(0,0) * ha_in(2*x,2*y) + kernel(1,0) * hw_in(2*x+1,2*y) + kernel(0,1) * hw_in(2*x,2*y+1) + kernel(1,1) * hw_in(2*x+1,2*y+1)) / 4;
        blur1(x, y) = (kernel(0,0) * blur0(2*x,2*y) + kernel(1,0) * blur0(2*x+1,2*y) + kernel(0,1) * blur0(2*x,2*y+1) + kernel(1,1) * blur0(2*x+1,2*y+1)) / 4;
        blur2(x, y) = (kernel(0,0) * blur1(2*x,2*y) + kernel(1,0) * blur1(2*x+1,2*y) + kernel(0,1) * blur1(2*x,2*y+1) + kernel(1,1) * blur1(2*x+1,2*y+1)) / 4;
        //blur3(x, y) = (kernel(0,0) * blur2(2*x,2*y) + kernel(1,0) * blur2(2*x+1,2*y) + kernel(0,1) * blur2(2*x,2*y+1) + kernel(1,1) * blur2(2*x+1,2*y+1)) / 4;
        
        // 3x3 conv
        //blur0(x, y) = kernel(0,0) * ha_in(2*x,2*y) + kernel(1,0) * hw_in(2*x+1,2*y) + kernel(0,1) * hw_in(2*x,2*y+1) + kernel(1,1) * hw_in(2*x+1,2*y+1) + kernel(1,2) * hw_in(2*x+1,2*y+2) + kernel(2,1) * hw_in(2*x+2,2*y+1) + kernel(2,2) * hw_in(2*x+2,2*y+2) + kernel(0,2) * hw_in(2*x+0,2*y+2) + kernel(2,0) * hw_in(2*x+2,2*y+0);
        //blur1(x, y) = kernel(0,0) * blur0(2*x,2*y) + kernel(1,0) * blur0(2*x+1,2*y) + kernel(0,1) * blur0(2*x,2*y+1) + kernel(1,1) * blur0(2*x+1,2*y+1) + kernel(1,2) * blur0(2*x+1,2*y+2) + kernel(2,1) * blur0(2*x+2,2*y+1) + kernel(2,2) * blur0(2*x+2,2*y+2) + kernel(0,2) * blur0(2*x+0,2*y+2) + kernel(2,0) * blur0(2*x+2,2*y+0);
        //blur2(x, y) = kernel(0,0) * blur1(2*x,2*y) + kernel(1,0) * blur1(2*x+1,2*y) + kernel(0,1) * blur1(2*x,2*y+1) + kernel(1,1) * blur1(2*x+1,2*y+1) + kernel(1,2) * blur1(2*x+1,2*y+2) + kernel(2,1) * blur1(2*x+2,2*y+1) + kernel(2,2) * blur1(2*x+2,2*y+2) + kernel(0,2) * blur1(2*x+0,2*y+2) + kernel(2,0) * blur1(2*x+2,2*y+0);
        //blur3(x, y) = kernel(0,0) * blur2(2*x,2*y) + kernel(1,0) * blur2(2*x+1,2*y) + kernel(0,1) * blur2(2*x,2*y+1) + kernel(1,1) * blur2(2*x+1,2*y+1) + kernel(1,2) * blur2(2*x+1,2*y+2) + kernel(2,1) * blur2(2*x+2,2*y+1) + kernel(2,2) * blur2(2*x+2,2*y+2) + kernel(0,2) * blur2(2*x+0,2*y+2) + kernel(2,0) * blur2(2*x+2,2*y+0);

        // 4x4 conv
        //blur0(x, y) = kernel(0,0) * ha_in(2*x,2*y) + kernel(1,0) * hw_in(2*x+1,2*y) + kernel(0,1) * hw_in(2*x,2*y+1) + kernel(1,1) * hw_in(2*x+1,2*y+1) + kernel(1,2) * hw_in(2*x+1,2*y+2) + kernel(2,1) * hw_in(2*x+2,2*y+1) + kernel(2,2) * hw_in(2*x+2,2*y+2) + kernel(0,2) * hw_in(2*x+0,2*y+2) + kernel(2,0) * hw_in(2*x+2,2*y+0) + kernel(0,3) * hw_in(2*x+0,2*y+3) + kernel(1,3) * hw_in(2*x+1,2*y+3) + kernel(2,3) * hw_in(2*x+2,2*y+3) + kernel(3,3) * hw_in(2*x+3,2*y+3) + kernel(3,2) * hw_in(2*x+3,2*y+2) + kernel(3,1) * hw_in(2*x+3,2*y+1) + kernel(3,0) * hw_in(2*x+3,2*y+0);
        //blur1(x, y) = kernel(0,0) * blur0(2*x,2*y) + kernel(1,0) * blur0(2*x+1,2*y) + kernel(0,1) * blur0(2*x,2*y+1) + kernel(1,1) * blur0(2*x+1,2*y+1) + kernel(1,2) * blur0(2*x+1,2*y+2) + kernel(2,1) * blur0(2*x+2,2*y+1) + kernel(2,2) * blur0(2*x+2,2*y+2) + kernel(0,2) * blur0(2*x+0,2*y+2) + kernel(2,0) * blur0(2*x+2,2*y+0) + kernel(0,3) * blur0(2*x+0,2*y+3) + kernel(1,3) * blur0(2*x+1,2*y+3) + kernel(2,3) * blur0(2*x+2,2*y+3) + kernel(3,3) * blur0(2*x+3,2*y+3) + kernel(3,2) * blur0(2*x+3,2*y+2) + kernel(3,1) * blur0(2*x+3,2*y+1) + kernel(3,0) * blur0(2*x+3,2*y+0);
        //blur2(x, y) = kernel(0,0) * blur1(2*x,2*y) + kernel(1,0) * blur1(2*x+1,2*y) + kernel(0,1) * blur1(2*x,2*y+1) + kernel(1,1) * blur1(2*x+1,2*y+1) + kernel(1,2) * blur1(2*x+1,2*y+2) + kernel(2,1) * blur1(2*x+2,2*y+1) + kernel(2,2) * blur1(2*x+2,2*y+2) + kernel(0,2) * blur1(2*x+0,2*y+2) + kernel(2,0) * blur1(2*x+2,2*y+0) + kernel(0,3) * blur1(2*x+0,2*y+3) + kernel(1,3) * blur1(2*x+1,2*y+3) + kernel(2,3) * blur1(2*x+2,2*y+3) + kernel(3,3) * blur1(2*x+3,2*y+3) + kernel(3,2) * blur1(2*x+3,2*y+2) + kernel(3,1) * blur1(2*x+3,2*y+1) + kernel(3,0) * blur1(2*x+3,2*y+0);
        //blur3(x, y) = kernel(0,0) * blur2(2*x,2*y) + kernel(1,0) * blur2(2*x+1,2*y) + kernel(0,1) * blur2(2*x,2*y+1) + kernel(1,1) * blur2(2*x+1,2*y+1) + kernel(1,2) * blur2(2*x+1,2*y+2) + kernel(2,1) * blur2(2*x+2,2*y+1) + kernel(2,2) * blur2(2*x+2,2*y+2) + kernel(0,2) * blur2(2*x+0,2*y+2) + kernel(2,0) * blur2(2*x+2,2*y+0) + kernel(0,3) * blur2(2*x+0,2*y+3) + kernel(1,3) * blur2(2*x+1,2*y+3) + kernel(2,3) * blur2(2*x+2,2*y+3) + kernel(3,3) * blur2(2*x+3,2*y+3) + kernel(3,2) * blur2(2*x+3,2*y+2) + kernel(3,1) * blur2(2*x+3,2*y+1) + kernel(3,0) * blur2(2*x+3,2*y+0);

        // 5x5 conv
        //blur0(x, y) = kernel(0,0) * ha_in(2*x,2*y) + kernel(1,0) * hw_in(2*x+1,2*y) + kernel(0,1) * hw_in(2*x,2*y+1) + kernel(1,1) * hw_in(2*x+1,2*y+1) + kernel(1,2) * hw_in(2*x+1,2*y+2) + kernel(2,1) * hw_in(2*x+2,2*y+1) + kernel(2,2) * hw_in(2*x+2,2*y+2) + kernel(0,2) * hw_in(2*x+0,2*y+2) + kernel(2,0) * hw_in(2*x+2,2*y+0) + kernel(0,3) * hw_in(2*x+0,2*y+3) + kernel(1,3) * hw_in(2*x+1,2*y+3) + kernel(2,3) * hw_in(2*x+2,2*y+3) + kernel(3,3) * hw_in(2*x+3,2*y+3) + kernel(3,2) * hw_in(2*x+3,2*y+2) + kernel(3,1) * hw_in(2*x+3,2*y+1) + kernel(3,0) * hw_in(2*x+3,2*y+0) + kernel(0,4) * hw_in(2*x+0,2*y+4) + kernel(1,4) * hw_in(2*x+1,2*y+4) + kernel(2,4) * hw_in(2*x+2,2*y+4) + kernel(3,4) * hw_in(2*x+3,2*y+4) + kernel(4,4) * hw_in(2*x+4,2*y+4) + kernel(4,3) * hw_in(2*x+4,2*y+3) + kernel(4,2) * hw_in(2*x+4,2*y+2) + kernel(4,1) * hw_in(2*x+4,2*y+1) + kernel(4,0) * hw_in(2*x+4,2*y+0);
        //blur1(x, y) = kernel(0,0) * blur0(2*x,2*y) + kernel(1,0) * blur0(2*x+1,2*y) + kernel(0,1) * blur0(2*x,2*y+1) + kernel(1,1) * blur0(2*x+1,2*y+1) + kernel(1,2) * blur0(2*x+1,2*y+2) + kernel(2,1) * blur0(2*x+2,2*y+1) + kernel(2,2) * blur0(2*x+2,2*y+2) + kernel(0,2) * blur0(2*x+0,2*y+2) + kernel(2,0) * blur0(2*x+2,2*y+0) + kernel(0,3) * blur0(2*x+0,2*y+3) + kernel(1,3) * blur0(2*x+1,2*y+3) + kernel(2,3) * blur0(2*x+2,2*y+3) + kernel(3,3) * blur0(2*x+3,2*y+3) + kernel(3,2) * blur0(2*x+3,2*y+2) + kernel(3,1) * blur0(2*x+3,2*y+1) + kernel(3,0) * blur0(2*x+3,2*y+0) + kernel(0,4) * blur0(2*x+0,2*y+4) + kernel(1,4) * blur0(2*x+1,2*y+4) + kernel(2,4) * blur0(2*x+2,2*y+4) + kernel(3,4) * blur0(2*x+3,2*y+4) + kernel(4,4) * blur0(2*x+4,2*y+4) + kernel(4,3) * blur0(2*x+4,2*y+3) + kernel(4,2) * blur0(2*x+4,2*y+2) + kernel(4,1) * blur0(2*x+4,2*y+1) + kernel(4,0) * blur0(2*x+4,2*y+0);
        //blur2(x, y) = kernel(0,0) * blur1(2*x,2*y) + kernel(1,0) * blur1(2*x+1,2*y) + kernel(0,1) * blur1(2*x,2*y+1) + kernel(1,1) * blur1(2*x+1,2*y+1) + kernel(1,2) * blur1(2*x+1,2*y+2) + kernel(2,1) * blur1(2*x+2,2*y+1) + kernel(2,2) * blur1(2*x+2,2*y+2) + kernel(0,2) * blur1(2*x+0,2*y+2) + kernel(2,0) * blur1(2*x+2,2*y+0) + kernel(0,3) * blur1(2*x+0,2*y+3) + kernel(1,3) * blur1(2*x+1,2*y+3) + kernel(2,3) * blur1(2*x+2,2*y+3) + kernel(3,3) * blur1(2*x+3,2*y+3) + kernel(3,2) * blur1(2*x+3,2*y+2) + kernel(3,1) * blur1(2*x+3,2*y+1) + kernel(3,0) * blur1(2*x+3,2*y+0) + kernel(0,4) * blur1(2*x+0,2*y+4) + kernel(1,4) * blur1(2*x+1,2*y+4) + kernel(2,4) * blur1(2*x+2,2*y+4) + kernel(3,4) * blur1(2*x+3,2*y+4) + kernel(4,4) * blur1(2*x+4,2*y+4) + kernel(4,3) * blur1(2*x+4,2*y+3) + kernel(4,2) * blur1(2*x+4,2*y+2) + kernel(4,1) * blur1(2*x+4,2*y+1) + kernel(4,0) * blur1(2*x+4,2*y+0);
        //blur3(x, y) = kernel(0,0) * blur2(2*x,2*y) + kernel(1,0) * blur2(2*x+1,2*y) + kernel(0,1) * blur2(2*x,2*y+1) + kernel(1,1) * blur2(2*x+1,2*y+1) + kernel(1,2) * blur2(2*x+1,2*y+2) + kernel(2,1) * blur2(2*x+2,2*y+1) + kernel(2,2) * blur2(2*x+2,2*y+2) + kernel(0,2) * blur2(2*x+0,2*y+2) + kernel(2,0) * blur2(2*x+2,2*y+0) + kernel(0,3) * blur2(2*x+0,2*y+3) + kernel(1,3) * blur2(2*x+1,2*y+3) + kernel(2,3) * blur2(2*x+2,2*y+3) + kernel(3,3) * blur2(2*x+3,2*y+3) + kernel(3,2) * blur2(2*x+3,2*y+2) + kernel(3,1) * blur2(2*x+3,2*y+1) + kernel(3,0) * blur2(2*x+3,2*y+0) + kernel(0,4) * blur2(2*x+0,2*y+4) + kernel(1,4) * blur2(2*x+1,2*y+4) + kernel(2,4) * blur2(2*x+2,2*y+4) + kernel(3,4) * blur2(2*x+3,2*y+4) + kernel(4,4) * blur2(2*x+4,2*y+4) + kernel(4,3) * blur2(2*x+4,2*y+3) + kernel(4,2) * blur2(2*x+4,2*y+2) + kernel(4,1) * blur2(2*x+4,2*y+1) + kernel(4,0) * blur2(2*x+4,2*y+0);

        Func hw_output;
        //hw_output(x, y) = blur[num_levels-1](x, y);
        hw_output(x, y) =  blur2(x, y);
        output(x, y) = cast<uint8_t>( hw_output(x, y) );

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {


        } else if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, imgSize);
          output.bound(y, 0, imgSize);

          if (schedule == 0) {
            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
              .hw_accelerate(xi, xo);

            blur3.compute_at(hw_output, xo);
            blur2.compute_at(hw_output, xo);
            blur1.compute_at(hw_output, xo);
            blur0.compute_at(hw_output, xo);


            hw_in.stream_to_accelerator();
            
          } else if (schedule == 1) {
            // Share the compute kernels
            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
              .hw_accelerate(xi, xo);

            blur3.compute_at(hw_output, xo);
            blur2.compute_at(hw_output, xo);
            blur1.compute_at(hw_output, xo);
            blur0.compute_at(hw_output, xo);

            // Share the compute kernels
            blur3.compute_share_root(y);
            blur2.compute_share(blur3);
            blur1.compute_share(blur3);
            blur0.compute_share(blur3);

            // Assign coarse-grain loops
            std::vector<Func> funcs = {blur3, blur2, blur1, blur0, hw_in.in()};
            for (auto& func : funcs) {
              func.coarse_grain_loop(y);
            }
            hw_output.coarse_grain_loop(yi);

            hw_in.stream_to_accelerator();

          } else if (schedule == 2) {
            // Shared compute kernels; unroll x dim by 2
            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
              .hw_accelerate(xi, xo);

            blur3.compute_at(hw_output, xo).unroll(x, 2);
            blur2.compute_at(hw_output, xo).unroll(x, 2);
            blur1.compute_at(hw_output, xo).unroll(x, 2);
            blur0.compute_at(hw_output, xo).unroll(x, 2);

            hw_in.in().unroll(x, 2);

            // Share the compute kernels
            blur3.compute_share_root(y);
            blur2.compute_share(blur3);
            blur1.compute_share(blur3);
            blur0.compute_share(blur3);

            // Assign coarse-grain loops
            std::vector<Func> funcs = {blur3, blur2, blur1, blur0, hw_in.in()};
            for (auto& func : funcs) {
              func.coarse_grain_loop(y);
            }
            hw_output.coarse_grain_loop(yi);

            hw_in.stream_to_accelerator();

          } else if (schedule == 3) {
            // Shared compute kernels; unroll x dim by 4
            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
              .hw_accelerate(xi, xo);

            blur3.compute_at(hw_output, xo).unroll(x, 4);
            blur2.compute_at(hw_output, xo).unroll(x, 4);
            blur1.compute_at(hw_output, xo).unroll(x, 4);
            blur0.compute_at(hw_output, xo).unroll(x, 4);

            hw_in.in().unroll(x, 4);

            // Share the compute kernels
            blur3.compute_share_root(y);
            blur2.compute_share(blur3);
            blur1.compute_share(blur3);
            blur0.compute_share(blur3);

            // Assign coarse-grain loops
            std::vector<Func> funcs = {blur3, blur2, blur1, blur0, hw_in.in()};
            for (auto& func : funcs) {
              func.coarse_grain_loop(y);
            }
            hw_output.coarse_grain_loop(yi);

            hw_in.stream_to_accelerator();

          } else if (schedule == 4) {
            // Shared compute kernels; unroll x dim by 4; share latter kernels
            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
              .hw_accelerate(xi, xo);

            blur3.compute_at(hw_output, xo).unroll(x, 4);
            blur2.compute_at(hw_output, xo).unroll(x, 4);
            blur1.compute_at(hw_output, xo).unroll(x, 4);
            blur0.compute_at(hw_output, xo).unroll(x, 4);

            hw_in.in().unroll(x, 4);

            // Share the compute kernels
            blur3.compute_share_root(y);
            blur2.compute_share(blur3);
            blur1.compute_share(blur3);

            // Assign coarse-grain loops
            std::vector<Func> funcs = {blur3, blur2, blur1, blur0, hw_in.in()};
            for (auto& func : funcs) {
              func.coarse_grain_loop(y);
            }
            hw_output.coarse_grain_loop(yi);

            hw_in.stream_to_accelerator();

          } else if (schedule == 5) {
            // Shared compute kernels; 1 kernel x4, latter kernels shared, x2
            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
              .hw_accelerate(xi, xo);

            blur3.compute_at(hw_output, xo).unroll(x, 2);
            blur2.compute_at(hw_output, xo).unroll(x, 2);
            blur1.compute_at(hw_output, xo).unroll(x, 2);
            blur0.compute_at(hw_output, xo).unroll(x, 4);

            hw_in.in().unroll(x, 4);

            // Share the compute kernels
            blur3.compute_share_root(y);
            blur2.compute_share(blur3);
            blur1.compute_share(blur3);

            // Assign coarse-grain loops
            std::vector<Func> funcs = {blur3, blur2, blur1, blur0, hw_in.in()};
            for (auto& func : funcs) {
              func.coarse_grain_loop(y);
            }
            hw_output.coarse_grain_loop(yi);

            hw_in.stream_to_accelerator();
            
          } else {
            std::cout << "No schedule yet for schedule=" << (int)schedule << std::endl;
            exit(1);
          }
          
        } else {    // schedule to CPU
          output.bound(x, 0, imgSize);
          output.bound(y, 0, imgSize);

          hw_output.compute_root();

          hw_output
            .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .hw_accelerate(xi, xo);
          
          blur3.compute_at(hw_output, xo);
          //blur2.compute_at(blur3, y);
          //blur1.compute_at(blur3, y);
          blur0.compute_at(hw_output, xo);

          /*output.tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .vectorize(xi, 8)
            .fuse(xo, yo, xo)
            .parallel(xo);*/

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PyramidGaussianBlur, pyramid_blur)

