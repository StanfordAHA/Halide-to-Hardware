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
    Input<Buffer<uint8_t>>  input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 2};

    GeneratorParam<uint16_t> schedule{"schedule", 3};    // default: 0
    GeneratorParam<uint16_t> myunroll{"myunroll", 2};    // default: 2
    GeneratorParam<uint16_t> mywidth{"mywidth", 300-6};      // default: 120
    //Input<int32_t> tileSize_x{"tileSize_x", 64, 8, 128};    // default: 64. bounded between 8 and 128
    //Input<int32_t> tileSize_y{"tileSize_y", 64, 8, 128};    // default: 64. bounded between 8 and 128

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y"), c("c");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        Func hw_input, gray;
        hw_input(c, x, y) = i16(input(x+(blockSize+4)/2, y+(blockSize+4)/2, c));

        // create a grayscale image
        gray(x, y) = cast<uint16_t>((77 * cast<uint16_t>(hw_input(0, x, y))
                                     + 150 * cast<uint16_t>(hw_input(1, x, y))
                                     + 29 * cast<uint16_t>(hw_input(2, x, y))) >> 8);

        // sobel filter
        Func grad_x_unclamp, grad_y_unclamp, grad_x, grad_y;
        Func kernel_x, kernel_y;
        RDom r(-1, 3, -1, 3);
        kernel_x(x,y) = i16(0);
        kernel_x(-1,-1) = i16(-1);    kernel_x(-1,0) = i16(-2);     kernel_x(-1, 1) = i16(-1);
        kernel_x(1, -1) = i16(1);     kernel_x(1, 0) = i16(2);      kernel_x( 1, 1) = i16(1);
        kernel_y(x,y) = i16(0);
        kernel_y(-1, 1) = i16( 1);    kernel_y( 0, 1) = i16( 2);    kernel_y( 1, 1) = i16( 1);
        kernel_y(-1,-1) = i16(-1);    kernel_y( 0,-1) = i16(-2);    kernel_y( 1,-1) = i16(-1);

        grad_x_unclamp(x, y) = i16(0);
        grad_x_unclamp(x, y) += gray(x+r.x, y+r.y) * i16(kernel_x(r.x, r.y));
        grad_y_unclamp(x, y) = i16(0);
        grad_y_unclamp(x, y) += gray(x+r.x, y+r.y) * i16(kernel_y(r.x, r.y));

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
        cim_output(x,y) = cast<int16_t>(select( is_max && (cim(x, y) >= threshold), i16(255), i16(0)));
        hw_output(x, y) = cim_output(x,y);
        //hw_output(x, y) = cast<uint8_t>(cim(x,y));
        //hw_output(x, y) = cast<uint8_t>(lgxx(x,y));

        output(x, y) = cast<uint8_t>(hw_output(x, y));


        /* THE SCHEDULE */
        if (get_target().has_feature(Target::Clockwork)) {

          if (schedule == 1) { // do host and glb tiling
            const int tileSize = 58;
            const int numHostTiles = 3;
            const int numTiles = 2;
            const int glbSize = tileSize * numTiles;
            const int outputSize = numHostTiles * glbSize;
            const int inputSize = outputSize + blockSize-1;

            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
              .reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .reorder(xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);

            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.compute_at(hw_output, xo);
            grad_y_unclamp.compute_at(hw_output, xo);
            grad_x_unclamp.update().unroll(r.x).unroll(r.y);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y);

            lgxx.update().unroll(box.x).unroll(box.y);
            lgyy.update().unroll(box.x).unroll(box.y);
            lgxy.update().unroll(box.x).unroll(box.y);

            gray.compute_at(hw_output, xo);

            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(c);  // hw input bound
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 2) { // do the big parrot
            const int tileWidth = 58;
            const int tileHeight = 94;
            const int numHostTiles = 5;
            const int numTiles = 5;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTiles * glbWidth;
            const int outputHeight = numHostTiles * glbHeight;

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .reorder(xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);

            lxx.compute_at(hw_output, xo);
            lyy.compute_at(hw_output, xo);
            lxy.compute_at(hw_output, xo);
            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            cim.compute_at(hw_output, xo);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y);
            kernel_y.unroll(x).unroll(y);

            grad_x_unclamp.compute_at(hw_output, xo);
            grad_y_unclamp.compute_at(hw_output, xo);
            grad_x_unclamp.update().unroll(r.x).unroll(r.y);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y);

            lgxx.update().unroll(box.x).unroll(box.y);
            lgyy.update().unroll(box.x).unroll(box.y);
            lgxy.update().unroll(box.x).unroll(box.y);

            gray.compute_at(hw_output, xo);

            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(c);  // hw input bound
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 3) { // do big parrot with unroll
            const int unroll = myunroll;
            //const int tileWidth = 128-6; // for unroll=2
            const int tileWidth = mywidth;
            //const int tileHeight = 255;
            const int tileHeight = 255;
            //const int tileHeight = 66;
            const int numHostTilesX = 1; //12;
            const int numHostTilesY = 1; //10;
            //const int numHostTilesX = 1;
            //const int numHostTilesY = 1;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in()
              .unroll(xi, unroll, TailStrategy::RoundUp);

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .reorder(xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(xi, unroll, TailStrategy::RoundUp);

            cim.compute_at(hw_output, xo).unroll(x, unroll, TailStrategy::RoundUp);

            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            lgxx.update().unroll(box.x).unroll(box.y).unroll(x, unroll, TailStrategy::RoundUp);
            lgyy.update().unroll(box.x).unroll(box.y).unroll(x, unroll, TailStrategy::RoundUp);
            lgxy.update().unroll(box.x).unroll(box.y).unroll(x, unroll, TailStrategy::RoundUp);
            lgxx.unroll(x, unroll, TailStrategy::RoundUp);
            lgyy.unroll(x, unroll, TailStrategy::RoundUp);
            lgxy.unroll(x, unroll, TailStrategy::RoundUp);

            lxx.compute_at(hw_output, xo).unroll(x, unroll, TailStrategy::RoundUp);
            lyy.compute_at(hw_output, xo).unroll(x, unroll, TailStrategy::RoundUp);
            lxy.compute_at(hw_output, xo).unroll(x, unroll, TailStrategy::RoundUp);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y).unroll(x, unroll, TailStrategy::RoundUp);
            kernel_y.unroll(x).unroll(y).unroll(x, unroll, TailStrategy::RoundUp);

            grad_x_unclamp.compute_at(hw_output, xo);
            grad_y_unclamp.compute_at(hw_output, xo);
            grad_x_unclamp.update().unroll(r.x).unroll(r.y).unroll(x, unroll, TailStrategy::RoundUp);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y).unroll(x, unroll, TailStrategy::RoundUp);
            grad_x_unclamp.unroll(x, unroll, TailStrategy::RoundUp);
            grad_y_unclamp.unroll(x, unroll, TailStrategy::RoundUp);

            gray.compute_at(hw_output, xo).unroll(x, unroll, TailStrategy::RoundUp);

            hw_input.in().in().compute_at(hw_output, xo); // represents the mem tile
            hw_input.in().in()
              .unroll(c)
              .unroll(x, unroll, TailStrategy::RoundUp);

            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(c)
              .unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 31) { // do big parrot with outermost unroll
            const int unroll = myunroll;
            //const int tileWidth = 128-6; // for unroll=2
            const int tileWidth = mywidth;
            //const int tileHeight = 255;
            const int tileHeight = 255;
            //const int tileHeight = 66;
            const int numHostTilesX = 1; //12;
            const int numHostTilesY = 1; //10;
            //const int numHostTilesX = 1;
            //const int numHostTilesY = 1;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            Var xu, xn;
            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_output.in()
              .unroll(xi, unroll, TailStrategy::RoundUp);

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight)
              .reorder(xi, yi, xo, yo);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.split(xi,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,yi,xu).unroll(xu);

            cim.compute_at(hw_output, xo).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            lgxx.compute_at(hw_output, xo);
            lgyy.compute_at(hw_output, xo);
            lgxy.compute_at(hw_output, xo);
            lgxx.update().unroll(box.x).unroll(box.y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lgyy.update().unroll(box.x).unroll(box.y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lgxy.update().unroll(box.x).unroll(box.y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lgxx.split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lgyy.split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lgxy.split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            lxx.compute_at(hw_output, xo).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lyy.compute_at(hw_output, xo).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            lxy.compute_at(hw_output, xo).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            kernel_x.compute_at(hw_output, xo);
            kernel_y.compute_at(hw_output, xo);
            kernel_x.unroll(x).unroll(y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            kernel_y.unroll(x).unroll(y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            grad_x_unclamp.compute_at(hw_output, xo);
            grad_y_unclamp.compute_at(hw_output, xo);
            grad_x_unclamp.update().unroll(r.x).unroll(r.y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            grad_y_unclamp.update().unroll(r.x).unroll(r.y).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            grad_x_unclamp.split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
            grad_y_unclamp.split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            gray.compute_at(hw_output, xo).split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            hw_input.in().in().compute_at(hw_output, xo); // represents the mem tile
            hw_input.in().in()
              .unroll(c)
              .split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);

            hw_input.in().compute_at(hw_output.in(), xo); // represents the glb level
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(c)
              //.split(x,xn,xu,unroll,TailStrategy::RoundUp).reorder(xn,y,xu).unroll(xu);
              .unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_input.compute_root()
              .accelerator_input();

            


            
          } else if (schedule == 11) { // do big parrot with new scheduling primitives
            const int unroll = myunroll;
            const int tileWidth = mywidth;
            const int tileHeight = 255;
            const int numHostTilesX = 5; //12;
            const int numHostTilesY = 10; //10;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            // Create three level memory hierarchy with iteration_order (and set rate)
            auto level1 = IterLevel("CGRA", {{x, tileWidth}, {y, tileHeight}});
            auto level2 = IterLevel("GLB",  {{x, 1}, {y, 1}});
            auto level3 = IterLevel("host", {{x, 5}, {y, 10}});
            hw_output.output_rate(myunroll)
              .iteration_order({level1, level2, level3});

            // Specify compute variables for memory hierarchy
            auto x0 = Var("x0");
            auto compute_cgra = hw_output.get_looplevel("CGRA", x0);
            auto compute_glb = hw_output.get_looplevel("GLB", x0);
            auto compute_host = LoopLevel::root();

            // Create hardware accelerator
            hw_output.get_memory_level("GLB")
              .hw_accelerate(Var("x1"), x0);

            // Create memories (and set rate)
            hw_output.get_memory_level("GLB").output_rate(myunroll)
              .create_memories({cim, lgxx, lgyy, lgxy, lxx, lyy, lxy,
                    grad_x_unclamp, grad_y_unclamp, gray}, compute_cgra);
            
            kernel_x.compute_at(compute_cgra).unroll(x).unroll(y).unroll(x, unroll, TailStrategy::RoundUp);
            kernel_y.compute_at(compute_cgra).unroll(x).unroll(y).unroll(x, unroll, TailStrategy::RoundUp);

            // Stream input to accelerator (and set rate)
            hw_input.output_rate(myunroll)
              .stream_to_accelerator({"CGRA", "GLB", "host"},
                                     {compute_cgra, compute_glb, compute_host});
            hw_input.in().get_memory_level("CGRA").unroll(c);
            hw_input.get_memory_level("GLB").unroll(c);



            

            
          } else { // default, basically sch5 with all buffers, 1 pixel/cycle
            output.bound(x, 0, tileSize_x);
            output.bound(y, 0, tileSize_y);

            hw_output.compute_root();
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

            gray.compute_at(hw_output, xo);
            
            //padded16.compute_at(hw_output, xo);
            hw_input.in().unroll(c);
            hw_input.stream_to_accelerator();
            //hw_input_copy.compute_root();
          }


        } else if (get_target().has_feature(Target::CoreIR) || get_target().has_feature(Target::HLS)) {

        } else {    // schedule to CPU
          if (schedule == 1 || schedule == 2 || schedule == 3) {
            const int vec = 16;
            output.split(y, y, yi, 32)
              .parallel(y)
              .vectorize(x, vec);
            gray.store_at(output, y)
              .compute_at(output, yi)
              .vectorize(x, vec);
            grad_x.store_at(output, y)
              .compute_at(output, yi)
              .vectorize(x, vec);
            grad_y.store_at(output, y)
              .compute_at(output, yi)
              .vectorize(x, vec);
            grad_x.compute_with(grad_y, x);
          }
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HarrisCornerDetector, harris_color)

