/*
 * An application that performs a basic camera pipeline.
 * Stages of the pipeline are hot pixel suppression, demosaicking,
 * color corrections, and applying a camera curve.
 */

#include "Halide.h"

namespace {

  using namespace Halide;
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  int32_t matrix[3][4] = {{ 200, -44,  17, -3900},
                          {-38,  159, -21, -2541},
                          {-8, -73,  228, -2008}};


  class CameraPipeline : public Halide::Generator<CameraPipeline> {
    
  public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    GeneratorParam<float> gamma{"gamma", /*default=*/1.0};
    GeneratorParam<float> contrast{"contrast", /*default=*/1.0};

    Func interleave_x(Func a, Func b) {
      Func out;
      out(x, y) = select((x%2)==0, a(x, y), b(x-1, y));
      return out;
    }

    Func interleave_y(Func a, Func b) {
      Func out;
      out(x, y) = select((y%2)==0, a(x, y), b(x, y-1));
      return out;
    }

    Expr avg(Expr a, Expr b) {
      return (a + b + 1) >> 1;
    }
    
    // Performs hot pixel suppression by comparing to local pixels.
    Func hot_pixel_suppression(Func input) {
      Func denoised("denoised");
      Expr max_value = max(max(input(x-2, y), input(x+2, y)),
                           max(input(x, y-2), input(x, y+2)));
      Expr min_value = min(min(input(x-2, y), input(x+2, y)),
                           min(input(x, y-2), input(x, y+2)));
      
      denoised(x, y) = clamp(input(x,y), min_value, max_value);
      return denoised;
    }

    // Demosaics a raw input image. Recall that the values appear as:
    //   R G R G R G R G
    //   G B G B G B G B
    //   R G R G R G R G
    //   G B G B G B G B
    Func demosaic(Func raw) {
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

      // Give more convenient names to the four channels we know
      Func r_r, g_gr, g_gb, b_b;
      g_gr(x, y) = raw(x, y);//deinterleaved(x, y, 0);
      r_r(x, y)  = raw(x+1, y);//deinterleaved(x, y, 1);
      b_b(x, y)  = raw(x, y+1);//deinterleaved(x, y, 2);
      g_gb(x, y) = raw(x+1, y+1);//deinterleaved(x, y, 3);

      // These are the ones we need to interpolate
      Func b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b;

      // First calculate green at the red and blue sites
      // Try interpolating vertically and horizontally. Also compute
      // differences vertically and horizontally. Use interpolation in
      // whichever direction had the smallest difference.
      Expr gv_r  = avg(g_gb(x, y-1), g_gb(x, y));
      Expr gvd_r = absd(g_gb(x, y-1), g_gb(x, y));
      Expr gh_r  = avg(g_gr(x+1, y), g_gr(x, y));
      Expr ghd_r = absd(g_gr(x+1, y), g_gr(x, y));

      g_r(x, y)  = select(ghd_r < gvd_r, gh_r, gv_r);

      Expr gv_b  = avg(g_gr(x, y+1), g_gr(x, y));
      Expr gvd_b = absd(g_gr(x, y+1), g_gr(x, y));
      Expr gh_b  = avg(g_gb(x-1, y), g_gb(x, y));
      Expr ghd_b = absd(g_gb(x-1, y), g_gb(x, y));

      g_b(x, y)  = select(ghd_b < gvd_b, gh_b, gv_b);

      // Next interpolate red at gr by first interpolating, then
      // correcting using the error green would have had if we had
      // interpolated it in the same way (i.e. add the second derivative
      // of the green channel at the same place).
      Expr correction;
      correction = g_gr(x, y) - avg(g_r(x, y), g_r(x-1, y));
      r_gr(x, y) = correction + avg(r_r(x-1, y), r_r(x, y));

      // Do the same for other reds and blues at green sites
      correction = g_gr(x, y) - avg(g_b(x, y), g_b(x, y-1));
      b_gr(x, y) = correction + avg(b_b(x, y), b_b(x, y-1));

      correction = g_gb(x, y) - avg(g_r(x, y), g_r(x, y+1));
      r_gb(x, y) = correction + avg(r_r(x, y), r_r(x, y+1));

      correction = g_gb(x, y) - avg(g_b(x, y), g_b(x+1, y));
      b_gb(x, y) = correction + avg(b_b(x, y), b_b(x+1, y));

      // Now interpolate diagonally to get red at blue and blue at
      // red. Hold onto your hats; this gets really fancy. We do the
      // same thing as for interpolating green where we try both
      // directions (in this case the positive and negative diagonals),
      // and use the one with the lowest absolute difference. But we
      // also use the same trick as interpolating red and blue at green
      // sites - we correct our interpolations using the second
      // derivative of green at the same sites.
      correction = g_b(x, y)  - avg(g_r(x, y), g_r(x-1, y+1));
      Expr rp_b  = correction + avg(r_r(x, y), r_r(x-1, y+1));
      Expr rpd_b = absd(r_r(x, y), r_r(x-1, y+1));

      correction = g_b(x, y)  - avg(g_r(x-1, y), g_r(x, y+1));
      Expr rn_b  = correction + avg(r_r(x-1, y), r_r(x, y+1));
      Expr rnd_b = absd(r_r(x-1, y), r_r(x, y+1));

      r_b(x, y)  = select(rpd_b < rnd_b, rp_b, rn_b);

      // Same thing for blue at red
      correction = g_r(x, y)  - avg(g_b(x, y), g_b(x+1, y-1));
      Expr bp_r  = correction + avg(b_b(x, y), b_b(x+1, y-1));
      Expr bpd_r = absd(b_b(x, y), b_b(x+1, y-1));

      correction = g_r(x, y)  - avg(g_b(x+1, y), g_b(x, y-1));
      Expr bn_r  = correction + avg(b_b(x+1, y), b_b(x, y-1));
      Expr bnd_r = absd(b_b(x+1, y), b_b(x, y-1));

      b_r(x, y)  =  select(bpd_r < bnd_r, bp_r, bn_r);

      // Interleave the resulting channels
      Func r = interleave_y(interleave_x(r_gr, r_r),
                            interleave_x(r_b, r_gb));
      Func g = interleave_y(interleave_x(g_gr, g_r),
                            interleave_x(g_b, g_gb));
      Func b = interleave_y(interleave_x(b_gr, b_r),
                            interleave_x(b_b, b_gb));

      Func demosaicked("demosaicked");
      demosaicked(x, y, c) = select(c == 0, r(x, y),
                                    c == 1, g(x, y),
                                    b(x, y));
      return demosaicked;
    }

    // Applies a color correction matrix to redefine rgb values.
    Func color_correct(Func input, int32_t matrix[3][4]) {
      Expr ir = cast<int32_t>(input(x, y, 0));
      Expr ig = cast<int32_t>(input(x, y, 1));
      Expr ib = cast<int32_t>(input(x, y, 2));

      Expr r = matrix[0][3] + matrix[0][0] * ir + matrix[0][1] * ig + matrix[0][2] * ib;
      Expr g = matrix[1][3] + matrix[1][0] * ir + matrix[1][1] * ig + matrix[1][2] * ib;
      Expr b = matrix[2][3] + matrix[2][0] * ir + matrix[2][1] * ig + matrix[2][2] * ib;

      r = cast<int16_t>(r/256);
      g = cast<int16_t>(g/256);
      b = cast<int16_t>(b/256);

      Func corrected;
      corrected(x, y, c) = select(c == 0, r,
                                  c == 1, g,
                                  b);

      return corrected;
    }


    // Applies a non-linear camera curve.
    Func apply_curve(Func input, Func curve) {
      // copied from FCam

      Func hw_output("hw_output");
      Expr in_val = clamp(input(x, y, c), 0, 1023);
      hw_output(c, x, y) = select(input(x, y, c) < 0, 0,
                                  input(x, y, c) >= 1024, 255,
                                  curve(in_val));

      return hw_output;
    }

    
    void generate() {

      Func hw_input;
      hw_input(x,y) = input(x,y);

      Func denoised;
      denoised = hot_pixel_suppression(hw_input);

      Func demosaicked;
      demosaicked = demosaic(denoised);

      Func color_corrected;
      color_corrected = color_correct(demosaicked, matrix);

      Func curve;
      {
        Expr xf = x/1024.0f;
        Expr g = pow(xf, 1.0f/gamma);
        Expr b = 2.0f - (float) pow(2.0f, contrast/100.0f);
        Expr a = 2.0f - 2.0f*b;
        Expr val = select(g > 0.5f,
                          1.0f - (a*(1.0f-g)*(1.0f-g) + b*(1.0f-g)),
                          a*g*g + b*g);
        curve(x) = cast<uint8_t>(clamp(val*256.0f, 0.0f, 255.0f));
      }

      Func hw_output;
      hw_output = apply_curve(color_corrected, curve);

      output(c, x, y) = hw_output(c, x, y);

      output.bound(c, 0, 3);
      output.bound(x, 0, 58);
      output.bound(y, 0, 58);

        
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {
        hw_input.compute_root();
        hw_output.compute_root();
          
        hw_output.tile(x, y, xo, yo, xi, yi, 64-6,64-6)
          .reorder(c,xi,yi,xo,yo);;
          
        denoised.linebuffer();
        //.unroll(x).unroll(y);
        demosaicked.linebuffer();
          //.unroll(c).unroll(x).unroll(y);

        curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM

        hw_output.accelerate({hw_input}, c, xo, {});

        //hw_output.unroll(c).unroll(xi, 2);
        hw_output.unroll(c);

        hw_input.stream_to_accelerator();
          
      } else {    // schedule to CPU
        output.tile(x, y, xo, yo, xi, yi, 64, 64)
          .compute_root();

        output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4);
      }
    }
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(CameraPipeline, camera_pipeline)

