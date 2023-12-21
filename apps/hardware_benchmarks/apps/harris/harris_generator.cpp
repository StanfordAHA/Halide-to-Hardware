/*
 * An application for detecting corners in images. It computes
 * gradients in the x and y direction, and then uses Harris's
 * method to calculate cornerness efficiently.
 */

#include "Halide.h"

namespace {

using namespace Halide;
using namespace Halide::ConciseCasts;

// Size of blur for gradients.
int blockSize = 3;

// k is a sensitivity parameter for detecting corners.
// k should vary from 0.04 to 0.15 according to literature.
int shiftk = 4; // equiv to k = 0.0625

// Threshold for cornerness measure.
int threshold = 1;

int tileSize_x = 58;
int tileSize_y = 58;

class HarrisCornerDetector : public Halide::Generator<HarrisCornerDetector> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<uint8_t> schedule{"schedule", 0};    // default: 0
    //Input<int32_t> tileSize_x{"tileSize_x", 64, 8, 128};    // default: 64. bounded between 8 and 128
    //Input<int32_t> tileSize_y{"tileSize_y", 64, 8, 128};    // default: 64. bounded between 8 and 128

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        Func padded16, padded, hw_input_copy;
        padded(x, y) = input(x+3, y+3);
        padded16(x, y) = cast<int16_t>(padded(x, y));
        //hw_input_copy(x, y) = cast<int16_t>(input(x+3,y+3));
        //padded16(x, y) = hw_input_copy(x, y);

        // sobel filter
        Func grad_x_unclamp, grad_y_unclamp, grad_x, grad_y;
        Func kernel_x, kernel_y;
        RDom r(-1, 3, -1, 3);
        if (schedule == 1 || schedule == 2) {
          grad_x_unclamp(x, y) = cast<int16_t>(  -padded16(x-1,y-1) +   padded16(x+1,y-1)
                                               -2*padded16(x-1,y)   + 2*padded16(x+1,y)
                                                 -padded16(x-1,y+1) +   padded16(x+1,y+1));
          grad_y_unclamp(x, y) = cast<int16_t>(   padded16(x-1,y+1) -   padded16(x-1,y-1) +
                                                2*padded16(x,  y+1) - 2*padded16(x,  y-1) +
                                                  padded16(x+1,y+1) -   padded16(x+1,y-1));
        } else {
          kernel_x(x,y) = i16(0);
          kernel_x(-1,-1) = i16(-1);    kernel_x(-1,0) = i16(-2);     kernel_x(-1, 1) = i16(-1);
          kernel_x(1, -1) = i16(1);     kernel_x(1, 0) = i16(2);      kernel_x( 1, 1) = i16(1);
          kernel_y(x,y) = i16(0);
          kernel_y(-1, 1) = i16( 1);    kernel_y( 0, 1) = i16( 2);    kernel_y( 1, 1) = i16( 1);
          kernel_y(-1,-1) = i16(-1);    kernel_y( 0,-1) = i16(-2);    kernel_y( 1,-1) = i16(-1);

          grad_x_unclamp(x, y) = i16(0);
          grad_x_unclamp(x, y) += padded16(x+r.x, y+r.y) * i16(kernel_x(r.x, r.y));
          grad_y_unclamp(x, y) = i16(0);
          grad_y_unclamp(x, y) += padded16(x+r.x, y+r.y) * i16(kernel_y(r.x, r.y));
        }

        grad_x(x, y) = clamp(grad_x_unclamp(x,y), -180, 180); // 180^2 cannot overflow 15 bits
        grad_y(x, y) = clamp(grad_y_unclamp(x,y), -180, 180);

        // product of gradients
        Func grad_xx, grad_yy, grad_xy;
        grad_xx(x, y) = cast<int16_t>(grad_x(x,y)) * cast<int16_t>(grad_x(x,y));
        grad_yy(x, y) = cast<int16_t>(grad_y(x,y)) * cast<int16_t>(grad_y(x,y));
        grad_xy(x, y) = cast<int16_t>(grad_x(x,y)) * cast<int16_t>(grad_y(x,y));

        // shift gradients
        Func lxx, lyy, lxy;
        //lxx(x, y) = grad_xx(x, y) >> 7;
        //lyy(x, y) = grad_yy(x, y) >> 7;
        //lxy(x, y) = grad_xy(x, y) >> 7;
        lxx(x, y) = cast<int16_t>(grad_x(x,y)) * cast<int16_t>(grad_x(x,y)) >> 6;
        lyy(x, y) = cast<int16_t>(grad_y(x,y)) * cast<int16_t>(grad_y(x,y)) >> 6;
        lxy(x, y) = cast<int16_t>(grad_x(x,y)) * cast<int16_t>(grad_y(x,y)) >> 6;


        // box filter (i.e. windowed sum)
        Func lgxx, lgyy, lgxy;
        //RDom box(0, blockSize, 0, blockSize);
        RDom box(-blockSize/2, blockSize, -blockSize/2, blockSize);

        if (schedule == 1 || schedule == 2) {
          lgxx(x, y) =
            lxx(x-1, y-1) + lxx(x, y-1) + lxx(x+1, y-1) +
            lxx(x-1, y) + lxx(x, y) + lxx(x+1, y) +
            lxx(x-1, y+1) + lxx(x, y+1) + lxx(x+1, y+1);
          lgyy(x, y) =
            lyy(x-1, y-1) + lyy(x, y-1) + lyy(x+1, y-1) +
            lyy(x-1, y) + lyy(x, y) + lyy(x+1, y) +
            lyy(x-1, y+1) + lyy(x, y+1) + lyy(x+1, y+1);
          lgxy(x, y) =
            lxy(x-1, y-1) + lxy(x, y-1) + lxy(x+1, y-1) +
            lxy(x-1, y) + lxy(x, y) + lxy(x+1, y) +
            lxy(x-1, y+1) + lxy(x, y+1) + lxy(x+1, y+1);
        } else {
          lgxx(x, y) += lxx(x+box.x, y+box.y);
          lgyy(x, y) += lyy(x+box.x, y+box.y);
          lgxy(x, y) += lxy(x+box.x, y+box.y);
        }

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
        //cim(x, y) = det - (trace*trace >> shiftk);
        cim(x, y) = det - ((lgxx8+lgyy8)*(lgxx8+lgyy8) >> shiftk);

        // Perform non-maximal suppression
        Func hw_output;
        Expr is_max =
          cim(x, y) > cim(x-1, y-1) && cim(x, y) > cim(x, y-1) &&
          cim(x, y) > cim(x+1, y-1) && cim(x, y) > cim(x-1, y) &&
          cim(x, y) > cim(x+1, y) && cim(x, y) > cim(x-1, y+1) &&
          cim(x, y) > cim(x, y+1) && cim(x, y) > cim(x+1, y+1);
        Func cim_output;
        cim_output(x,y) = cast<int16_t>(select( is_max && (cim(x, y) >= threshold), 255, 0));
        hw_output(x, y) = cim_output(x,y);
        //hw_output(x, y) = cast<uint8_t>(cim(x,y));
        //hw_output(x, y) = cast<uint8_t>(lgxx(x,y));

        output(x, y) = cast<uint8_t>(hw_output(x, y));


        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          int tileSize = 58;

          hw_output.compute_root();
          //kernel.compute_at(hw_output, xo);

          if (schedule == 1) { // no buffers
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            padded16.stream_to_accelerator();

          } else if (schedule == 2) { // few buffers
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);

            padded16.stream_to_accelerator();

          } else if (schedule == 3) { // all buffers, 1 pixel per 9 cycles
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            grad_x.compute_at(hw_output, xo);
            grad_y.compute_at(hw_output, xo);
            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);
            cim_output.compute_at(hw_output, xo);

            //kernel_x.compute_at(hw_output, xo);
            //kernel_y.compute_at(hw_output, xo);
            kernel_x.compute_at(hw_output, xo).store_in(MemoryType::ROM);
            kernel_y.compute_at(hw_output, xo).store_in(MemoryType::ROM);
            kernel_x.unroll(x, 3).unroll(y, 3);
            kernel_y.unroll(x, 3).unroll(y, 3);

            lgxx.unroll(x, 3).unroll(y, 3);
            lgxy.unroll(x, 3).unroll(y, 3);
            lgyy.unroll(x, 3).unroll(y, 3);

            padded16.stream_to_accelerator();

          } else if (schedule == 4) { // unroll some
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            grad_x.compute_at(hw_output, xo);
            grad_y.compute_at(hw_output, xo);
            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);
            cim_output.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo).store_in(MemoryType::ROM);
            kernel_y.compute_at(hw_output, xo).store_in(MemoryType::ROM);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.update().unroll(r.x);
            grad_y_unclamp.update().unroll(r.x);

            lgxx.unroll(x, 3).unroll(y, 3);
            lgxy.unroll(x, 3).unroll(y, 3);
            lgyy.unroll(x, 3).unroll(y, 3);
            lgxx.update().unroll(box.x);
            lgyy.update().unroll(box.x);
            lgxy.update().unroll(box.x);

            padded16.stream_to_accelerator();

          } else if (schedule == 5) { // unroll all
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            grad_x.compute_at(hw_output, xo);
            grad_y.compute_at(hw_output, xo);
            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);
            cim_output.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.update().unroll(r.x).unroll(r.y);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y);

            lgxx.update().unroll(box.x).unroll(box.y);
            lgyy.update().unroll(box.x).unroll(box.y);
            lgxy.update().unroll(box.x).unroll(box.y);

            padded16.stream_to_accelerator();

          } else if (schedule == 6) { // unroll all, unroll 2x
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            hw_output.unroll(xi, 2);
            padded16.in().unroll(x, 2);
            grad_x_unclamp.unroll(x, 2);
            grad_y_unclamp.unroll(x, 2);
            lxx.unroll(x, 2);
            lyy.unroll(x, 2);
            lxy.unroll(x, 2);
            lgxx.unroll(x, 2);
            lgyy.unroll(x, 2);
            lgxy.unroll(x, 2);
            cim.unroll(x, 2);
            cim_output.unroll(x, 2);

            grad_x_unclamp.compute_at(hw_output, xo);
            grad_y_unclamp.compute_at(hw_output, xo);
            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);
            cim_output.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.update().unroll(r.x).unroll(r.y).unroll(x, 2);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y).unroll(x, 2);

            lgxx.update().unroll(box.x).unroll(box.y).unroll(x, 2);
            lgyy.update().unroll(box.x).unroll(box.y).unroll(x, 2);
            lgxy.update().unroll(box.x).unroll(box.y).unroll(x, 2);

            padded16.stream_to_accelerator();

          } else if (schedule == 7) { // increase tile size

            auto largeTileSize = (tileSize+6)*2 - 6;//122;
            //cim.compute_root();
            hw_output
              .tile(x, y, xo, yo, xi, yi, largeTileSize, largeTileSize)
              .hw_accelerate(xi, xo);

            grad_x.compute_at(hw_output, xo);
            grad_y.compute_at(hw_output, xo);
            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);
            cim_output.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.update().unroll(r.x).unroll(r.y);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y);

            lgxx.update().unroll(box.x).unroll(box.y);
            lgyy.update().unroll(box.x).unroll(box.y);
            lgxy.update().unroll(box.x).unroll(box.y);

            padded16.stream_to_accelerator();

          } else if (schedule == 8) { // end at cim
            cim.compute_root();
            cim
              .tile(x, y, xo, yo, xi, yi, tileSize+2, tileSize+2)
              .hw_accelerate(xi, xo);

            grad_x.compute_at(cim, xo);
            grad_y.compute_at(cim, xo);
            lxx.compute_at(cim, xo);
            lyy.compute_at(cim, xo);
            lxy.compute_at(cim, xo);
            lgxx.compute_at(cim, xo);
            lgyy.compute_at(cim, xo);
            lgxy.compute_at(cim, xo);

            kernel_x.compute_at(cim, xo);
            kernel_y.compute_at(cim, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.update().unroll(r.x).unroll(r.y);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y);

            lgxx.update().unroll(box.x).unroll(box.y);
            lgyy.update().unroll(box.x).unroll(box.y);
            lgxy.update().unroll(box.x).unroll(box.y);

            padded16.stream_to_accelerator();

          } else { // default, basically sch5 with all buffers, 1 pixel/cycle
            hw_output
              //.tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .tile(x, y, xo, yo, xi, yi, tileSize_x, tileSize_y)
              .hw_accelerate(xi, xo);
            //padded16.stream_to_accelerator();

            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);
            cim_output.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            //grad_x.compute_at(hw_output, xo);
            //grad_y.compute_at(hw_output, xo);
            grad_x_unclamp.compute_at(hw_output, xo);
            grad_y_unclamp.compute_at(hw_output, xo);
            grad_x_unclamp.update().unroll(r.x).unroll(r.y);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y);

            lgxx.update().unroll(box.x).unroll(box.y);
            lgyy.update().unroll(box.x).unroll(box.y);
            lgxy.update().unroll(box.x).unroll(box.y);

            //padded16.compute_at(hw_output, xo);
            padded16.stream_to_accelerator();
            //hw_input_copy.compute_root();
          }

          output.bound(x, 0, tileSize_x);
          output.bound(y, 0, tileSize_y);

       } else if (get_target().has_feature(Target::CoreIR) || get_target().has_feature(Target::HLS)) {

          grad_x.bound(x, -2, 62);
          grad_x.bound(y, -2, 62);
          grad_y.bound(x, -2, 62);
          grad_y.bound(y, -2, 62);

          //grad_xx.bound(x, -2, 62);
          //grad_xx.bound(y, -2, 62);
          //grad_xy.bound(x, -2, 62);
          //grad_xy.bound(y, -2, 62);
          //grad_yy.bound(x, -2, 62);
          //grad_yy.bound(y, -2, 62);

          lxx.bound(x, -2, 62);
          lxx.bound(y, -2, 62);
          lxy.bound(x, -2, 62);
          lxy.bound(y, -2, 62);
          lyy.bound(x, -2, 62);
          lyy.bound(y, -2, 62);

          lgxx.bound(x, -1, 60);
          lgxx.bound(y, -1, 60);
          lgxy.bound(x, -1, 60);
          lgxy.bound(y, -1, 60);
          lgyy.bound(x, -1, 60);
          lgyy.bound(y, -1, 60);

          hw_output.bound(x, 0, 58);
          hw_output.bound(y, 0, 58);
          output.bound(x, 0, 58);
          output.bound(y, 0, 58);
          cim_output.bound(x, 0, 58);
          cim_output.bound(y, 0, 58);

          //output.tile(x, y, xo, yo, xi, yi, 64, 64);
          //padded16.compute_root();
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
          //cim.linebuffer();
          cim_output.linebuffer();

          lgxx.update().unroll(box.x).unroll(box.y);
          lgyy.update().unroll(box.x).unroll(box.y);
          lgxy.update().unroll(box.x).unroll(box.y);

          padded16.store_at(hw_output, xo).compute_at(hw_output, xi);
          padded16.stream_to_accelerator();

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

          output.fuse(xo, yo, xo).parallel(xo).vectorize(xi, 4);
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HarrisCornerDetector, harris)

