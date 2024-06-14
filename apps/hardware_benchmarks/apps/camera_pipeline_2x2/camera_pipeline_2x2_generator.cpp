/*
 * An application that performs a basic camera pipeline.
 * Stages of the pipeline are hot pixel suppression, demosaicking,
 * color corrections, and applying a camera curve.
 */

#include "Halide.h"

namespace {
int blockSize = 9;
//int blockSize = 5;
//int blockSize = 1;

  using namespace Halide;
  using namespace Halide::ConciseCasts;
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  int16_t matrix[3][4] = {{549, -103,   7, -10221},
                          {-96,  373,  62,  -7254},
                          {-31, -261, 883,  -5563}};

  class CameraPipeline : public Halide::Generator<CameraPipeline> {
    
  public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    GeneratorParam<float> gamma{"gamma", /*default=*/2.0};
    GeneratorParam<float> contrast{"contrast", /*default=*/50.0};
    GeneratorParam<uint8_t> schedule{"schedule", 3};    // default: 3
    GeneratorParam<uint8_t> width{"width", 0};          // default: 0
    GeneratorParam<uint8_t> myunroll{"myunroll", 1};    // default: 1

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
    
    // Performs hot pixel suppression by comparing to local pixels.
    Func hot_pixel_suppression(Func input) {
      Func denoised("denoised");
      Expr max_value = max(max(input(x-2, y), input(x+2, y)),
                           max(input(x, y-2), input(x, y+2)));
      Expr min_value = min(min(input(x-2, y), input(x+2, y)),
                           min(input(x, y-2), input(x, y+2)));
      
      //denoised(x, y) = clamp(input(x,y), min_value, max_value);
      denoised(x, y) = clamp(input(x,y), 0, max_value);
      //denoised(x, y) = input(x, y);
      return denoised;
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
      //demosaicked.bound(c, 0, 3);
      return demosaicked;
    }

    Expr mul1(Expr a, Expr b) {
      return i16(( (i32(a)) * (i32(b)) ) >> 8);
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
      Expr ir = clamp(input(x, y, 0), 0, 10000);
      Expr ig = clamp(input(x, y, 1), 0, 10000);
      Expr ib = clamp(input(x, y, 2), 0, 10000);
      Expr r = (matrix[0][3]>>8) + mul1(matrix[0][0], ir) + mul1(matrix[0][1], ig) + mul1(matrix[0][2], ib);
      Expr g = (matrix[1][3]>>8) + mul1(matrix[1][0], ir) + mul1(matrix[1][1], ig) + mul1(matrix[1][2], ib);
      Expr b = (matrix[2][3]>>8) + mul1(matrix[2][0], ir) + mul1(matrix[2][1], ig) + mul1(matrix[2][2], ib);

      Func corrected("corrected");
      corrected(x, y, c) = select(c == 0, r,
                                  c == 1, g,
                                  b);
      return corrected;
    }


    // Applies a non-linear camera curve.
    Func apply_curve(Func input, Func curve) {
      // copied from FCam

      Func curved("curved");
      Expr in_val = clamp(input(x, y, c), u16(0), u16(1023));
      curved(x, y, c) = curve(u16(in_val));

      return curved;
    }

    const int tWidth = 256-8;
    const int tHeight = 192-8;
    const int nTiles = 1;
    
    void generate() {

      Func hw_input, hw_input_temp, hw_input_shuffle, hw_input_shift;
      //hw_input_temp(x,y) = u16(input(x+(blockSize-1)/2, y+(blockSize-1)/2));
      hw_input_temp(x,y) = u16(input(x, y));

      if (get_target().has_feature(Target::Clockwork)) {
        hw_input_shuffle(x, y, c) = hw_input_temp(2*x + c/2, 2*y + c%2);

        //hw_input(x, y) = hw_input_shuffle(x/4 + 622*(y%2), y/2, x%4);
        int iWidth = (tWidth * nTiles + blockSize-1) / 4;
        hw_input_shift(x, y) = hw_input_shuffle(x/4 + iWidth*(y%2), y/2, x%4);
        hw_input(x, y) = hw_input_shift(x+(blockSize-1)/2, y+(blockSize-1)/2);
      } else {
        hw_input(x, y) = hw_input_temp(x+(blockSize-1)/2, y+(blockSize-1)/2);
      }

      Func hw_input_copy;
      
      Func denoised;
      denoised = hot_pixel_suppression(hw_input);

      // Give more convenient names to the four channels we know
      Func r_r, g_gr, g_gb, b_b;
      g_gr(x, y) = denoised(2*x, 2*y);//deinterleaved(x, y, 0);
      r_r(x, y)  = denoised(2*x+1, 2*y);//deinterleaved(x, y, 1);
      b_b(x, y)  = denoised(2*x, 2*y+1);//deinterleaved(x, y, 2);
      g_gb(x, y) = denoised(2*x+1, 2*y+1);//deinterleaved(x, y, 3);

      Func demosaicked;
      Func b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b;
      demosaicked = demosaic(g_gr, r_r, b_b, g_gb,
                             b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b);

      Func color_corrected;
      color_corrected = color_correct(demosaicked, matrix);

      Func curve;
      {
        Expr xf = clamp(cast<float>(x)/1024.0f, 0.f, 1.f);
        Expr g = pow(xf, 1.0f/gamma);
        Expr b = 2.0f - (float) pow(2.0f, contrast/100.0f);
        Expr a = 2.0f - 2.0f*b;
        Expr val = select(g > 0.5f,
                          1.0f - (a*(1.0f-g)*(1.0f-g) + b*(1.0f-g)),
                          a*g*g + b*g);
        curve(x) = u16(clamp(val*256.0f, 0.0f, 255.0f));
        //curve(x) = clamp(val*256.0f, 0.0f, 255.0f);
      }

      Func hw_output, curve_out, output_shuffle;
      curve_out = apply_curve(color_corrected, curve);
      hw_output(c, x, y) = curve_out(x, y, c);
      //hw_output(c, x, y) = demosaicked(x, y, c);
      //hw_output(c, x, y) = denoised(x, y);

      Var k;
      if (get_target().has_feature(Target::Clockwork)) {
        int iWidth = tWidth * nTiles / 4;
        output_shuffle(c, k, x, y) = u8(hw_output(c, (x%iWidth)*4 + k, x/iWidth + 2*y));
        //output(x, y, c) = output_shuffle(c, y%2 + 2*(x%2), max(x/2 - 1, 0), y/2);
        output(x, y, c) = output_shuffle(c, y%2 + 2*(x%2), x/2, y/2);
      } else {
        //output(x, y, c) = u8(hw_output(c, x+2, y));
        output(x, y, c) = u8(hw_output(c, x, y));
      }


      //curve.bound(x, 0, 256);
      output.bound(c, 0, 3);
        
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {
        hw_input.store_at(hw_output, xo).compute_at(denoised, x);
        hw_output.compute_root();

        hw_output.accelerate({hw_input}, xi, xo, {});
        hw_output.tile(x, y, xo, yo, xi, yi, 64-6,64-6)
          .reorder(c,xi,yi,xo,yo);

        curve_out.compute_at(hw_output, xo);
        //hw_output.unroll(c).unroll(xi, 2);
        hw_output.unroll(c);
        
        demosaicked.linebuffer();
        demosaicked.unroll(c);
        //demosaicked.reorder(c, x, y);

        denoised.linebuffer();
        //.unroll(x).unroll(y);

        curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
        
        hw_input.stream_to_accelerator();

      } else if (get_target().has_feature(Target::Clockwork)) {

          if (schedule == 1) { // host and glb tiling
            const int numHostTiles = 4;
            const int numTiles = 3;
            const int tileSize = 58;
            const int glbSize = tileSize * numTiles;
            const int outputSize = numHostTiles * glbSize;

            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
              .reorder(c, xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(c);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .reorder(c, xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(c);

            curve_out.compute_at(hw_output, xo);
            curve_out.unroll(c);
        
            color_corrected.compute_at(hw_output, xo);
            color_corrected.unroll(c);
        
            demosaicked.compute_at(hw_output, xo);
            demosaicked
              .reorder(c, x, y)
              .unroll(c);

            denoised.compute_at(hw_output, xo);
            //.unroll(x).unroll(y);

            g_gr.compute_at(hw_output, xo);
            r_r.compute_at(hw_output, xo);
            b_b.compute_at(hw_output, xo);
            g_gb.compute_at(hw_output, xo);
        
            curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
            
            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            //hw_input.in().unroll(c);  // hw input bound
            
            hw_input.compute_root()
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

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .reorder(c, xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(c);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .reorder(c, xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(c);

            curve_out.compute_at(hw_output, xo);
            curve_out.unroll(c);
        
            color_corrected.compute_at(hw_output, xo);
            color_corrected.unroll(c);
        
            demosaicked.compute_at(hw_output, xo);
            demosaicked
              .reorder(c, x, y)
              .unroll(c);

            denoised.compute_at(hw_output, xo);
            //.unroll(x).unroll(y);

            g_gr.compute_at(hw_output, xo);
            r_r.compute_at(hw_output, xo);
            b_b.compute_at(hw_output, xo);
            g_gb.compute_at(hw_output, xo);
        
            curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
            
            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            //hw_input.in().unroll(c);  // hw input bound
            
            hw_input.compute_root()
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

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            Var xii, yii, xio, yio;
            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .split(yi, yio, yii, unrolly)
              .reorder(c, yii, xi, yio, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(c)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(xi, unrollx, TailStrategy::RoundUp);

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .split(yi, yio, yii, unrolly)
              .reorder(c, yii, xi, yio, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(c)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(xi, unrollx, TailStrategy::RoundUp);

            curve_out.compute_at(hw_output, xo);
            curve_out.unroll(c)
              .split(y, yio, yii, unrolly).reorder(c, yii, x, yio)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(x, unrollx, TailStrategy::RoundUp);
        
            color_corrected.compute_at(hw_output, xo);
            color_corrected.unroll(c)
              .split(y, yio, yii, unrolly).reorder(c, yii, x, yio)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(x, unrollx, TailStrategy::RoundUp);
        
            demosaicked.compute_at(hw_output, xo);
            demosaicked
              .split(y, yio, yii, unrolly).reorder(c, yii, x, yio)
              .unroll(c)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(x, unrollx, TailStrategy::RoundUp);

            denoised.compute_at(hw_output, xo)
              .split(y, yio, yii, unrolly).reorder(yii, x, yio)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(x, unrollx);
            //.unroll(x).unroll(y);

            bool buffer_memories = true;
            if (buffer_memories) {
              b_r.compute_at(hw_output, xo);
              g_r.compute_at(hw_output, xo);
              b_gr.compute_at(hw_output, xo);
              r_gr.compute_at(hw_output, xo);
              b_gb.compute_at(hw_output, xo);
              r_gb.compute_at(hw_output, xo);
              r_b.compute_at(hw_output, xo);
              g_b.compute_at(hw_output, xo);
            }

            g_gr.compute_at(hw_output, xo);
            r_r.compute_at(hw_output, xo);
            b_b.compute_at(hw_output, xo);
            g_gb.compute_at(hw_output, xo);
            
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
        
            curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
            curve.store_in(MemoryType::ROM);
            // unroll by x?

            hw_input.in().in().compute_at(hw_output, xo); // represents the mem tile
            hw_input.in().in()
              .split(y, yio, yii, unrolly).reorder(yii, x, yio)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(x, unrollx, TailStrategy::RoundUp);
            
            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in()
              .split(y, yio, yii, unrolly).reorder(yii, x, yio)
              .unroll(yii, unrolly, TailStrategy::RoundUp)
              .unroll(x, unrollx, TailStrategy::RoundUp);
            
            hw_input.compute_root()
              .accelerator_input();
            
        } else {
          output.bound(x, 0, 64-blockSize+1);
          output.bound(y, 0, 64-blockSize+1);
            
          hw_output.compute_root();

          hw_output.tile(x, y, xo, yo, xi, yi, 64-blockSize+1,64-blockSize+1)
            .reorder(c,xi,yi,xo,yo)
            .reorder_storage(c, x, y)
            .hw_accelerate(xi, xo);
          hw_output.unroll(c);
          //hw_output.unroll(c).unroll(xi, 2);
        
          //curve_out.reorder(c, x, y).reorder_storage(c, x, y);
          curve_out.compute_at(hw_output, xo);
          curve_out.unroll(c);
        
          color_corrected.compute_at(hw_output, xo);
          color_corrected.unroll(c);
        
          demosaicked.compute_at(hw_output, xo);
          demosaicked.unroll(c);
          //demosaicked.reorder(c, x, y);

          denoised.compute_at(hw_output, xo);
          //.unroll(x).unroll(y);

          g_gr.compute_at(hw_output, xo);
          r_r.compute_at(hw_output, xo);
          b_b.compute_at(hw_output, xo);
          g_gb.compute_at(hw_output, xo);

          //b_r.compute_at(hw_output, xo);
          //g_r.compute_at(hw_output, xo);
          //b_gr.compute_at(hw_output, xo);
          //r_gr.compute_at(hw_output, xo);
          //b_gb.compute_at(hw_output, xo);
          //r_gb.compute_at(hw_output, xo);
          //r_b.compute_at(hw_output, xo);
          //g_b.compute_at(hw_output, xo);
        
          curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
        
          //hw_input_copy.compute_at(hw_output, xo);
          hw_input.stream_to_accelerator();
          //hw_input.compute_root();
        }
        
      } else {    // schedule to CPU
        if (schedule == 1 || schedule == 2 || schedule == 3) {
          Var yii;
          const int strip_size = 2;
          const int vec = 4;

          output
            .compute_root()
            .reorder(c, x, y)
            .split(y, yi, yii, 2, TailStrategy::RoundUp)
            .split(yi, yo, yi, strip_size / 2)
            .vectorize(x, 2 * vec, TailStrategy::RoundUp)
            .unroll(c)
            .parallel(yo);

          denoised
            .compute_at(output, yi)
            .store_at(output, yo)
            .prefetch(input, y, 2)
            //.fold_storage(y, 4)
            .tile(x, y, x, y, xi, yi, 2 * vec, 2)
            .vectorize(xi)
            .unroll(yi);

          demosaicked
            .compute_at(output, yi)
            .store_at(output, yo)
            .fold_storage(y, 4)
            .reorder(c, x, y)
            .vectorize(x, 2 * vec, TailStrategy::RoundUp)
            .unroll(c);

          curve_out
            .compute_at(output, yi)
            .store_at(output, yo)
            .reorder(c, x, y)
            .tile(x, y, x, y, xi, yi, 2 * vec, 2, TailStrategy::RoundUp)
            .vectorize(xi)
            .unroll(yi)
            .unroll(c);

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
    }
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(CameraPipeline, camera_pipeline_2x2)
