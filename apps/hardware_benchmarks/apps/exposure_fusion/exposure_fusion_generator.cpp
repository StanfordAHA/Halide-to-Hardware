#include "Halide.h"

namespace {

  using namespace Halide;
  using namespace Halide::ConciseCasts;
  using std::vector;
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");

void fill_funcnames(vector<Func>& funcs, std::string name) {
  for (size_t i=0; i<funcs.size(); ++i) {
      funcs[i] = Func(name + "_" + std::to_string(i));
    }
  }

  class ExposureFusion : public Halide::Generator<ExposureFusion> {
    
  public:
    Input<Buffer<uint8_t>>  input_bright{"input_bright", 3};
    Input<Buffer<uint8_t>>  input_dark{"input_dark", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    int pyramid_levels = 3;
    int bright_weighting = 3; // this would be used if a single frame is used
    int ksize = 23;
    int shift = 11;
    
    void generate() {

      Func hw_input_bright, hw_input_dark;
      hw_input_bright(x,y,c) = u16(input_bright(x+shift, y+shift, c));
      hw_input_dark(x,y,c) = u16(input_dark(x+shift, y+shift, c));

      // Create exposure weight
      Func weight_dark, weight_bright, weight_sum, weight_dark_norm, weight_bright_norm;
      weight_dark(x, y, c)   = 2 * hw_input_dark(x, y, c);
      weight_bright(x, y, c) = 2 * hw_input_bright(x, y, c);
      weight_sum(x, y, c) = weight_dark(x,y,c) + weight_bright(x,y,c);
      
      weight_dark_norm(x,y,c)   = weight_dark(x,y,c)   / weight_sum(x,y,c);
      weight_bright_norm(x,y,c) = weight_bright(x,y,c) / weight_sum(x,y,c);

      // Create gaussian pyramids of the weights
      vector<Func> dark_weight_gpyramid(pyramid_levels);
      vector<Func> bright_weight_gpyramid(pyramid_levels);
      dark_weight_gpyramid =   gaussian_pyramid(weight_dark_norm, pyramid_levels, "dweight");
      bright_weight_gpyramid = gaussian_pyramid(weight_bright_norm, pyramid_levels, "bweight");

      // Create laplacian pyramids of the input images
      vector<Func> dark_input_lpyramid(pyramid_levels);
      vector<Func> dark_input_gpyramid(pyramid_levels);
      vector<Func> bright_input_lpyramid(pyramid_levels);
      vector<Func> bright_input_gpyramid(pyramid_levels);
      dark_input_lpyramid =   laplacian_pyramid(hw_input_dark, pyramid_levels,
                                                dark_input_gpyramid, "dinput");
      bright_input_lpyramid = laplacian_pyramid(hw_input_bright, pyramid_levels,
                                                bright_input_gpyramid, "binput");

      // Merge the input pyramids using the weight pyramids
      vector<Func> merged_pyramid(pyramid_levels);
      merged_pyramid = merge_pyramids(dark_weight_gpyramid,
                                      bright_weight_gpyramid,
                                      dark_input_lpyramid,
                                      bright_input_lpyramid,
                                      "merge_pyr");

      // Collapse the merged pyramid to create a single image
      Func blended_image;
      blended_image = flatten_pyramid(merged_pyramid);
      
      Func hw_output;
      hw_output(x, y, c) = u8(blended_image(x, y, c));

      output(x, y, c) = hw_output(x, y, c);

      output.bound(c, 0, 3);
      output.bound(x, 0, 64-ksize+1);
      output.bound(y, 0, 64-ksize+1);

        
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {

        hw_output.compute_root();

        hw_output.tile(x, y, xo, yo, xi, yi, 64-ksize+1,64-ksize+1)
          .reorder(xi,yi,c,xo,yo)
          .hw_accelerate(xi, xo);
        //hw_output.unroll(c);

        blended_image.compute_at(hw_output, xo);

        for (size_t i=0; i<merged_pyramid.size(); ++i) {
          merged_pyramid[i].compute_at(hw_output, xo);
          
          dark_input_lpyramid[i].compute_at(hw_output, xo);
          bright_input_lpyramid[i].compute_at(hw_output, xo);
          dark_input_gpyramid[i].compute_at(hw_output, xo);
          bright_input_gpyramid[i].compute_at(hw_output, xo);
          
          dark_weight_gpyramid[i].compute_at(hw_output, xo);
          bright_weight_gpyramid[i].compute_at(hw_output, xo);
        }

        weight_sum.compute_at(hw_output, xo);
        
        hw_input_bright.stream_to_accelerator();
        hw_input_dark.stream_to_accelerator();
        
      } else {    // schedule to CPU
        output.compute_root();
        for (size_t i=0; i<merged_pyramid.size(); ++i) {
          merged_pyramid[i].compute_root();
          dark_input_lpyramid[i].compute_root();
          bright_input_lpyramid[i].compute_root();
          dark_weight_gpyramid[i].compute_root();
          bright_weight_gpyramid[i].compute_root();
        }
      }
    }

private:
    Var x, y, c, k;

    // Downsample with a 1 3 3 1 filter
    Func downsample(Func f) {
        using Halide::_;
        Func downx, downy;
        downx(x, y, _) = (f(2*x-1, y, _) + 3.0f * (f(2*x, y, _) + f(2*x+1, y, _)) + f(2*x+2, y, _)) / 8.0f;
        downy(x, y, _) = (downx(x, 2*y-1, _) + 3.0f * (downx(x, 2*y, _) + downx(x, 2*y+1, _)) + downx(x, 2*y+2, _)) / 8.0f;
        return downy;
    }

    // Upsample using bilinear interpolation (1 3 3 1)
    Func upsample(Func f) {
        using Halide::_;
        Func upx, upy;
        upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
        upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);
        return upy;
    }

    // Create a gaussian pyramid
    vector<Func> gaussian_pyramid(Func f, int num_levels, std::string name) {
      using Halide::_;
      
      vector<Func> gPyramid(num_levels);
      fill_funcnames(gPyramid, name + "_gpyr");
      
      gPyramid[0](x, y, _) = f(x, y, _);
      for (int j = 1; j < num_levels; j++) {
        gPyramid[j](x, y, _) = downsample(gPyramid[j-1])(x, y, _);
      }

      return gPyramid;
    }

    // Create a laplacian pyramid
    vector<Func> laplacian_pyramid(Func f, int num_levels, vector<Func> &gPyramid, std::string name) {
      using Halide::_;
      
      gPyramid = gaussian_pyramid(f, num_levels, name + "_gpyr");

      vector<Func> lPyramid(num_levels);
      fill_funcnames(lPyramid, name + "_lpyr");
      
      // The last level is the same as the last level of the guassian pyramid.
      lPyramid[num_levels-1](x, y, _) = gPyramid[num_levels-1](x, y, _);

      // Create the laplacian pyramid from the last level up to the first.
      for (int j = num_levels-2; j >= 0; j--) {
        lPyramid[j](x, y, _) = gPyramid[j](x, y, _) - upsample(gPyramid[j+1])(x, y, _);
      }

      return lPyramid;
    }

    // Blend two pyramids together with weights
    vector<Func> merge_pyramids(vector<Func> weights0, vector<Func> weights1,
                                vector<Func> img0, vector<Func> img1, std::string name) {
      using Halide::_;
      
      assert(weights0.size() == weights1.size());
      assert(weights0.size() == img0.size());
      assert(weights0.size() == img1.size());
      int num_levels = weights0.size();
      
      vector<Func> blended_pyramid(num_levels);
      fill_funcnames(blended_pyramid, name);
      for (int i=0; i<num_levels; ++i) {
        blended_pyramid[i](x, y, _) =
          weights0[i](x,y,_) * img0[i](x,y,_) +
          weights1[i](x,y,_) * img1[i](x,y,_);
      }
      
      return blended_pyramid;
    }

    // Flatten a pyramid
    Func flatten_pyramid(vector<Func> &pyramid, vector<Func> &upsampled) {
      using Halide::_;

      Func lImg;
      Func gImg;
      int num_levels = pyramid.size();
      for (int level = num_levels-1; level > 0; --level) {
        gImg = pyramid[level];
        lImg = pyramid[level-1];
        Func upsampled = upsample(gImg);
        lImg(x,y,_) = lImg(x,y,_) * upsampled(x,y,_);
      }
      
      return lImg;
    }
    
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(ExposureFusion, exposure_fusion)
