/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 3;
int imgSize = 64;

class LaplacianPyramid : public Halide::Generator<LaplacianPyramid> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // Create a reduction domain of the correct bounds.
        RDom win(0, blockSize, 0, blockSize);

        Func hw_input, f0;
        Func casted;
        casted = Halide::BoundaryConditions::repeat_edge(input);
        hw_input(x, y) = cast<uint16_t>(casted(x, y));
        
        f0(x, y) = hw_input(x, y);

        /*
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
        //kernel(x) = cast<uint16_t>(kernel_f(x) * 64 / sum_kernel[blockSize-1]);
        //kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
        //                             sum_kernel[blockSize*blockSize-1]);
        kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
                                     sum_kernel[blockSize*blockSize-1]);
        */

        Func kernel("kernel");
        kernel(x,y) = cast<uint16_t>(0);
        kernel(0,0) = cast<uint16_t>(3);       kernel(1,0) = cast<uint16_t>(21);       kernel(2,0) = cast<uint16_t>(3);
        kernel(0,1) = cast<uint16_t>(21);       kernel(1,1) = cast<uint16_t>(158);       kernel(2,1) = cast<uint16_t>(21);
        kernel(0,2) = cast<uint16_t>(3);       kernel(1,2) = cast<uint16_t>(21);       kernel(2,2) = cast<uint16_t>(3);

        // Use a 2D filter to blur the input
        Func blur_unnormalized, l0;
        blur_unnormalized(x, y) = cast<uint16_t>(0);
        blur_unnormalized(x, y) += kernel(win.x, win.y) * f0(x+win.x, y+win.y);
        l0(x, y) = blur_unnormalized(x-1, y-1) / 256;

        Func h0;
        h0(x, y) = f0(x, y) - l0(x, y);

        Func f1;
        f1(x,y) = l0(x*2, y*2);

        Func f1_blur_unnormalized, l1;
        f1_blur_unnormalized(x, y) = cast<uint16_t>(0);
        f1_blur_unnormalized(x, y) += kernel(win.x, win.y) * f1(x+win.x, y+win.y);
        l1(x, y) = f1_blur_unnormalized(x-1, y-1) / 256;

        Func h1;
        h1(x, y) = f1(x, y) - l1(x, y);

        Func f2;
        f2(x,y) = l1(x*2, y*2);

        Func f2_temp, l1_up;
        f2_temp(x,y) = f2(x/2,y/2);
        
        Func f2_temp_blur_unnormalized;
        f2_temp_blur_unnormalized(x, y) = cast<uint16_t>(0);
        f2_temp_blur_unnormalized(x, y) += kernel(win.x, win.y) * f2_temp(x+win.x, y+win.y);
        l1_up(x, y) = f2_temp_blur_unnormalized(x-1, y-1) / 256;

        Func f1_up;
        f1_up(x,y) = l1_up(x, y) + h1(x,y);

        Func f1_temp, l0_up;
        f1_temp(x,y) = f1_up(x/2,y/2);
        
        Func f1_temp_blur_unnormalized;
        f1_temp_blur_unnormalized(x, y) = cast<uint16_t>(0);
        f1_temp_blur_unnormalized(x, y) += kernel(win.x, win.y) * f1_temp(x+win.x, y+win.y);
        l0_up(x, y) = f1_temp_blur_unnormalized(x-1, y-1) / 256;

        Func f0_up;
        f0_up(x,y) = l0_up(x, y) + h0(x,y);       


        Func hw_output;

        hw_output(x,y) = f0_up(x,y);

        output(x, y) = cast<uint8_t>(hw_output(x, y));

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

          hw_output.bound(x, 0, imgSize);
          hw_output.bound(y, 0, imgSize);
          output.bound(x, 0, imgSize);
          output.bound(y, 0, imgSize);
          blur_unnormalized.bound(x, 0, imgSize);
          blur_unnormalized.bound(y, 0, imgSize);

          //hw_input.compute_root();
          //kernel.compute_root();
          hw_output.compute_root();

          hw_output
            //            .compute_at(output, xo)
            .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .hw_accelerate(xi, xo);

          blur_unnormalized.update()
            .unroll(win.x, blockSize)
            .unroll(win.y, blockSize);

          blur_unnormalized.linebuffer();

          //hw_output.accelerate({hw_input}, xi, xo);
          hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, imgSize);
          output.bound(y, 0, imgSize);

          //hw_input.compute_root();
          //kernel.compute_root();
          hw_output.compute_root();

          kernel.compute_at(hw_output, xo);

          hw_output
            .tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .hw_accelerate(xi, xo);

          f0_up.compute_at(hw_output, xo);

          f1_temp_blur_unnormalized.update()
            .unroll(win.x, blockSize)
            .unroll(win.y, blockSize);
          f1_temp_blur_unnormalized.compute_at(hw_output, xo);

          f1_up.compute_at(hw_output, xo);
          f1_temp.compute_at(hw_output, xo);
          l0_up.compute_at(hw_output, xo);

          f2_temp_blur_unnormalized.update()
            .unroll(win.x, blockSize)
            .unroll(win.y, blockSize);
          f2_temp_blur_unnormalized.compute_at(hw_output, xo);

          f2_temp.compute_at(hw_output, xo);
          l1_up.compute_at(hw_output, xo);

          f2.compute_at(hw_output, xo);
          h1.compute_at(hw_output, xo);
          l1.compute_at(hw_output, xo);

          f1_blur_unnormalized.update()
            .unroll(win.x, blockSize)
            .unroll(win.y, blockSize);
          f1_blur_unnormalized.compute_at(hw_output, xo);

          f1.compute_at(hw_output, xo);
          h0.compute_at(hw_output, xo);
          l0.compute_at(hw_output, xo);

          blur_unnormalized.update()
            .unroll(win.x, blockSize)
            .unroll(win.y, blockSize);

          blur_unnormalized.compute_at(hw_output, xo);

          hw_input.stream_to_accelerator();//.compute_root();//at(output, xo);
          //hw_input.compute_root();
          
          //hw_input.compute_at(hw_output, xo);
          //input_copy.stream_to_accelerator();
          //input_copy.compute_root();
          std::vector<Func> fs = {hw_input.in(), blur_unnormalized,
                                   l0, h0, f1, f1_blur_unnormalized,
                                   l1, h1, f2,
                                   f2_temp, f2_temp_blur_unnormalized,
                                   l1_up, f1_up,
                                   f1_temp, f1_temp_blur_unnormalized,
                                   l0_up, f0_up};
          for (auto& f : fs) {
            f.coarse_grain_loop(y);
          }
          hw_output.coarse_grain_loop(yi);
          
        } else {    // schedule to CPU

          /*output.tile(x, y, xo, yo, xi, yi, imgSize, imgSize)
            .vectorize(xi, 8)
            .fuse(xo, yo, xo)
            .parallel(xo);*/

        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LaplacianPyramid, laplacian_pyramid)

