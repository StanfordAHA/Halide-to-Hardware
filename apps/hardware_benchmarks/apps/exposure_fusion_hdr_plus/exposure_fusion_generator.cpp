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
    Input<Buffer<uint16_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    int pyramid_levels = 3;
    int bright_weighting = 3; // this would be used if a single frame is used
    int ksize = 23;
    int shift = 11;
    
    // CHANGE THE NAME OF THIS FUNCTION. MUL2 FIXED POINT ISN'T QUITE AN ACCURATE DESCRIPTION 
    Expr mul2_fixed_point(Expr a, Expr b) {
      return u16(( (u32(a)) * (u32(b)) ) >> 8);
    }
    
    Expr mul2(Expr a, Expr b) {
      return u16(( (u32(a)) * (u32(b)) ));
    }

    Expr mul2_fixed_point_signed(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 8);
    }

    Func GammaCorrect(Func in, Func out, float gamma_exponent){
      out(x, y) = pow(in(x, y), gamma_exponent);
    }

    void generate() {

      Func hw_input, hw_input_8, hw_input_float;
      hw_input_8 = Halide::BoundaryConditions::repeat_edge(input);
      hw_input(x,y,c) = u16(hw_input_8(x,y,c));
      hw_input_float(x, y, c) = cast<float>(hw_input_8(x, y, c));

       // Create dark and bright versions of image 
      Func hw_input_bright, hw_input_dark;
      
      // input dark is grayscale version of input
      // Grayscale = 0.299R + 0.587G + 0.114B; break G down into two equal halves
      //hw_input_dark(x, y) = u16((77 * u16(hw_input(x, y, 0)) + 150 * u16(hw_input(x, y, 1)) + 29 * u16(hw_input(x, y, 2))) >> 8);

      //hw_input_dark(x, y) = u16(mul2_fixed_point(cast<uint16_t>(0.299f * 256.0f), hw_input(x, y, 0)) + mul2_fixed_point(cast<uint16_t>(0.587f * 256.0f), hw_input(x, y, 1)) + mul2_fixed_point(cast<uint16_t>(0.114f * 256.0f), hw_input(x, y, 2)));
      
      //hw_input_dark(x, y) = u16(mul2_fixed_point(cast<uint16_t>(0.299f * 256.0f), hw_input(x, y, 0)) + mul2_fixed_point(cast<uint16_t>(0.587f * 256.0f), hw_input(x, y, 1)) + mul2_fixed_point(cast<uint16_t>(0.114f * 256.0f), hw_input(x, y, 2)));
      hw_input_dark(x, y) = 0.299f * hw_input_float(x, y, 0) + 0.587f * hw_input_float(x, y, 1) + 0.114f * hw_input_float(x, y, 2);

      // input bright is input dark, multiplied by a scale factor (3.25 f)
      //hw_input_bright(x, y) = u16((832 * u16(hw_input_dark(x, y))) >> 8);
      //hw_input_bright(x, y) = mul2_fixed_point(cast<uint16_t>(3.25f * 256.0f), hw_input_dark(x, y));
      hw_input_bright(x, y) = 3.25f * hw_input_dark(x, y);


      // Gamma correct the dark and bright images
      Func hw_input_bright_gamma_corr, hw_input_dark_gamma_corr;
      //hw_input_bright_gamma_corr(x, y) = hw_input_bright(x, y);
      //hw_input_dark_gamma_corr(x, y) = hw_input_dark(x, y);


      float my_gamma_exponent = 1.f/2.2f;
      //hw_input_bright_gamma_corr(x, y) = pow(cast<float>(hw_input_bright(x, y)), my_gamma_exponent);
      //hw_input_dark_gamma_corr(x, y) = pow(cast<float>(hw_input_dark(x, y)), my_gamma_exponent);
      hw_input_bright_gamma_corr(x, y) = pow(hw_input_bright(x, y), my_gamma_exponent);
      hw_input_dark_gamma_corr(x, y) = pow(hw_input_dark(x, y), my_gamma_exponent);


      /* COMMENTING OUT THIS SECTION OF CODE FOR NOW, SINCE IT ISN'T USED ELSEWHERE 
      // RGB-TO-YUV (Y-channel is calculated above as input dark)
      Func input_u, input_v;
      
      // yuv.U = 0.492f * (rgb.B - yuv.Y)
      Func b_y_diff;

      // BUG FIX: SILENT OVERFLOW WHEN DOING U16 - I16: HALIDE CONVERTS IT TO I16 - I16. RANGE OF UNSIGNED OPERAND GETS HALVED. 
      b_y_diff(x, y) = u32(hw_input(x, y, 2)) - hw_input_dark(x, y);
      input_u(x, y) = cast<int16_t>(mul2_fixed_point_signed(b_y_diff(x, y), cast<int16_t>(0.492f * 256.0f)));

      // yuv.V = 0.877f * (rgb.R - yuv.Y)
      Func r_y_diff;
      r_y_diff(x, y) = u32(hw_input(x, y, 0)) - hw_input_dark(x, y);
      input_v(x, y) = cast<int16_t>(mul2_fixed_point_signed(r_y_diff(x, y), cast<int16_t>(0.877f * 256.0f)));
      */


      // Create exposure weight
      Func weight_dark, weight_bright, weight_sum, weight_dark_norm, weight_bright_norm;
      //weight_dark(x,y)   = 2 * hw_input_dark(x,y);
      //weight_bright(x,y) = 2 * hw_input_bright(x,y);

      // What is this weighting function??? 
      //weight_dark(x,y)   = hw_input_dark(x,y) << 1;
      //weight_bright(x,y) = hw_input_bright(x,y) << 1;

      //weight_dark(x, y) = exp(-12.5f * cast<float>((hw_input_dark(x, y) - cast<uint16_t>(0.5f * 256.0f))* (hw_input_dark(x, y) - cast<uint16_t>(0.5f * 256.0f))));
      //weight_bright(x, y) = exp(-12.5f * cast<float>((hw_input_bright(x, y) - cast<uint16_t>(0.5f * 256.0f))* (hw_input_bright(x, y) - cast<uint16_t>(0.5f * 256.0f))));

      //weight_dark(x, y) = 1.0f;
      //weight_bright(x, y) = 0.0f;


      // Forming weights before doing gamma correction. Divide pixel intensity by 255.f to bring it into [0, 1] range
      //weight_dark(x, y) = exp(-12.5f * cast<float>(((cast<float>(hw_input_dark(x, y))/255.f) - 0.5f) *  cast<float>((cast<float>(hw_input_dark(x, y))/255.f) - 0.5f)));
      //weight_bright(x, y) = exp(-12.5f * cast<float>(((cast<float>(hw_input_bright(x, y))/255.f) - 0.5f) *  cast<float>((cast<float>(hw_input_bright(x, y))/255.f) - 0.5f)));


      // weight_dark(x, y) = exp(-12.5f * ((((hw_input_dark(x, y))/255.f) - 0.5f) *  (((hw_input_dark(x, y))/255.f) - 0.5f)));
      // weight_bright(x, y) = exp(-12.5f * ((((hw_input_bright(x, y))/255.f) - 0.5f) * (((hw_input_bright(x, y))/255.f) - 0.5f)));

      weight_dark(x, y) = exp(-12.5f * ((((hw_input_dark(x, y))/16384.f) - 0.5f) *  (((hw_input_dark(x, y))/16384.f) - 0.5f)));
      weight_bright(x, y) = exp(-12.5f * ((((hw_input_bright(x, y))/16384.f) - 0.5f) * (((hw_input_bright(x, y))/16384.f) - 0.5f)));




      //weight_dark(x, y) = exp(-12.5f * cast<float>(((cast<float>(hw_input_dark_gamma_corr(x, y))/12.41f) - 0.5f) *  cast<float>((cast<float>(hw_input_dark_gamma_corr(x, y))/12.41f) - 0.5f)));
      //weight_bright(x, y) = exp(-12.5f * cast<float>(((cast<float>(hw_input_bright_gamma_corr(x, y))/12.41f) - 0.5f) *  cast<float>((cast<float>(hw_input_bright_gamma_corr(x, y))/12.41f) - 0.5f)));

      //weight_dark(x, y) = 0.5f;
      //weight_bright(x, y) = 0.5f;

      //weight_sum(x,y) = weight_dark(x,y) + weight_bright(x,y) + u32(1);

      // WHAT'S THIS +1 FOR???
      //weight_sum(x,y) = weight_dark(x,y) + weight_bright(x,y) + 1.0f;
      weight_sum(x,y) = weight_dark(x,y) + weight_bright(x,y);
      
      // WHY MULTIPLY BY 128???
      weight_dark_norm(x,y)   = weight_dark(x,y)   * 128.0f / weight_sum(x,y);
      weight_bright_norm(x,y) = weight_bright(x,y) * 128.0f / weight_sum(x,y);


      // this thing is probably 0 because of the datatype. That's why he multiplied by 128
      //weight_dark_norm(x,y)   = weight_dark(x,y) / weight_sum(x,y);
      //weight_bright_norm(x,y) = weight_bright(x,y) / weight_sum(x,y);
      //weight_dark_norm(x,y)   = (weight_dark(x,y) << 7) / weight_sum(x,y);
      //weight_bright_norm(x,y) = (weight_bright(x,y) << 7) / weight_sum(x,y);

      //weight_dark_norm(x,y,c)   = weight_dark(x,y,c)   * 128 ;
      //weight_bright_norm(x,y,c) = weight_bright(x,y,c) * 128 ;

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
      dark_input_lpyramid =   laplacian_pyramid(hw_input_dark_gamma_corr, pyramid_levels,
                                                dark_input_gpyramid, "dinput");
      bright_input_lpyramid = laplacian_pyramid(hw_input_bright_gamma_corr, pyramid_levels,
                                                bright_input_gpyramid, "binput");

      // Merge the input pyramids using the weight pyramids
      vector<Func> merged_pyramid(pyramid_levels);
      merged_pyramid = merge_pyramids(dark_weight_gpyramid,
                                      bright_weight_gpyramid,
                                      dark_input_lpyramid,
                                      bright_input_lpyramid,
                                      "merge_pyr");

      // Collapse the merged pyramid to create a single image
      Func blended_image, blended_image_gamma;
      vector<Func> upsampled(pyramid_levels);
      blended_image_gamma = flatten_pyramid(merged_pyramid, upsampled);

      // Undo the gamma correction
      float my_reverse_gamma_exponent = 2.2f;
      //blended_image(x, y) = pow(cast<float>(blended_image_gamma(x, y)), my_reverse_gamma_exponent);
      blended_image(x, y) = pow(blended_image_gamma(x, y), my_reverse_gamma_exponent);
      Func hw_input_dark_reverse_gamma_corr, hw_input_bright_reverse_gamma_corr;
      hw_input_dark_reverse_gamma_corr(x, y) = pow(hw_input_dark_gamma_corr(x, y), 2.2f);
      hw_input_bright_reverse_gamma_corr(x, y) = pow(hw_input_bright_gamma_corr(x, y), 2.2f);
      //GammaCorrect(blended_image_gamma, blended_image, 2.2f);
      

      // YUV-TO-RGB conversion
      Func hw_output, hw_output_signed;
      
      // R = blended_image (Y) + 1.14f * yuv.V
      Func r_out_signed, g_out_signed, b_out_signed;
      //r_out_signed(x, y) = cast<int16_t>(u32(hw_input_dark(x,y))); 
      //r_out_signed(x, y) = cast<int16_t>(u32(hw_input_bright(x,y))); 
      //r_out_signed(x, y) = cast<int16_t>(u32(hw_input_dark(x,y)) + mul2_fixed_point_signed(input_v(x, y), cast<int16_t>(1.14f * 256.0f)));
      //r_out_signed(x, y) = cast<int16_t>(u32(blended_image(x,y)));
      //r_out_signed(x, y) = cast<int16_t>(u32(blended_image(x,y)) + mul2_fixed_point_signed(input_v(x, y), cast<int16_t>(1.14f * 256.0f)));
      

      Func r_channel_scale_factor;
      //r_channel_scale_factor(x, y) = cast<float>(hw_input(x, y, 0))/cast<float>(hw_input_dark(x, y));
      r_channel_scale_factor(x, y) = hw_input_float(x, y, 0)/hw_input_dark(x, y);
      //r_out_signed(x, y) = mul2_fixed_point(cast<uint16_t>(blended_image(x, y)), cast<uint16_t>(r_channel_scale_factor(x, y) * 256.0f));
      r_out_signed(x, y) = blended_image(x, y) * r_channel_scale_factor(x, y);

      //r_out_signed(x, y) = cast<int16_t>(u32(cast<uint16_t>(blended_image(x, y) * 256.0f)) + mul2_fixed_point_signed(input_v(x, y), cast<int16_t>(1.14f * 256.0f)));
      //r_out_signed(x, y) = cast<int16_t>(blended_image(x, y));
      //r_out_signed(x, y) = mul2_fixed_point(cast<uint16_t>(weight_dark_norm(x,y)/128.0f * 256.0f), cast<uint16_t>(hw_input_dark_reverse_gamma_corr(x,y))) +
      //     mul2_fixed_point(cast<uint16_t>(weight_bright_norm(x,y)/128.0f * 256.0f), cast<uint16_t>(hw_input_bright_reverse_gamma_corr(x,y)));
      //r_out_signed(x, y) = cast<int16_t>(hw_input_bright_reverse_gamma_corr(x, y));

      // G = blended_image (Y) - 0.395f * yuv.U - 0.581f * yuv.V
      //g_out_signed(x, y) = cast<int16_t>(u32(hw_input_dark(x,y))); 
      //g_out_signed(x, y) = cast<int16_t>(u32(hw_input_bright(x,y))); 
      //g_out_signed(x, y) = cast<int16_t>(u32(hw_input_dark(x,y)) - mul2_fixed_point_signed(input_u(x, y), cast<int16_t>(0.395f * 256.0f)) - mul2_fixed_point_signed(input_v(x, y), cast<int16_t>(0.581f * 256.0f)));
      //g_out_signed(x, y) = cast<int16_t>(u32(blended_image(x,y)));
      //g_out_signed(x, y) = cast<int16_t>(u32(blended_image(x,y)) - mul2_fixed_point_signed(input_u(x, y), cast<int16_t>(0.395f * 256.0f)) - mul2_fixed_point_signed(input_v(x, y), cast<int16_t>(0.581f * 256.0f)));
      Func g_channel_scale_factor;
      //g_channel_scale_factor(x, y) = cast<float>(hw_input(x, y, 1))/cast<float>(hw_input_dark(x, y));
      g_channel_scale_factor(x, y) = hw_input_float(x, y, 1)/hw_input_dark(x, y);
      //g_out_signed(x, y) = mul2_fixed_point(cast<uint16_t>(blended_image(x, y)), cast<uint16_t>(g_channel_scale_factor(x, y) * 256.0f));
      g_out_signed(x, y) = blended_image(x, y) * g_channel_scale_factor(x, y);

      //g_out_signed(x, y) = cast<int16_t>(u32(cast<uint16_t>(blended_image(x, y) * 256.0f)) - mul2_fixed_point_signed(input_u(x, y), cast<int16_t>(0.395f * 256.0f)) - mul2_fixed_point_signed(input_v(x, y), cast<int16_t>(0.581f * 256.0f)));
      //g_out_signed(x, y) = cast<int16_t>(blended_image(x, y));
      //g_out_signed(x, y) = mul2_fixed_point(cast<uint16_t>(weight_dark_norm(x,y)/128.0f * 256.0f), cast<uint16_t>(hw_input_dark_reverse_gamma_corr(x,y))) +
      //     mul2_fixed_point(cast<uint16_t>(weight_bright_norm(x,y)/128.0f * 256.0f), cast<uint16_t>(hw_input_bright_reverse_gamma_corr(x,y)));
      //g_out_signed(x, y) = cast<int16_t>(hw_input_bright_reverse_gamma_corr(x, y));

      // B = blended_image (Y) + 2.033f * yuv.U
      //b_out_signed(x, y) = cast<int16_t>(u32(hw_input_dark(x,y))); 
      //b_out_signed(x, y) = cast<int16_t>(u32(hw_input_bright(x,y))); 
      //b_out_signed(x, y) = cast<int16_t>(u32(hw_input_dark(x,y)) + mul2_fixed_point_signed(input_u(x, y), cast<int16_t>(2.033f * 256.0f)));
      //b_out_signed(x, y) = cast<int16_t>(u32(blended_image(x,y)));
      //b_out_signed(x, y) = cast<int16_t>(u32(blended_image(x,y)) + mul2_fixed_point_signed(input_u(x, y), cast<int16_t>(2.033f * 256.0f)));
      Func b_channel_scale_factor;
      //b_channel_scale_factor(x, y) = cast<float>(hw_input(x, y, 2))/cast<float>(hw_input_dark(x, y));
      b_channel_scale_factor(x, y) = hw_input_float(x, y, 2)/hw_input_dark(x, y);
      //b_out_signed(x, y) = mul2_fixed_point(cast<uint16_t>(blended_image(x, y)), cast<uint16_t>(b_channel_scale_factor(x, y) * 256.0f));
      b_out_signed(x, y) = blended_image(x, y) * b_channel_scale_factor(x, y);


      //b_out_signed(x, y) = cast<int16_t>(u32(cast<uint16_t>(blended_image(x, y) * 256.0f)) + mul2_fixed_point_signed(input_u(x, y), cast<int16_t>(2.033f * 256.0f)));
      //b_out_signed(x, y) = cast<int16_t>(blended_image(x, y));
      //b_out_signed(x, y) = mul2_fixed_point(cast<uint16_t>(weight_dark_norm(x,y)/128.0f * 256.0f), cast<uint16_t>(hw_input_dark_reverse_gamma_corr(x,y))) +
      //     mul2_fixed_point(cast<uint16_t>(weight_bright_norm(x,y)/128.0f * 256.0f), cast<uint16_t>(hw_input_bright_reverse_gamma_corr(x,y)));
      //b_out_signed(x, y) = cast<int16_t>(hw_input_bright_reverse_gamma_corr(x, y));



      // MAYBE WITH THE NEW FORMULA, DON'T NEED TO DO THIS SIGNED CAST ANYMORE 
      //hw_output_signed(x,y,c) = cast<int16_t>(select(c == 0, r_out_signed(x, y),
      //                           c == 1, g_out_signed(x, y),
      //                                   b_out_signed(x, y)));
      hw_output_signed(x,y,c) = select(c == 0, r_out_signed(x, y),
                                 c == 1, g_out_signed(x, y),
                                         b_out_signed(x, y));

      
     
      //hw_output(x,y,c) = dark_input_lpyramid[pyramid_levels-1](x,y,c);
      //hw_output(x,y,c) = dark_input_gpyramid[pyramid_levels-1](x,y,c);

      //hw_output(x, y, c) = clamp(hw_output_signed(x, y, c), u16(0), u16(255));

      //hw_output(x,y,c) = select(c == 0, r_out_signed(x, y),
      //                          c == 1, g_out_signed(x, y),
      //                                  b_out_signed(x, y));
      //hw_output(x, y, c) = clamp(hw_output_signed(x, y, c), u16(0), u16(255));
      hw_output(x, y, c) = hw_output_signed(x, y, c);

      Expr minRaw = 25;
      Expr maxRaw = 16368;
      Expr invRange = 1.0f / (maxRaw - minRaw);

      
      //output(x,y,c) = u8(hw_output(x,y,c));
      output(x,y,c) = u8(clamp(cast<float>(hw_output(x,y,c) - minRaw) * invRange, 0.0f, 1.0f) * 256.f);

      output.bound(c, 0, 3);
      //output.bound(x, 0, 64-ksize+1);
      //output.bound(y, 0, 64-ksize+1);

        
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



    Func downsample_float(Func f_in, Expr size) {
        Var x, y;
        using Halide::_;
        Func f, down;
        f(x, y, _) = f_in(x, y, _);

        down(x, y, _) = (1.f/64.f) * f(size*x-1, size*y-1, _) + (3.f/64.f) * f(size*x-1, size*y, _) + (3.f/64.f) * f(size*x-1, size*y+1, _) + (1.f/64.f) * f(size*x-1, size*y+2, _) 
                        + (3.f/64.f) * f(size*x, size*y-1, _) + (9.f/64.f) * f(size*x, size*y, _) + (9.f/64.f) * f(size*x, size*y+1, _) + (3.f/64.f) * f(size*x, size*y+2, _) 
                        + (3.f/64.f) * f(size*x+1, size*y-1, _) + (9.f/64.f) * f(size*x+1, size*y, _) + (9.f/64.f) * f(size*x+1, size*y+1, _) + (3.f/64.f) * f(size*x+1, size*y+2, _) 
                        + (1.f/64.f) * f(size*x+2, size*y-1, _) + (3.f/64.f) * f(size*x+2, size*y, _) + (3.f/64.f) * f(size*x+2, size*y+1, _) + (1.f/64.f) * f(size*x+2, size*y+2, _);
        return down;
    }


    Func upsample_float_size_2(Func f_in) {
      Var x, y;
      using Halide::_;
      Func up;

      up(x, y, _) = (0.75f * 0.75f) * f_in(x/2, y/2, _) + (0.75f * 0.25f) * f_in(x/2, (y/2) - 1 + 2*(y % 2), _) + 
      (0.25f * 0.75f) * f_in((x/2) - 1 + 2*(x % 2), y/2, _) + (0.25f * 0.25f) * f_in((x/2) - 1 + 2*(x % 2), (y/2) - 1 + 2*(y % 2), _);

      return up;
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
        //gPyramid[j](x,y,_) = downsample(gPyramid[j-1])(x,y,_);
        gPyramid[j](x,y,_) = downsample_float(gPyramid[j-1], 2)(x,y,_);
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
        lPyramid[j](x,y,_) = gPyramid[j](x,y,_) - upsample_float_size_2(gPyramid[j+1])(x,y,_);
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
          //(weights0[i](x,y,_) * img0[i](x,y,_) +
          // weights1[i](x,y,_) * img1[i](x,y,_)) / 128;
          //(weights0[i](x,y,_) * img0[i](x,y,_) +
          // weights1[i](x,y,_) * img1[i](x,y,_)) >> 7;

          //(weights0[i](x,y,_) * img0[i](x,y,_) +
          // weights1[i](x,y,_) * img1[i](x,y,_));


          // MOST RECENT CODE
          // SHIFT WEIGHT RIGHT BY 7 BEFORE MULTPLYING BY 256? CAN'T DO IT AFTER THE FACT MAYBE 
          //(mul2_fixed_point(cast<uint16_t>(weights0[i](x,y,_)/128.0f * 256.0f), cast<uint16_t>(img0[i](x,y,_))) +
          // mul2_fixed_point(cast<uint16_t>(weights1[i](x,y,_)/128.0f * 256.0f), cast<uint16_t>(img1[i](x,y,_))));

          (weights0[i](x,y,_)/128.f * img0[i](x,y,_) +
          weights1[i](x,y,_)/128.f * img1[i](x,y,_));



          //(mul2_fixed_point(cast<uint16_t>(weights0[i](x,y,_)/128.0f * 256.0f), img0[i](x,y,_)) +
          // mul2_fixed_point(cast<uint16_t>(weights1[i](x,y,_)/128.0f * 256.0f), img1[i](x,y,_)));


          //(mul2_fixed_point(cast<uint16_t>(weights0[i](x,y,_) * 256.0f), img0[i](x,y,_)) +
          // mul2_fixed_point(cast<uint16_t>(weights1[i](x,y,_) * 256.0f), img1[i](x,y,_)));
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
        upsampled[level-1](x,y,_) = pyramid[level-1](x,y,_) + upsample_float_size_2(upsampled[level])(x,y,_);
      }
      
      return upsampled[0];
    }
    
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(ExposureFusion, exposure_fusion)
