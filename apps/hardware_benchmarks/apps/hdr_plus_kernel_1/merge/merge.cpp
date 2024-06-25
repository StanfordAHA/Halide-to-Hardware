#define T_SIZE 16
#include "Halide.h"

namespace {

using namespace Halide::ConciseCasts;

class Merge : public Halide::Generator<Merge> {
public:
    GeneratorParam<uint8_t>     scale{"scale", 8, 1, 16}; 
    GeneratorParam<uint16_t>     min_dist{"min_dist", 16, 1, 128}; 
    GeneratorParam<uint16_t>     max_dist{"max_dist", 256, 32, 4096}; 

    // Operate on raw bayer image: so 2 channels, plus 1 channel b/c receiving multiple images 
    Input<Buffer<uint8_t>>  input{"input", 3};
    Input<Buffer<int16_t>>  alignment{"alignment", 4};

    // Outputting a single (merged) Bayer raw frame
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        // Algorithm

        //In the Google paper, they convert spatial image into frequency image
        //and apply mergin for each frequency.
        // MO: Looks like we're not doing that here...computing weight based on distance in spatial domain

        // In the paper, they claim to use 16x16 tiles 
        RDom r_tile(0, 16, 0, 16);

        // This should be for the 4 color panes of the bayer raw: R, G, G, B
        RDom r_channel(0, 4);


        // This is for the number of images being merged 
        RDom r_imgs(0, input.dim(2).extent());

        Func clamped_input = Halide::BoundaryConditions::repeat_edge(input);



        Func align_clamped = Halide::BoundaryConditions::repeat_edge(alignment);
        Func clamped_input_shuffle;
        clamped_input_shuffle(x, y, c, n) = clamped_input(2*x + c/2, 2*y + c%2, n);


        Expr offset_x = align_clamped(tx, ty, 0, n);
        Expr offset_y = align_clamped(tx, ty, 1, n);
        Expr ref_x = tx*T_SIZE + r_tile.x;
        Expr ref_y = ty*T_SIZE + r_tile.y;

        // Find the corresponding tile in the alternate frame, using the results of alignment 
        Expr alt_x = ref_x + offset_x;
        Expr alt_y = ref_y + offset_y;

        // Here "c" should represent the Bayer color plane, but I don't see where "c" is defined? 
        // ANS: It is defined as a variable in the private section

        // Pixel in reference frame and alternate frame respectively 
        Expr ref_val = clamped_input_shuffle(ref_x, ref_y, c, 0);
        Expr alt_val = clamped_input_shuffle(alt_x, alt_y, c, n);

        // ???
        Expr scale_shift = scale;
        Func dist_channel, dist_channel_norm;

        // Summing over a given tile; dist_channel represents distance for that color channel  
        dist_channel(tx, ty, c, n) = sum(u32(abs(i16(ref_val) - i16(alt_val))));

        // Summing over channels: WHY?? This does not seem to process color chanels independently, as described in paper 
        // What is scale_shift??
        // We already check dist<=min_dist here. Why is it checked again when computing the weight few lines below? 
        //Expr dist = (sum(dist_channel(tx, ty, r_channel, n))) >> scale_shift;
        //Expr dist_norm = select(dist <= min_dist, 1, dist);


        dist_channel_norm(tx, ty, c, n) = select(u32(dist_channel(tx, ty, c, n)) <= min_dist, 1, dist_channel(tx, ty, c, n));
        /* TODO
         * 1. Convert this weight function to fixed point operation.
         * 2. Change division to shift.
         * 3. Check these U32s. Maybe they should be a different datatype
         */

        // Each color channel has its own weight tensor
        Func weight, sum_weight;
        weight(tx, ty, c, n) = select(n == 0, 256,
                u32(dist_channel_norm(tx, ty, c, n)) < min_dist, 256,
                u32(dist_channel_norm(tx, ty, c, n)) > max_dist, 0, 
                256 / u32(dist_channel_norm(tx, ty, c, n)));

        // Summing over images: WHY??
        // ANS: To create denominator for weighted sum expr below. Each color channel has its own weight tensor 
        sum_weight(tx, ty, c) = sum(u16(weight(tx, ty, c, r_imgs)));

        // I think these are recomputed so r_imgs is the only reduction domain in the final sum 
        ref_x = tx*T_SIZE + xi;
        ref_y = ty*T_SIZE + yi;
        alt_x = ref_x + offset_x;
        alt_y = ref_y + offset_y;
        ref_val = clamped_input_shuffle(ref_x, ref_y, c, 0);
        alt_val = clamped_input_shuffle(alt_x, alt_y, c, n);
        
        // Why shift by 4? Explicit tiling of the image? 
        // Ans: I think tile_x and tile_y are the (x,y) location of a tile (think of tiles as grid points themselves)
        Expr ix, iy, tile_x, tile_y;
        tile_x = x >> 4;
        tile_y = y >> 4;
        ix = x - tile_x*16; // this looks very weird 
        iy = y - tile_y*16;
        Func val;
        val(xi, yi, tx, ty, c, n) = select(n == 0, ref_val, alt_val);
        // Weighted sum of all frames (reference frame and all alternate frames)
        // TODO: Unshuffle back into bayer pattern before sending output 
        // DON'T understand this indexing at all...
        // 1.What's the difference between tile_x/tile_y and tx/ty?
        // 2. ix and iy just look wrong and weird
        Func output_shuffle;
        output_shuffle(x, y, c) = u8(sum(u32(weight(tile_x, tile_y, c, r_imgs)) * u32(val(ix, iy, tile_x, tile_y, c, r_imgs))) / sum_weight(tile_x, tile_y, c));

        // Unshuffle: should be the inverse of the shuffle operation done prior 
        output(x, y) = output_shuffle((x-c/2)*1/2, (y-c%2)*1/2, c);

        // Now have no idea whether the schedule below will work...since I have changed the way "c" is used 

        // Schedule
      Var xo("xo"), yo("yo"), xi("xi"), yi("yi"), outer("outer");
      output.reorder(c, x, y).tile(x, y, xo, yo, xi, yi, 64, 64).fuse(xo, yo, outer).parallel(outer);
        dist_channel.reorder(c, tx, ty, n);
        weight.compute_root().parallel(ty, 4).vectorize(tx, 4);
        sum_weight.store_at(output, outer).compute_at(output, yi).parallel(ty, 4).vectorize(tx, 4);
        val.store_at(output, outer).compute_at(output, yi); 
    }
private:
    Var x, y, xi, yi, tx, ty, c, n;

};
}
HALIDE_REGISTER_GENERATOR(Merge, merge)
