/*
 * An application that performs a simplified version of Google's HDR+ pipeline 
 * 
 */
#define T_SIZE 16
#define LOG_2_T_SIZE 4
#define G_T_SIZE 8
#include "Halide.h"

namespace {
int blockSize = 9;
//int blockSize = 5;
//int blockSize = 1;
constexpr int maxJ = 6;

using namespace Halide;
using namespace Halide::ConciseCasts;
using std::vector;
Var x("x"), y("y"), tx("tx"), ty("ty"), xy("xy"), xo("xo"), yo("yo"), xi("xi"), yi("yi"), c("c"), n("n"), k("k");

void fill_funcnames(vector<Func>& funcs, std::string name) {
  for (size_t i=0; i<funcs.size(); ++i) {
      funcs[i] = Func(name + "_" + std::to_string(i));
    }
  }

  // int16_t matrix[3][4] = {{549, -103,   7, -10221},
  //                         {-96,  373,  62,  -7254},
  //                         {-31, -261, 883,  -5563}};
  
   
  /*
  int16_t matrix[3][4] = {{476, -180,  -40, 0},
                          {-60,  426,  -110, 0},
                          {2, -168, 420, 0}};
  */

  // Divide everything above by 256 
  // float matrix[3][4] = {{1.859375, -0.703125,  -0.15625, 0},
  //                       {-0.234375,  1.6640625,  -0.4296875, 0},
  //                       {0.0078125, -0.65625, 1.640625, 0}};



  // For taxi 
  // float matrix[3][4] = {{1.7734375, -0.765625,  -0.0078125, 0},
  //                       {-0.2578125,  1.5078125,  -0.25, 0},
  //                       {0, -0.7265625, 1.7265625, 0}};

  // For taxi 
  // Multiply everything above by 256 
  int16_t matrix[3][4] = {{454, -196,  -2, 0},
                          {-66,  386,  -64, 0},
                          {0, -186, 442, 0}};

class HDRPlus : public Halide::Generator<HDRPlus> {
public:
    // FOR ALIGN
    //GeneratorParam<int>     pyramid_levels{"pyramid_levels", 4, 1, maxJ};
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 5};
    GeneratorParam<int>     total_downsample_factor{"total_downsample_factor", 16}; // 2^ pyramid_levels-1

    // FOR MERGE 
    GeneratorParam<uint8_t>      normalization_shift{"normalization_shift", 8, 1, 16}; 
    //GeneratorParam<uint16_t>     min_dist{"min_dist", 16, 1, 128}; 
    //GeneratorParam<uint16_t>     max_dist{"max_dist", 256, 32, 4096};
    //GeneratorParam<uint16_t>     min_dist{"min_dist", 5, 1, 128}; 
    //GeneratorParam<float>     min_dist{"min_dist", 0.005}; 
    //GeneratorParam<float>     min_dist{"min_dist", 5.1}; 
    GeneratorParam<uint16_t>     min_dist{"min_dist", 5}; 
    //GeneratorParam<uint16_t>     max_dist{"max_dist", 26, 32, 4096};
    //GeneratorParam<float>     max_dist{"max_dist", 0.025}; 
    //GeneratorParam<float>     max_dist{"max_dist", 25.6}; 
    GeneratorParam<uint16_t>     max_dist{"max_dist", 26}; 
    //GeneratorParam<uint16_t>     dist_max_min_diff{"dist_max_min_diff", 240};  
    //GeneratorParam<uint16_t>     dist_max_min_diff{"dist_max_min_diff", 21};  
    //GeneratorParam<float>      dist_max_min_diff{"dist_max_min_diff", 0.02}; 
    //GeneratorParam<float>      dist_max_min_diff{"dist_max_min_diff", 20.5}; 
    GeneratorParam<uint16_t>      dist_max_min_diff{"dist_max_min_diff", 21}; 


    // FOR CAMERA PIPELINE
    GeneratorParam<float> gamma{"gamma", /*default=*/2.0}; // default: 2.0; USED: 3.2 
    GeneratorParam<float> contrast{"contrast", /*default=*/100.0}; // default: 50, USED: 100
    GeneratorParam<uint8_t> schedule{"schedule", 3};    // default: 3
    GeneratorParam<uint8_t> width{"width", 0};          // default: 0
    GeneratorParam<uint8_t> myunroll{"myunroll", 1};    // default: 1
    GeneratorParam<float> blackLevel{"blackLevel", 1};    // default: 25
    GeneratorParam<float> whiteLevel{"whiteLevel", 1023};    // default: 1023
    //GeneratorParam<float> whiteLevel{"whiteLevel", 16383};    // default: 1023
    //GeneratorParam<float> whiteLevel{"whiteLevel", 3072};    // default: 1023

    // FOR EXPOSURE FUSION
    int ef_pyramid_levels = 4;
    int bright_weighting = 3; // this would be used if a single frame is used
    int ksize = 23;
    int shift = 11;

    // Operate on raw bayer image: so 2 channels, plus 1 channel b/c receiving multiple images
    Input<Buffer<uint16_t>>  input{"input", 3};
    //Input<Buffer<float>>  input{"input", 3};

    // Output a single 8-bit RGB image 
    Output<Buffer<uint8_t>> output{"output", 3};

    Expr mul2(Expr a, Expr b) {
      return u16(( (u32(a)) * (u32(b)) ) >> 8);
    }

    Func interleave_x(Func a, Func b) {
      Func out;
      out(x, y) = select((x%2)==0, a(x/2, y), b(x/2, y));
      return out;
    }

    Func interleave_y(Func a, Func b) {
      Func out;
      out(x, y) = select((y%2)==0, a(x, y/2), b(x, y/2));
      return out;
    }

    Expr avg(Expr a, Expr b) {
      //return (a + b + 1) >> 1;
      return (a + b) >> 1;
    }

     Expr avg_float(Expr a, Expr b) {
      //return (a + b + 1) >> 1;
      return (a + b)/2;
    }

    Expr absd_float(Expr a, Expr b) {
      //return (a + b + 1) >> 1;
      return abs(a - b);
    }
    
    // Performs hot pixel suppression by comparing to local pixels.
    Func hot_pixel_suppression(Func input) {
      Func denoised("denoised");
      Expr max_value = max(max(input(x-2, y), input(x+2, y)),
                           max(input(x, y-2), input(x, y+2)));
      Expr min_value = min(min(input(x-2, y), input(x+2, y)),
                           min(input(x, y-2), input(x, y+2)));
      
      //denoised(x, y) = clamp(input(x,y), min_value, max_value);
      denoised(x, y) = clamp(input(x,y), u16(0), max_value);
      //denoised(x, y) = input(x, y);
      return denoised;
    }


    Func compute_channel_maximum(Func channel_in) {
      //Expr bayer_color_channel_width = input.dim(0).extent()/2;
      const int bayer_color_channel_width = 2092;
      //Expr bayer_color_channel_height = input.dim(1).extent()/2;
      const int bayer_color_channel_height = 1548;
      //Expr bayer_color_channel_height = 1000;
      Halide::RDom r(0, bayer_color_channel_width, 0, bayer_color_channel_height); 
      Func channel_max;
      channel_max() = Halide::maximum(channel_in(r.x, r.y));
      return channel_max;
    }



    Func my_demosaic(Func input){
      
      Var x, y;

      Expr row_is_even = (y%2 == 0);
      Expr col_is_even = (x%2 == 0);

      Expr is_red_point = row_is_even && !(col_is_even);
      Expr is_blue_point = !(row_is_even) && col_is_even;
      Expr is_green_r_point = row_is_even && col_is_even;
      Expr is_green_b_point = !(row_is_even) && !(col_is_even);


      //Expr neighbor_blue_red = (input(x+1, y+1) + input(x+1, y-1) + input(x-1, y+1) + input(x-1, y-1))/4.0f;
      Expr neighbor_blue_red = (input(x+1, y+1) + input(x+1, y-1) + input(x-1, y+1) + input(x-1, y-1)) >> 2;
      Expr neighbor_red_blue = neighbor_blue_red;
      
      //Expr neighbor_green_red = (input(x-1, y) + input(x+1, y) + input(x, y-1) + input(x, y+1))/4.0f;
      Expr neighbor_green_red = (input(x-1, y) + input(x+1, y) + input(x, y-1) + input(x, y+1)) >> 2;
      Expr neighbor_green_blue = neighbor_green_red;

      // Expr neighbor_red_green_r = (input(x-1, y) + input(x+1, y))/2.0f;
      // Expr neighbor_blue_green_r = (input(x, y-1) + input(x, y+1))/2.0f;
      Expr neighbor_red_green_r = (input(x-1, y) + input(x+1, y)) >> 1;
      Expr neighbor_blue_green_r = (input(x, y-1) + input(x, y+1)) >> 1;

      Expr neighbor_red_green_b = neighbor_blue_green_r;
      Expr neighbor_blue_green_b = neighbor_red_green_r;

      Expr r = select(is_red_point, input(x, y), (select(is_blue_point, neighbor_red_blue, (select(is_green_r_point, neighbor_red_green_r, neighbor_red_green_b)))));  
      
      Expr g = select(is_green_r_point || is_green_b_point, input(x, y), (select(is_red_point, neighbor_green_red, neighbor_green_blue)));

      Expr b = select(is_blue_point, input(x, y), (select(is_red_point, neighbor_blue_red, (select(is_green_r_point, neighbor_blue_green_r, neighbor_blue_green_b)))));
    
      Func demosaicked("demosaicked");
      
      demosaicked(x, y, c) = select(c == 0, r,
                                   c == 1, g,
                                   b);


      // demosaicked(x, y, c) = cast<int16_t>(select(c == 0, r,
      //                         c == 1, g,
      //                         b));

      
      //demosaicked(x, y, c) = 0.0f;
      return demosaicked;
    }


    // Demosaics a raw input image. Recall that the values appear as:
    //   R G R G R G R G
    //   G B G B G B G B
    //   R G R G R G R G
    //   G B G B G B G B
    Func demosaic(Func g_gr, Func r_r, Func b_b, Func g_gb,
      Func b_r, Func g_r, Func b_gr, Func r_gr, Func b_gb, Func r_gb, Func r_b, Func g_b) {
      // The demosaic algorithm is optimized for HLS schedule
      // such that the bound analysis can derive a constant window
      // and shift step without needed to unroll 'demosaic' into
      // a 2x2 grid.
      //
      // The changes made from the original is that there is no
      // explict downsample and upsample in 'deinterleave' and
      // 'interleave', respectively.
      // All the intermediate functions are the same size as the
      // raw image although only pixels at even coordinates are used.

      // These are the values we already know from the input
      // x_y = the value of channel x at a site in the input of channel y
      // gb refers to green sites in the blue rows
      // gr refers to green sites in the red rows

      // These are the ones we need to interpolate
      //Func b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b;

      // First calculate green at the red and blue sites
      // Try interpolating vertically and horizontally. Also compute
      // differences vertically and horizontally. Use interpolation in
      // whichever direction had the smallest difference.
      //Expr gv_r  = avg(g_gb(x, y-1), g_gb(x, y));
      //Expr gvd_r = absd(g_gb(x, y-1), g_gb(x, y));
      //Expr gh_r  = avg(g_gr(x+1, y), g_gr(x, y));
      //Expr ghd_r = absd(g_gr(x+1, y), g_gr(x, y));

      Expr gv_r  = avg_float(g_gb(x, y-1), g_gb(x, y));
      Expr gvd_r = absd_float(g_gb(x, y-1), g_gb(x, y));
      Expr gh_r  = avg_float(g_gr(x+1, y), g_gr(x, y));
      Expr ghd_r = absd_float(g_gr(x+1, y), g_gr(x, y));

      g_r(x, y)  = select(ghd_r < gvd_r, gh_r, gv_r);

      // Expr gv_b  = avg(g_gr(x, y+1), g_gr(x, y));
      // Expr gvd_b = absd(g_gr(x, y+1), g_gr(x, y));
      // Expr gh_b  = avg(g_gb(x-1, y), g_gb(x, y));
      // Expr ghd_b = absd(g_gb(x-1, y), g_gb(x, y));

      Expr gv_b  = avg_float(g_gr(x, y+1), g_gr(x, y));
      Expr gvd_b = absd_float(g_gr(x, y+1), g_gr(x, y));
      Expr gh_b  = avg_float(g_gb(x-1, y), g_gb(x, y));
      Expr ghd_b = absd_float(g_gb(x-1, y), g_gb(x, y));

      g_b(x, y)  = select(ghd_b < gvd_b, gh_b, gv_b);

      // Next interpolate red at gr by first interpolating, then
      // correcting using the error green would have had if we had
      // interpolated it in the same way (i.e. add the second derivative
      // of the green channel at the same place).
      Expr correction;
      /*
      correction = g_gr(x, y) - avg(g_r(x, y), g_r(x-1, y));
      r_gr(x, y) = correction + avg(r_r(x-1, y), r_r(x, y));

      // Do the same for other reds and blues at green sites
      correction = g_gr(x, y) - avg(g_b(x, y), g_b(x, y-1));
      b_gr(x, y) = correction + avg(b_b(x, y), b_b(x, y-1));

      correction = g_gb(x, y) - avg(g_r(x, y), g_r(x, y+1));
      r_gb(x, y) = correction + avg(r_r(x, y), r_r(x, y+1));

      correction = g_gb(x, y) - avg(g_b(x, y), g_b(x+1, y));
      b_gb(x, y) = correction + avg(b_b(x, y), b_b(x+1, y));
      */

      correction = g_gr(x, y) - avg_float(g_r(x, y), g_r(x-1, y));
      r_gr(x, y) = correction + avg_float(r_r(x-1, y), r_r(x, y));

      // Do the same for other reds and blues at green sites
      correction = g_gr(x, y) - avg_float(g_b(x, y), g_b(x, y-1));
      b_gr(x, y) = correction + avg_float(b_b(x, y), b_b(x, y-1));

      correction = g_gb(x, y) - avg_float(g_r(x, y), g_r(x, y+1));
      r_gb(x, y) = correction + avg_float(r_r(x, y), r_r(x, y+1));

      correction = g_gb(x, y) - avg_float(g_b(x, y), g_b(x+1, y));
      b_gb(x, y) = correction + avg_float(b_b(x, y), b_b(x+1, y));

      // Now interpolate diagonally to get red at blue and blue at
      // red. Hold onto your hats; this gets really fancy. We do the
      // same thing as for interpolating green where we try both
      // directions (in this case the positive and negative diagonals),
      // and use the one with the lowest absolute difference. But we
      // also use the same trick as interpolating red and blue at green
      // sites - we correct our interpolations using the second
      // derivative of green at the same sites.
      // correction = g_b(x, y)  - avg(g_r(x, y), g_r(x-1, y+1));
      // Expr rp_b  = correction + avg(r_r(x, y), r_r(x-1, y+1));
      // Expr rpd_b = absd(r_r(x, y), r_r(x-1, y+1));

      correction = g_b(x, y)  - avg_float(g_r(x, y), g_r(x-1, y+1));
      Expr rp_b  = correction + avg_float(r_r(x, y), r_r(x-1, y+1));
      Expr rpd_b = absd_float(r_r(x, y), r_r(x-1, y+1));


      // correction = g_b(x, y)  - avg(g_r(x-1, y), g_r(x, y+1));
      // Expr rn_b  = correction + avg(r_r(x-1, y), r_r(x, y+1));
      // Expr rnd_b = absd(r_r(x-1, y), r_r(x, y+1));

      correction = g_b(x, y)  - avg_float(g_r(x-1, y), g_r(x, y+1));
      Expr rn_b  = correction + avg_float(r_r(x-1, y), r_r(x, y+1));
      Expr rnd_b = absd_float(r_r(x-1, y), r_r(x, y+1));

      r_b(x, y)  = select(rpd_b < rnd_b, rp_b, rn_b);

      // Same thing for blue at red
      // correction = g_r(x, y)  - avg(g_b(x, y), g_b(x+1, y-1));
      // Expr bp_r  = correction + avg(b_b(x, y), b_b(x+1, y-1));
      // Expr bpd_r = absd(b_b(x, y), b_b(x+1, y-1));

      correction = g_r(x, y)  - avg_float(g_b(x, y), g_b(x+1, y-1));
      Expr bp_r  = correction + avg_float(b_b(x, y), b_b(x+1, y-1));
      Expr bpd_r = absd_float(b_b(x, y), b_b(x+1, y-1));

      // correction = g_r(x, y)  - avg(g_b(x+1, y), g_b(x, y-1));
      // Expr bn_r  = correction + avg(b_b(x+1, y), b_b(x, y-1));
      // Expr bnd_r = absd(b_b(x+1, y), b_b(x, y-1));

      correction = g_r(x, y)  - avg_float(g_b(x+1, y), g_b(x, y-1));
      Expr bn_r  = correction + avg_float(b_b(x+1, y), b_b(x, y-1));
      Expr bnd_r = absd_float(b_b(x+1, y), b_b(x, y-1));

      b_r(x, y)  =  select(bpd_r < bnd_r, bp_r, bn_r);

      // Interleave the resulting channels
      Func r = interleave_y(interleave_x(r_gr, r_r),
                            interleave_x(r_b, r_gb));
      Func g = interleave_y(interleave_x(g_gr, g_r),
                            interleave_x(g_b, g_gb));
      Func b = interleave_y(interleave_x(b_gr, b_r),
                            interleave_x(b_b, b_gb));

      Func demosaicked("demosaicked");
      //demosaicked(x, y, c) = cast<int16_t>(select(c == 0, r(x, y),
      //                              c == 1, g(x, y),
      //                              b(x, y)));
      demosaicked(x, y, c) = select(c == 0, r(x, y),
                                    c == 1, g(x, y),
                                    b(x, y));
      //demosaicked.bound(c, 0, 3);
      return demosaicked;
    }


    // Multiply with maximum precision and make result 8.8 p
    Expr mul1(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 8);
    }


    Expr mul_1023(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 10);
    }


    Expr mul_1023_unsigned(Expr a, Expr b) {
      return u16(( (u32(a)) * (u32(b)) ) >> 10);
    }

    // below are ways to do smul1 with only umul1
    Expr mul1_ma(Expr a, Expr b) {
      return i16(( (i32(a) & 0xffff) * (i32(b)) ) >> 8);
    }
    
    Expr umul1(Expr a, Expr b) {
      return i16(( (i32(a)&0xffff) * (i32(b)&0xffff) ) >> 8);
    }
    Expr smul1_verbose(Expr a, Expr b) {
      return umul1(a, b) +
        i16( (i16(0xFF00) * select((a>>15) == (b>>15), i16(0),
                                   select(a>>15 == 1, i16(b), i16(a)) ) ));
    }
    Expr mul1n_verbose(Expr a, Expr b) {
      return mul1_ma(a, b) +
        i16( select((a>>15) == 0, i16(0), i16(b) * i16(0xFF00)));
    }
    Expr mul1n(Expr a, Expr b) {
      return mul1_ma(a, b) +
        i16(b) * i16(0xFF00);
    }
    Expr mul1p(Expr a, Expr b) {
      return mul1(a, b);
    }


    
    // Applies a color correction matrix to redefine rgb values.
    // Matrix is defined in 8.8 fixed point
    //Func color_correct(Func input, int32_t matrix[3][4]) {
    Func color_correct(Func input, int16_t matrix[3][4]) {
    //Func color_correct(Func input, float matrix[3][4]) {
      //Expr ir = clamp(input(x, y, 0), 0, 10000);
      //Expr ig = clamp(input(x, y, 1), 0, 10000);
      //Expr ib = clamp(input(x, y, 2), 0, 10000);

      // Clipping at 0, but not putting an upper bound
      Expr ir = select(input(x, y, 0) < 0, 0, input(x, y, 0));
      Expr ig = select(input(x, y, 1) < 0, 0, input(x, y, 1));
      Expr ib = select(input(x, y, 2) < 0, 0, input(x, y, 2));


      Expr r = (matrix[0][3]>>8) + mul1(matrix[0][0], ir) + mul1(matrix[0][1], ig) + mul1(matrix[0][2], ib);
      Expr g = (matrix[1][3]>>8) + mul1(matrix[1][0], ir) + mul1(matrix[1][1], ig) + mul1(matrix[1][2], ib);
      Expr b = (matrix[2][3]>>8) + mul1(matrix[2][0], ir) + mul1(matrix[2][1], ig) + mul1(matrix[2][2], ib);


      // Expr r = matrix[0][0] * ir + matrix[0][1] * ig + matrix[0][2] * ib;
      // Expr g = matrix[1][0] * ir + matrix[1][1] * ig + matrix[1][2] * ib;
      // Expr b = matrix[2][0] * ir + matrix[2][1] * ig + matrix[2][2] * ib;

      Func corrected("corrected");
      corrected(x, y,  c) = select(c == 0, r,
                                  c == 1, g,
                                  b);
      return corrected;
    }

  
    // Applies a non-linear camera curve.
    Func apply_curve(Func input, Func curve) {
      // copied from FCam

      Func curved("curved");

      // This clamping is here to handle negative output from demosaicking int datatype
      // Anything <0 is converted to 0. 
      //Expr in_val = clamp(input(x, y, c), u16(0), u16(1023));
      //Expr in_val = clamp(input(x, y, c), 0.0f, 1023.f);

      //Expr in_val = select(input(x, y, c) < 0, 0.0f, input(x, y, c));
      Expr in_val = select(input(x, y, c) < 0, u16(0), u16(input(x, y, c)));
      curved(x, y, c) = curve(u16(in_val));

      //curved(x, y, c) = in_val;
      //curved(x, y, c) = curve(in_val);

      return curved;
    }

    Func apply_forward_gamma_corr(Func input, Func forward_gamma_corr) {

      Func gamma_corrected;

      // FIXME: THIS U16 CAST IS TEMPORARY 
      //Expr in_val = u16(input(x, y));
      Expr in_val = input(x, y);
      gamma_corrected(x, y) = forward_gamma_corr(in_val);

      return gamma_corrected;
    }

    Func apply_final_forward_gamma_corr(Func input, Func forward_gamma_corr) {

      Func gamma_corrected;

      // FIXME: THIS U16 CAST IS TEMPORARY 
      //Expr in_val = u16(input(x, y));
      Expr in_val = input(x, y, c);
      gamma_corrected(x, y, c) = forward_gamma_corr(in_val);

      return gamma_corrected;
    }


      Func apply_reverse_gamma_corr(Func input, Func reverse_gamma_corr) {

      Func gamma_corrected;

      // FIXME: THIS U16 CAST IS TEMPORARY 
      //Expr in_val = u16(input(x, y));
      Expr in_val = input(x, y);
      gamma_corrected(x, y) = reverse_gamma_corr(in_val);

      return gamma_corrected;
    }

    Func gamma_correction (Func input, float gamma_factor){
      Func output;
      Expr pixel_out = pow(input(x, y, c), gamma_factor);
      output(x, y, c) = pixel_out;
      return output;
    }

    const int tWidth = 256-8;
    const int tHeight = 192-8;
    const int nTiles = 1;

    void generate() {
        // Algorithm
    
        /* 
         * ALIGN STEP
         */
        const int J = pyramid_levels;

       Func clamped_input, clamped_input_float;

       /* Func: clamped_input
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): deinterleaved, val (in merge)
        */
        clamped_input = Halide::BoundaryConditions::repeat_edge(input);
  

       /* Func: deinterleaved
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): gray
        */
        Func deinterleaved;
        deinterleaved(x, y, c, n) = select(c == 0, clamped_input(2 * x, 2 * y, n), (select(c == 1, clamped_input(2 * x + 1, 2 * y, n), 
                                            (select(c == 2, clamped_input(2 * x, 2 * y + 1, n), clamped_input(2 * x + 1, 2 * y + 1, n))))));


        //Var c;
        //clamped_input_shuffle(x, y, c, n) = clamped_input(2*x + c/2, 2*y + c%2, n);


       // STEP 1: Convert to grayscale 

       /* Func: gray
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): pyramids in align, dist_tile (in merge)
        */
        Func gray;   
        gray(x, y, n) = u16((deinterleaved(x, y, 1, n) + deinterleaved(x, y, 0, n) + deinterleaved(x, y, 3, n) + deinterleaved(x, y, 2, n)) >> 2); 
       
        //STEP 2: Downsample to form image pyramids 
     
        Expr initialGaussWidth[J];
        Expr initialGaussHeight[J];

        initialGaussWidth[0] = input.width()/2;
        initialGaussWidth[1] = input.width()/4;
        initialGaussWidth[2] = input.width()/8;
        initialGaussWidth[3] = input.width()/16;
        initialGaussWidth[4] = input.width()/32;

        initialGaussHeight[0] = input.height()/2;
        initialGaussHeight[1] = input.height()/4;
        initialGaussHeight[2] = input.height()/8;
        initialGaussHeight[3] = input.height()/16;
        initialGaussHeight[4] = input.height()/32;


       /* Func: gPyramid[J]
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): dist & scores calculation (in align)
        */
        Func gPyramid[J];
        gPyramid[0](x, y, n) = gray(x, y, n);
        for (int j = 1; j < J; j++) {
            // In the google paper, they claim they downsample the bottom level of the pyramid by a factor of 2 and all higher levels by a factor of 4
            Expr gauss_width = input.width()/2;
            Expr gauss_height = input.height()/2;
            if (j == 1)
                //gPyramid[j](x, y, n) = downsample_float_hdr(gPyramid[j-1], 2, gauss_width, gauss_height)(x, y, n);
                gPyramid[j](x, y, n) = downsample_u16_hdr(gPyramid[j-1], 2, initialGaussWidth[j-1], initialGaussHeight[j-1])(x, y, n);
            else
                //gPyramid[j](x, y, n) = downsample_float_hdr(gPyramid[j-1], 4)(x, y, n);
                //gPyramid[j](x, y, n) = downsample_float_hdr(gPyramid[j-1], 2, gauss_width, gauss_height)(x, y, n);
                gPyramid[j](x, y, n) = downsample_u16_hdr(gPyramid[j-1], 2, initialGaussWidth[j-1], initialGaussHeight[j-1])(x, y, n);

            gauss_width = gauss_width/2;
            gauss_height = gauss_height/2;
        }

        //gPyramid[0].trace_stores();
        //gPyramid[1].trace_stores();
        //gPyramid[2].trace_stores();
        //gPyramid[3].trace_stores();
        //gPyramid[4].trace_stores();

        // STEP 3: Align pyramids and upsample the alignment back to the bottom layer 
        Func initialAlign;
        initialAlign(tx, ty, xy, n) = 0;
        Func alignPyramid[J];
        Expr min_align[J];
        Expr max_align[J];
        Expr gauss_width[J];
        Expr gauss_height[J];
        min_align[J-1] = i16(Expr(-4));
        max_align[J-1] = i16(Expr(4));


        //gauss_width[J-1] = (input.width()/2)/total_downsample_factor;
        //gauss_height[J-1] = (input.height()/2)/total_downsample_factor;


        gauss_width[4] = 39;
        gauss_width[3] = 78;
        gauss_width[2] = 156;
        gauss_width[1] = 312;
        gauss_width[0] = 625;


        gauss_height[4] = 35;
        gauss_height[3] = 70;
        gauss_height[2] = 140;
        gauss_height[1] = 280;
        gauss_height[0] = 560;


        Expr upsample_flow_gauss_widths[J];
        Expr upsample_flow_gauss_heights[J];

        upsample_flow_gauss_widths[4] = 2; 
        upsample_flow_gauss_widths[3] = 3; 
        upsample_flow_gauss_widths[2] = 5; 
        upsample_flow_gauss_widths[1] = 10; 
        upsample_flow_gauss_widths[0] = 20; 
      
        upsample_flow_gauss_heights[4] = 2;
        upsample_flow_gauss_heights[3] = 3;
        upsample_flow_gauss_heights[2] = 5;
        upsample_flow_gauss_heights[1] = 9;
        upsample_flow_gauss_heights[0] = 18;

      
        // In the google paper, they say that for the last pyramid level, they work with tile sizes of 8x8 and search radius of 4. Upsample factor is of course 4. 
        //alignPyramid[J-1](tx, ty, xy, n) = 
            //align_layer(gPyramid[J-1], initialAlign, 8, 4, 4, 0, min_align[J-1], max_align[J-1])(tx, ty, xy, n);
        //    align_layer(gPyramid[J-1], initialAlign, 16, 4, 2, gauss_width[J-1], gauss_height[J-1], min_align[J-1], max_align[J-1], upsample_flow_gauss_widths[J-1], upsample_flow_gauss_heights[J-1])(tx, ty, xy, n);
        
        /* ALIGN PYRAMID LEVEL 4*/
        Var tx_lvl_4, ty_lvl_4, xy_lvl_4, n_lvl_4;
        Var x_s_lvl_4, y_s_lvl_4;

        RDom r_tile_lvl_4(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_4(-4, 9, -4, 9);
        

       /* Func: coarse_offset_lvl_4
        * dtype: i16
        * True range: [0]
        * Consumer(s): alignPyramid[4]
        */
        Func coarse_offset_lvl_4;
        //coarse_offset_lvl_4(tx_lvl_4, ty_lvl_4, xy_lvl_4, n_lvl_4) = 2 * i32(ceil(upsample_float_size_2_for_alignment(initialAlign, upsample_flow_gauss_widths[4], upsample_flow_gauss_heights[4])(tx_lvl_4, ty_lvl_4, xy_lvl_4, n_lvl_4)));
        coarse_offset_lvl_4(tx_lvl_4, ty_lvl_4, xy_lvl_4, n_lvl_4) = i16(2 * upsample_u16_size_2_for_alignment(initialAlign, upsample_flow_gauss_widths[4], upsample_flow_gauss_heights[4])(tx_lvl_4, ty_lvl_4, xy_lvl_4, n_lvl_4));
    
        Expr x_ref_lvl_4 = clamp(tx_lvl_4 * T_SIZE + r_tile_lvl_4.x, 0, gauss_width[4]-1);
        Expr y_ref_lvl_4 = clamp(ty_lvl_4 * T_SIZE + r_tile_lvl_4.y, 0, gauss_height[4]-1);

        Expr x_cmp_lvl_4 = clamp(tx_lvl_4 * T_SIZE + r_tile_lvl_4.x + coarse_offset_lvl_4(tx_lvl_4, ty_lvl_4, 0, n_lvl_4) + x_s_lvl_4, 0, gauss_width[4]-1);
        Expr y_cmp_lvl_4 = clamp(ty_lvl_4 * T_SIZE + r_tile_lvl_4.y + coarse_offset_lvl_4(tx_lvl_4, ty_lvl_4, 1, n_lvl_4) + y_s_lvl_4, 0, gauss_height[4]-1);

        Expr dist_lvl_4 = abs(i16(gPyramid[4](x_ref_lvl_4, y_ref_lvl_4, 0)) - i16(gPyramid[4](x_cmp_lvl_4, y_cmp_lvl_4, n_lvl_4))); 

       /* Func: scores_lvl_4
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[4]
        */
        Func scores_lvl_4;
        scores_lvl_4(tx_lvl_4, ty_lvl_4, x_s_lvl_4, y_s_lvl_4, n_lvl_4) = sum(u32(dist_lvl_4));

        Tuple min_coor_lvl_4 = argmin(scores_lvl_4(tx_lvl_4, ty_lvl_4, r_search_lvl_4.x, r_search_lvl_4.y, n_lvl_4));


       /* Func: alignPyramid[4]
        * dtype: i16
        * True range: [-4, 4] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        alignPyramid[4](tx_lvl_4, ty_lvl_4, xy_lvl_4, n_lvl_4) = select(n_lvl_4 == 0, i16(0),
                    xy_lvl_4 == 0, i16(min_coor_lvl_4[0]) + coarse_offset_lvl_4(tx_lvl_4, ty_lvl_4, 0, n_lvl_4),
                    i16(min_coor_lvl_4[1]) + coarse_offset_lvl_4(tx_lvl_4, ty_lvl_4, 1, n_lvl_4)); 



        //alignPyramid[4].trace_stores();


        /* ALIGN PYRAMID LEVEL 3*/
        Var tx_lvl_3, ty_lvl_3, xy_lvl_3, n_lvl_3;
        Var x_s_lvl_3, y_s_lvl_3;
        
        RDom r_tile_lvl_3(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_3(-4, 9, -4, 9);
        
        
       /* Func: coarse_offset_lvl_3
        * dtype: i16
        * True range: [-8, 8]
        * Consumer(s): alignPyramid[3]
        */
        Func coarse_offset_lvl_3;
        coarse_offset_lvl_3(tx_lvl_3, ty_lvl_3, xy_lvl_3, n_lvl_3) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid[4], upsample_flow_gauss_widths[3], upsample_flow_gauss_heights[3])(tx_lvl_3, ty_lvl_3, xy_lvl_3, n_lvl_3));
        //coarse_offset_lvl_3.trace_stores();

        Expr x_ref_lvl_3 = clamp(tx_lvl_3 * T_SIZE + r_tile_lvl_3.x, 0, gauss_width[3]-1);
        Expr y_ref_lvl_3 = clamp(ty_lvl_3 * T_SIZE + r_tile_lvl_3.y, 0, gauss_height[3]-1);

        Expr x_cmp_lvl_3 = clamp(tx_lvl_3 * T_SIZE + r_tile_lvl_3.x + coarse_offset_lvl_3(tx_lvl_3, ty_lvl_3, 0, n_lvl_3) + x_s_lvl_3, 0, gauss_width[3]-1);
        Expr y_cmp_lvl_3 = clamp(ty_lvl_3 * T_SIZE + r_tile_lvl_3.y + coarse_offset_lvl_3(tx_lvl_3, ty_lvl_3, 1, n_lvl_3) + y_s_lvl_3, 0, gauss_height[3]-1);

        Expr dist_lvl_3 = abs(i16(gPyramid[3](x_ref_lvl_3, y_ref_lvl_3, 0)) - i16(gPyramid[3](x_cmp_lvl_3, y_cmp_lvl_3, n_lvl_3))); 

       /* Func: scores_lvl_3
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[3]
        */
        Func scores_lvl_3;
        scores_lvl_3(tx_lvl_3, ty_lvl_3, x_s_lvl_3, y_s_lvl_3, n_lvl_3) = sum(u32(dist_lvl_3));
        Tuple min_coor_lvl_3 = argmin(scores_lvl_3(tx_lvl_3, ty_lvl_3, r_search_lvl_3.x, r_search_lvl_3.y, n_lvl_3));


       /* Func: alignPyramid[3]
        * dtype: i16
        * True range: [-12, 12] (worst case)
        * Consumer(s): coarse_offset_lvl_3
        */
        alignPyramid[3](tx_lvl_3, ty_lvl_3, xy_lvl_3, n_lvl_3) = select(n_lvl_3 == 0, i16(0),
                    xy_lvl_3 == 0, i16(min_coor_lvl_3[0]) + coarse_offset_lvl_3(tx_lvl_3, ty_lvl_3, 0, n_lvl_3),
                    i16(min_coor_lvl_3[1]) + coarse_offset_lvl_3(tx_lvl_3, ty_lvl_3, 1, n_lvl_3)); 
 

        //alignPyramid[3].trace_stores();

        /* ALIGN PYRAMID LEVEL 2*/
        Var tx_lvl_2, ty_lvl_2, xy_lvl_2, n_lvl_2;
        Var x_s_lvl_2, y_s_lvl_2;
      
        RDom r_tile_lvl_2(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_2(-4, 9, -4, 9);
        
        //coarse_offset_lvl_2(tx_lvl_2, ty_lvl_2, xy_lvl_2, n_lvl_2) = i32(ceil(2 * upsample_float_size_2_for_alignment(alignPyramid[3], upsample_flow_gauss_widths[2], upsample_flow_gauss_heights[2])(tx_lvl_2, ty_lvl_2, xy_lvl_2, n_lvl_2)));
        
       /* Func: coarse_offset_lvl_2
        * dtype: i16
        * True range: [-24, 24]
        * Consumer(s): alignPyramid[2]
        */
        Func coarse_offset_lvl_2;
        coarse_offset_lvl_2(tx_lvl_2, ty_lvl_2, xy_lvl_2, n_lvl_2) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid[3], upsample_flow_gauss_widths[2], upsample_flow_gauss_heights[2])(tx_lvl_2, ty_lvl_2, xy_lvl_2, n_lvl_2));
        coarse_offset_lvl_2.trace_stores();

        Expr x_ref_lvl_2 = clamp(tx_lvl_2 * T_SIZE + r_tile_lvl_2.x, 0, gauss_width[2]-1);
        Expr y_ref_lvl_2 = clamp(ty_lvl_2 * T_SIZE + r_tile_lvl_2.y, 0, gauss_height[2]-1);

        Expr x_cmp_lvl_2 = clamp(tx_lvl_2 * T_SIZE + r_tile_lvl_2.x + coarse_offset_lvl_2(tx_lvl_2, ty_lvl_2, 0, n_lvl_2) + x_s_lvl_2, 0, gauss_width[2]-1);
        Expr y_cmp_lvl_2 = clamp(ty_lvl_2 * T_SIZE + r_tile_lvl_2.y + coarse_offset_lvl_2(tx_lvl_2, ty_lvl_2, 1, n_lvl_2) + y_s_lvl_2, 0, gauss_height[2]-1);

        Expr dist_lvl_2 = abs(i16(gPyramid[2](x_ref_lvl_2, y_ref_lvl_2, 0)) - i16(gPyramid[2](x_cmp_lvl_2, y_cmp_lvl_2, n_lvl_2))); 


       /* Func: scores_lvl_2
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[2]
        */
        Func scores_lvl_2;
        scores_lvl_2(tx_lvl_2, ty_lvl_2, x_s_lvl_2, y_s_lvl_2, n_lvl_2) = sum(u32(dist_lvl_2));
        scores_lvl_2.trace_stores();
        Tuple min_coor_lvl_2 = argmin(scores_lvl_2(tx_lvl_2, ty_lvl_2, r_search_lvl_2.x, r_search_lvl_2.y, n_lvl_2));


       /* Func: alignPyramid[2]
        * dtype: i16
        * True range: [-28, 28] (worst case)
        * Consumer(s): coarse_offset_lvl_2
        */
        alignPyramid[2](tx_lvl_2, ty_lvl_2, xy_lvl_2, n_lvl_2) = select(n_lvl_2 == 0, i16(0),
                    xy_lvl_2 == 0, i16(min_coor_lvl_2[0]) + coarse_offset_lvl_2(tx_lvl_2, ty_lvl_2, 0, n_lvl_2),
                    i16(min_coor_lvl_2[1]) + coarse_offset_lvl_2(tx_lvl_2, ty_lvl_2, 1, n_lvl_2)); 

        
        //alignPyramid[2].trace_stores();

        /* ALIGN PYRAMID LEVEL 1*/
        Var tx_lvl_1, ty_lvl_1, xy_lvl_1, n_lvl_1;
        Var x_s_lvl_1, y_s_lvl_1;
      
        RDom r_tile_lvl_1(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_1(-4, 9, -4, 9);
        
        //coarse_offset_lvl_1(tx_lvl_1, ty_lvl_1, xy_lvl_1, n_lvl_1) = 2 * i32(ceil(upsample_float_size_2_for_alignment(alignPyramid[2], upsample_flow_gauss_widths[1], upsample_flow_gauss_heights[1])(tx_lvl_1, ty_lvl_1, xy_lvl_1, n_lvl_1)));
        
       /* Func: coarse_offset_lvl_1
        * dtype: i16
        * True range: [-56, 56]
        * Consumer(s): alignPyramid[1]
        */
        Func coarse_offset_lvl_1;
        coarse_offset_lvl_1(tx_lvl_1, ty_lvl_1, xy_lvl_1, n_lvl_1) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid[2], upsample_flow_gauss_widths[1], upsample_flow_gauss_heights[1])(tx_lvl_1, ty_lvl_1, xy_lvl_1, n_lvl_1));
        //coarse_offset_lvl_1.trace_stores();

        Expr x_ref_lvl_1 = clamp(tx_lvl_1 * T_SIZE + r_tile_lvl_1.x, 0, gauss_width[1]-1);
        Expr y_ref_lvl_1 = clamp(ty_lvl_1 * T_SIZE + r_tile_lvl_1.y, 0, gauss_height[1]-1);

        Expr x_cmp_lvl_1 = clamp(tx_lvl_1 * T_SIZE + r_tile_lvl_1.x + coarse_offset_lvl_1(tx_lvl_1, ty_lvl_1, 0, n_lvl_1) + x_s_lvl_1, 0, gauss_width[1]-1);
        Expr y_cmp_lvl_1 = clamp(ty_lvl_1 * T_SIZE + r_tile_lvl_1.y + coarse_offset_lvl_1(tx_lvl_1, ty_lvl_1, 1, n_lvl_1) + y_s_lvl_1, 0, gauss_height[1]-1);

        Expr dist_lvl_1 = abs(i16(gPyramid[1](x_ref_lvl_1, y_ref_lvl_1, 0)) - i16(gPyramid[1](x_cmp_lvl_1, y_cmp_lvl_1, n_lvl_1))); 

       /* Func: scores_lvl_1
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[1]
        */
        Func scores_lvl_1;
        scores_lvl_1(tx_lvl_1, ty_lvl_1, x_s_lvl_1, y_s_lvl_1, n_lvl_1) = sum(u32(dist_lvl_1));
        //scores_lvl_1.trace_stores();
        Tuple min_coor_lvl_1 = argmin(scores_lvl_1(tx_lvl_1, ty_lvl_1, r_search_lvl_1.x, r_search_lvl_1.y, n_lvl_1));

       /* Func: alignPyramid[1]
        * dtype: i16
        * True range: [-60, 60] (worst case)
        * Consumer(s): coarse_offset_lvl_1
        */
        alignPyramid[1](tx_lvl_1, ty_lvl_1, xy_lvl_1, n_lvl_1) = select(n_lvl_1 == 0, i16(0),
                    xy_lvl_1 == 0, i16(min_coor_lvl_1[0]) + coarse_offset_lvl_1(tx_lvl_1, ty_lvl_1, 0, n_lvl_1),
                    i16(min_coor_lvl_1[1]) + coarse_offset_lvl_1(tx_lvl_1, ty_lvl_1, 1, n_lvl_1)); 

        //alignPyramid[1].trace_stores();

        
         /* ALIGN PYRAMID LEVEL 0*/
        Var tx_lvl_0, ty_lvl_0, xy_lvl_0, n_lvl_0;
        Var x_s_lvl_0, y_s_lvl_0;
        
        RDom r_tile_lvl_0(0, T_SIZE, 0, T_SIZE);
        RDom r_search_lvl_0(-4, 9, -4, 9);
        
        //coarse_offset_lvl_0(tx_lvl_0, ty_lvl_0, xy_lvl_0, n_lvl_0) = 2 * i32(ceil(upsample_float_size_2_for_alignment(alignPyramid[1], upsample_flow_gauss_widths[0], upsample_flow_gauss_heights[0])(tx_lvl_0, ty_lvl_0, xy_lvl_0, n_lvl_0)));
              
       /* Func: coarse_offset_lvl_0
        * dtype: i16
        * True range: [-120, 120]
        * Consumer(s): alignPyramid[0]
        */
        Func coarse_offset_lvl_0;
        coarse_offset_lvl_0(tx_lvl_0, ty_lvl_0, xy_lvl_0, n_lvl_0) = i16(2 * upsample_u16_size_2_for_alignment(alignPyramid[1], upsample_flow_gauss_widths[0], upsample_flow_gauss_heights[0])(tx_lvl_0, ty_lvl_0, xy_lvl_0, n_lvl_0));
        //coarse_offset_lvl_0.trace_stores();

        Expr x_ref_lvl_0 = clamp(tx_lvl_0 * T_SIZE + r_tile_lvl_0.x, 0, gauss_width[0]-1);
        Expr y_ref_lvl_0 = clamp(ty_lvl_0 * T_SIZE + r_tile_lvl_0.y, 0, gauss_height[0]-1);

        Expr x_cmp_lvl_0 = clamp(tx_lvl_0 * T_SIZE + r_tile_lvl_0.x + coarse_offset_lvl_0(tx_lvl_0, ty_lvl_0, 0, n_lvl_0) + x_s_lvl_0, 0, gauss_width[0]-1);
        Expr y_cmp_lvl_0 = clamp(ty_lvl_0 * T_SIZE + r_tile_lvl_0.y + coarse_offset_lvl_0(tx_lvl_0, ty_lvl_0, 1, n_lvl_0) + y_s_lvl_0, 0, gauss_height[0]-1);

        Expr dist_lvl_0 = abs(i16(gPyramid[0](x_ref_lvl_0, y_ref_lvl_0, 0)) - i16(gPyramid[0](x_cmp_lvl_0, y_cmp_lvl_0, n_lvl_0))); 


       /* Func: scores_lvl_0
        * dtype: u32
        * True range: [0, 261888] (worst case)
        * Consumer(s): alignPyramid[0]
        */
        Func scores_lvl_0;
        scores_lvl_0(tx_lvl_0, ty_lvl_0, x_s_lvl_0, y_s_lvl_0, n_lvl_0) = sum(u32(dist_lvl_0));
        //scores_lvl_0.trace_stores();
        Tuple min_coor_lvl_0 = argmin(scores_lvl_0(tx_lvl_0, ty_lvl_0, r_search_lvl_0.x, r_search_lvl_0.y, n_lvl_0));


       /* Func: alignPyramid[0]
        * dtype: i16
        * True range: [-124, 124] (worst case)
        * Consumer(s): coarse_offset_lvl_0
        */
        alignPyramid[0](tx_lvl_0, ty_lvl_0, xy_lvl_0, n_lvl_0) = select(n_lvl_0 == 0, i16(0),
                    xy_lvl_0 == 0, i16(min_coor_lvl_0[0]) + coarse_offset_lvl_0(tx_lvl_0, ty_lvl_0, 0, n_lvl_0),
                    i16(min_coor_lvl_0[1]) + coarse_offset_lvl_0(tx_lvl_0, ty_lvl_0, 1, n_lvl_0)); 

        //alignPyramid[0].trace_stores();
        
       /* Func: align_output
        * dtype: i16
        * True range: [-124, 124] (worst case)
        * Consumer(s): dist calculation (in merge)
        */
        Func align_output;
        align_output(tx, ty, xy, n) = alignPyramid[0](tx, ty, xy, n);
        Expr max_tx = (input.width()/2)/(T_SIZE);
        Expr max_ty = (input.height()/2)/(T_SIZE);
  
     
        /* 
         * END ALIGN STEP
         */


        /* 
         * MERGE STEP
         */

        
        Var tx_image, ty_image; 

        // In the paper, they claim to use 16x16 tiles 
        RDom r_tile(0, T_SIZE, 0, T_SIZE);


        // Use 8x8 tiles for grayscsle image  
        RDom G_R_tile(0, G_T_SIZE, 0, G_T_SIZE);

        // This should be for the 4 color panes of the bayer raw: R, G, G, B
        RDom r_channel(0, 4);


        // This is for the number of images being merged 
        RDom r_imgs(0, input.dim(2).extent());


        // I THINK HOW TO DO OVERLAPING TILES IS DIVIDE T_SIZE BY 2 IN THIS EXPRESSION. 
        Expr offset_x = align_output(tx_image/4, ty_image/4, 0, n);
        Expr offset_y = align_output(tx_image/4, ty_image/4, 1, n);
        //Expr ref_x = tx*T_SIZE/2 + r_tile.x;
        //Expr ref_y = ty*T_SIZE/2 + r_tile.y;
        Expr ref_x = clamp((tx_image*T_SIZE/2)/2 + G_R_tile.x, 0, input.width()/2 - 1);
        Expr ref_y = clamp((ty_image*T_SIZE/2)/2 + G_R_tile.y, 0, input.height()/2 - 1);

        // Find the corresponding tile in the alternate frame, using the results of alignment 
        Expr alt_x = clamp((tx_image*T_SIZE/2)/2 + G_R_tile.x + offset_x, 0, input.width()/2 - 1);
        Expr alt_y = clamp((ty_image*T_SIZE/2)/2 + G_R_tile.y + offset_y, 0, input.height()/2 - 1);

        // Here "c" should represent the Bayer color plane, but I don't see where "c" is defined? 
        // ANS: It is defined as a variable in the private section

        // Pixel in reference frame and alternate frame respectively 
        // Using deinterleaved because merging is done separately for each color channel 
        //Expr ref_val = deinterleaved(ref_x, ref_y, c, 0);
        //Expr alt_val = deinterleaved(alt_x, alt_y, c, n);
        Expr ref_val = gray(ref_x, ref_y, 0);
        Expr alt_val = gray(alt_x, alt_y, n);
  
        // ???
        Expr norm_shift = normalization_shift;
        Func dist_tile, dist_tile_norm;

        // Summing over a given tile; dist_tile represents distance for that color channel
        // the iteration domain here is r_tile.x and r_tile.
        
       /* Func: dist_tile
        * dtype: u16
        * True range: [0, 65472] (worst case)
        * Consumer(s): dist_tile_norm
        * Notes: ref_val and alt_val each have range [0, 1023]. Use i16 b/c doing subtraction. 
        */
        //dist_tile(tx_image, ty_image, n) = sum(i32(abs(i16(ref_val) - i16(alt_val))));
        dist_tile(tx_image, ty_image, n) = sum(u16(abs(i16(ref_val) - i16(alt_val))));
        //dist_tile.trace_stores();


        // Dividing by 64.f b/c currently using 8x8 tiles (grayscale)
        //dist_tile_norm(tx_image, ty_image, n) = dist_tile(tx_image, ty_image, n)/64.f;

       /* Func: dist_tile_norm
        * dtype: u16
        * True range: [0, 1023]
        * Consumer(s): dist_tile_norm
        */
        dist_tile_norm(tx_image, ty_image, n) = dist_tile(tx_image, ty_image, n) >> 6;
        //dist_tile_norm.trace_stores();

       /* Func: weight
        * dtype: u16
        * True range: [0, 256] 
        * Consumer(s): sum_weight, output_tiled
        * Notes: Using [0-256] to represent the [0.f-1.f] decimal range of the weights
        * Notes: The multiplication and division shown is done in i32 b/c i16 cannot represent 256 * 1023 (1023 is max value of dist_tile_norm)
        * Notes: One INT2F and one F2INT conversion necessary for the fpdiv. 
        */
        Func weight;
        //weight(tx_image, ty_image, n) = select(n == 0, 1.0f, 1.0f - min(1.0f, max(0.0f, dist_tile_norm(tx_image, ty_image, n) - cast<float>(min_dist))/(cast<float>(dist_max_min_diff))));
        weight(tx_image, ty_image, n) = select(n == 0, u16(256), u16(256) - u16(min(i32(256), max(i32(0), i32(i16(dist_tile_norm(tx_image, ty_image, n)) - i16(min_dist)) * i32(256))    / (i32(dist_max_min_diff)   ))));
        //weight.trace_stores();


       /* Func: sum_weight
        * dtype: u16
        * True range: [0, 3069] 
        * Consumer(s): output_tiled_normalized_cosined
        */
        Func sum_weight;
        sum_weight(tx_image, ty_image) = sum(weight(tx_image, ty_image, r_imgs));
        //sum_weight.trace_stores();

        ref_x = (tx_image*(T_SIZE/2)) + xi;
        ref_y = (ty_image*(T_SIZE/2)) + yi;
        alt_x = (tx_image*(T_SIZE/2)) + xi + (2*offset_x);
        alt_y = (ty_image*(T_SIZE/2)) + yi+ (2*offset_y);
  
        ref_val = clamped_input(ref_x, ref_y, 0);
        alt_val = clamped_input(alt_x, alt_y, n);

        Expr x_index = select(n == 0, ref_x, alt_x);
        Expr y_index = select(n == 0, ref_y, alt_y);
    
        Expr x_index_in_bounds = ((x_index >= 0) && (x_index < input.width()));
        Expr y_index_in_bounds = ((y_index >= 0) && (y_index < input.height()));

       /* Func: val
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): output_tiled
        */    
        Func val;
        val(xi, yi, tx_image, ty_image, n) = select(x_index_in_bounds && y_index_in_bounds, clamped_input(x_index, y_index, n), u16(0));
        //val.trace_stores();


       /* Func: output_tiled
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): output_tiled_normalized_cosined
        * Notes: Shifting right by 8 to undo the [0.f-1.f] -> [0-256] transform done to weights earlier
        * Notes: Using U32 for the multiplication b/c 256 * 1023 = 261,888 is worst case, which cannot be represented by u16
        */ 
        Func output_tiled;
        output_tiled(xi, yi, tx_image, ty_image) = sum(u16((u32(weight(tx_image, ty_image, r_imgs)) * u32(val(xi, yi, tx_image, ty_image, r_imgs))) >> 8));




       /* Func: output_tiled
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): output_tiled_normalized_cosined
        * Notes: Shifting right by 8 to undo the [0.f-1.f] -> [0-256] transform done to weights earlier
        * Notes: Using U32 for the multiplication b/c 256 * 1023 = 261,888 is worst case, which cannot be represented by u16
        * Notes: One INT2F and one F2INT conversion necessary: multiplication by cos weight is fpmul. Cast back to u16 at the end.
        * Notes: Question: Can sum_weight * 256 be done in an integer dataype? 
        */ 
        Func output_tiled_normalized_cosined;
        // output_tiled_normalized_cosined(xi, yi, tx_image, ty_image) = output_tiled(xi, yi, tx_image, ty_image) * raised_cosine_weight(xi) * raised_cosine_weight(yi) * (1.0f/sum_weight(tx_image, ty_image) * (256.f));
        output_tiled_normalized_cosined(xi, yi, tx_image, ty_image) = u16(((output_tiled(xi, yi, tx_image, ty_image) * raised_cosine_weight(xi) * raised_cosine_weight(yi))/(sum_weight(tx_image, ty_image))) * 256.f);        
        //output_tiled_normalized_cosined.trace_stores();


       /* Func: final_merge_output
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): merge_output
        */ 
        Func final_merge_output;
        final_merge_output(x, y) = u16(0);
        //2 * num_tiles - 1, to account for overlapping tiles 
        // TODO: Fix these formulas 
        // Expr num_tx_locs =  2 * (((input.width())/2) >> LOG_2_T_SIZE) - 1;
        // Expr num_ty_locs =  2 * (((input.height())/2) >> LOG_2_T_SIZE) - 1;
        Expr num_tx_locs =  156;
        Expr num_ty_locs = 139;

        RDom tile_RDom(0, num_tx_locs, 0, num_ty_locs, 0, T_SIZE, 0, T_SIZE);
        Expr x_prime = (tile_RDom.x * (T_SIZE/2)) + tile_RDom.z;
        Expr y_prime = (tile_RDom.y * (T_SIZE/2)) + tile_RDom.w;
        //final_merge_output(x_prime, y_prime, c) += mul2(final_merge_output_tiled(tile_RDom.z, tile_RDom.w, tile_RDom.x, tile_RDom.y, c), mul2(cast<uint16_t>(raised_cosine_weight(tile_RDom.z) * 256.0f),  cast<uint16_t>(raised_cosine_weight(tile_RDom.w) * 256.0f)));
        final_merge_output(x_prime, y_prime) += output_tiled_normalized_cosined(tile_RDom.z, tile_RDom.w, tile_RDom.x, tile_RDom.y);


       /* Func: merge_output
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): hot_pixel_suppresion (camera pipeline)
        */ 
        Func merge_output;
        merge_output(x, y) = final_merge_output(x, y);
        //merge_output.trace_stores();
        //output(x, y, c) = u8(merge_output(x, y) * 255.f);
        /* 
         * END MERGE STEP
         */



        /* 
         * BEGIN CAMERA PIPELINE 
         */
         Func cp_hw_input, cp_hw_input_temp, cp_hw_input_shuffle, cp_hw_input_shift;
        //cp_hw_input_temp(x,y) = u16(input(x+(blockSize-1)/2, y+(blockSize-1)/2));
        //cp_hw_input_temp(x,y) = u16(input(x, y));
        //cp_hw_input_temp(x,y) = cast<float>(merge_output(x, y) * 1024.f);
        //cp_hw_input_temp(x,y) = cast<float>(merge_output(x, y) * 16383.f);

        //cp_hw_input_temp(x,y) = cast<float>(merge_output(x, y));
        cp_hw_input_temp(x,y) = merge_output(x, y);
        

        if (get_target().has_feature(Target::Clockwork)) {
            cp_hw_input_shuffle(x, y, c) = cp_hw_input_temp(2*x + c/2, 2*y + c%2);

            //cp_hw_input(x, y) = cp_hw_input_shuffle(x/4 + 622*(y%2), y/2, x%4);
            int iWidth = (tWidth * nTiles + blockSize-1) / 4;
            cp_hw_input_shift(x, y) = cp_hw_input_shuffle(x/4 + iWidth*(y%2), y/2, x%4);
            cp_hw_input(x, y) = cp_hw_input_shift(x+(blockSize-1)/2, y+(blockSize-1)/2);
        } else {
            cp_hw_input(x, y) = cp_hw_input_temp(x+(blockSize-1)/2, y+(blockSize-1)/2);
        }

        
        Func denoised;
        denoised = hot_pixel_suppression(cp_hw_input);

        // Give more convenient names to the four channels we know
        Func r_r, g_gr, g_gb, b_b;
        g_gr(x, y) = denoised(2*x, 2*y);//deinterleaved(x, y, 0);
        r_r(x, y)  = denoised(2*x+1, 2*y);//deinterleaved(x, y, 1);
        b_b(x, y)  = denoised(2*x, 2*y+1);//deinterleaved(x, y, 2);
        g_gb(x, y) = denoised(2*x+1, 2*y+1);//deinterleaved(x, y, 3);

        //denoised.trace_stores();
    
        Func demosaicked, my_demosaicked;
        Func b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b;
        demosaicked = demosaic(g_gr, r_r, b_b, g_gb,
                                b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b);

        my_demosaicked = my_demosaic(denoised);
        //demosaicked = demosaic(g_gr_wb, r_r_wb, b_b_wb, g_gb_wb,
        //                       b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b);

        Func color_corrected;
        color_corrected = color_correct(my_demosaicked, matrix);

        Func curve;
        {

            // BL, WL CHANGE
            Expr minRaw = blackLevel;
            Expr maxRaw = whiteLevel;
            Expr invRange = 1.0f / (maxRaw - minRaw);

            // BL, WL CHANGE
            //Expr xf = clamp(cast<float>(x)/1024.0f, 0.f, 1.f);

            Expr xf = clamp(cast<float>(x - minRaw) * invRange, 0.0f, 1.0f);
            Expr g = pow(xf, 1.0f/gamma);
            Expr b = 2.0f - (float) pow(2.0f, contrast/100.0f);
            Expr a = 2.0f - 2.0f*b;
            Expr val = select(g > 0.5f,
                            1.0f - (a*(1.0f-g)*(1.0f-g) + b*(1.0f-g)),
                            a*g*g + b*g);

            // BL, WL CHANGE
            //curve(x) = u16(clamp(val*256.0f, 0.0f, 255.0f));

            //curve(x) = select(x <= minRaw, 0, select(x > maxRaw, u16(255), u16(clamp(val*256.0f, 0.0f, 255.0f))));
            //curve(x) = select(x <= minRaw, 0.0f, select(x > maxRaw, 1.0f, clamp(val, 0.0f, 1.0f)));
            //curve(x) = select(x <= minRaw, 0.0f, select(x > maxRaw, 3072.f, clamp(val * 3072.f, 0.0f, 3072.f)));

            // curve(x) = select(x <= minRaw, 0.0f, select(x > maxRaw, 1023.f, clamp(val * 1023.f, 0.0f, 1023.f)));
            curve(x) = select(x <= minRaw, u16(0), select(x > maxRaw, u16(1023), u16(clamp(val * 1023.f, 0.0f, 1023.f))));

            //curve(x) = select(x <= minRaw, 0.0f, select(x > maxRaw, 255.f, clamp(val*256.0f, 0.0f, 255.0f)));
            //curve(x) = clamp(val*256.0f, 0.0f, 255.0f);
        }

        Func cp_hw_output, curve_out, output_shuffle, gamma_corr_out;
        curve_out = apply_curve(color_corrected, curve);
        //curve_out = apply_curve(my_demosaicked, curve);
        //gamma_corr_out = gamma_correction(curve_out, 1.1f);
        //curve_out = apply_curve(color_corrected, curve);
        cp_hw_output(c, x, y) = curve_out(x, y, c);
        //cp_hw_output(c, x, y) = demosaicked(x, y, c);
        //cp_hw_output(c, x, y) = denoised(x, y);


        Func cp_output;

        Var k;
        if (get_target().has_feature(Target::Clockwork)) {
            int iWidth = tWidth * nTiles / 4;
            output_shuffle(c, k, x, y) = u8(cp_hw_output(c, (x%iWidth)*4 + k, x/iWidth + 2*y));
            //output(x, y, c) = output_shuffle(c, y%2 + 2*(x%2), max(x/2 - 1, 0), y/2);
            cp_output(x, y, c) = output_shuffle(c, y%2 + 2*(x%2), x/2, y/2);
        } else {
            //output(x, y, c) = u8(cp_hw_output(c, x+2, y));


            //cp_output(x, y, c) = u8(cp_hw_output(c, x, y));
            //cp_output(x, y, c) = cp_hw_output(c, x, y)/1023.f;
            cp_output(x, y, c) = cp_hw_output(c, x, y);

        }

        //output(x, y, c) = u8((cp_output(x, y, c)/3072.f) * 255.f);
        //output(x, y, c) = u8((cp_output(x, y, c)/1023.f) * 255.f);
        //output(x, y, c) = u8(cp_output(x, y, c) * 255.f);


        //curve.bound(x, 0, 256);
      
        //cp_output.trace_stores();

    
       /* 
        * END CAMERA PIPELINE 
        */



     /* 
      * BEGIN EXPOSURE FUSION 
      */

     
     /* Func: ef_hw_input
      * dtype: u16
      * True range: [0, 1023] 
      * Consumer(s): ef_hw_input_dark, r_channel_scale_factor, g_channel_scale_factor, b_channel_scale_factor
      */ 
      Func ef_hw_input;
      ef_hw_input(x, y, c) = cp_output(x, y, c);

      // Create dark and bright versions of image 
      Func ef_hw_input_bright, ef_hw_input_dark;
      
      // input dark is grayscale version of input
      //ef_hw_input_dark(x, y) = (0.299f * ef_hw_input_float(x, y, 0)) + (0.587f * ef_hw_input_float(x, y, 1)) + (0.114f * ef_hw_input_float(x, y, 2));
      //ef_hw_input_dark(x, y) = u16((0.299f * ef_hw_input(x, y, 0)) + (0.587f * ef_hw_input(x, y, 1)) + (0.114f * ef_hw_input(x, y, 2)));


     /* Func: ef_hw_input_dark
      * dtype: u16
      * True range: [0, 1023] 
      * Notes: intermediate values are represeted in u32 b/c u16 cannot represent the scaling factor * 1023 (max value)
      * Consumer(s): ef_hw_input_dark_gamma_corr, ef_hw_input_bright, r_channel_scale_factor, g_channel_scale_factor, b_channel_scale_factor
      */ 
      ef_hw_input_dark(x, y) = u16((77 * u32(ef_hw_input(x, y, 0)) + 150 * u32(ef_hw_input(x, y, 1)) + 29 * u32(ef_hw_input(x, y, 2))) >> 8);
      //ef_hw_input_dark.trace_stores();


     /* Func: ef_hw_input_bright
      * dtype: u16
      * True range: [0, 2302] 
      * Consumer(s): ef_hw_input_bright_gamma_corr
      */ 
      // input bright is input dark, multiplied by a scale factor (2.25 f)
      //ef_hw_input_bright(x, y) = 2.25f * ef_hw_input_dark(x, y);
      //ef_hw_input_bright(x, y) = u16(2.25f * ef_hw_input_dark(x, y));
      ef_hw_input_bright(x, y) = (ef_hw_input_dark(x, y) << 1) + (ef_hw_input_dark(x, y) >> 2);


      // Gamma correct the dark and bright images
      float forward_gamma_exponent = 1.f/2.2f;

     /* Func: forward_gamma_corr
      * Notes: This should be synthesized into a lookup table
      * LUT input: [0-2302]
      * LUT output: corresponding gamma corrected pixel in same approximate range
      */ 
      Func forward_gamma_corr;
      {
        forward_gamma_corr(x) = u16(pow(cast<float>(x)/1023.f, forward_gamma_exponent) * 1023.f);

      }
      
     /* Func: ef_hw_input_bright_gamma_corr
      * dtype: u16
      * True range: [0, 2302] 
      * Consumer(s): bright_input_lpyramid
      */ 
      Func ef_hw_input_bright_gamma_corr;
      ef_hw_input_bright_gamma_corr = apply_forward_gamma_corr(ef_hw_input_bright, forward_gamma_corr);


     /* Func: ef_hw_input_dark_gamma_corr
      * dtype: u16
      * True range: [0, 1023] 
      * Consumer(s): dark_input_lpyramid
      */ 
      Func ef_hw_input_dark_gamma_corr;
      ef_hw_input_dark_gamma_corr = apply_forward_gamma_corr(ef_hw_input_dark, forward_gamma_corr);

      //ef_hw_input_dark_gamma_corr.trace_stores();
      //ef_hw_input_bright_gamma_corr.trace_stores();

      // Create exposure weight
      Func weight_dark, weight_bright, ef_weight_sum, weight_dark_norm, weight_bright_norm;


     /* Func: weight_dark/weight_bright
      * dtype: float
      * True range: [0.f-1.f]
      * Consumer(s): ef_weight_sum, weight_dark_norm
      * Notes: Subtraction done w/ signed datatype as per usual; subtract 512 which represents the "middle" intensity
      * Notes: divide by 1023.f to bring into [0.f-1.f] range before applying exp()
      * Notes: this whole thing should probably just be a pre-computed LUT; LUT input: [0-2032], LUT output: weight [0.1-1.f]
      * Notes: INT2F conversion required here 
      */ 
      weight_dark(x, y) = exp(-12.5f * (((i16(ef_hw_input_dark(x, y)) - i16(512))/1023.f) *  ((i16(ef_hw_input_dark(x, y)) - i16(512))/1023.f)));
      weight_bright(x, y) = exp(-12.5f * (((i16(ef_hw_input_bright(x, y)) - i16(512))/1023.f) * ((i16(ef_hw_input_bright(x, y)) - i16(512))/1023.f)));
      //weight_bright.trace_stores();



     /* Func: ef_weight_sum
      * dtype: float
      * True range: [0.f-2.f]
      * Consumer(s): weight_dark_norm, weight_bright_norm
      * Notes: As a design decision, chose to keep this in fp, as the F2INT may not be worth it, with a required INT2F conversion coming up in the next stage (weight_norms)
      */ 
      ef_weight_sum(x,y) = weight_dark(x,y) + weight_bright(x,y);
      
      
      //weight_dark_norm(x,y)   = weight_dark(x,y)   * 128.0f / ef_weight_sum(x,y);
      //weight_bright_norm(x,y) = weight_bright(x,y) * 128.0f / ef_weight_sum(x,y);


     /* Func: weight_dark_norm/weight_bright_norm
      * dtype: u16
      * True range: [0-1023]
      * Consumer(s): dark/bright_weight_gpyramid 
      * Notes: As a design decision, chose to keep this in fp, as the F2INT may not be worth it, with a required INT2F conversion coming up in the next stage (weight_norms)
      */ 
      weight_dark_norm(x,y)   = u16((weight_dark(x,y) / ef_weight_sum(x,y)) * 1023.f);
      weight_bright_norm(x,y) = u16((weight_bright(x,y) / ef_weight_sum(x,y)) * 1023.f);

      //weight_dark_norm.trace_stores();

   
      // Create gaussian pyramids of the weights
      vector<Func> dark_weight_gpyramid(ef_pyramid_levels);
      vector<Func> bright_weight_gpyramid(ef_pyramid_levels);


     /* Func: dark/bright weight gpyramid
      * dtype: u16
      * True range: [0-1023]
      * Consumer(s): merged_pyramid
      */ 
      dark_weight_gpyramid = gaussian_pyramid(weight_dark_norm, ef_pyramid_levels, "dweight");
      bright_weight_gpyramid = gaussian_pyramid(weight_bright_norm, ef_pyramid_levels, "bweight");

      //dark_weight_gpyramid[0].trace_stores();
      //bright_weight_gpyramid[0].trace_stores();

      // Create laplacian pyramids of the input images
      vector<Func> dark_input_lpyramid(ef_pyramid_levels);
      vector<Func> dark_input_gpyramid(ef_pyramid_levels);
      vector<Func> bright_input_lpyramid(ef_pyramid_levels);
      vector<Func> bright_input_gpyramid(ef_pyramid_levels);


     /* Func: dark/bright_input_lpyramid
      * dtype: i16
      * True range: [-1023 to +1023]
      * Consumer(s): merged_pyramid
      */ 
      dark_input_lpyramid =   laplacian_pyramid(ef_hw_input_dark_gamma_corr, ef_pyramid_levels,
                                                dark_input_gpyramid, "dinput");
      bright_input_lpyramid = laplacian_pyramid(ef_hw_input_bright_gamma_corr, ef_pyramid_levels,
                                                bright_input_gpyramid, "binput");

      //dark_input_lpyramid[0].trace_stores();    
      //bright_input_lpyramid[3].trace_stores();          

     /* Func: merged_pyramid
      * dtype: i16
      * True range: [-1023 to +1023]
      * Consumer(s): initial_blended_image (flatten_pyramid)
      */ 
      vector<Func> merged_pyramid(ef_pyramid_levels);
      merged_pyramid = merge_pyramids(dark_weight_gpyramid,
                                      bright_weight_gpyramid,
                                      dark_input_lpyramid,
                                      bright_input_lpyramid,
                                      "merge_pyr");


      //merged_pyramid[0].trace_stores();

      // Collapse the merged pyramid to create a single image
      Func blended_image, initial_blended_image, intermediate_blended_image;
      vector<Func> upsampled(ef_pyramid_levels);


     /* Func: initial_blended_image
      * dtype: i16
      * True range: [-2048 to +2048] (worst case? unsure)
      * Consumer(s): blended_image 
      */ 
      initial_blended_image = flatten_pyramid(merged_pyramid, upsampled);
      //initial_blended_image.trace_stores();

      // Undo the gamma correction
      float reverse_gamma_exponent = 2.2f;


     /* Func: reverse_gamma_corr
      * Notes: This should be synthesized into a lookup table
      * LUT input: data in [-2048 to +2048], worst case. For typical case, consider fact that initial_blended_image max and min are 1012.0 and -8.0 respectively for taxi
      * LUT output: data in the [0, 1023] range. 
      */ 
      Func reverse_gamma_corr;
      {
        reverse_gamma_corr(x) = u16(pow(cast<float>(x)/1023.f, reverse_gamma_exponent) * 1023.f);
      }


      /* Func: blended_image
       * dtype: u16
       * True range: [0, 1023]
       * Consumer(s): initial_r_out, initial_g_out, initial_b_out
       */
      blended_image = apply_reverse_gamma_corr(initial_blended_image, reverse_gamma_corr);
      blended_image.trace_stores();

      //blended_image(x, y) = pow(intermediate_blended_image(x, y), 1.f/2.2f);

      // YUV-TO-RGB conversion
      Func ef_hw_output, initial_ef_hw_output;
      Func initial_r_out, initial_g_out, initial_b_out;
  

      /* Func: r_channel_scale_factor 
       * dtype: u16
       * True range: [0, 1023] 
       * Consumer(s): initial_r_out
       * Notes: The [0-1023] range is used to represent the [0.f-1.f] decimal range of r_channel_scale_factor
       * Notes: INT2F and F2INT conversion required here
       */
      Func r_channel_scale_factor;
      r_channel_scale_factor(x, y) = u16(select(ef_hw_input_dark(x, y) == 0, 0.0f, cast<float>(ef_hw_input(x, y, 0))/cast<float>(ef_hw_input_dark(x, y))) * 1023.f);


      /* Func: initial_r_out 
       * dtype: u16
       * True range: [0, 1023] (typical case; worse case unknown...)
       * Consumer(s): initial_ef_hw_output
       */
      initial_r_out(x, y) = mul_1023_unsigned(blended_image(x, y), r_channel_scale_factor(x, y));


      /* Func: g_channel_scale_factor 
       * dtype: u16
       * True range: [0, 1023] 
       * Consumer(s): initial_g_out
       * Notes: The [0-1023] range is used to represent the [0.f-1.f] decimal range of g_channel_scale_factor
       * Notes: INT2F and F2INT conversion required here
       */
      Func g_channel_scale_factor;
      g_channel_scale_factor(x, y) = u16(select(ef_hw_input_dark(x, y) == 0, 0.0f, cast<float>(ef_hw_input(x, y, 1))/cast<float>(ef_hw_input_dark(x, y))) * 1023.f);

      
      /* Func: initial_g_out 
       * dtype: u16
       * True range: [0, 1023] (typical case; worse case unknown...)
       * Consumer(s): initial_ef_hw_output
       */
      initial_g_out(x, y) = mul_1023_unsigned(blended_image(x, y), g_channel_scale_factor(x, y));
  
      
      /* Func: b_channel_scale_factor 
       * dtype: u16
       * True range: [0, 1023] 
       * Consumer(s): initial_b_out
       * Notes: The [0-1023] range is used to represent the [0.f-1.f] decimal range of b_channel_scale_factor
       * Notes: INT2F and F2INT conversion required here
       */
      Func b_channel_scale_factor;
      b_channel_scale_factor(x, y) = u16(select(ef_hw_input_dark(x, y) == 0, 0.0f, cast<float>(ef_hw_input(x, y, 2))/cast<float>(ef_hw_input_dark(x, y))) * 1023.f);


      /* Func: initial_b_out 
       * dtype: u16
       * True range: [0, 1023] (typical case; worst case unknown...)
       * Consumer(s): initial_ef_hw_output
       */
      initial_b_out(x, y) = mul_1023_unsigned(blended_image(x, y), b_channel_scale_factor(x, y));


      /* Func: initial_ef_hw_output
       * dtype: u16
       * True range: [0, 1023] (typical case; worst case unknown...)
       * Consumer(s): ef_hw_output
       */
      initial_ef_hw_output(x,y,c) = select(c == 0, initial_r_out(x, y),
                                 c == 1, initial_g_out(x, y),
                                         initial_b_out(x, y));


     /* Func: final_forward_gamma_corr
      * Notes: This should be synthesized into a lookup table
      * LUT input: [0-1023]
      * LUT output: corresponding gamma corrected pixel in [0-255] range 
      */ 
      forward_gamma_exponent = 1.f/2.2f;
      Func final_forward_gamma_corr;
      {
        final_forward_gamma_corr(x) = u8(pow(cast<float>(x)/1023.f, forward_gamma_exponent) * 255.f);
      }


      /* Func: ef_hw_output
       * dtype: u8
       * True range: [0, 255]
       * Consumer(s): output
       */
      ef_hw_output = apply_final_forward_gamma_corr(initial_ef_hw_output, final_forward_gamma_corr);


      /* Func: output
       * dtype: u8
       * True range: [0, 255]
       */
      output(x,y,c) = ef_hw_output(x,y,c);
      // output.bound(c, 0, 3);

       /* 
        * END EXPOSURE FUSION 
        */


        // Schedule
        //Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");

        // ALIGN SCHEDULE 
        Var xo("xo"), yo("yo"), outer("outer");
        align_output.reorder(xy, n, tx, ty).tile(tx, ty, xo, yo, xi, yi, 4, 4).fuse(xo, yo, outer).parallel(outer);
        //align_output.reorder(xy, n, tx, ty).parallel(outer);
        align_output.compute_root();
        //deinterleaved.compute_root();
        //clamped_input_float.compute_root();
        //deinterleaved.compute_root();
        gray.compute_root().parallel(y, 32).vectorize(x, 8);
        
        for (int j = 1; j < J; j++) {
            gPyramid[j]
                .compute_root().parallel(y, 8).vectorize(x, 8);


            // USE THIS FOR PRINTING COARSE OFFSET
            // if (j <= 0)
            //    alignPyramid[j]
            //   .store_at(align_output, outer).compute_at(align_output, yi);
            // else
            //   //alignPyramid[j]
            //   //.store_at(align_output, outer).compute_at(align_output, yi);
            //   alignPyramid[j].compute_at(coarse_offset_lvl_0, n_lvl_0);
            

            // USE THIS FOR PRINTING SCORES
            alignPyramid[j].compute_root();
        }
        //coarse_offset_lvl_4.compute_root();
        //coarse_offset_lvl_3.compute_root();
        //coarse_offset_lvl_2.compute_root();
        //coarse_offset_lvl_1.compute_root();
        //coarse_offset_lvl_0.compute_root();

        scores_lvl_0.compute_root();

        // alignPyramid[0-].compute_root()
        alignPyramid[0].compute_at(align_output, yi);


        //MERGE SCHEDULE 
        //output_shuffle.reorder(c, x, y).tile(x, y, xo, yo, xi, yi, 64, 64).fuse(xo, yo, outer).parallel(outer);
        merge_output.reorder(x, y).tile(x, y, xo, yo, xi, yi, 64, 64).fuse(xo, yo, outer).parallel(outer);
        merge_output.compute_root();
        //dist_tile.reorder(c, tx, ty, n).compute_root();
        dist_tile.reorder(tx_image, ty_image, n).compute_root();
        dist_tile_norm.compute_root();
        weight.compute_root().parallel(ty_image, 4).vectorize(tx_image, 4);
        sum_weight.store_at(merge_output, outer).compute_at(merge_output, yi).parallel(ty_image, 4).vectorize(tx_image, 4);
        val.store_at(merge_output, outer).compute_at(merge_output, yi);


        output_tiled.store_at(merge_output, outer).compute_at(merge_output, yi);
        output_tiled_normalized_cosined.store_at(merge_output, outer).compute_at(merge_output, yi);
        final_merge_output.store_at(merge_output, outer).compute_at(merge_output, yi);
        //my_debug_tile.store_at(merge_output, outer).compute_at(merge_output, yi);


        // CAMERA PIPELINE SCHEDULE
        if (get_target().has_feature(Target::CoreIR)) {
            cp_hw_input.store_at(cp_hw_output, xo).compute_at(denoised, x);
            cp_hw_output.compute_root();

            cp_hw_output.accelerate({cp_hw_input}, xi, xo, {});
            cp_hw_output.tile(x, y, xo, yo, xi, yi, 64-6,64-6)
            .reorder(c,xi,yi,xo,yo);

            curve_out.compute_at(cp_hw_output, xo);
            //cp_hw_output.unroll(c).unroll(xi, 2);
            cp_hw_output.unroll(c);
            
            demosaicked.linebuffer();
            demosaicked.unroll(c);
            //demosaicked.reorder(c, x, y);

            denoised.linebuffer();
            //.unroll(x).unroll(y);

            curve.compute_at(cp_hw_output, xo).unroll(x);  // synthesize curve to a ROM
            
            cp_hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {

            if (schedule == 1) { // host and glb tiling
                const int numHostTiles = 4;
                const int numTiles = 3;
                const int tileSize = 58;
                const int glbSize = tileSize * numTiles;
                const int outputSize = numHostTiles * glbSize;

                cp_output.bound(x, 0, outputSize);
                cp_output.bound(y, 0, outputSize);

                cp_hw_output.in().compute_root();

                cp_hw_output.in()
                .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
                .reorder(c, xi, yi, xo, yo)
                .hw_accelerate(xi, xo);
                cp_hw_output.in().unroll(c);

                Var xii, yii, xio, yio;
                cp_hw_output
                .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
                .reorder(c, xi, yi, xo, yo);
                cp_hw_output.compute_at(cp_hw_output.in(), xo);
                cp_hw_output.store_in(MemoryType::GLB);
                cp_hw_output.unroll(c);

                curve_out.compute_at(cp_hw_output, xo);
                curve_out.unroll(c);
            
                color_corrected.compute_at(cp_hw_output, xo);
                color_corrected.unroll(c);

                demosaicked.compute_at(cp_hw_output, xo);
                demosaicked
                .reorder(c, x, y)
                .unroll(c);

                denoised.compute_at(cp_hw_output, xo);
                //.unroll(x).unroll(y);

                g_gr.compute_at(cp_hw_output, xo);
                r_r.compute_at(cp_hw_output, xo);
                b_b.compute_at(cp_hw_output, xo);
                g_gb.compute_at(cp_hw_output, xo);
            
                curve.compute_at(cp_hw_output, xo).unroll(x);  // synthesize curve to a ROM
                
                cp_hw_input.in().compute_at(cp_hw_output.in(), xo); // represents the glb level
                cp_hw_input.in().store_in(MemoryType::GLB);
                //cp_hw_input.in().unroll(c);  // hw input bound
                
                cp_hw_input.compute_root()
                .accelerator_input();

            } else if (schedule == 2) { // big parrot
                const int tileWidth = 64;
                const int tileHeight = 56;
                const int numHostTiles = 11;
                const int numTiles = 3;
                const int glbWidth = tileWidth * numTiles;
                const int glbHeight = tileHeight * numTiles;
                const int outputWidth = numHostTiles * glbWidth;
                const int outputHeight = numHostTiles * glbHeight;

                cp_output.bound(x, 0, outputWidth);
                cp_output.bound(y, 0, outputHeight);

                cp_hw_output.in().compute_root();

                cp_hw_output.in()
                .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
                .reorder(c, xi, yi, xo, yo)
                .hw_accelerate(xi, xo);
                cp_hw_output.in().unroll(c);

                Var xii, yii, xio, yio;
                cp_hw_output
                .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
                .reorder(c, xi, yi, xo, yo);
                cp_hw_output.compute_at(cp_hw_output.in(), xo);
                cp_hw_output.store_in(MemoryType::GLB);
                cp_hw_output.unroll(c);

                curve_out.compute_at(cp_hw_output, xo);
                curve_out.unroll(c);
            
                color_corrected.compute_at(cp_hw_output, xo);
                color_corrected.unroll(c);
            
                demosaicked.compute_at(cp_hw_output, xo);
                demosaicked
                .reorder(c, x, y)
                .unroll(c);

                denoised.compute_at(cp_hw_output, xo);
                //.unroll(x).unroll(y);

                g_gr.compute_at(cp_hw_output, xo);
                r_r.compute_at(cp_hw_output, xo);
                b_b.compute_at(cp_hw_output, xo);
                g_gb.compute_at(cp_hw_output, xo);
            
                curve.compute_at(cp_hw_output, xo).unroll(x);  // synthesize curve to a ROM
                
                cp_hw_input.in().compute_at(cp_hw_output.in(), xo); // represents the glb level
                cp_hw_input.in().store_in(MemoryType::GLB);
                //cp_hw_input.in().unroll(c);  // hw input bound
                
                cp_hw_input.compute_root()
                .accelerator_input();

            } else if (schedule == 3) { // big parrot with unroll
                const int unrollx = 2;
                const int unrolly = 2;
                //const int tileWidth = 64-8;
                const int tileWidth = tWidth;//256-8;
                //const int tileHeight = 64-8;
                const int tileHeight = tHeight;//192-8;
                const int numHostTiles = nTiles;//10;
                const int numTiles = 1; // number of tiles in the glb
                const int glbWidth = tileWidth * numTiles;
                const int glbHeight = tileHeight * numTiles;
                const int outputWidth = numHostTiles * glbWidth;
                const int outputHeight = numHostTiles * glbHeight;

                cp_output.bound(x, 0, outputWidth);
                cp_output.bound(y, 0, outputHeight);

                cp_hw_output.in().compute_root();

                Var xii, yii, xio, yio;
                cp_hw_output.in()
                .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
                .split(yi, yio, yii, unrolly)
                .reorder(c, yii, xi, yio, xo, yo)
                .hw_accelerate(xi, xo);
                cp_hw_output.in().unroll(c)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(xi, unrollx, TailStrategy::RoundUp);

                cp_hw_output
                .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
                .split(yi, yio, yii, unrolly)
                .reorder(c, yii, xi, yio, xo, yo);
                cp_hw_output.compute_at(cp_hw_output.in(), xo);
                cp_hw_output.store_in(MemoryType::GLB);
                cp_hw_output.unroll(c)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(xi, unrollx, TailStrategy::RoundUp);

                curve_out.compute_at(cp_hw_output, xo);
                curve_out.unroll(c)
                .split(y, yio, yii, unrolly).reorder(c, yii, x, yio)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(x, unrollx, TailStrategy::RoundUp);
            
                color_corrected.compute_at(cp_hw_output, xo);
                color_corrected.unroll(c)
                .split(y, yio, yii, unrolly).reorder(c, yii, x, yio)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(x, unrollx, TailStrategy::RoundUp);
            
                demosaicked.compute_at(cp_hw_output, xo);
                demosaicked
                .split(y, yio, yii, unrolly).reorder(c, yii, x, yio)
                .unroll(c)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(x, unrollx, TailStrategy::RoundUp);

                denoised.compute_at(cp_hw_output, xo)
                .split(y, yio, yii, unrolly).reorder(yii, x, yio)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(x, unrollx);
                //.unroll(x).unroll(y);

                bool buffer_memories = true;
                if (buffer_memories) {
                b_r.compute_at(cp_hw_output, xo);
                g_r.compute_at(cp_hw_output, xo);
                b_gr.compute_at(cp_hw_output, xo);
                r_gr.compute_at(cp_hw_output, xo);
                b_gb.compute_at(cp_hw_output, xo);
                r_gb.compute_at(cp_hw_output, xo);
                r_b.compute_at(cp_hw_output, xo);
                g_b.compute_at(cp_hw_output, xo);
                }

                g_gr.compute_at(cp_hw_output, xo);
                r_r.compute_at(cp_hw_output, xo);
                b_b.compute_at(cp_hw_output, xo);
                g_gb.compute_at(cp_hw_output, xo);
                
                if (false) { // these buffers should not be unrolled
                g_gr
                    .split(y, yio, yii, unrolly, TailStrategy::RoundUp).reorder(yii, x, yio)
                    .unroll(x, unrollx, TailStrategy::RoundUp)
                    .unroll(yii, unrolly, TailStrategy::RoundUp);
                r_r
                    .split(y, yio, yii, unrolly, TailStrategy::RoundUp).reorder(yii, x, yio)
                    .unroll(x, unrollx, TailStrategy::RoundUp)
                    .unroll(yii, unrolly, TailStrategy::RoundUp);
                b_b
                    .split(y, yio, yii, unrolly, TailStrategy::RoundUp).reorder(yii, x, yio)
                    .unroll(x, unrollx, TailStrategy::RoundUp)
                    .unroll(yii, unrolly, TailStrategy::RoundUp);
                g_gb
                    .split(y, yio, yii, unrolly, TailStrategy::RoundUp).reorder(yii, x, yio)
                    .unroll(x, unrollx, TailStrategy::RoundUp)
                    .unroll(yii, unrolly, TailStrategy::RoundUp);
                }
            
                curve.compute_at(cp_hw_output, xo).unroll(x);  // synthesize curve to a ROM
                curve.store_in(MemoryType::ROM);
                // unroll by x?

                cp_hw_input.in().in().compute_at(cp_hw_output, xo); // represents the mem tile
                cp_hw_input.in().in()
                .split(y, yio, yii, unrolly).reorder(yii, x, yio)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(x, unrollx, TailStrategy::RoundUp);
                
                cp_hw_input.in().compute_at(cp_hw_output.in(), xo); // represents the glb level
                cp_hw_input.in().store_in(MemoryType::GLB);
                cp_hw_input.in()
                .split(y, yio, yii, unrolly).reorder(yii, x, yio)
                .unroll(yii, unrolly, TailStrategy::RoundUp)
                .unroll(x, unrollx, TailStrategy::RoundUp);
                
                cp_hw_input.compute_root()
                .accelerator_input();
                
            } else {
            cp_output.bound(x, 0, 64-blockSize+1);
            cp_output.bound(y, 0, 64-blockSize+1);
                
            cp_hw_output.compute_root();

            cp_hw_output.tile(x, y, xo, yo, xi, yi, 64-blockSize+1,64-blockSize+1)
                .reorder(c,xi,yi,xo,yo)
                .reorder_storage(c, x, y)
                .hw_accelerate(xi, xo);
            cp_hw_output.unroll(c);
            //cp_hw_output.unroll(c).unroll(xi, 2);
            
            //curve_out.reorder(c, x, y).reorder_storage(c, x, y);
            curve_out.compute_at(cp_hw_output, xo);
            curve_out.unroll(c);
            
            color_corrected.compute_at(cp_hw_output, xo);
            color_corrected.unroll(c);
            
            demosaicked.compute_at(cp_hw_output, xo);
            demosaicked.unroll(c);
            //demosaicked.reorder(c, x, y);

            denoised.compute_at(cp_hw_output, xo);
            //.unroll(x).unroll(y);

            g_gr.compute_at(cp_hw_output, xo);
            r_r.compute_at(cp_hw_output, xo);
            b_b.compute_at(cp_hw_output, xo);
            g_gb.compute_at(cp_hw_output, xo);

            //b_r.compute_at(cp_hw_output, xo);
            //g_r.compute_at(cp_hw_output, xo);
            //b_gr.compute_at(cp_hw_output, xo);
            //r_gr.compute_at(cp_hw_output, xo);
            //b_gb.compute_at(cp_hw_output, xo);
            //r_gb.compute_at(cp_hw_output, xo);
            //r_b.compute_at(cp_hw_output, xo);
            //g_b.compute_at(cp_hw_output, xo);
            
            curve.compute_at(cp_hw_output, xo).unroll(x);  // synthesize curve to a ROM
            
            //cp_hw_input_copy.compute_at(cp_hw_output, xo);
            cp_hw_input.stream_to_accelerator();
            //cp_hw_input.compute_root();
            }
            
        } else {    // schedule to CPU
            if (schedule == 1 || schedule == 2 || schedule == 3) {
            Var yii;
            const int strip_size = 2;
            const int vec = 4;

            cp_output
                .compute_root()
                .reorder(c, x, y)
                .split(y, yi, yii, 2, TailStrategy::RoundUp)
                .split(yi, yo, yi, strip_size / 2)
                .vectorize(x, 2 * vec, TailStrategy::RoundUp)
                //.unroll(c)
                .parallel(yo);

            denoised
                .compute_at(cp_output, yi)
                //.compute_at(curve_out, yi)
                .store_at(cp_output, yo)
                .prefetch(input, y, 2)
                //.fold_storage(y, 4)
                .tile(x, y, x, y, xi, yi, 2 * vec, 2)
                .vectorize(xi)
                .unroll(yi);

            //max_g_gr.compute_at(output, yi);
            //max_r_r.compute_at(output, yi);
            //max_b_b.compute_at(output, yi);
            //max_g_gb.compute_at(output, yi);
            
            demosaicked
                .compute_at(cp_output, yi)
                .store_at(cp_output, yo)
                .fold_storage(y, 4)
                .reorder(c, x, y)
                .vectorize(x, 2 * vec, TailStrategy::RoundUp)
                .unroll(c);

            curve_out
                .compute_at(cp_output, yi)
                //.compute_at(output, yo)
                .store_at(cp_output, yo)
                .reorder(c, x, y)
                .tile(x, y, x, y, xi, yi, 2 * vec, 2, TailStrategy::RoundUp)
                .vectorize(xi)
                .unroll(yi);
                //.unroll(c);

            color_corrected
                .compute_at(curve_out, x)
                .reorder(c, x, y)
                .vectorize(x)
                .unroll(c);

            //demosaicked->intermed_compute_at.set({processed, yi});
            //demosaicked->intermed_store_at.set({processed, yo});
            //demosaicked->output_compute_at.set({curved, x});

            // We can generate slightly better code if we know the splits divide the extent.
            //processed
            //.bound(c, 0, 3);
                //.bound(x, 0, ((out_width) / (2 * vec)) * (2 * vec))
                //.bound(y, 0, (out_height / strip_size) * strip_size);
            }
        }

      // EXPOSURE FUSION SCHEDULE
      if (get_target().has_feature(Target::CoreIR)) {

      } else if (get_target().has_feature(Target::Clockwork)) {

        // ef_hw_output.compute_root();

        // ef_hw_output.tile(x, y, xo, yo, xi, yi, 64-ksize+1,64-ksize+1)
        //   .reorder(xi,yi,c,xo,yo)
        //   .hw_accelerate(xi, xo);
        //ef_hw_output.unroll(c);

        //blended_image.compute_at(ef_hw_output, xo);

        // for (size_t i=0; i<merged_pyramid.size(); ++i) {
        //   merged_pyramid[i].compute_at(ef_hw_output, xo);
          
        //   dark_input_lpyramid[i].compute_at(ef_hw_output, xo);
        //   bright_input_lpyramid[i].compute_at(ef_hw_output, xo);
        //   dark_input_gpyramid[i].compute_at(ef_hw_output, xo);
        //   bright_input_gpyramid[i].compute_at(ef_hw_output, xo);
          
        //   dark_weight_gpyramid[i].compute_at(ef_hw_output, xo);
        //   bright_weight_gpyramid[i].compute_at(ef_hw_output, xo);
        // }

        // ef_weight_sum.compute_at(ef_hw_output, xo);
        
        ef_hw_input_bright.stream_to_accelerator();
        ef_hw_input_dark.stream_to_accelerator();
        
      } else {    // schedule to CPU
        // COPYING CLOCKWORK SCHEDULE
        output.compute_root();

        output.tile(x, y, xo, yo, xi, yi, 64-ksize+1,64-ksize+1)
          .reorder(xi,yi,xo,yo);


        initial_blended_image.compute_at(output, xo);
        //intermediate_blended_image.compute_at(output, xo);
        blended_image.compute_at(output, xo);

        for (size_t i=0; i<merged_pyramid.size(); ++i) {
          merged_pyramid[i].compute_at(output, xo);
          
          dark_input_lpyramid[i].compute_at(output, xo);
          bright_input_lpyramid[i].compute_at(output, xo);
          dark_input_gpyramid[i].compute_at(output, xo);
          bright_input_gpyramid[i].compute_at(output, xo);
          
          dark_weight_gpyramid[i].compute_at(output, xo);
          bright_weight_gpyramid[i].compute_at(output, xo);
        }


        r_channel_scale_factor.compute_at(output, xo);
        b_channel_scale_factor.compute_at(output, xo);
        g_channel_scale_factor.compute_at(output, xo);

        ef_hw_input_dark.compute_at(output, xo);
        ef_hw_input_bright.compute_at(output, xo);
        weight_dark.compute_at(output, xo);
        weight_bright.compute_at(output, xo);
        ef_hw_input_dark_gamma_corr.compute_at(output, xo);
        ef_hw_input_bright_gamma_corr.compute_at(output, xo);
        weight_dark_norm.compute_at(output, xo);
        weight_bright_norm.compute_at(output, xo);
        ef_weight_sum.compute_at(output, xo);
        //ef_hw_output_gamma.compute_at(output, xo);
        ef_hw_output.compute_at(output, xo);
      }
    }
private:
    //Var x, y, tx, ty, xy, xi, yi, c, n;

    Func downsample(Func f_in, Expr size) {
        Var x, y;
        using Halide::_;
        Func f, downx, downy;
        f(x, y, _) = u16(f_in(x, y, _));
        downx(x, y, _) = 1 * f(size*x-2, y, _) + 4 * f(size*x-1, y, _) + 6 * f(size*x, y, _) + 4 * f(size*x+1, y, _) + 1 * f(size*x+2, y, _);
        // Question: why is this u8? Should I just use a different (trusted) downsample function? Also, why is it shifted right by 8???
        downy(x, y, _) = u16((1 * downx(x, size*y-2, _) + 4 * downx(x, size*y-1, _) + 6 * downx(x, size*y, _) + 4 * downx(x, size*y+1, _) + 1 * downx(x, size*y+2, _)) >> 8);
        return downy;
    }


  

    Func upsample_float_size_2(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var x, y;
      using Halide::_;
      Func up;


      up(x, y, _) = (0.75f * 0.75f) * f_in(x/2, y/2, _) 
      + (0.75f * 0.25f) * f_in(x/2, clamp((y/2) - 1 + 2*(y % 2), 0, gauss_height-1), _)
      + (0.25f * 0.75f) * f_in(clamp((x/2) - 1 + 2*(x % 2), 0, gauss_width-1), y/2, _) 
      + (0.25f * 0.25f) * f_in(clamp((x/2) - 1 + 2*(x % 2), 0, gauss_width-1), clamp((y/2) - 1 + 2*(y % 2), 0, gauss_height-1), _);

      return up;
    }


    Func upsample_float_size_2_for_alignment(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var tx, ty, xy, n;
      using Halide::_;
      Func up, up_float, f_in_shift;


      f_in_shift(tx, ty, xy, n) = f_in(tx, ty, xy, n);

      up_float(tx, ty, xy, n) = (0.75f * 0.75f) * f_in(tx/2, ty/2, xy, n) 
      + (0.75f * 0.25f) * f_in(tx/2, clamp((ty/2) - 1 + 2*(ty % 2), 0, gauss_height-1), xy, n)
      + (0.25f * 0.75f) * f_in(clamp((tx/2) - 1 + 2*(tx % 2), 0, gauss_width-1), ty/2, xy, n) 
      + (0.25f * 0.25f) * f_in(clamp((tx/2) - 1 + 2*(tx % 2), 0, gauss_width-1), clamp((ty/2) - 1 + 2*(ty % 2), 0, gauss_height-1), xy, n);

      up(tx, ty, xy, n) = up_float(tx, ty, xy, n);

      return up;
    }




    Func upsample_u16_size_2_for_alignment(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var tx, ty, xy, n;
      using Halide::_;
      Func up, up_pre_shift, f_in_shift;


     /* Func: up_pre_shift
      * dtype: u16
      * True range: [0, 16368] (worst case)
      * Consumer(s): up
      */
      up_pre_shift(tx, ty, xy, n) = (9) * f_in(tx/2, ty/2, xy, n) 
      + (3) * f_in(tx/2, clamp((ty/2) - 1 + 2*(ty % 2), 0, gauss_height-1), xy, n)
      + (3) * f_in(clamp((tx/2) - 1 + 2*(tx % 2), 0, gauss_width-1), ty/2, xy, n) 
      + (1) * f_in(clamp((tx/2) - 1 + 2*(tx % 2), 0, gauss_width-1), clamp((ty/2) - 1 + 2*(ty % 2), 0, gauss_height-1), xy, n);

      
       /* Func: up
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): returned by upsample_u16_size_2_for_alignment
        */
      up(tx, ty, xy, n) = up_pre_shift(tx, ty, xy, n) >> 4;


      return up;
    }
    

     Func downsample_float_hdr(Func f_in, Expr size, Expr gauss_width, Expr gauss_height) {
        Var x, y, n;
        using Halide::_;
        Func f, down;
    
        f(x, y, n) = f_in(x, y, n);
        Expr x_index_0 = clamp(size*x-1, 0, gauss_width-1);
        Expr x_index_1 = clamp(size*x, 0, gauss_width-1);
        Expr x_index_2 = clamp(size*x+1, 0, gauss_width-1);
        Expr x_index_3 = clamp(size*x+2, 0, gauss_width-1);


        Expr y_index_0 = clamp(size*y-1, 0, gauss_height-1);
        Expr y_index_1 = clamp(size*y, 0, gauss_height-1);
        Expr y_index_2 = clamp(size*y+1, 0, gauss_height-1);
        Expr y_index_3 = clamp(size*y+2, 0, gauss_height-1);

        // down(x, y, n) = (1.f/64.f) * f(x_index_0, y_index_0, n) + (3.f/64.f) * f(x_index_0, y_index_1, n) + (3.f/64.f) * f(x_index_0, y_index_2, n) + (1.f/64.f) * f(x_index_0, y_index_3, n) 
        //                 + (3.f/64.f) * f(x_index_1, y_index_0, n) + (9.f/64.f) * f(x_index_1, y_index_1, n) + (9.f/64.f) * f(x_index_1, y_index_2, n) + (3.f/64.f) * f(x_index_1, y_index_3, n) 
        //                 + (3.f/64.f) * f(x_index_2, y_index_0, n) + (9.f/64.f) * f(x_index_2, y_index_1, n) + (9.f/64.f) * f(x_index_2, y_index_2, n) + (3.f/64.f) * f(x_index_2, y_index_3, n) 
        //                 + (1.f/64.f) * f(x_index_3, y_index_0, n) + (3.f/64.f) * f(x_index_3, y_index_1, n) + (3.f/64.f) * f(x_index_3, y_index_2, n) + (1.f/64.f) * f(x_index_3, y_index_3, n);

        down(x, y, n) = (1.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (3.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (3.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (1.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n);
        return down;
    }


     Func downsample_u16_hdr(Func f_in, Expr size, Expr gauss_width, Expr gauss_height) {
        Var x, y, n;
        using Halide::_;
        Func f, down_pre_shift, down;
    
        f(x, y, n) = f_in(x, y, n);
        Expr x_index_0 = clamp(size*x-1, 0, gauss_width-1);
        Expr x_index_1 = clamp(size*x, 0, gauss_width-1);
        Expr x_index_2 = clamp(size*x+1, 0, gauss_width-1);
        Expr x_index_3 = clamp(size*x+2, 0, gauss_width-1);


        Expr y_index_0 = clamp(size*y-1, 0, gauss_height-1);
        Expr y_index_1 = clamp(size*y, 0, gauss_height-1);
        Expr y_index_2 = clamp(size*y+1, 0, gauss_height-1);
        Expr y_index_3 = clamp(size*y+2, 0, gauss_height-1);


       /* Func: down_pre_shift
        * dtype: u16
        * True range: [0, 65472] (worst case)
        * Consumer(s): down
        */
        down_pre_shift(x, y, n) = (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n) 
                        + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1), n) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1), n) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1), n) + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1), n);
        

       /* Func: down
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): returned by downsample_u16_hdr
        */
        down(x, y, n) = down_pre_shift(x, y, n) >> 6;

        return down;
    }

    Expr raised_cosine_weight(Expr in){
        const float PI = 3.141592f;
        return 0.5f - (0.5f * cos(2.0f * PI * (in + 0.5f) / T_SIZE));
    }

    /*
     * @param gauss: input gaussian pyramid
     * @param prev_alignment : alignment from upper level of pyramid 
     * @param tile_s: tile size to align
     * @param search_s: the search radius
     * @param upsample: TODO
     * @param align_min: The minimum alignment value to clamp to (usually comes from upper level)
     * @param align_max: The maximum alignment value to clamp to (usually comes from upper level)
     * @ return: TODO
     */
    Func align_layer(Func gauss, Func prev_alignment , Expr tile_s, Expr search_s, Expr upsample, Expr gauss_width, Expr gauss_height, Expr align_min, Expr align_max, Expr upsample_flow_gauss_width, Expr upsample_flow_gauss_height) {
     //Func align_layer(Func gauss, Func prev_alignment , Expr tile_s, Expr search_s, Expr upsample, Expr gauss_width, Expr gauss_height, Expr align_min, Expr align_max) {   
        Var tx, ty, xy, n;
        /* TODO
         * Alignment parameters (e.g. L1/L2 distance, tile size, search region size)
         * differs by level.
         * For now, use L1 distance.
         */

        // Algorithm
        // x_s and y_s stand for x search and y search 
        Var x_s, y_s;
        Func coarse_offset;
        Func scores;
        Func alignment;

        //Func clamped_gauss; 
        //clamped_gauss = Halide::BoundaryConditions::repeat_edge(gauss);

        // tx and ty represent the tile location

        // Iteration WITHIN a tile 
        RDom r_tile(0, tile_s, 0, tile_s);

        // Iteration for the search (over the search radius)
        //RDom r_search(-1 * search_s, 2 * search_s + 1, -1 * search_s, 2 * search_s + 1);
        RDom r_search(-1 * search_s, search_s + 1, -1 * search_s, search_s + 1);
        //RDom r_search(-4, 5, -4, 5);
        

        // CGRA does not support division. Convert division to shift.
        //FIXME: DOUBLE-CHECK THIS UPSAMPLING 
        //coarse_offset(tx, ty, xy, n) = clamp(upsample * prev_alignment ((tx+upsample >> 1) / upsample, (ty+upsample >> 1) / upsample, xy, n), align_min, align_max);
        
        //coarse_offset(tx, ty, xy, n) = clamp(upsample * prev_alignment ((tx+(upsample/2)) / upsample, (ty+(upsample/2)) / upsample, xy, n), align_min, align_max);
        //coarse_offset(tx, ty, xy, n) = clamp(upsample * prev_alignment ((tx >> tile_size_shift_factor), (ty >> tile_size_shift_factor), xy, n), align_min, align_max);
        //coarse_offset(tx, ty, xy, n) = clamp(upsample * prev_alignment (tx, ty, xy, n), align_min, align_max);
        //coarse_offset(tx, ty, xy, n) = i32(upsample * prev_alignment (tx, ty, xy, n));
        //coarse_offset(tx, ty, xy, n) = upsample * prev_alignment (tx, ty, xy, n);


        // Note that guass_width and gauss_height should be the number of tiles in each dimension. 
        //coarse_offset(tx, ty, xy, n) = upsample * upsample_float_size_2_for_alignment(prev_alignment, (((gauss_width/2)-1)/T_SIZE + 1), (((gauss_height/2)-1)/T_SIZE + 1))(tx, ty, xy, n);
        coarse_offset(tx, ty, xy, n) = upsample * upsample_float_size_2_for_alignment(prev_alignment, upsample_flow_gauss_width, upsample_flow_gauss_height)(tx, ty, xy, n);


        // Coordinates for the reference image (what we are aligning to)
        Expr x_ref = clamp(tx * tile_s + r_tile.x, 0, gauss_width-1);
        Expr y_ref = clamp(ty * tile_s + r_tile.y, 0, gauss_height-1);

        // Coordinates for the comparison frame (what is being aligned)
        //Expr x_cmp = clamp(x_ref + coarse_offset(tx, ty, 0, n) + x_s, 0, input.width()/2-1);
        //Expr y_cmp = clamp(y_ref + coarse_offset(tx, ty, 1, n) + y_s, 0, input.height()/2-1);

        Expr x_cmp = clamp(x_ref + coarse_offset(tx, ty, 0, n) + x_s, 0, gauss_width-1);
        Expr y_cmp = clamp(y_ref + coarse_offset(tx, ty, 1, n) + y_s, 0, gauss_height-1);


        // FIXME: double-check this distance computation. Maybe the datatypes should be i16. There is a chance it is returning unsigned,
        // even if the result is negative
        // Maybe just cast the result of the subtraction as i16() 
        // L1 distance between reference frame and comparison frame 
        //Expr dist = u16(abs(i16(gauss(x_ref, y_ref, 0)) - i16(gauss(x_cmp, y_cmp, n))));
        //Expr dist = u16(abs(u16(gauss(x_ref, y_ref, 0)) - u16(gauss(x_cmp, y_cmp, n))));
        //Expr dist = u16(abs(i32(i32(gauss(x_ref, y_ref, 0)) - i32(gauss(x_cmp, y_cmp, n)))));
        Expr dist = abs(gauss(x_ref, y_ref, 0) - gauss(x_cmp, y_cmp, n));


        // SUM over all pixels in tile. 
        scores(tx, ty, x_s, y_s, n) = sum(dist);

        // THIS is confusing. What is it minimizing over??? And what is this tuple that is returned? 
        // Ans: Looks like it would minimize over r_search.x and r_search.y, since tx and ty are "fixed" from the perspective of this function 
        // Brute force minimize means test all positions and find the best one. 
        Tuple min_coor = argmin(scores(tx, ty, r_search.x, r_search.y, n));

        
        //alignment(tx, ty, n) = select(n == 0, i16(0), i16(min_coor[0] + coarse_offset(tx, ty, n)),
        //            i16(min_coor[1] + coarse_offset(tx, ty, n))); 


        //alignment(tx, ty, xy, n) = select(n == 0, i16(0),
        //            xy == 0, i16(min_coor[0] + coarse_offset(tx, ty, 0, n)),
        //            i16(min_coor[1] + coarse_offset(tx, ty, 1, n))); 


        alignment(tx, ty, xy, n) = select(n == 0, 0,
                    xy == 0, min_coor[0] + coarse_offset(tx, ty, 0, n),
                    min_coor[1] + coarse_offset(tx, ty, 1, n)); 


        //alignment(tx, ty, xy, n) = scores(tx, ty, xy, xy, n);




        //alignment(tx, ty, 0, n) = select(n == 0, i16(0), i16(min_coor[0] + coarse_offset(tx, ty, n))); 
        //alignment(tx, ty, 1, n) = select(n == 0, i16(0), i16(min_coor[1] + coarse_offset(tx, ty, n))); 


        return alignment;
    }

    Func downsample_float_ef(Func f_in, Expr size, Expr gauss_width, Expr gauss_height) {
        Var x, y;
        using Halide::_;
        Func f, down;
    
        f(x, y) = f_in(x, y);
        Expr x_index_0 = clamp(size*x-1, 0, gauss_width-1);
        Expr x_index_1 = clamp(size*x, 0, gauss_width-1);
        Expr x_index_2 = clamp(size*x+1, 0, gauss_width-1);
        Expr x_index_3 = clamp(size*x+2, 0, gauss_width-1);


        Expr y_index_0 = clamp(size*y-1, 0, gauss_height-1);
        Expr y_index_1 = clamp(size*y, 0, gauss_height-1);
        Expr y_index_2 = clamp(size*y+1, 0, gauss_height-1);
        Expr y_index_3 = clamp(size*y+2, 0, gauss_height-1);

        // down(x, y) = (1.f/64.f) * f(x_index_0, y_index_0) + (3.f/64.f) * f(x_index_0, y_index_1) + (3.f/64.f) * f(x_index_0, y_index_2) + (1.f/64.f) * f(x_index_0, y_index_3) 
        //                 + (3.f/64.f) * f(x_index_1, y_index_0) + (9.f/64.f) * f(x_index_1, y_index_1) + (9.f/64.f) * f(x_index_1, y_index_2) + (3.f/64.f) * f(x_index_1, y_index_3) 
        //                 + (3.f/64.f) * f(x_index_2, y_index_0) + (9.f/64.f) * f(x_index_2, y_index_1) + (9.f/64.f) * f(x_index_2, y_index_2) + (3.f/64.f) * f(x_index_2, y_index_3) 
        //                 + (1.f/64.f) * f(x_index_3, y_index_0) + (3.f/64.f) * f(x_index_3, y_index_1) + (3.f/64.f) * f(x_index_3, y_index_2) + (1.f/64.f) * f(x_index_3, y_index_3);

        down(x, y) = (1.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (3.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (3.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (1.f/64.f) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1)) 
                        + (3.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (9.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (9.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (3.f/64.f) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1)) 
                        + (3.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (9.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (9.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (3.f/64.f) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1)) 
                        + (1.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (3.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (3.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (1.f/64.f) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1));
        return down;
    }

    Func downsample_u16_ef(Func f_in, Expr size, Expr gauss_width, Expr gauss_height) {
        Var x, y;
        using Halide::_;
        Func f, down_pre_shift, down;
    
        f(x, y) = f_in(x, y);
        Expr x_index_0 = clamp(size*x-1, 0, gauss_width-1);
        Expr x_index_1 = clamp(size*x, 0, gauss_width-1);
        Expr x_index_2 = clamp(size*x+1, 0, gauss_width-1);
        Expr x_index_3 = clamp(size*x+2, 0, gauss_width-1);


        Expr y_index_0 = clamp(size*y-1, 0, gauss_height-1);
        Expr y_index_1 = clamp(size*y, 0, gauss_height-1);
        Expr y_index_2 = clamp(size*y+1, 0, gauss_height-1);
        Expr y_index_3 = clamp(size*y+2, 0, gauss_height-1);


       /* Func: down_pre_shift
        * dtype: u16
        * True range: [0, 65472] (worst case)
        * Consumer(s): down
        */
        down_pre_shift(x, y) = (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (3) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (1) * f(clamp(size*x-1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1)) 
                        + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (9) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (3) * f(clamp(size*x, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1)) 
                        + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (9) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (3) * f(clamp(size*x+1, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1)) 
                        + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y-1, 0, gauss_height-1)) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y, 0, gauss_height-1)) + (3) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+1, 0, gauss_height-1)) + (1) * f(clamp(size*x+2, 0, gauss_width-1), clamp(size*y+2, 0, gauss_height-1));
        
       /* Func: down
        * dtype: u16
        * True range: [0, 1023] 
        * Consumer(s): returned by downsample_u16_ef
        */
        down(x, y) = down_pre_shift(x, y) >> 6;
        return down;
    }


    // Func upsample_float_size_2_ef(Func f_in) {
    //   Var x, y;
    //   using Halide::_;
    //   Func up;

    //   up(x, y, _) = (0.75f * 0.75f) * f_in(x/2, y/2, _) + (0.75f * 0.25f) * f_in(x/2, (y/2) - 1 + 2*(y % 2), _) + 
    //   (0.25f * 0.75f) * f_in((x/2) - 1 + 2*(x % 2), y/2, _) + (0.25f * 0.25f) * f_in((x/2) - 1 + 2*(x % 2), (y/2) - 1 + 2*(y % 2), _);

    //   return up;
    // }

    Func upsample_float_size_2_ef(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var x, y;
      using Halide::_;
      Func up, up_float, f_in_shift;


      f_in_shift(x, y) = f_in(x, y);

      up_float(x, y) = (0.75f * 0.75f) * f_in(x/2, y/2) 
      + (0.75f * 0.25f) * f_in(x/2, clamp((y/2) - 1 + 2*(y % 2), 0, gauss_height-1))
      + (0.25f * 0.75f) * f_in(clamp((x/2) - 1 + 2*(x % 2), 0, gauss_width-1), y/2) 
      + (0.25f * 0.25f) * f_in(clamp((x/2) - 1 + 2*(x % 2), 0, gauss_width-1), clamp((y/2) - 1 + 2*(y % 2), 0, gauss_height-1));


      up(x, y) = up_float(x, y);

      return up;
    }



    Func upsample_u16_size_2_ef(Func f_in, Expr gauss_width, Expr gauss_height) {
      Var x, y;
      using Halide::_;
      Func up, up_pre_shift, f_in_shift;


      f_in_shift(x, y) = f_in(x, y);


     /* Func: up_pre_shift
      * dtype: u16
      * True range: [0, 16368] (worst case)
      * Consumer(s): up
      */
      up_pre_shift(x, y) = (9) * f_in(x/2, y/2) 
      + (3) * f_in(x/2, clamp((y/2) - 1 + 2*(y % 2), 0, gauss_height-1))
      + (3) * f_in(clamp((x/2) - 1 + 2*(x % 2), 0, gauss_width-1), y/2) 
      + (1) * f_in(clamp((x/2) - 1 + 2*(x % 2), 0, gauss_width-1), clamp((y/2) - 1 + 2*(y % 2), 0, gauss_height-1));


     /* Func: up
      * dtype: u16
      * True range: [0, 1023] 
      * Consumer(s): returned by upsample_u16_size_2_ef
      */
      up(x, y) = up_pre_shift(x, y) >> 4;

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
      
      Expr pyramid_widths[ef_pyramid_levels];
      Expr pyramid_heights[ef_pyramid_levels];

      // pyramid_widths[0] = 1248;
      // pyramid_widths[1] = 624;
      // pyramid_widths[2] = 312;
      // pyramid_widths[3] = 156;

      pyramid_widths[0] = 1248;
      pyramid_widths[1] = 624;
      pyramid_widths[2] = 312;
      pyramid_widths[3] = 156;

      // pyramid_heights[0] = 1120;
      // pyramid_heights[1] = 560;
      // pyramid_heights[2] = 280; 
      // pyramid_heights[3] = 140; 

      pyramid_heights[0] = 1120;
      pyramid_heights[1] = 560;
      pyramid_heights[2] = 284; 
      pyramid_heights[3] = 140; 

      vector<Func> gPyramid(num_levels);
      fill_funcnames(gPyramid, name + "_gpyr");
      std::cout << "pyramid has " << num_levels << " levels" << std::endl;
      
      gPyramid[0](x,y,_) = f(x,y,_);

      for (int j = 1; j < num_levels; j++) {
        std::cout << "connecting " << j << " to " << j-1<< std::endl;
        //gPyramid[j](x,y,_) = downsample(gPyramid[j-1])(x,y,_);
        //gPyramid[j](x,y,_) = downsample_float_ef(gPyramid[j-1], 2, pyramid_widths[j-1], pyramid_heights[j-1])(x,y,_);
        gPyramid[j](x,y,_) = downsample_u16_ef(gPyramid[j-1], 2, pyramid_widths[j-1], pyramid_heights[j-1])(x,y,_);
      }

      std::cout << "next" << std::endl;

      return gPyramid;
    }

    // Create a laplacian pyramid
    vector<Func> laplacian_pyramid(Func f, int num_levels, vector<Func> &gPyramid, std::string name) {
      using Halide::_;

      Expr pyramid_widths[ef_pyramid_levels];
      Expr pyramid_heights[ef_pyramid_levels];


      pyramid_widths[0] = 1248;
      pyramid_widths[1] = 624;
      pyramid_widths[2] = 312;
      pyramid_widths[3] = 156;

      pyramid_heights[0] = 1120;
      pyramid_heights[1] = 560;
      pyramid_heights[2] = 284; 
      pyramid_heights[3] = 140; 
      
      gPyramid = gaussian_pyramid(f, num_levels, name + "_gpyr");

      vector<Func> lPyramid(num_levels);
      fill_funcnames(lPyramid, name + "_lpyr");
      
      // The last level is the same as the last level of the gaussian pyramid.
      lPyramid[num_levels-1](x,y,_) = gPyramid[num_levels-1](x,y,_);

      // Create the laplacian pyramid from the last level up to the first.
      for (int j = num_levels-2; j >= 0; j--) {
        std::cout << "Building laplacian pyramid level " << j << std::endl;

        //TODO: Cast the data to i16 BEFORE doing the subtraction. And REMEMBER that the datatype is now i16. 
        //lPyramid[j](x,y,_) = gPyramid[j](x,y,_) - upsample_float_size_2_ef(gPyramid[j+1], pyramid_widths[j+1], pyramid_heights[j+1])(x,y,_);
        lPyramid[j](x,y,_) = i16(gPyramid[j](x,y,_)) - i16(upsample_u16_size_2_ef(gPyramid[j+1], pyramid_widths[j+1], pyramid_heights[j+1])(x,y,_));
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
        std::cout << "Merging pyramids on level " << i << std::endl;
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

          // (weights0[i](x,y,_)/128.f * img0[i](x,y,_) +
          // weights1[i](x,y,_)/128.f * img1[i](x,y,_));

          // ((weights0[i](x,y,_) * img0[i](x,y,_)) +
          // (weights1[i](x,y,_) * img1[i](x,y,_)));


          mul_1023(weights0[i](x,y,_), img0[i](x,y,_)) + mul_1023(weights1[i](x,y,_), img1[i](x,y,_));



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

      Expr pyramid_widths[ef_pyramid_levels];
      Expr pyramid_heights[ef_pyramid_levels];

      // pyramid_widths[0] = 1248;
      // pyramid_widths[1] = 624;
      // pyramid_widths[2] = 312;
      // pyramid_widths[3] = 156;

      pyramid_widths[0] = 1248;
      pyramid_widths[1] = 624;
      pyramid_widths[2] = 312;
      pyramid_widths[3] = 156;

      // pyramid_heights[0] = 1120;
      // pyramid_heights[1] = 560;
      // pyramid_heights[2] = 280; 
      // pyramid_heights[3] = 140; 

      pyramid_heights[0] = 1120;
      pyramid_heights[1] = 560;
      pyramid_heights[2] = 284; 
      pyramid_heights[3] = 140; 
      
      //init the last upsampled stencil to be the deepest gaussian pyramid
      int num_levels = pyramid.size();
      upsampled[num_levels-1] = pyramid[num_levels-1];

      //Blend to the top
      for (int level = num_levels-1; level > 0; --level) {
        std::cout << "Flattening pyramids on level " << level << std::endl;
        //upsampled[level-1](x,y,_) = pyramid[level-1](x,y,_) + upsample_float_size_2_ef(upsampled[level], pyramid_widths[level], pyramid_heights[level])(x,y,_);
        upsampled[level-1](x,y,_) = pyramid[level-1](x,y,_) + upsample_u16_size_2_ef(upsampled[level], pyramid_widths[level], pyramid_heights[level])(x,y,_);
      }
      
      std::cout << "Done flattening" << std::endl;
      return upsampled[0];
    }
  };
}
HALIDE_REGISTER_GENERATOR(HDRPlus, hdr_plus)