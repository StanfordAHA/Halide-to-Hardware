/*
 * An application for detecting corners in images. It compares
 * pixels in a ring around a centerpoint. A corner is determined
 * if a sufficiently long arc is found. Based on the FAST corner
 * detector.
 */

#include "Halide.h"

namespace {

using namespace Halide;

  // Values compared to the middle pixel is determined dark/light if
  // the difference is greater than the threshold.
  int8_t threshold = 10;

  // A corner is found if the number of contiguous pixel values exceeds the
  // minimum length for a segment.
  int8_t min_seg_len = 9;
  
class FastCornerDetector : public Halide::Generator<FastCornerDetector> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */
        Var x("x"), y("y"), l("l");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        Func hw_input, in_copy;
        //in_copy(x, y) = cast<uint16_t>(input(x+3, y+3));
        //hw_input(x, y) = in_copy(x, y);
        hw_input(x, y) = cast<uint16_t>(input(x+3, y+3));
                                      
        // Map stencil indices to contiguous segment.
        Func segment;
        segment(x,y,l) = select(l==0,  hw_input(x  , y-3),
                                l==1,  hw_input(x+1, y-3),
                                l==2,  hw_input(x+2, y-2),
                                l==3,  hw_input(x+3, y-1),
                                l==4,  hw_input(x+3, y),
                                l==5,  hw_input(x+3, y+1),
                                l==6,  hw_input(x+2, y+2),
                                l==7,  hw_input(x+1, y+3),
                                l==8,  hw_input(x,   y+3),
                                l==9,  hw_input(x-1, y+3),
                                select(l==10, hw_input(x-2, y+2),
                                       l==11, hw_input(x-3, y+1),
                                       l==12, hw_input(x-3, y),
                                       l==13, hw_input(x-3, y-1),
                                       l==14, hw_input(x-2, y-2),
                                       l==15, hw_input(x-1, y-3),
                                       l==16, hw_input(x,   y-3),
                                       l==17, hw_input(x+1, y-3),
                                       l==18, hw_input(x+2, y-2),
                                       l==19, hw_input(x+3, y-1),
                                       select(l==20, hw_input(x+3, y),
                                              l==21, hw_input(x+3, y+1),
                                              l==22, hw_input(x+2, y+2),
                                              l==23, hw_input(x+1, y+3),
                                              l==24, hw_input(x,   y+3),
                                              l==25, hw_input(x-1, y+3),
                                              l==26, hw_input(x-2, y+2),
                                              l==27, hw_input(x-3, y+1),
                                              l==28, hw_input(x-3, y),
                                              l==29, hw_input(x-3, y-1),
                                              select(l==30, hw_input(x-2, y-2),
                                                     l==31, hw_input(x-1, y-3),
                                                     l==32, hw_input(x,   y-3), 0))));

        // compare center pixel to surrounding segment
        Func lighter, darker;
        lighter(x,y,l)  = cast<uint16_t>(segment(x,y,l) > (hw_input(x,y) + threshold));
        darker(x,y,l)   = cast<uint16_t>(segment(x,y,l) < (hw_input(x,y) - threshold));

        // Check all min_length windows in light and dark segments.
        Func contiguous_lighter, contiguous_darker;
        for (int i=0; i<min_seg_len; i++) {
          if (i==0) {
            contiguous_lighter(x,y,l) = lighter(x,y,l);
            contiguous_darker(x,y,l)  = darker( x,y,l);
          } else {
            contiguous_lighter(x,y,l) = lighter(x,y,l+i) & contiguous_lighter(x,y,l);
            contiguous_darker(x,y,l)  = darker( x,y,l+i) & contiguous_darker(x,y,l);
          }
        }



        // // calculate the largest contiguous segment (how many overlapping of min length).
        Func corners_light, corners_dark;
        for (int i=0; i<15; i++) {
          if (i==0) {
            corners_light(x,y) = lighter(x,y,i);
            corners_dark(x,y)  = darker(x,y,i);
          } else {
            corners_light(x,y) = lighter(x,y,i) & corners_light(x,y);
            corners_dark(x,y)  = darker(x,y,i) & corners_dark(x,y);
          }
        }

        Func corners_combined;
        corners_combined(x,y) = corners_light(x,y) | corners_dark(x,y);

        Func hw_output;
        hw_output(x,y) = cast<uint8_t>(corners_combined(x,y))*255;
        output(x, y) = hw_output(x, y);
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, 64-6);
          output.bound(y, 0, 64-6);

          hw_output.compute_root();
          hw_output
            .tile(x, y, xo, yo, xi, yi, 64-6,64-6)
            .hw_accelerate(xi, xo);
          
          corners_combined.compute_at(hw_output,xo);
          corners_light.compute_at(hw_output,xo);
          corners_dark.compute_at(hw_output,xo);

          contiguous_lighter.compute_at(hw_output,xo);
          contiguous_darker.compute_at(hw_output,xo);

          lighter.compute_at(hw_output,xo);
          darker.compute_at(hw_output,xo);
          
          hw_input.stream_to_accelerator();

        } else {    // schedule to CPU
          output.tile(x, y, xo, yo, xi, yi, 64-6, 64-6)
            .compute_root();

          output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(FastCornerDetector, fast_corner)

