/*
 * An application for taking a raw image and demosaicking the image
 * to an RGB image.
 */

#include "Halide.h"

namespace {

using namespace Halide;

class Demosaic : public Halide::Generator<Demosaic> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 3};

    // assumption that uint8_t phase = 0;
    int imgsize = 62;

    void generate() {
        /* THE ALGORITHM */
        Var c("c"), x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        Func hw_input, hw_input_copy;
        //hw_input_copy(x,y) = cast<uint16_t>(input(x+1,y+1));
        //hw_input(x,y) = hw_input_copy(x,y);
        hw_input(x,y) = cast<uint16_t>(input(x+1,y+1));

        // common patterns: average of four surrounding pixels
        Func neswNeighbors, diagNeighbors;
        // Should this actually be /4 outside of the hw input?
        neswNeighbors(x, y) = (hw_input(x-1, y)   + hw_input(x+1, y) +
                               hw_input(x,   y-1) + hw_input(x,   y+1)) / 4;
        diagNeighbors(x, y) = (hw_input(x-1, y-1) + hw_input(x+1, y-1) +
                               hw_input(x-1, y+1) + hw_input(x+1, y+1)) / 4;
        //neswNeighbors(x, y) = (hw_input(x,  y+1) + hw_input(x+2, y+1) +
        //                       hw_input(x+1,y)   + hw_input(x+1, y+2)) / 4;
        //diagNeighbors(x, y) = (hw_input(x,  y)   + hw_input(x+2, y) +
        //                       hw_input(x,  y+2) + hw_input(x+2, y+2)) / 4;

        // common patterns: average of two adjacent pixels
        Func vNeighbors, hNeighbors;
        vNeighbors(x, y) = (hw_input(x, y-1) + hw_input(x, y+1))/2;
        hNeighbors(x, y) = (hw_input(x-1, y) + hw_input(x+1, y))/2;
        //vNeighbors(x, y) = (hw_input(x+1,y) + hw_input(x+1, y+2))/2;
        //hNeighbors(x, y) = (hw_input(x, y+1) + hw_input(x+2, y+1))/2;

        // output pixels depending on image layout.
        // Generally, image looks like
        //    R G R G R G R G
        //    G B G B G B G B
        //    R G R G R G R G
        //    G B G B G B G B
        Func green, red, blue;
        green(x, y) = select((y % 2) == (0),
                             select((x % 2) == (0), neswNeighbors(x, y), hw_input(x, y)), // First row, RG
                             select((x % 2) == (0), hw_input(x, y),      neswNeighbors(x, y))); // Second row, GB

        red(x, y) = select((y % 2) == (0),
                           select((x % 2) == (0), hw_input(x, y),   hNeighbors(x, y)), // First row, RG
                           select((x % 2) == (0), vNeighbors(x, y), diagNeighbors(x, y))); // Second row, GB

        blue(x, y) = select((y % 2) == (0),
                            select((x % 2) == (0), diagNeighbors(x, y), vNeighbors(x, y)), // First row, RG
                            select((x % 2) == (0), hNeighbors(x, y),    hw_input(x, y))); // Second row, GB

        // output all channels
        Func demosaic, hw_output;
        demosaic(x,y,c) = select(c == 0, red(x, y),
                                 c == 1, green(x, y),
                                 blue(x, y));

        hw_output(c,x,y) = demosaic(x,y,c);
        output(x,y,c) = cast<uint8_t>(hw_output(c,x,y));
        
        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {

          hw_output.compute_root();
          
          hw_output.tile(x, y, xo, yo, xi, yi, 62,62)
            .hw_accelerate(xi, xo);
          demosaic.reorder(c, x, y);

          hw_output.unroll(c);
          
          demosaic.linebuffer();
          demosaic.unroll(c);

          hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);
          hw_input.stream_to_accelerator();

        } else if (get_target().has_feature(Target::Clockwork) || get_target().has_feature(Target::Pono)) {
          output.bound(c, 0, 3);
          output.bound(x, 0, 62);
          output.bound(y, 0, 62);

          Var xi,yi, xo,yo;

          hw_output.compute_root();

          hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
            //.reorder(c, xi, yi, xo, yo)
            .hw_accelerate(xi, xo);
          //hw_output.unroll(c);

          demosaic.compute_at(hw_output, xo);

          demosaic.unroll(c);

          hw_input.stream_to_accelerator();
          //hw_input.compute_at(hw_output, xo);
          //hw_input_copy.compute_root();

        } else {    // schedule to CPU
          output.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Demosaic, demosaic)

