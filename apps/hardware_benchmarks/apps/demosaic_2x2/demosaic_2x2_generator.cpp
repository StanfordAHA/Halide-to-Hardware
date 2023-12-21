/*
 * An application that performs a basic camera pipeline.
 * Stages of the pipeline are hot pixel suppression, demosaicking,
 * color corrections, and applying a camera curve.
 */

#include "Halide.h"

namespace {
int ksize = 9;
  using namespace Halide;
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");

  class DemosaicPipeline : public Halide::Generator<DemosaicPipeline> {
    
  public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

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
      return (a + b + 1) >> 1;
    }
    
    // Demosaics a raw input image. Recall that the values appear as:
    //   R G R G R G R G
    //   G B G B G B G B
    //   R G R G R G R G
    //   G B G B G B G B
    Func demosaic(Func deinterleaved) {
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
      //g_gr(x, y) = deinterleaved(x, y, 0);
      //r_r(x, y)  = deinterleaved(x, y, 1);
      //b_b(x, y)  = deinterleaved(x, y, 2);
      //g_gb(x, y) = deinterleaved(x, y, 3);
      g_gr(x, y) = deinterleaved(2*x, 2*y);//deinterleaved(x, y, 0);
      r_r(x, y)  = deinterleaved(2*x+1, 2*y);//deinterleaved(x, y, 1);
      b_b(x, y)  = deinterleaved(2*x, 2*y+1);//deinterleaved(x, y, 2);
      g_gb(x, y) = deinterleaved(2*x+1, 2*y+1);//deinterleaved(x, y, 3);


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
      //demosaicked(x, y, c) = select(c == 0, r(x, y),
      //                              c == 1, g(x, y),
      //                              b(x, y));
      demosaicked(x, y, c) = select(c == 0, r(x, y),
                                    c == 1, g(x, y),
                                    /*c==2*/b(x, y));      
      return demosaicked;
    }

    void generate() {

      Func hw_input;
      hw_input(x,y) = cast<uint16_t>(input(x+(ksize-1)/2, y+(ksize-1)/2));

      Func deinterleaved;
      deinterleaved(x,y,c) = select(c == 0, hw_input(x*2,     y*2),
                                    c == 1, hw_input(x*2 + 1, y*2),
                                    c == 2, hw_input(x*2,     y*2 + 1),
                                    /*c==3*/hw_input(x*2 + 1, y*2 + 1));
                                    

      Func demosaicked;
      //demosaicked = demosaic(deinterleaved);
      demosaicked = demosaic(hw_input);

      Func hw_output;
      hw_output(x,y,c) = demosaicked(x,y,c);
      //hw_output(c,x,y) = cast<uint8_t>(demosaicked(x,y,c));

      output(x, y, c) = cast<uint8_t>(hw_output(x, y, c));
      //output(x, y, c) = hw_output(c, x, y);
            
      /* THE SCHEDULE */
      if (get_target().has_feature(Target::CoreIR)) {
        output.bound(x, 0, 64-2);
        output.bound(y, 0, 64-2);
        output.bound(c, 0, 3);
        
        hw_input.compute_root();
        hw_output.compute_root();
          
        hw_output.tile(x, y, xo, yo, xi, yi, 64-2,64-2);
        hw_output.accelerate({hw_input}, xi, xo, {});
        hw_output.unroll(c);
          
        demosaicked.linebuffer()
          .unroll(c).unroll(x).unroll(y);

        hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);

      } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
        output.bound(c, 0, 3);
        output.bound(x, 0, 64-ksize+1);
        output.bound(y, 0, 64-ksize+1);

        hw_output.compute_root();

        hw_output.tile(x, y, xo, yo, xi, yi, 64-ksize+1,64-ksize+1)
          .reorder(xi,yi,c,xo,yo)
          .hw_accelerate(xi, xo);
        
        hw_output.unroll(c);
        
        demosaicked.compute_at(hw_output, xo);
        demosaicked.unroll(c);
        //demosaicked.reorder(c, x, y);

        //deinterleaved.unroll(c);
        
        //hw_input_copy.compute_at(hw_output, xo);
        //hw_input.unroll(x,2).unroll(y,2);
        hw_input.stream_to_accelerator();
        //hw_input.in().unroll(x,2).unroll(y,2);
        //hw_input.compute_root();

        //Var yio, yii;
        //hw_input.in()
        //  .split(y, yio, yii, 2).reorder(yii, x, yio)
        //  .unroll(yii, 2, TailStrategy::RoundUp)
        //  .unroll(x, 2, TailStrategy::RoundUp);


          
      } else {    // schedule to CPU

      }
    }
  };

}  // namespace

HALIDE_REGISTER_GENERATOR(DemosaicPipeline, demosaic_2x2)

