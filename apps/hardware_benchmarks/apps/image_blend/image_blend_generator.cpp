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

  class ImageBlend : public Halide::Generator<ImageBlend> {
    
  public:
    Input<Buffer<uint8_t>>  input_left{"input_left", 3};
    Input<Buffer<uint8_t>>  input_right{"input_right", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    int pyramid_levels = 3;
    int16_t ksize = 23;
    int16_t shift = 11;
    int16_t scale = 512; // this is the weight scaling used until pyramid blending
    
    void generate() {

      Func hw_input_right, hw_input_right8, hw_input_left, hw_input_left8;
      //hw_input_right(x,y,c) = u16(input_right(x+shift, y+shift, c));
      hw_input_right8 = Halide::BoundaryConditions::repeat_edge(input_right);
      hw_input_right(x,y,c) = u16(hw_input_right8(x,y,c));
      //hw_input_left(x,y,c) = u16(input_left(x+shift, y+shift, c));
      hw_input_left8 = Halide::BoundaryConditions::repeat_edge(input_left);
      hw_input_left(x,y,c) = u16(hw_input_left8(x,y,c));

      // Create exposure weight: make it three regions:
      //        regions: left, blend, right
      //   left weights: -------\__________
      //  right weights: _______/----------
      Func weight_left, weight_right, weight_sum, weight_left_norm, weight_right_norm;
      int16_t length = 128;
      int16_t blend_len = 32;
      int16_t regionL = length/2 - blend_len/2;
      int16_t regionR = length/2 + blend_len/2;
      int16_t shift = regionL;
      int16_t slope = scale / blend_len;
      weight_left(x,y,c)   = i16(select(x < regionL, scale,
                                        x > regionR, i16(0),
                                        scale - (x-shift) * slope));
      weight_right(x,y,c)  = i16(select(x < regionL, i16(0),
                                        x > regionR, scale,
                                        (x-shift) * slope));
      
      weight_left_norm(x,y,c)  = weight_left(x,y,c);
      weight_right_norm(x,y,c) = weight_right(x,y,c);

      // Create gaussian pyramids of the weights
      vector<Func> left_weight_gpyramid(pyramid_levels);
      vector<Func> right_weight_gpyramid(pyramid_levels);
      left_weight_gpyramid  = gaussian_pyramid(weight_left_norm, pyramid_levels, "lweight");
      right_weight_gpyramid = gaussian_pyramid(weight_right_norm, pyramid_levels, "rweight");

      // Create laplacian pyramids of the input images
      vector<Func> left_input_lpyramid(pyramid_levels);
      vector<Func> left_input_gpyramid(pyramid_levels);
      vector<Func> right_input_lpyramid(pyramid_levels);
      vector<Func> right_input_gpyramid(pyramid_levels);
      left_input_lpyramid  = laplacian_pyramid(hw_input_left, pyramid_levels,
                                               left_input_gpyramid, "linput");
      right_input_lpyramid = laplacian_pyramid(hw_input_right, pyramid_levels,
                                               right_input_gpyramid, "rinput");

      // Merge the input pyramids using the weight pyramids
      vector<Func> merged_pyramid(pyramid_levels);
      merged_pyramid = merge_pyramids(left_weight_gpyramid,
                                      right_weight_gpyramid,
                                      left_input_lpyramid,
                                      right_input_lpyramid,
                                      "merge_pyr");

      // Collapse the merged pyramid to create a single image
      Func blended_image;
      vector<Func> upsampled(pyramid_levels);
      fill_funcnames(upsampled, "flattened");
      blended_image = flatten_pyramid(merged_pyramid, upsampled);
      
      Func hw_output;
      hw_output(x,y,c) = blended_image(x,y,c);
      //hw_output(x,y,c) = left_input_lpyramid[pyramid_levels-1](x,y,c);
      //hw_output(x,y,c) = left_input_gpyramid[pyramid_levels-1](x,y,c);

      output(x,y,c) = u8(hw_output(x,y,c));

      output.bound(c, 0, 3);
      output.bound(x, 0, 128-ksize+1);
      output.bound(y, 0, 128-ksize+1);

        
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {

        hw_output.compute_root();

        hw_output.tile(x, y, xo, yo, xi, yi, 128-ksize+1,128-ksize+1)
          .reorder(xi,yi,c,xo,yo)
          .hw_accelerate(xi, xo);
        //hw_output.unroll(c);

        blended_image.compute_at(hw_output, xo);

        for (size_t i=0; i<merged_pyramid.size(); ++i) {
          merged_pyramid[i].compute_at(hw_output, xo);
          
          left_input_lpyramid[i].compute_at(hw_output, xo);
          right_input_lpyramid[i].compute_at(hw_output, xo);
          left_input_gpyramid[i].compute_at(hw_output, xo);
          right_input_gpyramid[i].compute_at(hw_output, xo);
          
          left_weight_gpyramid[i].compute_at(hw_output, xo);
          right_weight_gpyramid[i].compute_at(hw_output, xo);
        }

        weight_sum.compute_at(hw_output, xo);
        
        hw_input_right.stream_to_accelerator();
        hw_input_left.stream_to_accelerator();
        
      } else {    // schedule to CPU
        output.compute_root();
        for (size_t i=0; i<merged_pyramid.size(); ++i) {
          merged_pyramid[i].compute_root();
          left_input_lpyramid[i].compute_root();
          right_input_lpyramid[i].compute_root();
          left_weight_gpyramid[i].compute_root();
          right_weight_gpyramid[i].compute_root();
        }
      }
    }

private:
    Var x, y, c, k;

    // Downsample with a 1 3 3 1 filter
    //Func downsample(Func f) {
    //    using Halide::_;
    //    Func downx, downy;
    //    downx(x,y,_) = (f(2*x-1, y, _) + 3.0f * (f(2*x,y,_) + f(2*x+1, y, _)) + f(2*x+2, y, _)) / 8.0f;
    //    downy(x,y,_) = (downx(x, 2*y-1, _) + 3.0f * (downx(x, 2*y, _) + downx(x, 2*y+1, _)) + downx(x, 2*y+2, _)) / 8.0f;
    //    return downy;
    //}
    Func downsample(Func f) {
      using Halide::_;
      Func down;
      down(x,y,_) = f(x*2, y*2, _);
      return down;
    }
    
    // Upsample using bilinear interpolation (1 3 3 1)
    //Func upsample(Func f) {
    //    using Halide::_;
    //    Func upx, upy;
    //    upx(x,y,_) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2,y,_);
    //    upy(x,y,_) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x,y/2,_);
    //    return upy;
    //}
    Func upsample(Func f) {
      using Halide::_;
      Func up;
      up(x,y,_) = f(x/2, y/2, _);
      return up;
    }

    // Create a gaussian pyramid
    vector<Func> gaussian_pyramid(Func f, int num_levels, std::string name) {
      using Halide::_;
      
      vector<Func> gPyramid(num_levels);
      fill_funcnames(gPyramid, name + "_gpyr");
      std::cout << "pyramid has " << num_levels << " levels" << std::endl;
      
      gPyramid[0](x,y,_) = f(x,y,_);

      for (int j = 1; j < num_levels; j++) {
        std::cout << "connecting " << j << " to " << j-1<< std::endl;
        gPyramid[j](x,y,_) = downsample(gPyramid[j-1])(x,y,_);
      }

      std::cout << "next" << std::endl;

      return gPyramid;
    }

    // Create a laplacian pyramid
    vector<Func> laplacian_pyramid(Func f, int num_levels, vector<Func> &gPyramid, std::string name) {
      using Halide::_;
      
      gPyramid = gaussian_pyramid(f, num_levels, name + "_gpyr");

      vector<Func> lPyramid(num_levels);
      fill_funcnames(lPyramid, name + "_lpyr");
      
      // The last level is the same as the last level of the gaussian pyramid.
      lPyramid[num_levels-1](x,y,_) = gPyramid[num_levels-1](x,y,_);

      // Create the laplacian pyramid from the last level up to the first.
      for (int j = num_levels-2; j >= 0; j--) {
        lPyramid[j](x,y,_) = gPyramid[j](x,y,_) - upsample(gPyramid[j+1])(x,y,_);
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
        blended_pyramid[i](x,y,_) =
          (weights0[i](x,y,_) * img0[i](x,y,_) +
           weights1[i](x,y,_) * img1[i](x,y,_)) / scale;
      }
      
      return blended_pyramid;
    }

    // Flatten a pyramid
    Func flatten_pyramid(vector<Func> &pyramid, vector<Func> &upsampled) {
      using Halide::_;

      
      //init the last upsampled stencil to be the deepest gaussian pyramid
      int num_levels = pyramid.size();
      upsampled[num_levels-1] = pyramid[num_levels-1];

      //Blend to the top
      for (int level = num_levels-1; level > 0; --level) {
        upsampled[level-1](x,y,_) = pyramid[level-1](x,y,_) + upsample(upsampled[level])(x,y,_);
      }
      
      return upsampled[0];
    }
    
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(ImageBlend, image_blend)
