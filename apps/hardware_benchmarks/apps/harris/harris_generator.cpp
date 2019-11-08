/*
 * An application for detecting corners in images. It computes
 * gradients in the x and y direction, and then uses Harris's
 * method to calculate cornerness efficiently.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
int blockSize = 3;

// k is a sensitivity parameter for detecting corners.
// k should vary from 0.04 to 0.15 according to literature.
int shiftk = 4; // equiv to k = 0.0625

// Threshold for cornerness measure.
int threshold = 1;

class HarrisCornerDetector : public Halide::Generator<HarrisCornerDetector> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        Func padded16, padded;
        padded(x, y) = input(x+3, y+3);
        padded16(x, y) = cast<int16_t>(padded(x, y));
                                      

        // sobel filter
        Func grad_x_unclamp, grad_y_unclamp, grad_x, grad_y;
        grad_x_unclamp(x, y) = cast<int16_t>(  -padded16(x-1,y-1) +   padded16(x+1,y-1)
                                             -2*padded16(x-1,y)   + 2*padded16(x+1,y)
                                               -padded16(x-1,y+1) +   padded16(x+1,y+1));
        grad_y_unclamp(x, y) = cast<int16_t>(   padded16(x-1,y+1) -   padded16(x-1,y-1) +
                                              2*padded16(x,  y+1) - 2*padded16(x,  y-1) +
                                                padded16(x+1,y+1) -   padded16(x+1,y-1));

        grad_x(x, y) = clamp(grad_x_unclamp(x,y), -255, 255);
        grad_y(x, y) = clamp(grad_y_unclamp(x,y), -255, 255);
        
        // product of gradients
        Func grad_xx, grad_yy, grad_xy;
        grad_xx(x, y) = cast<int32_t>(grad_x(x,y)) * cast<int32_t>(grad_x(x,y));
        grad_yy(x, y) = cast<int32_t>(grad_y(x,y)) * cast<int32_t>(grad_y(x,y));
        grad_xy(x, y) = cast<int32_t>(grad_x(x,y)) * cast<int32_t>(grad_y(x,y));

        // shift gradients
        Func lxx, lyy, lxy;
        lxx(x, y) = grad_xx(x, y) >> 7;
        lyy(x, y) = grad_yy(x, y) >> 7;
        lxy(x, y) = grad_xy(x, y) >> 7;

        // box filter (i.e. windowed sum)
        Func lgxx, lgyy, lgxy;
        RDom box(-blockSize/2, blockSize, -blockSize/2, blockSize);
        lgxx(x, y) += lxx(x+box.x, y+box.y);
        lgyy(x, y) += lyy(x+box.x, y+box.y);
        lgxy(x, y) += lxy(x+box.x, y+box.y);

        Expr lgxx8 = lgxx(x,y) >> 6;
        Expr lgyy8 = lgyy(x,y) >> 6;
        Expr lgxy8 = lgxy(x,y) >> 6;

        // calculate Cim
        //        int scale = (1 << (Ksize-1)) * blockSize;
        //        Expr lgx = cast<float>(grad_gx(x, y) / scale / scale);
        //        Expr lgy = cast<float>(grad_gy(x, y) / scale / scale);
        //        Expr lgxy = cast<float>(grad_gxy(x, y) / scale / scale);

        // scale==12, so dividing by 144
        // approx~ 1>>7==divide by 128
        Func cim;
        Expr det = lgxx8*lgyy8 - lgxy8*lgxy8;
        Expr trace = lgxx8 + lgyy8;
        cim(x, y) = det - (trace*trace >> shiftk);

        // Perform non-maximal suppression
        Func hw_output;
        Expr is_max = cim(x, y) > cim(x-1, y-1) && cim(x, y) > cim(x, y-1) &&
            cim(x, y) > cim(x+1, y-1) && cim(x, y) > cim(x-1, y) &&
            cim(x, y) > cim(x+1, y) && cim(x, y) > cim(x-1, y+1) &&
            cim(x, y) > cim(x, y+1) && cim(x, y) > cim(x+1, y+1);
        hw_output(x, y) = cast<uint8_t>(select( is_max && (cim(x, y) >= threshold),
                                                255,
                                                0));
        output(x, y) = hw_output(x, y);
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR) || get_target().has_feature(Target::HLS)) {

          //output.tile(x, y, xo, yo, xi, yi, 64, 64);
          //padded16.compute_at(output, xo);
          padded16.compute_root();
          //hw_output.compute_at(output, xo);
          hw_output.compute_root();

          //int tileSize = 8;
          int tileSize = 58;
          hw_output
            .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
            .accelerate({padded16}, xi, xo);
            //.hw_accelerate(xi, xo);
          //padded16.stream_to_accelerator();
        
          grad_x.linebuffer();
          grad_y.linebuffer();
          lxx.linebuffer();
          lyy.linebuffer();
          lxy.linebuffer();
          lgxx.linebuffer();
          lgyy.linebuffer();
          lgxy.linebuffer();
          cim.linebuffer();

          lgxx.update(0).unroll(box.x).unroll(box.y);
          lgyy.update(0).unroll(box.x).unroll(box.y);
          lgxy.update(0).unroll(box.x).unroll(box.y);

          //padded16.stream_to_accelerator();
          
        } else {    // schedule to CPU
          output.tile(x, y, xo, yo, xi, yi, 58, 58);
        
          grad_x.compute_at(output, xo).vectorize(x, 8);
          grad_y.compute_at(output, xo).vectorize(x, 8);
          grad_xx.compute_at(output, xo).vectorize(x, 4);
          grad_yy.compute_at(output, xo).vectorize(x, 4);
          grad_xy.compute_at(output, xo).vectorize(x, 4);

          //grad_xx.compute_with(grad_yy, x);
          //grad_xy.compute_with(grad_yy, x);
          
          lgxx.compute_at(output, xo).vectorize(x, 4);
          lgyy.compute_at(output, xo).vectorize(x, 4);
          lgxy.compute_at(output, xo).vectorize(x, 4);
          cim.compute_at(output, xo).vectorize(x, 4);

          lgxx.update(0).unroll(box.x).unroll(box.y);
          lgyy.update(0).unroll(box.x).unroll(box.y);
          lgxy.update(0).unroll(box.x).unroll(box.y);

          output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HarrisCornerDetector, harris)

