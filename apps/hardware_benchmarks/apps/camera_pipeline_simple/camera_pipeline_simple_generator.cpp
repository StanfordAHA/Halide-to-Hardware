/*
 * An application that performs a basic camera pipeline.
 * Stages of the pipeline are hot pixel suppression, demosaicking,
 * color corrections, and applying a camera curve.
 */

#include "Halide.h"

namespace {

  using namespace Halide;
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  int16_t matrix[3][4] = {{ 200, -44,  17, -3900},
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
      // common patterns: average of four surrounding pixels
      Func neswNeighbors, diagNeighbors;
      neswNeighbors(x, y) = (raw(x-1, y)   + raw(x+1, y) +
                             raw(x,   y-1) + raw(x,   y+1)/4);
      diagNeighbors(x, y) = (raw(x-1, y-1) + raw(x+1, y-1) +
                             raw(x-1, y+1) + raw(x+1, y+1)/4);

      // common patterns: average of two adjacent pixels
      Func vNeighbors, hNeighbors;
      vNeighbors(x, y) = (raw(x, y-1) + raw(x, y+1))/2;
      hNeighbors(x, y) = (raw(x-1, y) + raw(x+1, y))/2;

      // output pixels depending on image layout.
      // Generally, image looks like
      //    R G R G R G R G
      //    G B G B G B G B
      //    R G R G R G R G
      //    G B G B G B G B
      Func green, red, blue;
      green(x, y) = select((y % 2) == (0),
                           select((x % 2) == (0), neswNeighbors(x, y), raw(x, y)), // First row, RG
                           select((x % 2) == (0), raw(x, y),           neswNeighbors(x, y))); // Second row, GB

      red(x, y) = select((y % 2) == (0),
                         select((x % 2) == (0), raw(x, y),        hNeighbors(x, y)), // First row, RG
                         select((x % 2) == (0), vNeighbors(x, y), diagNeighbors(x, y))); // Second row, GB

      blue(x, y) = select((y % 2) == (0),
                          select((x % 2) == (0), diagNeighbors(x, y), vNeighbors(x, y)), // First row, RG
                          select((x % 2) == (0), hNeighbors(x, y),    raw(x, y))); // Second row, GB

      // output all channels
      Func demosaicked;
      demosaicked(x,y,c) = cast<uint8_t>(select(c == 0, red(x, y),
                                                c == 1, green(x, y),
                                                blue(x, y)));
      return demosaicked;
    }

    // Applies a color correction matrix to redefine rgb values.
    Func color_correct(Func input, int16_t matrix[3][4]) {
      Expr ir = cast<int16_t>(input(x, y, 0));
      Expr ig = cast<int16_t>(input(x, y, 1));
      Expr ib = cast<int16_t>(input(x, y, 2));

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
      //hw_output(c, x, y) = select(input(x, y, c) < 0, 0,
      //                            input(x, y, c) >= 1024, 255,
      //                            curve(in_val));

      hw_output(x, y, c) = curve(in_val);
      //hw_output(x, y, c) = cast<uint8_t>(in_val);

      return hw_output;
    }

    
    void generate() {

      Func hw_input;
      hw_input(x,y) = cast<uint16_t>(input(x+3,y+3));

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

      output(x, y, c) = hw_output(x, y, c);
      
      output.bound(x, 0, 64-6);
      output.bound(y, 0, 64-6);
      output.bound(c, 0, 3);
      curve.bound(x, 0, 1023);
      
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {
        hw_input.compute_root();
        hw_output.compute_root();
          
        hw_output.tile(x, y, xo, yo, xi, yi, 64-6,64-6)
          .reorder(c, xi, yi, xo, yo);
          
        denoised.linebuffer()
          .unroll(x).unroll(y);
        demosaicked.linebuffer()
          .reorder(c, x, y)
          .unroll(c);//.unroll(x).unroll(y);

        curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM

        hw_output.accelerate({hw_input}, xi, xo, {});
        //hw_output.unroll(c).unroll(xi, 2);
        hw_output.unroll(c);
          
      } else {    // schedule to CPU
        output.tile(x, y, xo, yo, xi, yi, 64-6, 64-6)
          .compute_root();

        output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4);
      }
    }
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(CameraPipeline, camera_pipeline_simple)

