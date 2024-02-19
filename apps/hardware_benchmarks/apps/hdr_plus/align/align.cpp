#include "Halide.h"

namespace {

constexpr int maxJ = 6;

using namespace Halide::ConciseCasts;

class Alignment : public Halide::Generator<Alignment> {
public:
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 4, 1, maxJ}; 
    // Operate on raw bayer image: so 2 channels, plus 1 channel b/c receiving multiple images
    Input<Buffer<uint16_t>>  input{"input", 3};
    Output<Buffer<int16_t>> output{"output", 3};

    void generate() {
        // Algorithm
        const int J = pyramid_levels;
            
        Func clamped = Halide::BoundaryConditions::repeat_edge(input);

        // STEP 1: Convert to grayscale 
        Func gray;
        // gray should average four nearest neighbor pixels from bayer raw in a 2x2 grid 
        //TODO: Double-check this arithmetic. Should we be adding in higher precision? 
        gray(x, y, n) = u16(clamped(x, y, n)) + u16(clamped(x+1, y, n)) + u16(clamped(x, y+1, n)) + u16(clamped(x+1, y+1, n)) >> 16; 


        // STEP 2: Downsample to form image pyramids 
        Func gPyramid[maxJ];
        gPyramid[0](x, y, n) = gray(x, y, n);
        for (int j = 1; j < J; j++) {
            // In the google paper, they claim they downsample the bottom level of the pyramid by a factor of 2 and all higher levels by a factor of 4
            if (j == 1)
                gPyramid[j](x, y, n) = downsample(gPyramid[j-1], 2)(x, y, n);
            else
                gPyramid[j](x, y, n) = downsample(gPyramid[j-1], 4)(x, y, n);
        }

        // STEP 3: Align pyramids and upsample the alignment back to the bottom layer 
        Func initialAlign;
        initialAlign(tx, ty, n) = 0;
        Func alignPyramid[maxJ];
        Expr min_align[maxJ];
        Expr max_align[maxJ];
        min_align[J-1] = i16(Expr(-4));
        max_align[J-1] = i16(Expr(4));

        // In the google paper, they say that for the last pyramid level, they work with tile sizes of 8x8 and search radius of 4. Upsample factor is of course 4. 
        alignPyramid[J-1](tx, ty, n) = 
            align_layer(gPyramid[J-1], initialAlign, 8, 4, 4, min_align[J-1], max_align[J-1])(tx, ty, n);
        for (int j = J-2; j >= 0; j--) {
            if (j == 0) {
                min_align[j] = 2 * min_align[j+1] - 1;
                max_align[j] = 2 * max_align[j+1] + 1;
                alignPyramid[j](tx, ty, n) = 
                    align_layer(gPyramid[j], alignPyramid[j+1], 16, 1, 2, min_align[j], max_align[j])(tx, ty, n);
            } else {
                min_align[j] = 4 * min_align[j+1] - 4;
                max_align[j] = 4 * max_align[j+1] + 4;
                alignPyramid[j](tx, ty, n) = 
                    align_layer(gPyramid[j], alignPyramid[j+1], 16, 4, 4, min_align[j], max_align[j])(tx, ty, n);
            }
        }
        
        // should there be a + input here??? the align_layer function returns the coordinates, NOT the aligned image... 
        // FIXME: ACTUALLY, note that this may be fine because merge.cpp takes this into account 
        //output(tx, ty, n) = alignPyramid[0](tx, ty, n);
        output(tx, ty, n) = input(tx, ty, n) + alignPyramid[0](tx, ty, n);

        // Schedule
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");
        output.reorder(n, tx, ty).tile(tx, ty, xo, yo, xi, yi, 8, 8).fuse(xo, yo, outer).parallel(outer);
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
    Var x, y, tx, ty, n;

    Func downsample(Func f_in, Expr size) {
        using Halide::_;
        Func f, downx, downy;
        f(x, y, _) = u16(f_in(x, y, _));
        downx(x, y, _) = 1 * f(size*x-2, y, _) + 4 * f(size*x-1, y, _) + 6 * f(size*x, y, _) + 4 * f(size*x+1, y, _) + 1 * f(size*x+2, y, _);
        downy(x, y, _) = u8((1 * downx(x, size*y-2, _) + 4 * downx(x, size*y-1, _) + 6 * downx(x, size*y, _) + 4 * downx(x, size*y+1, _) + 1 * downx(x, size*y+2, _)) >> 8);
        return downy;
    }

    /*
     * @param gauss: input gaussian pyramid
     * @param coarse_alignment: alignment from upper level of pyramid 
     * @param tile_s: tile size to align
     * @param search_s: the search radius
     * @param upsample: TODO
     * @param align_min: The minimum alignment value to clamp to (usually comes from upper level)
     * @param align_max: The maximum alignment value to clamp to (usually comes from upper level)
     * @ return: TODO
     */
    Func align_layer(Func gauss, Func coarse_alignment, Expr tile_s, Expr search_s, Expr upsample, Expr align_min, Expr align_max) {
        /* TODO
         * Alignment parameters (e.g. L1/L2 distance, tile size, search region size)
         * differs by level.
         * For now, use L1 distance.
         */

        // Algorithm
        // x_s and y_s stand for x search and y search 
        Var x_s, y_s;
        Func coarse_offset;
        Func scores;
        Func alignment;

        // tx and ty represent the tile location

        // Iteration WITHIN a tile 
        RDom r_tile(0, tile_s, 0, tile_s);

        // Iteration for the search 
        RDom r_search(-1 * search_s, 2 * search_s + 1, -1 * search_s, 2 * search_s + 1);

        // CGRA does not support division. Convert division to shift.
        coarse_offset(tx, ty, n) = clamp(upsample * coarse_alignment((tx+upsample >> 1) / upsample, (ty+upsample >> 1) / upsample, n), align_min, align_max);

        // Coordinates for the reference image (what we are aligning to)
        Expr x_ref = tx * tile_s + r_tile.x;
        Expr y_ref = ty * tile_s + r_tile.y;

        // Coordinates for the comparison frame (what is being aligned)
        Expr x_cmp = x_ref + coarse_offset(tx, ty, n) + x_s;
        Expr y_cmp = y_ref + coarse_offset(tx, ty, n) + y_s;

        // L1 distance between reference frame and comparison frame 
        Expr dist = u16(abs(i16(gauss(x_ref, y_ref, 0)) - i16(gauss(x_cmp, y_cmp, n))));

        // SUM over all pixels in tile and all search locations. Brute force minimize means test all positions and find the best one. 
        scores(tx, ty, x_s, y_s, n) = sum(dist);

        // THIS is confusing. What is it minimizing over??? And what is this tuple that is returned? 
        // Ans: Looks like it would minimize over r_search.x and r_search.y, since tx and ty are "fixed" from the perspective of this function 
        Tuple min_coor = argmin(scores(tx, ty, r_search.x, r_search.y, n));
        
        alignment(tx, ty, n) = select(n == 0, i16(0), i16(min_coor[0] + coarse_offset(tx, ty, n)),
                    i16(min_coor[1] + coarse_offset(tx, ty, n))); 

        return alignment;
    }
};
}
HALIDE_REGISTER_GENERATOR(Alignment, align)