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
  // int16_t matrix[3][4] = {{549, -103,   7, -10221},
  //                         {-96,  373,  62,  -7254},
  //                         {-31, -261, 883,  -5563}};
  
   

  int16_t matrix[3][4] = {{476, -180,  -40, 0},
                          {-60,  426,  -110, 0},
                          {2, -168, 420, 0}};
  

// NOTE: NEED TO EXPRESS THIS for 4 CHANNELS. ONLY HAVE 1 CHANNEL RN...
 /* uint16_t lens_shading_factors[13][17] = {{807, 704, 640, 593, 547, 501, 465, 447, 437, 443, 454, 479, 520, 554, 586, 629, 697},
{741, 651, 589, 538, 483, 440, 413, 399, 391, 394, 404, 424, 454, 500, 538, 576, 635},
{694, 613, 547, 484, 437, 399, 376, 360, 353, 357, 368, 390, 416, 453, 502, 545, 591},
{668, 588, 512, 450, 404, 369, 343, 325, 318, 323, 338, 360, 387, 423, 471, 521, 577},
{650, 565, 488, 427, 382, 344, 317, 298, 293, 298, 313, 336, 368, 400, 445, 504, 560},
{641, 551, 472, 413, 370, 330, 300, 283, 276, 282, 297, 322, 353, 391, 433, 488, 554},
{645, 549, 468, 410, 363, 326, 297, 279, 271, 277, 293, 320, 351, 389, 430, 487, 557},
{645, 555, 477, 416, 372, 334, 303, 286, 280, 285, 300, 326, 358, 393, 436, 491, 561},
{658, 577, 498, 433, 390, 353, 322, 307, 299, 303, 320, 343, 375, 408, 451, 508, 563},
{672, 600, 527, 463, 414, 381, 354, 334, 328, 332, 346, 370, 398, 432, 482, 530, 574},
{710, 628, 565, 507, 453, 415, 390, 374, 367, 370, 383, 403, 429, 466, 512, 557, 598},
{762, 674, 613, 562, 507, 463, 435, 418, 412, 413, 425, 444, 473, 514, 554, 590, 648},
{834, 729, 653, 614, 570, 525, 485, 467, 463, 467, 478, 501, 534, 564, 593, 639, 718}};*/
  

  /*int16_t lens_shading_factors[13][17][4] = {{{807, 807, 902, 799}, {704, 704, 847, 679}, {640, 640, 758, 606}, {593, 593, 698, 541}, {547, 547, 671, 497}, {501, 501, 570, 485}, {465, 465, 524, 458}, {447, 447, 496, 446}, {437, 437, 492, 426}, {443, 443, 493, 444}, {454, 454, 529, 434}, {479, 479, 546, 462}, {520, 520, 613, 510}, {554, 554, 667, 531}, {586, 586, 701, 565}, {629, 629, 743, 599}, {697, 697, 847, 647}},
{{741, 741, 855, 729}, {651, 651, 764, 625}, {589, 589, 692, 554}, {538, 538, 620, 498}, {483, 483, 557, 445}, {440, 440, 504, 412}, {413, 413, 466, 401}, {399, 399, 445, 391}, {391, 391, 437, 379}, {394, 394, 437, 388}, {404, 404, 457, 398}, {424, 424, 485, 411}, {454, 454, 523, 440}, {500, 500, 578, 477}, {538, 538, 631, 513}, {576, 576, 693, 543}, {635, 635, 793, 570}},
{{694, 694, 815, 663}, {613, 613, 720, 583}, {547, 547, 636, 519}, {484, 484, 555, 459}, {437, 437, 493, 412}, {399, 399, 448, 383}, {376, 376, 420, 357}, {360, 360, 395, 350}, {353, 353, 385, 346}, {357, 357, 384, 350}, {368, 368, 403, 361}, {390, 390, 428, 379}, {416, 416, 464, 398}, {453, 453, 517, 437}, {502, 502, 587, 485}, {545, 545, 648, 518}, {591, 591, 686, 547}},
{{668, 668, 822, 611}, {588, 588, 683, 560}, {512, 512, 582, 483}, {450, 450, 507, 422}, {404, 404, 450, 382}, {369, 369, 410, 357}, {343, 343, 366, 333}, {325, 325, 344, 321}, {318, 318, 338, 317}, {323, 323, 339, 324}, {338, 338, 363, 333}, {360, 360, 391, 352}, {387, 387, 435, 381}, {423, 423, 481, 405}, {471, 471, 533, 457}, {521, 521, 612, 511}, {577, 577, 673, 562}},
{{650, 650, 803, 594}, {565, 565, 660, 530}, {488, 488, 556, 460}, {427, 427, 481, 406}, {382, 382, 417, 368}, {344, 344, 371, 338}, {317, 317, 334, 314}, {298, 298, 312, 298}, {293, 293, 302, 295}, {298, 298, 312, 302}, {313, 313, 327, 313}, {336, 336, 356, 332}, {368, 368, 405, 363}, {400, 400, 454, 394}, {445, 445, 504, 436}, {504, 504, 573, 491}, {560, 560, 663, 554}},
{{641, 641, 768, 602}, {551, 551, 629, 530}, {472, 472, 534, 451}, {413, 413, 462, 391}, {370, 370, 401, 359}, {330, 330, 353, 328}, {300, 300, 311, 298}, {283, 283, 287, 284}, {276, 276, 283, 281}, {282, 282, 293, 285}, {297, 297, 308, 304}, {322, 322, 341, 322}, {353, 353, 387, 351}, {391, 391, 439, 385}, {433, 433, 488, 425}, {488, 488, 562, 479}, {554, 554, 666, 546}},
{{645, 645, 743, 635}, {549, 549, 628, 532}, {468, 468, 527, 454}, {410, 410, 448, 399}, {363, 363, 394, 357}, {326, 326, 342, 324}, {297, 297, 304, 298}, {279, 279, 283, 280}, {271, 271, 276, 271}, {277, 277, 285, 280}, {293, 293, 306, 296}, {320, 320, 340, 318}, {351, 351, 382, 346}, {389, 389, 436, 380}, {430, 430, 489, 426}, {487, 487, 571, 479}, {557, 557, 656, 541}},
{{645, 645, 764, 626}, {555, 555, 642, 536}, {477, 477, 539, 460}, {416, 416, 457, 403}, {372, 372, 404, 361}, {334, 334, 353, 329}, {303, 303, 315, 305}, {286, 286, 289, 286}, {280, 280, 282, 282}, {285, 285, 290, 290}, {300, 300, 314, 301}, {326, 326, 350, 321}, {358, 358, 392, 346}, {393, 393, 444, 385}, {436, 436, 495, 431}, {491, 491, 567, 489}, {561, 561, 666, 548}},
{{658, 658, 801, 656}, {577, 577, 688, 564}, {498, 498, 569, 476}, {433, 433, 487, 416}, {390, 390, 427, 375}, {353, 353, 377, 342}, {322, 322, 343, 318}, {307, 307, 315, 305}, {299, 299, 307, 300}, {303, 303, 316, 303}, {320, 320, 336, 315}, {343, 343, 369, 333}, {375, 375, 415, 366}, {408, 408, 463, 401}, {451, 451, 517, 438}, {508, 508, 588, 495}, {563, 563, 668, 531}},
{{672, 672, 827, 648}, {600, 600, 726, 569}, {527, 527, 621, 503}, {463, 463, 524, 442}, {414, 414, 467, 399}, {381, 381, 418, 364}, {354, 354, 383, 340}, {334, 334, 357, 328}, {328, 328, 345, 323}, {332, 332, 355, 324}, {346, 346, 375, 337}, {370, 370, 410, 356}, {398, 398, 447, 385}, {432, 432, 497, 416}, {482, 482, 556, 469}, {530, 530, 623, 511}, {574, 574, 687, 557}},
{{710, 710, 867, 680}, {628, 628, 744, 604}, {565, 565, 663, 550}, {507, 507, 578, 485}, {453, 453, 512, 430}, {415, 415, 469, 392}, {390, 390, 435, 368}, {374, 374, 412, 353}, {367, 367, 401, 350}, {370, 370, 410, 355}, {383, 383, 426, 366}, {403, 403, 454, 383}, {429, 429, 490, 408}, {466, 466, 532, 449}, {512, 512, 589, 501}, {557, 557, 645, 543}, {598, 598, 697, 580}},
{{762, 762, 884, 721}, {674, 674, 795, 645}, {613, 613, 707, 595}, {562, 562, 638, 548}, {507, 507, 584, 481}, {463, 463, 534, 432}, {435, 435, 499, 406}, {418, 418, 474, 390}, {412, 412, 466, 390}, {413, 413, 473, 392}, {425, 425, 482, 404}, {444, 444, 509, 425}, {473, 473, 545, 451}, {514, 514, 592, 496}, {554, 554, 643, 540}, {590, 590, 688, 578}, {648, 648, 771, 657}},
{{834, 834, 990, 807}, {729, 729, 891, 712}, {653, 653, 792, 647}, {614, 614, 734, 607}, {570, 570, 663, 544}, {525, 525, 615, 490}, {485, 485, 572, 446}, {467, 467, 534, 433}, {463, 463, 537, 447}, {467, 467, 533, 435}, {478, 478, 544, 462}, {501, 501, 585, 480}, {534, 534, 627, 503}, {564, 564, 657, 546}, {593, 593, 698, 564}, {639, 639, 739, 616}, {718, 718, 844, 690}}};

*/
  /*int16_t matrix[3][4] = {{-549, 103,   -7, 0},
                          {96,  -373,  -62,  0},
                          {31, 261, -883,  0}};*/
  
  class CameraPipeline : public Halide::Generator<CameraPipeline> {
    
  public:
    Input<Buffer<uint16_t>>  input{"input", 2};
    Input<Buffer<uint16_t>>  lens_shading_factors{"lens_shading_factors", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    GeneratorParam<float> gamma{"gamma", /*default=*/2.0};
    GeneratorParam<float> contrast{"contrast", /*default=*/50.0};
    GeneratorParam<uint8_t> schedule{"schedule", 3};    // default: 3
    GeneratorParam<uint8_t> width{"width", 0};          // default: 0
    GeneratorParam<uint8_t> myunroll{"myunroll", 1};    // default: 1
    GeneratorParam<float> blackLevel{"blackLevel", 25};    // default: 25
    GeneratorParam<float> whiteLevel{"whiteLevel", 1023};    // default: 1023

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


    // Upsample using bilinear interpolation
    Func upsample(Func f) {
        using Halide::_;
        Func upx, upy;
        upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
        upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);
        return upy;
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
      demosaicked(x, y, c) = cast<int16_t>(select(c == 0, r(x, y),
                                    c == 1, g(x, y),
                                    b(x, y)));
      //demosaicked.bound(c, 0, 3);
      return demosaicked;
    }


    // Multiply with maximum precision and make result 8.8 p
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
      corrected(x, y,  c) = select(c == 0, r,
                                  c == 1, g,
                                  b);
      return corrected;
    }

    Func lens_shading_correction(Func input) {

        Func lens_shading_corrected;
        Func clamped = Halide::BoundaryConditions::repeat_edge(lens_shading_factors);
        Expr pixel = input(x, y);
        lens_shading_corrected(x, y) = mul1(input(x, y), clamped(x, y));
        return lens_shading_corrected;
    }


    // Applies a non-linear camera curve.
    Func apply_curve(Func input, Func curve) {
      // copied from FCam

      Func curved("curved");

      // This clamping is here to handle negative output from demosaicking int datatype
      // Anything <0 is converted to 0. 
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


      //Func lens_shading_corrected;
      //lens_shading_corrected = lens_shading_correction(denoised);

      // Give more convenient names to the four channels we know
      Func r_r, g_gr, g_gb, b_b;
      // g_gr(x, y) = lens_shading_corrected(2*x, 2*y);//deinterleaved(x, y, 0);
      // r_r(x, y)  = lens_shading_corrected(2*x+1, 2*y);//deinterleaved(x, y, 1);
      // b_b(x, y)  = lens_shading_corrected(2*x, 2*y+1);//deinterleaved(x, y, 2);
      // g_gb(x, y) = lens_shading_corrected(2*x+1, 2*y+1);//deinterleaved(x, y, 3);

      g_gr(x, y) = denoised(2*x, 2*y);//deinterleaved(x, y, 0);
      r_r(x, y)  = denoised(2*x+1, 2*y);//deinterleaved(x, y, 1);
      b_b(x, y)  = denoised(2*x, 2*y+1);//deinterleaved(x, y, 2);
      g_gb(x, y) = denoised(2*x+1, 2*y+1);//deinterleaved(x, y, 3);

     
     
      // BEGIN WHITE BALANCING
      // Compute the maximum pixel value on each color channel
      // Func max_g_gr;
      // Func max_r_r;
      // Func max_b_b;
      // Func max_g_gb;
      
      // max_g_gr = compute_channel_maximum(g_gr);
      // max_r_r = compute_channel_maximum(r_r);
      // max_b_b = compute_channel_maximum(b_b);
      // max_g_gb = compute_channel_maximum(g_gb);


      // Compute white balanced pixel values by dividing by maximum, per-channel 
      // Func g_gr_wb, r_r_wb, b_b_wb, g_gb_wb;
      // g_gr_wb(x, y) = g_gr(x, y) * g_gr(x, y)/max_g_gr();
      // r_r_wb(x, y) = r_r(x, y) * r_r(x, y)/max_r_r();
      // b_b_wb(x, y) = b_b(x, y) * b_b(x, y)/max_b_b();
      // g_gb_wb(x, y) = g_gb(x, y) * g_gb(x, y)/max_g_gb();

      // g_gr_wb(x, y) = g_gr(x, y) * g_gr(x, y)/255;
      // r_r_wb(x, y) = r_r(x, y) * r_r(x, y)/255;
      // b_b_wb(x, y) = b_b(x, y) * b_b(x, y)/255;
      // g_gb_wb(x, y) = g_gb(x, y) * g_gb(x, y)/255;
      // END WHITE BALANCING 
      
      

      // Perform demosaicking on the white balanced pixels
      Func demosaicked;
      Func b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b;
      demosaicked = demosaic(g_gr, r_r, b_b, g_gb,
                             b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b);
      //demosaicked = demosaic(g_gr_wb, r_r_wb, b_b_wb, g_gb_wb,
      //                       b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b);

      Func color_corrected;
      color_corrected = color_correct(demosaicked, matrix);

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
        curve(x) = select(x <= minRaw, 0, select(x > maxRaw, 255, u16(clamp(val*256.0f, 0.0f, 255.0f))));
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

          //max_g_gr.compute_at(output, yi);
          //max_r_r.compute_at(output, yi);
          //max_b_b.compute_at(output, yi);
          //max_g_gb.compute_at(output, yi);
        
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
