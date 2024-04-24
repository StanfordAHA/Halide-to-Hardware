/*
 * An application that performs a simplified version of Google's HDR+ pipeline 
 * 
 */
#define T_SIZE 16
#define LOG_2_T_SIZE 4
#include "Halide.h"

namespace {

constexpr int maxJ = 6;

using namespace Halide;
using namespace Halide::ConciseCasts;

class HDRPlus : public Halide::Generator<HDRPlus> {
public:
    // FOR ALIGN
    GeneratorParam<int>     pyramid_levels{"pyramid_levels", 4, 1, maxJ};

    // FOR MERGE 
    GeneratorParam<uint8_t>      normalization_shift{"normalization_shift", 8, 1, 16}; 
    GeneratorParam<uint16_t>     min_dist{"min_dist", 16, 1, 128}; 
    GeneratorParam<uint16_t>     max_dist{"max_dist", 256, 32, 4096};
    GeneratorParam<uint16_t>     dist_max_min_diff{"dist_max_min_diff", 240};  

    // Operate on raw bayer image: so 2 channels, plus 1 channel b/c receiving multiple images
    Input<Buffer<uint16_t>>  input{"input", 3};
    Output<Buffer<uint16_t>> output{"output", 2};


    Expr mul4(Expr a, Expr b, Expr c, Expr d) {
      return u16(( (u32(a)) * (u32(b)) * (u32(c)) * (u32(d)) ) >> 8);
    }


    Expr mul3(Expr a, Expr b, Expr c) {
      return u16(( (u32(a)) * (u32(b)) * (u32(c)) ) >> 8);
    }


    Expr mul2(Expr a, Expr b) {
      return u16(( (u32(a)) * (u32(b)) ) >> 8);
    }

    void generate() {
        // Algorithm
        Var x, y, tx("tx"), ty, xy, xi, yi, c, n;

        /* 
         * ALIGN STEP
         */
        const int J = pyramid_levels;

        Func clamped_input = Halide::BoundaryConditions::repeat_edge(input);

        // Unroll into separate color channels 
        // FIXME: Add if statement to check if target is clockwork later 
        // Question: how can I be sure this will create 4 color channels???
        //Func clamped_input_shuffle;
        //Func r_r, g_gr, g_gb, b_b;


        //g_gr(x, y, n) = clamped_input(2*x, 2*y, n);//deinterleaved(x, y, 0);
        //r_r(x, y, n)  = clamped_input(2*x+1, 2*y, n);//deinterleaved(x, y, 1);
        //b_b(x, y, n)  = clamped_input(2*x, 2*y+1, n);//deinterleaved(x, y, 2);
        //g_gb(x, y, n) = clamped_input(2*x+1, 2*y+1, n);//deinterleaved(x, y, 3);

        Func deinterleaved;


        deinterleaved(x, y, c, n) = select(c == 0, clamped_input(2 * x, 2 * y, n), (select(c == 1, clamped_input(2 * x + 1, 2 * y, n), 
                                            (select(c == 2, clamped_input(2 * x, 2 * y + 1, n), clamped_input(2 * x + 1, 2 * y + 1, n))))));
        //deinterleaved(x, y, c, n) = mux(c,
        //                         {clamped_input(2 * x, 2 * y, n),
        //                          clamped_input(2 * x + 1, 2 * y, n),
        //                          clamped_input(2 * x, 2 * y + 1, n),
        //                          clamped_input(2 * x + 1, 2 * y + 1, n)});

        //Var c;
        //clamped_input_shuffle(x, y, c, n) = clamped_input(2*x + c/2, 2*y + c%2, n);


        // STEP 1: Convert to grayscale 
        Func gray;
        // gray should average four nearest neighbor pixels from bayer raw in a 2x2 grid 
        //TODO: Double-check this arithmetic. Should we be adding in higher precision? 
        //gray(x, y, n) = u16(u16(clamped(x, y, n)) + u16(clamped(x+1, y, n)) + u16(clamped(x, y+1, n)) + u16(clamped(x+1, y+1, n)) >> 2); 

        // Average of R, G, G, B
        // Grayscale = 0.299R + 0.587G + 0.114B; break G down into two equal halves
        gray(x, y, n) = u16((77 * u16(deinterleaved(x, y, 1, n)) + 75 * u16(deinterleaved(x, y, 0, n)) + 75 * u16(deinterleaved(x, y, 3, n)) + 29 * u16(deinterleaved(x, y, 2, n))) >> 8 );


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
        initialAlign(tx, ty, xy, n) = 0;
        Func alignPyramid[maxJ];
        Expr min_align[maxJ];
        Expr max_align[maxJ];
        min_align[J-1] = i16(Expr(-4));
        max_align[J-1] = i16(Expr(4));

        // In the google paper, they say that for the last pyramid level, they work with tile sizes of 8x8 and search radius of 4. Upsample factor is of course 4. 
        alignPyramid[J-1](tx, ty, xy, n) = 
            align_layer(gPyramid[J-1], initialAlign, 8, 4, 4, 0, min_align[J-1], max_align[J-1])(tx, ty, xy, n);
        for (int j = J-2; j >= 0; j--) {
            if (j == 0) {
                min_align[j] = 2 * min_align[j+1] - 1;
                max_align[j] = 2 * max_align[j+1] + 1;
                alignPyramid[j](tx, ty, xy, n) = 
                    align_layer(gPyramid[j], alignPyramid[j+1], 16, 1, 2, 0, min_align[j], max_align[j])(tx, ty, xy, n);

            } else if (j == J-2) {
                min_align[j] = 4 * min_align[j+1] - 4;
                max_align[j] = 4 * max_align[j+1] + 4;
                alignPyramid[j](tx, ty, xy, n) = 
                    align_layer(gPyramid[j], alignPyramid[j+1], 16, 4, 4, 1, min_align[j], max_align[j])(tx, ty, xy, n);
            } else {
                min_align[j] = 4 * min_align[j+1] - 4;
                max_align[j] = 4 * max_align[j+1] + 4;
                alignPyramid[j](tx, ty, xy, n) = 
                    align_layer(gPyramid[j], alignPyramid[j+1], 16, 4, 4, 0, min_align[j], max_align[j])(tx, ty, xy, n);
            }
        }
        
        // should there be a + input here??? the align_layer function returns the coordinates, NOT the aligned image... 
        // ACTUALLY, note that this may be fine because merge.cpp takes this into account 
        Func align_output;
        align_output(tx, ty, xy, n) = alignPyramid[0](tx, ty, xy, n);
        /* 
         * END ALIGN STEP
         */


        /* 
         * MERGE STEP
         */
        // In the paper, they claim to use 16x16 tiles 
        RDom r_tile(0, T_SIZE, 0, T_SIZE);

        // This should be for the 4 color panes of the bayer raw: R, G, G, B
        RDom r_channel(0, 4);


        // This is for the number of images being merged 
        RDom r_imgs(0, input.dim(2).extent());

        //Func clamped_input = Halide::BoundaryConditions::repeat_edge(input);



        //Func align_clamped = Halide::BoundaryConditions::repeat_edge(align_output);
        //Func clamped_input_shuffle;
        //clamped_input_shuffle(x, y, c, n) = clamped_input(2*x + c/2, 2*y + c%2, n);


        // I THINK HOW TO DO OVERLAPING TILES IS DIVIDE T_SIZE BY 2 IN THIS EXPRESSION. 
        Expr offset_x = align_output(tx, ty, 0, n);
        Expr offset_y = align_output(tx, ty, 1, n);
        Expr ref_x = tx*T_SIZE/2 + r_tile.x;
        Expr ref_y = ty*T_SIZE/2 + r_tile.y;

        // Find the corresponding tile in the alternate frame, using the results of alignment 
        Expr alt_x = ref_x + offset_x;
        Expr alt_y = ref_y + offset_y;

        // Here "c" should represent the Bayer color plane, but I don't see where "c" is defined? 
        // ANS: It is defined as a variable in the private section

        // Pixel in reference frame and alternate frame respectively 
        // Using deinterleaved because merging is done separately for each color channel 
        Expr ref_val = deinterleaved(ref_x, ref_y, c, 0);
        Expr alt_val = deinterleaved(alt_x, alt_y, c, n);

        // ???
        Expr norm_shift = normalization_shift;
        Func dist_channel, dist_channel_norm;

        // Summing over a given tile; dist_channel represents distance for that color channel
        // the iteration domain here is r_tile.x and r_tile.y
        //dist_channel(tx, ty, c, n) = sum(u32(abs(i16(ref_val) - i16(alt_val))));
        dist_channel(tx, ty, c, n) = sum(u32(abs(i32(i32(ref_val) - i32(alt_val)))));

        // Summing over channels: WHY?? This does not seem to process color chanels independently, as described in paper 
        // What is scale_shift??
        // We already check dist<=min_dist here. Why is it checked again when computing the weight few lines below? 
        //Expr dist = (sum(dist_channel(tx, ty, r_channel, n))) >> scale_shift;
        //Expr dist_norm = select(dist <= min_dist, 1, dist);

        
        //dist_channel_norm(tx, ty, c, n) = select(u32(dist_channel(tx, ty, c, n)) <= min_dist, 1, dist_channel(tx, ty, c, n));

        // Dist channel norm is average distance per pixel. Tiles are size 16x16, so 256 pixels per tile. So divide the total distance sum by 256 
        dist_channel_norm(tx, ty, c, n) = dist_channel(tx, ty, c, n) >> norm_shift;
        /* TODO
         * 1. Convert this weight function to fixed point operation.
         * 2. Change division to shift.
         * 3. Check these U32s. Maybe they should be a different datatype
         */

        // Each color channel has its own weight tensor
        Func weight, sum_weight, unscaled_normalized_weight, normalized_weight;
        /*weight(tx, ty, c, n) = select(n == 0, 65536,
                u32(dist_channel_norm(tx, ty, c, n)) < min_dist, 65536,
                u32(dist_channel_norm(tx, ty, c, n)) > max_dist, 0, 
                65536 / u32(dist_channel_norm(tx, ty, c, n)));*/

        weight(tx, ty, c, n) = select(n == 0, u16(dist_max_min_diff),
            u16(dist_channel_norm(tx, ty, c, n)) < min_dist, u16(dist_max_min_diff),
            u16(dist_channel_norm(tx, ty, c, n)) > max_dist, 0, 
            u16(u16(max_dist) - u16(dist_channel_norm(tx, ty, c, n)))); 

        // Summing over images: WHY??
        // ANS: To create denominator for weighted sum expr below. Each color channel has its own weight tensor 
        sum_weight(tx, ty, c) = sum(u32(weight(tx, ty, c, r_imgs)));

        // Normalize the weight by the sum of all weights so it lies between 0 and 1
        // TODO: Convert this division into a shift 
        unscaled_normalized_weight(tx, ty, c, n) = weight(tx, ty, c, n) * 1.0f/sum_weight(tx, ty, c);


        // THIS CAST IS LIKELY PROBLEMATIC 
        normalized_weight(tx, ty, c, n) = cast<uint16_t>(unscaled_normalized_weight(tx, ty, c, n) * 256.f);
        //normalized_weight.trace_stores();
        //print(normalized_weight(tx, ty, c, n));

        // I think these are recomputed so r_imgs is the only reduction domain in the final sum 
        ref_x = tx*T_SIZE/2 + xi;
        ref_y = ty*T_SIZE/2 + yi;
        alt_x = ref_x + offset_x;
        alt_y = ref_y + offset_y;
        ref_val = deinterleaved(ref_x, ref_y, c, 0);
        alt_val = deinterleaved(alt_x, alt_y, c, n);
        

        

        // Why shift by 4? Explicit tiling of the image? 
        // Ans: I think tile_x and tile_y are the (x,y) location of a tile (think of tiles as grid points themselves)
        Expr ix, iy, tile_x, tile_y;
        tile_x = x >> LOG_2_T_SIZE;
        tile_y = y >> LOG_2_T_SIZE;
        ix = x - tile_x*T_SIZE; // this looks very weird 
        //ix = x%T_SIZE;
        iy = y - tile_y*T_SIZE;
        //iy = y%T_SIZE;
        Func val;
        val(xi, yi, tx, ty, c, n) = select(n == 0, ref_val, alt_val);
        // Weighted sum of all frames (reference frame and all alternate frames)
        // TODO: Unshuffle back into bayer pattern before sending output 
        // DON'T understand this indexing at all...
        // 1.What's the difference between tile_x/tile_y and tx/ty?
        // 2. ix and iy just look wrong and weird

        RVar tee_x, tee_y;

        Func output_deinterleaved_tiled;

        //output_deinterleaved(x, y, c) = u16(0);
        //output_deinterleaved(x, y, c) = u16(sum(u32(weight(tile_x, tile_y, c, r_imgs)) * u32(val(ix, iy, tile_x, tile_y, c, r_imgs))) / sum_weight(tile_x, tile_y, c));
        //output_deinterleaved(tx * (T_SIZE) + xi, ty * (T_SIZE) + yi, c) = u16(sum(u32(weight(tx, ty, c, r_imgs)) * u32(val(xi, yi, tx, ty, c, r_imgs))) / sum_weight(tx, ty, c));
        //output_deinterleaved_tiled(xi, yi, tx, ty, c) = u16(0);
        //output_deinterleaved_tiled(xi, yi, tx, ty, c) += u16(sum(u32(weight(tx, ty, c, r_imgs)) * u32(val(xi, yi, tx, ty, c, r_imgs)) * raised_cosine_weight(xi) * raised_cosine_weight(yi)) / sum_weight(tx, ty, c));
        //output_deinterleaved_tiled(xi, yi, tx, ty, c) += u16(sum(u32(normalized_weight(tx, ty, c, r_imgs) * val(xi, yi, tx, ty, c, r_imgs) * raised_cosine_weight(xi) * raised_cosine_weight(yi))));
        //output_deinterleaved_tiled(xi, yi, tx, ty, c) += mul3(u16(sum(u32(unscaled_normalized_weight(tx, ty, c, r_imgs) * val(xi, yi, tx, ty, c, r_imgs)))), cast<uint16_t>(raised_cosine_weight(xi) * 256.0f) , cast<uint16_t>(raised_cosine_weight(yi) * 256.0f));
        output_deinterleaved_tiled(xi, yi, tx, ty, c) = u16(sum(u32(unscaled_normalized_weight(tx, ty, c, r_imgs) * val(xi, yi, tx, ty, c, r_imgs))));

        //output_deinterleaved_tiled(xi, yi, tx, ty, c) = u16(sum(u32(mul2(unscaled_normalized_weight(tx, ty, c, r_imgs), val(xi, yi, tx, ty, c, r_imgs)))));


        //output_deinterleaved_tiled(xi, yi, tx, ty, c) = mul3(pre_output_deinterleaved_tiled(xi, yi, tx, ty, c), ,);

        //print(output_deinterleaved_tiled(xi, yi, tx, ty, c));


        Func output_deinterleaved;
        output_deinterleaved(x, y, c) = u16(0);
        //output_deinterleaved(x_prime, y, c) += output_deinterleaved_tiled(ix, iy, tile_x, tile_y, c);

        //2 * num_tiles - 1, to account for overlapping tiles 
        Expr num_tx_locs =  2 * (((input.width())/2) >> LOG_2_T_SIZE) - 1;
        Expr num_ty_locs =  2 * (((input.height())/2) >> LOG_2_T_SIZE) - 1;

        RDom tile_RDom(0, num_tx_locs, 0, num_ty_locs, 0, T_SIZE, 0, T_SIZE);
        Expr x_prime = tile_RDom.x * T_SIZE/2 + tile_RDom.z;
        Expr y_prime = tile_RDom.y * T_SIZE/2 + tile_RDom.w;
        output_deinterleaved(x_prime, y_prime, c) += mul2(output_deinterleaved_tiled(tile_RDom.z, tile_RDom.w, tile_RDom.x, tile_RDom.y, c), mul2(cast<uint16_t>(raised_cosine_weight(tile_RDom.z) * 256.0f),  cast<uint16_t>(raised_cosine_weight(tile_RDom.w) * 256.0f)));

        //output_deinterleaved(tee_x * (T_SIZE/2) + r_tile.x, tee_y * (T_SIZE/2) + r_tile.y, c) += output_deinterleaved_tiled(r_tile.x, r_tile.y, tee_x, tee_y, c);
        //output_shuffle(x, y, c) = u8(sum(u32(val(ix, iy, tile_x, tile_y, c, r_imgs))));
        //output_shuffle(x, y, c) = u8(sum(u32(weight(tx, ty, c, r_imgs)) * u32(val(xi, yi, tx, ty, c, r_imgs))) / sum_weight(tx, ty, c));

        // Unshuffle: should be the inverse of the shuffle operation done prior
        // Deinterleave: should be the inverse of the interleaving operation done prior  
        //output(x, y) = output_shuffle(x/2, y/2, y%2 + 2*(x%2));
        //FIXME: NEED THE TWO SELECTS!!!
        Func row_r_result;
        Func row_b_result;

        // G R G R G R
        // Selecting between g_gr and r_r
        row_r_result(x, y) = select((x%2)==0, output_deinterleaved(x/2, y, 0), output_deinterleaved(x/2, y, 1));

        // B G B G B G
        // Selecting between b_b and g_gb 
        row_b_result(x, y) = select((x%2)==0, output_deinterleaved(x/2, y, 2), output_deinterleaved(x/2, y, 3));


        output(x, y) = select((y%2)==0, row_r_result(x, y/2), row_b_result(x, y/2));
        //output(x, y) = input(x, y, 0);

        /* 
         * END MERGE STEP
         */



        // Schedule
        //Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");
        Var xo("xo"), yo("yo"), outer("outer");
        align_output.reorder(xy, n, tx, ty).tile(tx, ty, xo, yo, xi, yi, 8, 8).fuse(xo, yo, outer).parallel(outer);
        align_output.compute_root();
        gray.compute_root().parallel(y, 32).vectorize(x, 8);
        
        for (int j = 1; j < J; j++) {
            gPyramid[j]
                .compute_root().parallel(y, 8).vectorize(x, 8);
            alignPyramid[j]
                .store_at(align_output, outer).compute_at(align_output, yi);
        }
        alignPyramid[0].compute_at(align_output, yi);

        //output_shuffle.reorder(c, x, y).tile(x, y, xo, yo, xi, yi, 64, 64).fuse(xo, yo, outer).parallel(outer);
        output.reorder(x, y).tile(x, y, xo, yo, xi, yi, 64, 64).fuse(xo, yo, outer).parallel(outer);
        dist_channel.reorder(c, tx, ty, n);
        weight.compute_root().parallel(ty, 4).vectorize(tx, 4);
        sum_weight.store_at(output, outer).compute_at(output, yi).parallel(ty, 4).vectorize(tx, 4);
        val.store_at(output, outer).compute_at(output, yi);
    }
private:
    //Var x, y, tx, ty, xy, xi, yi, c, n;

    Func downsample(Func f_in, Expr size) {
        Var x, y;
        using Halide::_;
        Func f, downx, downy;
        f(x, y, _) = u16(f_in(x, y, _));
        downx(x, y, _) = 1 * f(size*x-2, y, _) + 4 * f(size*x-1, y, _) + 6 * f(size*x, y, _) + 4 * f(size*x+1, y, _) + 1 * f(size*x+2, y, _);
        // Question: why is this u8? Should I just use a different (trusted) downsample function? 
        downy(x, y, _) = u16((1 * downx(x, size*y-2, _) + 4 * downx(x, size*y-1, _) + 6 * downx(x, size*y, _) + 4 * downx(x, size*y+1, _) + 1 * downx(x, size*y+2, _)) >> 8);
        return downy;
    }

    Expr raised_cosine_weight(Expr in){
        const float PI = 3.141592f;
        return 0.5f - 0.5f * cos(2.0f * PI * (in + 0.5f) / T_SIZE);
    }

    /*
     * @param gauss: input gaussian pyramid
     * @param prev_alignment : alignment from upper level of pyramid 
     * @param tile_s: tile size to align
     * @param search_s: the search radius
     * @param upsample: TODO
     * @param align_min: The minimum alignment value to clamp to (usually comes from upper level)
     * @param align_max: The maximum alignment value to clamp to (usually comes from upper level)
     * @ return: TODO
     */
    Func align_layer(Func gauss, Func prev_alignment , Expr tile_s, Expr search_s, Expr upsample, Expr tile_size_shift_factor, Expr align_min, Expr align_max) {
        Var tx, ty, xy, n;
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

        // Iteration for the search (over the search radius)
        RDom r_search(-1 * search_s, 2 * search_s + 1, -1 * search_s, 2 * search_s + 1);

        // CGRA does not support division. Convert division to shift.
        //FIXME: DOUBLE-CHECK THIS UPSAMPLING 
        coarse_offset(tx, ty, xy, n) = clamp(upsample * prev_alignment ((tx+upsample >> 1) / upsample, (ty+upsample >> 1) / upsample, xy, n), align_min, align_max);
        //coarse_offset(tx, ty, xy, n) = clamp(upsample * prev_alignment ((tx >> tile_size_shift_factor), (ty >> tile_size_shift_factor), xy, n), align_min, align_max);


        // Coordinates for the reference image (what we are aligning to)
        Expr x_ref = tx * tile_s + r_tile.x;
        Expr y_ref = ty * tile_s + r_tile.y;

        // Coordinates for the comparison frame (what is being aligned)
        Expr x_cmp = x_ref + coarse_offset(tx, ty, 0, n) + x_s;
        Expr y_cmp = y_ref + coarse_offset(tx, ty, 1, n) + y_s;


        // FIXME: double-check this distance computation. Maybe the datatypes should be i16. There is a chance it is returning unsigned,
        // even if the result is negative
        // Maybe just cast the result of the subtraction as i16() 
        // L1 distance between reference frame and comparison frame 
        //Expr dist = u16(abs(i16(gauss(x_ref, y_ref, 0)) - i16(gauss(x_cmp, y_cmp, n))));
        //Expr dist = u16(abs(u16(gauss(x_ref, y_ref, 0)) - u16(gauss(x_cmp, y_cmp, n))));
        Expr dist = u16(abs(i32(i32(gauss(x_ref, y_ref, 0)) - i32(gauss(x_cmp, y_cmp, n)))));

        // SUM over all pixels in tile. 
        scores(tx, ty, x_s, y_s, n) = sum(dist);

        // THIS is confusing. What is it minimizing over??? And what is this tuple that is returned? 
        // Ans: Looks like it would minimize over r_search.x and r_search.y, since tx and ty are "fixed" from the perspective of this function 
        // Brute force minimize means test all positions and find the best one. 
        Tuple min_coor = argmin(scores(tx, ty, r_search.x, r_search.y, n));
        
        //alignment(tx, ty, n) = select(n == 0, i16(0), i16(min_coor[0] + coarse_offset(tx, ty, n)),
        //            i16(min_coor[1] + coarse_offset(tx, ty, n))); 


        alignment(tx, ty, xy, n) = select(n == 0, i16(0),
                    xy == 0, i16(min_coor[0] + coarse_offset(tx, ty, 0, n)),
                    i16(min_coor[1] + coarse_offset(tx, ty, 1, n))); 


        /*alignment(tx, ty, xy, n) = select(n == 0, u16(0),
                    xy == 0, u16(min_coor[0] + coarse_offset(tx, ty, 0, n)),
                    u16(min_coor[1] + coarse_offset(tx, ty, 1, n)));*/


        //alignment(tx, ty, 0, n) = select(n == 0, i16(0), i16(min_coor[0] + coarse_offset(tx, ty, n))); 
        //alignment(tx, ty, 1, n) = select(n == 0, i16(0), i16(min_coor[1] + coarse_offset(tx, ty, n))); 


        return alignment;
    }
  };
}
HALIDE_REGISTER_GENERATOR(HDRPlus, hdr_plus)