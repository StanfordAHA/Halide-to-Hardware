#include "Halide.h"

namespace {

constexpr int maxJ = 6;

using namespace Halide::ConciseCasts;

class Alignment : public Halide::Generator<Alignment> {
public:
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 4, 2, maxJ}; 
    Input<Buffer<uint8_t>>  input{"input", 4};
    Output<Buffer<int16_t>> output{"output", 4};

    void generate() {
        // Algorithm
        const int J = pyramid_levels;

        Func clamped = Halide::BoundaryConditions::repeat_edge(input);
        Func gray;
        gray(x, y, n) = u8((77 * u16(clamped(x, y, 0, n)) + 150 * u16(clamped(x, y, 1, n)) + 29 * u16(clamped(x, y, 2, n))) >> 8 );

        Func gPyramid[maxJ];
        gPyramid[0](x, y, n) = gray(x, y, n);
        gPyramid[1](x, y, n) = downsample_r2(gPyramid[0])(x, y, n);
        for (int j = 2; j < J; j++) {
            gPyramid[j](x, y, n) = downsample_r4(gPyramid[j-1])(x, y, n);
        }

        Func initialAlign;
        initialAlign(tx, ty, xy, n) = 0;
        Func alignPyramid[maxJ];
        Expr min_align[maxJ];
        Expr max_align[maxJ];
        min_align[J-1] = i16(Expr(0));
        max_align[J-1] = i16(Expr(0));
        alignPyramid[J-1](tx, ty, xy, n) = 
            align_layer_t8_s4_l1(gPyramid[J-1], initialAlign, min_align[J-1], max_align[J-1])(tx, ty, xy, n);
        for (int j = J-2; j >= 1; j--) {
            min_align[j] = 4 * min_align[j+1] - 4;
            max_align[j] = 4 * max_align[j+1] + 4;
            alignPyramid[j](tx, ty, xy, n) = 
                align_layer_t16_s4_l1(gPyramid[j], alignPyramid[j+1], min_align[j+1], max_align[j+1])(tx, ty, xy, n);
        }
            alignPyramid[0](tx, ty, xy, n) = 
                align_layer_t16_s1_l1(gPyramid[0], alignPyramid[1], 4 * min_align[1] - 4, 4 * max_align[1]+ 4)(tx, ty, xy, n);
        
        
        output(tx, ty, xy, n) = alignPyramid[0](tx, ty, xy, n);

        // Schedule
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");
        output.reorder(xy, n, tx, ty).tile(tx, ty, xo, yo, xi, yi, 8, 8).fuse(xo, yo, outer).parallel(outer);
        gray.compute_root().parallel(y, 32).vectorize(x, 8);
        
        for (int j = 1; j < J; j++) {
            gPyramid[j]
                .compute_root().parallel(y, 8).vectorize(x, 8);
            alignPyramid[j]
                .store_at(output, outer).compute_at(output, yi);
        }
        alignPyramid[0].compute_at(output, yi);
    }
private:
    Var x, y, tx, ty, xy, n;

    Func downsample_r4(Func f_in) {
        using Halide::_;
        Func f, downx, downy;
        f(x, y, _) = u16(f_in(x, y, _));
        downx(x, y, _) = 2 * f(4*x-2, y, _) + 4 * f(4*x-1, y, _) + 5 * f(4*x, y, _) + 4 * f(4*x+1, y, _) + 2 * f(4*x+2, y, _);
        downy(x, y, _) = u8((2 * downx(x, 4*y-2, _) + 4 * downx(x, 4*y-1, _) + 5 * downx(x, 4*y, _) + 4 * downx(x, 4*y+1, _) + 2 * downx(x, 4*y+2, _)) >> 8);
        return downy;
    }

    Func downsample_r2(Func f_in) {
        using Halide::_;
        Func f, downx, downy;
        f(x, y, _) = u16(f_in(x, y, _));
        downx(x, y, _) = 2 * f(2*x-2, y, _) + 4 * f(2*x-1, y, _) + 5 * f(2*x, y, _) + 4 * f(2*x+1, y, _) + 2 * f(2*x+2, y, _);
        downy(x, y, _) = u8((2 * downx(x, 2*y-2, _) + 4 * downx(x, 2*y-1, _) + 5 * downx(x, 2*y, _) + 4 * downx(x, 2*y+1, _) + 2 * downx(x, 2*y+2, _)) >> 8);
        return downy;
    }

    Func align_layer_t16_s4_l1(Func gauss, Func coarse_alignment, Expr prev_min, Expr prev_max) {
        /* TODO
         * Alignment parameters (e.g. L1/L2 distance, tile size, search region size)
         * differs by level.
         * For now, use L1 distance, 16 x 16 tile size, +-4 search region as default.
         */

        // Algorithm
        Var x_s, y_s;
        Func coarse_offset;
        Func scores;
        Func alignment;

        RDom r_tile(-7, 16, -7, 16);          //tile size
        RDom r_search(-4, 9, -4, 9);        //search region
      
        coarse_offset(tx, ty, xy, n) = 4 * clamp(coarse_alignment(tx/4, ty/4, xy, n), prev_min, prev_max);
        
        Expr x_ref = tx * 16 + r_tile.x;
        Expr y_ref = ty * 16 + r_tile.y;
        Expr x_cmp = x_ref + coarse_offset(tx, ty, 0, n) + x_s;
        Expr y_cmp = y_ref + coarse_offset(tx, ty, 1, n) + y_s;

        Expr dist = u16(abs(i16(gauss(x_ref, y_ref, 0)) - i16(gauss(x_cmp, y_cmp, n))));
        scores(tx, ty, x_s, y_s, n) = sum(dist);
        Tuple min_coor = argmin(scores(tx, ty, r_search.x, r_search.y, n));
        alignment(tx, ty, xy, n) = select(n == 0, i16(0),
                    xy == 0, i16(min_coor[0] + coarse_offset(tx, ty, 0, n)),
                    i16(min_coor[1] + coarse_offset(tx, ty, 1, n))); 

        return alignment;
    }

    Func align_layer_t8_s4_l1(Func gauss, Func coarse_alignment, Expr prev_min, Expr prev_max) {
        /* TODO
         * Alignment parameters (e.g. L1/L2 distance, tile size, search region size)
         * differs by level.
         * For now, use L1 distance, 16 x 16 tile size, +-4 search region as default.
         */

        // Algorithm
        Var x_s, y_s;
        Func coarse_offset;
        Func scores;
        Func alignment;

        RDom r_tile(-3, 8, -3, 8);          //tile size
        RDom r_search(-4, 9, -4, 9);        //search region
      
        coarse_offset(tx, ty, xy, n) = 4 * clamp(coarse_alignment(tx/4, ty/4, xy, n), prev_min, prev_max);
        
        Expr x_ref = tx * 8 + r_tile.x;
        Expr y_ref = ty * 8 + r_tile.y;
        Expr x_cmp = x_ref + coarse_offset(tx, ty, 0, n) + x_s;
        Expr y_cmp = y_ref + coarse_offset(tx, ty, 1, n) + y_s;

        Expr dist = u16(abs(i16(gauss(x_ref, y_ref, 0)) - i16(gauss(x_cmp, y_cmp, n))));
        scores(tx, ty, x_s, y_s, n) = sum(dist);
        Tuple min_coor = argmin(scores(tx, ty, r_search.x, r_search.y, n));
        alignment(tx, ty, xy, n) = select(n == 0, i16(0),
                    xy == 0, i16(min_coor[0] + coarse_offset(tx, ty, 0, n)),
                    i16(min_coor[1] + coarse_offset(tx, ty, 1, n))); 

        return alignment;
    }
    Func align_layer_t16_s1_l1(Func gauss, Func coarse_alignment, Expr prev_min, Expr prev_max) {
        /* TODO
         * Alignment parameters (e.g. L1/L2 distance, tile size, search region size)
         * differs by level.
         * For now, use L1 distance, 16 x 16 tile size, +-4 search region as default.
         */

        // Algorithm
        Var x_s, y_s;
        Func coarse_offset;
        Func scores;
        Func alignment;

        RDom r_tile(-7, 16, -7, 16);          //tile size
        RDom r_search(-1, 3, -1, 3);        //search region
      
        coarse_offset(tx, ty, xy, n) = 2 * clamp(coarse_alignment(tx/2, ty/2, xy, n), prev_min, prev_max);
        
        Expr x_ref = tx * 16 + r_tile.x;
        Expr y_ref = ty * 16 + r_tile.y;
        Expr x_cmp = x_ref + coarse_offset(tx, ty, 0, n) + x_s;
        Expr y_cmp = y_ref + coarse_offset(tx, ty, 1, n) + y_s;

        Expr dist = u16(abs(i16(gauss(x_ref, y_ref, 0)) - i16(gauss(x_cmp, y_cmp, n))));
        scores(tx, ty, x_s, y_s, n) = sum(dist);
        Tuple min_coor = argmin(scores(tx, ty, r_search.x, r_search.y, n));
        alignment(tx, ty, xy, n) = select(n == 0, i16(0),
                    xy == 0, i16(min_coor[0] + coarse_offset(tx, ty, 0, n)),
                    i16(min_coor[1] + coarse_offset(tx, ty, 1, n))); 

        return alignment;
    }
};
}
HALIDE_REGISTER_GENERATOR(Alignment, align)
