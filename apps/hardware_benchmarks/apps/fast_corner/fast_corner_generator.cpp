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

        Func hw_in, in_copy;
        //in_copy(x, y) = cast<uint16_t>(input(x+3, y+3));
        //hw_in(x, y) = in_copy(x, y);
        hw_in(x, y) = cast<uint16_t>(input(x+3, y+3));
                                      
        // Map stencil indices to contiguous segment.
        Func segment;
        segment(x,y,l) = select(l==0,  hw_in(x  , y-3),
                                l==1,  hw_in(x+1, y-3),
                                l==2,  hw_in(x+2, y-2),
                                l==3,  hw_in(x+3, y-1),
                                l==4,  hw_in(x+3, y),
                                l==5,  hw_in(x+3, y+1),
                                l==6,  hw_in(x+2, y+2),
                                l==7,  hw_in(x+1, y+3),
                                l==8,  hw_in(x,   y+3),
                                l==9,  hw_in(x-1, y+3),
                                select(l==10, hw_in(x-2, y+2),
                                       l==11, hw_in(x-3, y+1),
                                       l==12, hw_in(x-3, y),
                                       l==13, hw_in(x-3, y-1),
                                       l==14, hw_in(x-2, y-2),
                                       l==15, hw_in(x-1, y-3),
                                       l==16, hw_in(x,   y-3),
                                       l==17, hw_in(x+1, y-3),
                                       l==18, hw_in(x+2, y-2),
                                       l==19, hw_in(x+3, y-1),
                                       select(l==20, hw_in(x+3, y),
                                              l==21, hw_in(x+3, y+1),
                                              l==22, hw_in(x+2, y+2),
                                              l==23, hw_in(x+1, y+3),
                                              l==24, hw_in(x,   y+3),
                                              l==25, hw_in(x-1, y+3),
                                              l==26, hw_in(x-2, y+2),
                                              l==27, hw_in(x-3, y+1),
                                              l==28, hw_in(x-3, y),
                                              l==29, hw_in(x-3, y-1),
                                              select(l==30, hw_in(x-2, y-2),
                                                     l==31, hw_in(x-1, y-3),
                                                     l==32, hw_in(x,   y-3), 0))));

        // compare center pixel to surrounding segment
        Func lighter, darker;
        lighter(x,y,l)  = segment(x,y,l) > (hw_in(x,y) + threshold);
        darker(x,y,l)   = segment(x,y,l) < (hw_in(x,y) - threshold);

        // Check all min_length windows in light and dark segments.
        Func contiguous_lighter[min_seg_len], contiguous_darker[min_seg_len];
        for (int i=0; i<min_seg_len; ++i) {
          if (i==0) {
            contiguous_lighter[i](x,y,l) = lighter(x,y,l);
            contiguous_darker[i](x,y,l)  = darker( x,y,l);
          } else {
            contiguous_lighter[i](x,y,l) = lighter(x,y,l+i) && contiguous_lighter[i-1](x,y,l+i);
            contiguous_darker[i](x,y,l)  = darker( x,y,l+i) && contiguous_darker[i-1](x,y,l+i);
          }
        }

        // use select to convert a 1bit to 16bit
        Func contiguous_lighter16, contiguous_darker16;
        contiguous_lighter16(x,y,l) = cast<uint16_t>(select(contiguous_lighter[min_seg_len-1](x,y,l), 1, 0));
        contiguous_darker16( x,y,l) = cast<uint16_t>(select(contiguous_darker[min_seg_len-1]( x,y,l), 1, 0));

        // calculate the largest contiguous segment (how many overlapping of min length).
        Func largest_seg_light, largest_seg_dark;
        RDom ring_win(0, 15);
        largest_seg_light(x, y, l) += contiguous_lighter16(x, y, l + ring_win);
        largest_seg_dark(x, y, l) += contiguous_darker16(x, y, l + ring_win);

        // Largest contiguous segment that is either lighter or darker.
        Func largest_seg;
        largest_seg(x,y) = max(largest_seg_dark(x,y,0), largest_seg_light(x,y,0));

        // Output largest segment size if window contains all light or dark pixels.
        // Note that window is equal to the minimum segment length.
        Func hw_output;
        hw_output(x,y) = cast<uint8_t>(13 * select(largest_seg(x,y) > 0,
                                                   largest_seg(x,y) + min_seg_len-1,
                                                   0));
        output(x, y) = hw_output(x, y);
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {
          output.bound(x, 0, 64-6);
          output.bound(y, 0, 64-6);
          //largest_seg.bound(x, 0, 64-6);
          //largest_seg.bound(y, 0, 64-6);
          
          hw_output.tile(x, y, xo, yo, xi, yi, 64-6,64-6);
          hw_output.compute_root();          
          //hw_output.accelerate({hw_in}, xi, xo, {});
          hw_output.hw_accelerate(xi, xo);
          
          lighter.compute_at(hw_output,xi).unroll(l);
          darker.compute_at(hw_output,xi).unroll(l);

          //largest_seg_light.compute_at(hw_output,xi).unroll(l);
          largest_seg_light.update().unroll(ring_win).unroll(l);
          //largest_seg_dark.compute_at(hw_output,xi).unroll(l);
          largest_seg_dark.update().unroll(ring_win).unroll(l);

          contiguous_lighter16.compute_at(hw_output,xo).unroll(l);
          contiguous_darker16.compute_at(hw_output,xo).unroll(l);

          hw_in.store_at(hw_output, xo).compute_at(hw_output, xi);
          hw_in.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork)) {
          output.bound(x, 0, 64-6);
          output.bound(y, 0, 64-6);
          //largest_seg.bound(x, 0, 64-6);
          //largest_seg.bound(y, 0, 64-6);

          hw_output.compute_root();
          hw_output
            .tile(x, y, xo, yo, xi, yi, 64-6,64-6)
            .hw_accelerate(xi, xo);
          
          lighter.compute_at(hw_output,xo).unroll(l);
          darker.compute_at(hw_output,xo).unroll(l);

          contiguous_lighter16.compute_at(hw_output,xo).unroll(l);
          contiguous_darker16.compute_at(hw_output,xo).unroll(l);

          largest_seg_light.compute_at(hw_output,xo).unroll(l);
          largest_seg_light.update().unroll(ring_win).unroll(l);
          largest_seg_dark.compute_at(hw_output,xo).unroll(l);
          largest_seg_dark.update().unroll(ring_win).unroll(l);


          hw_in.stream_to_accelerator();
            //in_copy.compute_root();

        } else {    // schedule to CPU
          output.tile(x, y, xo, yo, xi, yi, 64-6, 64-6)
            .compute_root();

          output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(FastCornerDetector, fast_corner)

