#define T_SIZE 16
#include "Halide.h"

namespace {

using namespace Halide::ConciseCasts;

class Merge : public Halide::Generator<Merge> {
public:
    GeneratorParam<uint8_t>     scale{"scale", 8, 1, 16}; 
    GeneratorParam<uint16_t>     min_dist{"min_dist", 16, 1, 128}; 
    GeneratorParam<uint16_t>     max_dist{"max_dist", 64, 32, 4096}; 
    Input<Buffer<uint8_t>>  input{"input", 4};
    Input<Buffer<int16_t>>  alignment{"alignment", 4};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        // Algorithm

        //In the Google paper, they convert spatial image into frequency image
        //and apply mergin for each frequency.

        RDom r_tile(0, 16, 0, 16);
        RDom r_channel(0, 3);
        RDom r_imgs(0, input.dim(3).extent());

        Func clamped = Halide::BoundaryConditions::repeat_edge(input);
        Func align_clamped = Halide::BoundaryConditions::repeat_edge(alignment);


        Expr offset_x = align_clamped(tx, ty, 0, n);
        Expr offset_y = align_clamped(tx, ty, 1, n);
        Expr ref_x = tx*T_SIZE + r_tile.x;
        Expr ref_y = ty*T_SIZE + r_tile.y;
        Expr alt_x = ref_x + offset_x;
        Expr alt_y = ref_y + offset_y;
        Expr ref_val = clamped(ref_x, ref_y, c, 0);
        Expr alt_val = clamped(alt_x, alt_y, c, n);

        Expr scale_shift = scale;
        Func dist_channel;
        dist_channel(tx, ty, c, n) = sum(u32(abs(i16(ref_val) - i16(alt_val))));
        Expr dist = (sum(dist_channel(tx, ty, r_channel, n))) >> scale_shift;
        Expr dist_norm = select(dist <= min_dist, 1, dist);

        /* TODO
         * 1. Convert this weight function to fixed point operation.
         * 2. Change division to shift.
         */
        Func weight, sum_weight;
        weight(tx, ty, n) = select(n == 0, 256,
                dist_norm < min_dist, 256,
                print(dist_norm) > max_dist, 0, 
                256 / dist_norm);
        sum_weight(tx, ty) = sum(u16(weight(tx, ty, r_imgs)));

        ref_x = tx*T_SIZE + xi;
        ref_y = ty*T_SIZE + yi;
        alt_x = ref_x + offset_x;
        alt_y = ref_y + offset_y;
        ref_val = clamped(ref_x, ref_y, c, 0);
        alt_val = clamped(alt_x, alt_y, c, n);
        
        Expr ix, iy, tile_x, tile_y;
        tile_x = x >> 4;
        tile_y = y >> 4;
        ix = x - tile_x*16;
        iy = y - tile_y*16;
        Func val, output_pre;
        val(xi, yi, tx, ty, c, n) = select(n == 0, ref_val, alt_val);
        output(x, y, c) = u8(sum(u32(weight(tile_x, tile_y, r_imgs)) * u32(val(ix, iy, tile_x, tile_y, c, r_imgs))) / sum_weight(tile_x, tile_y));


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
