/*
 * An application for applying a Gaussian blur.
 * It uses a 3x3 stencil with constant weights.
 */

#include "Halide.h"

namespace {

using namespace Halide;

// Size of blur for gradients.
const int blockSize = 3;

class GaussianBlur : public Halide::Generator<GaussianBlur> {
public:
    Input<Buffer<uint8_t>>  input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
  
    GeneratorParam<uint16_t> schedule{"schedule", 3};    // default: 3
    GeneratorParam<uint16_t> mywidth{"mywidth", 992};    // old default 368
    GeneratorParam<uint16_t> myheight{"myheight", 0};    // default: don't use
    GeneratorParam<uint16_t> myunroll{"myunroll", 16};   // default: 16

  //Input<int32_t> tilesize{"tilesize", 64, 8, 128}; // default 64. bounded between 8 and 128
  //int tilesize = imgSize / 2;
  

    void generate() {
        /* THE ALGORITHM */

        Var x("x"), y("y");
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

        // Create a reduction domain of the correct bounds.
        RDom win(0, blockSize, 0, blockSize);

        Func hw_input, input_copy;
        hw_input(x, y) = cast<uint16_t>(input(x, y));
        //input_copy(x, y) = cast<uint16_t>(input(x, y));
        //hw_input(x, y) = input_copy(x, y);

        // create the gaussia nkernel
        Func kernel_f;
        float sigma = 1.5f;
        kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);

        // create a normalized set of 8bit weights
        Func kernel;
        Expr sum_kernel[blockSize*blockSize];
        for (int idx=0, i=0; i<blockSize; ++i) {
          for (int j=0; j<blockSize; ++j) {
            if (i==0 && j==0) {
              sum_kernel[idx] = kernel_f(i-blockSize/2) * kernel_f(j-blockSize/2);
            } else {
              sum_kernel[idx] = kernel_f(i-blockSize/2) * kernel_f(j-blockSize/2) + sum_kernel[idx-1];
            }
            idx++;
          }
        }
        //kernel(x) = cast<uint16_t>(kernel_f(x) * 64 / sum_kernel[blockSize-1]);
        //kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
        //                             sum_kernel[blockSize*blockSize-1]);
        kernel(x,y) = cast<uint16_t>(kernel_f(x-blockSize/2) * kernel_f(y-blockSize/2) * 256.0f /
                                     sum_kernel[blockSize*blockSize-1]);

        // Use a 2D filter to blur the input
        Func blur_unnormalized, blur;
        blur_unnormalized(x, y) = cast<uint16_t>(0);
        //blur_unnormalized(x, y) += cast<uint16_t>( kernel(win.x, win.y) * hw_input(x+win.x, y+win.y) );
        blur_unnormalized(x, y) += kernel(win.x, win.y) * hw_input(x+win.x, y+win.y);
        blur(x, y) = blur_unnormalized(x, y) / 256;

        Func blur_x, blur_y;
        blur_x(x, y) = (input(x, y) + input(x+1, y) + input(x+2, y))/3;
        blur_y(x, y) = (blur_x(x, y) + blur_x(x, y+1) + blur_x(x, y+2))/3;

        Func hw_output;
        //hw_output(x, y) = blur(x, y) * 256;
        hw_output(x, y) = blur(x, y);
        //hw_output(x, y) = blur_y(x, y);
        output(x, y) = cast<uint8_t>( hw_output(x, y) );

        /* THE SCHEDULE */
        if (get_target().has_feature(Target::CoreIR)) {


        } else if (get_target().has_feature(Target::Clockwork)) {

          if (schedule == 1) {
            // use global buffer and large input image
            const int tileSize = 62;
            const int numTiles = 2;
            const int glbSize = tileSize * numTiles;
            const int numHostTiles = 5;
            const int outputSize = numHostTiles * glbSize;
            const int inputSize = outputSize + blockSize-1;
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbSize, glbSize)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(xi, 2);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);
            hw_output.unroll(xi, 2);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);
            blur_unnormalized.update().unroll(x, 2);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);
            blur.unroll(x, 2);

            hw_input.in().compute_at(hw_output.in(), xo);
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(x, 2);
            
            hw_input.compute_root()
              .accelerator_input();
            //hw_input.unroll(x, 2);

          } else if (schedule == 2) {
            // do the big tern
            const int tileWidth = 94;
            const int tileHeight = 62;
            const int numHostTiles = 9;
            const int numTiles = 7;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTiles * glbWidth;
            const int outputHeight = numHostTiles * glbHeight;
            
            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .hw_accelerate(xi, xo);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xo, yo, xi, yi, tileWidth, tileHeight);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.store_in(MemoryType::GLB);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);

            hw_input.in().compute_at(hw_output.in(), xo);
            hw_input.in().store_in(MemoryType::GLB);
            
            hw_input.compute_root()
              .accelerator_input();

          } else if (schedule == 3) {
            // do the big tern and unroll
            //const int unroll = 14;
            const int unroll = myunroll;
            //const int tileWidth = 248; // for unroll=8
            //const int tileWidth = 266; // for unroll=14
            //const int tileWidth = 42; // for unroll=14
            //const int tileWidth = 256;
            const int tileWidth = mywidth;
            //const int tileHeight = 799;
            const int tileHeight = myheight==0 ? 480 : myheight;
            //const int tileHeight = 196;
            //const int tileHeight = 62;
            const int numHostTilesX = 1;//6;
            const int numHostTilesY = 1;//5;
            //const int numHostTilesX = 16-0;
            //const int numHostTilesY = 20-0;
            //const int numHostTilesX = 1;
            //const int numHostTilesY = 1;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;
            
            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);
            //hw_input.in().bound(x, 0, glbWidth+2);
            //hw_input.in().bound(y, 0, glbHeight+2);
            //hw_input.bound(x, 0, outputWidth+2);
            //hw_input.bound(y, 0, outputHeight+2);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .hw_accelerate(xi, xo);
            hw_output.in().unroll(xi, unroll, TailStrategy::RoundUp);
            hw_output.in().store_in(MemoryType::GLB);

            Var xii("xii"), yii("yii"), xio("xio"), yio("yio");
            hw_output
              .tile(x, y, xio, yio, xii, yii, tileWidth, tileHeight);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.unroll(xii, unroll, TailStrategy::RoundUp);
            
            //blur.compute_at(hw_output, xio);
            //auto compute_loop_level = yii;
            auto compute_loop_level = xio;
            blur.store_at(hw_output, xio).compute_at(hw_output, compute_loop_level);
            blur.unroll(x, unroll, TailStrategy::RoundUp);

            //kernel.compute_root();
            //kernel.compute_at(hw_output, compute_loop_level).unroll(x).unroll(y);
            
            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);
            blur_unnormalized.update().unroll(x, unroll, TailStrategy::RoundUp);
            blur_unnormalized.unroll(x, unroll, TailStrategy::RoundUp);
            blur_unnormalized.compute_at(hw_output, compute_loop_level);

            hw_input.in().in().compute_at(hw_output, compute_loop_level); // represents the mem tile
            //if (compute_loop_level == yii) { hw_input.in().in().reorder(y, x); }
            hw_input.in().in().unroll(x, unroll, TailStrategy::RoundUp);

            hw_input.in().compute_at(hw_output.in(), xo);
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_input.compute_root()
              .accelerator_input();

            kernel.unroll(x).unroll(y).compute_at(hw_output, xio);

          } else if (schedule == 31) {
            // do the big tern and unroll
            //const int unroll = 14;
            const int unroll = myunroll;
            //const int tileWidth = 248; // for unroll=8
            //const int tileWidth = 266; // for unroll=14
            //const int tileWidth = 42; // for unroll=14
            //const int tileWidth = 256;
            const int tileWidth = mywidth;
            const int tileHeight = 196;
            //const int tileHeight = 62;
            const int numHostTilesX = 16-0;
            const int numHostTilesY = 20-0;
            //const int numHostTilesX = 1;
            //const int numHostTilesY = 1;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;
            
            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);
            //hw_input.in().bound(x, 0, glbWidth+2);
            //hw_input.in().bound(y, 0, glbHeight+2);
            //hw_input.bound(x, 0, outputWidth+2);
            //hw_input.bound(y, 0, outputHeight+2);

            hw_output.in().compute_root();

            hw_output.in()
              .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
              .hw_accelerate(xi, xo);
            hw_output.in().parallel(xi, unroll, TailStrategy::RoundUp);
            hw_output.in().store_in(MemoryType::GLB);

            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xio, yio, xii, yii, tileWidth, tileHeight);
            hw_output.compute_at(hw_output.in(), xo);
            hw_output.parallel(xii, unroll, TailStrategy::RoundUp);

            blur.compute_at(hw_output, xio);
            blur.parallel(x, unroll, TailStrategy::RoundUp);
            
            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);
            blur_unnormalized.update().parallel(x, unroll, TailStrategy::RoundUp);
            blur_unnormalized.parallel(x, unroll, TailStrategy::RoundUp);
            blur_unnormalized.compute_at(hw_output, xio);

            hw_input.in().in().compute_at(hw_output, xio); // represents the mem tile
            hw_input.in().in().parallel(x, unroll, TailStrategy::RoundUp);

            hw_input.in().compute_at(hw_output.in(), xo);
            hw_input.in().store_in(MemoryType::GLB);
            hw_input.in().unroll(x, unroll, TailStrategy::RoundUp);
            
            hw_input.compute_root()
              .accelerator_input();





            
          } else if (schedule == 4) {
            // Perform host tiliing
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            const int tileSize = outputSize / 2; // four small tiles
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output.compute_root();

            hw_output
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);
            
            hw_input.stream_to_accelerator();

          } else if (schedule == 5) { // just an xcel definition
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output
              .compute_root()
              .hw_accelerate(x, Var::outermost());
            hw_input.stream_to_accelerator();

          } else if (schedule == 6) { // 62x62 tile
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output
              .compute_root()
              .tile(x, y, xi, xo, yi, yo, outputSize, outputSize)
              .reorder(xi, yi, xo, yo)
              .hw_accelerate(xi, xo);
            hw_input.stream_to_accelerator();

          } else if (schedule == 7) { // unrolled compute
            const int inputSize = 64;
            const int outputSize = inputSize-blockSize+1;
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            hw_output
              .compute_root()
              .tile(x, y, xi, xo, yi, yo, outputSize, outputSize)
              .hw_accelerate(xi, xo);

            blur_unnormalized.update()
              .unroll(win.x).unroll(win.y);
            
            hw_input.stream_to_accelerator();

          } else if (schedule == 8) {
            // do the big tern and unroll
            //const int unroll = 14;
            const int unroll = myunroll;
            //const int tileWidth = 248; // for unroll=8
            //const int tileWidth = 266; // for unroll=14
            //const int tileWidth = 42; // for unroll=14
            //const int tileWidth = 256;
            const int tileWidth = mywidth;
            const int tileHeight = 196;
            //const int tileHeight = 62;
            const int numHostTilesX = 23-0;
            const int numHostTilesY = 20-0;
            //const int numHostTilesX = 1;
            //const int numHostTilesY = 1;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;
            
            output.bound(x, 0, outputWidth);
            output.bound(y, 0, outputHeight);
            //hw_input.in().bound(x, 0, glbWidth+2);
            //hw_input.in().bound(y, 0, glbHeight+2);
            //hw_input.bound(x, 0, outputWidth+2);
            //hw_input.bound(y, 0, outputHeight+2);

            //hw_output.in().compute_root();
            //hw_output.in()
            //  .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
            //  .hw_accelerate(xi, xo);
            //hw_output.in().unroll(xi, unroll, TailStrategy::RoundUp);
            //hw_output.in().store_in(MemoryType::GLB);

            hw_output.compute_root();
            
            Var xii, yii, xio, yio;
            hw_output
              .tile(x, y, xio, yio, xii, yii, tileWidth, tileHeight)
              .hw_accelerate(xii, xio);
            //hw_output.compute_at(hw_output.in(), xo);
            hw_output.unroll(xii, unroll, TailStrategy::RoundUp);

            blur.compute_at(hw_output, xio);
            blur.unroll(x, unroll, TailStrategy::RoundUp);
            
            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);
            blur_unnormalized.update().unroll(x, unroll, TailStrategy::RoundUp);
            blur_unnormalized.unroll(x, unroll, TailStrategy::RoundUp);
            blur_unnormalized.compute_at(hw_output, xio);

            hw_input.in().compute_at(hw_output, xio); // represents the mem tile
            hw_input.in().unroll(x, unroll, TailStrategy::RoundUp);
            hw_input.compute_root().accelerator_input();

            //hw_input.in().in().compute_at(hw_output, xio); // represents the mem tile
            //hw_input.in().in().unroll(x, unroll, TailStrategy::RoundUp);
            //hw_input.in().compute_at(hw_output.in(), xo);
            //hw_input.in().store_in(MemoryType::GLB);
            //hw_input.in().unroll(x, unroll, TailStrategy::RoundUp);
            //hw_input.compute_root().accelerator_input();




            
          } else if (schedule == 9) {
            // Use new scheduling with iteration_order
            const int unroll = myunroll;
            const int tileWidth = mywidth;
            const int tileHeight = 196;
            const int numHostTilesX = 16-0;
            const int numHostTilesY = 20-0;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            // Create three level memory hierarchy with iteration_order
            auto level1 = IterLevel("CGRA", {{x, tileWidth}, {y, tileHeight}});
            auto level2 = IterLevel("GLB",  {{x, 1}, {y, 1}});
            auto level3 = IterLevel("host", {{x, 16}, {y, 20}});
            hw_output
              .iteration_order({level1, level2, level3});

            // Specify compute variables for memory hierarchy
            auto x0 = Var("x0");
            auto compute_cgra = hw_output.get_looplevel("CGRA", x0);
            auto compute_glb = hw_output.get_looplevel("GLB", x0);
            auto compute_host = LoopLevel::root();

            // Create hardware accelerator
            hw_output.get_memory_level("GLB")
              .hw_accelerate(Var("x1"), x0);

            // Create memories (the old way)
            blur.compute_at(compute_cgra);
            blur_unnormalized.compute_at(compute_cgra);
            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            // Stream input to accelerator
            hw_input
              .stream_to_accelerator({"CGRA", "GLB", "host"},
                                     {compute_cgra, compute_glb, compute_host});





            
            
          } else if (schedule == 10) {
            // Use new scheduling with iteration_order and create_memories
            const int unroll = myunroll;
            const int tileWidth = mywidth;
            const int tileHeight = 196;
            const int numHostTilesX = 16-0;
            const int numHostTilesY = 20-0;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            // Create three level memory hierarchy with iteration_order
            auto level1 = IterLevel("CGRA", {{x, tileWidth}, {y, tileHeight}});
            auto level2 = IterLevel("GLB",  {{x, 1}, {y, 1}});
            auto level3 = IterLevel("host", {{x, 16}, {y, 20}});
            hw_output
              .iteration_order({level1, level2, level3});

            // Specify compute variables for memory hierarchy
            auto x0 = Var("x0");
            auto compute_cgra = hw_output.get_looplevel("CGRA", x0);
            auto compute_glb = hw_output.get_looplevel("GLB", x0);
            auto compute_host = LoopLevel::root();

            // Create hardware accelerator
            hw_output.get_memory_level("GLB")
              .hw_accelerate(Var("x1"), x0);

            // Create memories
            hw_output.get_memory_level("GLB")
              .create_memories({blur, blur_unnormalized}, compute_cgra, x, 1);

            // Stream input to accelerator
            hw_input
              .stream_to_accelerator({"CGRA", "GLB", "host"},
                                     {compute_cgra, compute_glb, compute_host});



            

            

          } else if (schedule == 11) {
            // Use new scheduling with iteration_order, create_memories, and output_rate
            const int unroll = myunroll;
            const int tileWidth = mywidth;
            const int tileHeight = 196;
            const int numHostTilesX = 16-0;
            const int numHostTilesY = 20-0;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;

            // Create three level memory hierarchy with iteration_order (and set rate)
            auto level1 = IterLevel("CGRA", {{x, tileWidth}, {y, tileHeight}});
            auto level2 = IterLevel("GLB",  {{x, 1}, {y, 1}});
            auto level3 = IterLevel("host", {{x, 16}, {y, 20}});
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
              .create_memories({blur, blur_unnormalized}, compute_cgra);

            // Stream input to accelerator (and set rate)
            hw_input.output_rate(myunroll)
              .stream_to_accelerator({"CGRA", "GLB", "host"},
                                     {compute_cgra, compute_glb, compute_host});







            
          } else if (schedule == 12) {
            // do the big tern and unroll
            const int unroll = myunroll;
            const int tileWidth = mywidth;
            const int tileHeight = 196;
            const int numHostTilesX = 16-0;
            const int numHostTilesY = 20-0;
            const int numTiles = 1;
            const int glbWidth = tileWidth * numTiles;
            const int glbHeight = tileHeight * numTiles;
            const int outputWidth = numHostTilesX * glbWidth;
            const int outputHeight = numHostTilesY * glbHeight;
            
            //output.bound(x, 0, outputWidth);
            //output.bound(y, 0, outputHeight);

            //hw_output.in().compute_root();
                        
            //hw_output.iteration_order({{x, 2}, {x, tileWidth}, {y, tileHeight},
            //                           {x, 1}, {y, 1},
            //                           {x, 16}, {y, 20}});

            auto level1 = IterLevel("CGRA", {{x, tileWidth}, {y, tileHeight}});
            auto level2 = IterLevel("GLB",  {{x, 1}, {y, 1}});
            auto level3 = IterLevel("host", {{x, 16}, {y, 20}});
            hw_output.output_rate(myunroll)
              .iteration_order({level1, level2, level3});

            //hw_output.get_memory_level("GLB").hw_accelerate(Var("x1"), x0);
            auto x0 = Var("x0");
            auto compute_cgra = hw_output.get_looplevel("CGRA", x0);
            auto compute_glb = hw_output.get_looplevel("GLB", x0);
            auto compute_host = LoopLevel::root();
            
            hw_output.get_memory_level("GLB")
              .output_rate(myunroll)
              .hw_accelerate(Var("x1"), x0)
              .create_memories({blur, blur_unnormalized}, compute_cgra);
            
            
            hw_input.output_rate(myunroll)
              .stream_to_accelerator({"CGRA", "GLB", "host"},
                                     {compute_cgra, compute_glb, compute_host});

            
            //hw_output.in()
            //  .tile(x, y, xo, yo, xi, yi, glbWidth, glbHeight)
            //  .hw_accelerate(xi, xo);
            //hw_output.in().store_in(MemoryType::GLB);
            //
            //Var xii, yii, xio, yio;
            //hw_output
            //  .tile(x, y, xio, yio, xii, yii, tileWidth, tileHeight);
            //hw_output.compute_at(hw_output.in(), xo);
            //
            ////blur.compute_at(compute_cgra);
            //blur.unroll(x, unroll, TailStrategy::RoundUp);
            
            //blur_unnormalized.update()
            //  .unroll(win.x, blockSize)
            //  .unroll(win.y, blockSize);
            //blur_unnormalized.update().unroll(x, unroll, TailStrategy::RoundUp);
            //blur_unnormalized.unroll(x, unroll, TailStrategy::RoundUp);
            /////blur_unnormalized.compute_at(compute_cgra);
            //
            //hw_input.in().in().compute_at(hw_output, xio); // represents the mem tile
            //hw_input.in().in().unroll(x, unroll, TailStrategy::RoundUp);
            //
            //hw_input.in().compute_at(hw_output.in(), xo);
            //hw_input.in().store_in(MemoryType::GLB);
            //hw_input.in().unroll(x, unroll, TailStrategy::RoundUp);
            //
            //hw_input.compute_root()
            //  .accelerator_input();
/*
// declarative schedule version:
            accelerator(hw_input -> hw_output)
              .output_rate(14, matched, roundup)
              //.fill_entire_CGRA(16x32)
              .create_memories({blur, blur_unnormalized})
              //.buffer_all()
              .mem_hierarchy({MEM:{64x64}, GLB:{64x512}})
              //.fill_memories({MEM:2048, GLB:128kB})

// image bounds (use the same as Halide)

//split and reorder?
////  ---- GLB (DRAM) Level---
////  for oc in range(32):
////  ------ Memory Tile (SRAM) Level----
////  for ic in range(4):
////  for oy in range(56):
////  for ox in range(56):
////  ----- Regfile Level----
////  for ic in range(1):
////  for oc in range(1):
////  for fy in range(5):
////  for fx in range(5):
////  parallel ic 16 times
////  parallel oc 8 times

accelerator({hw_input, hw_kernel} -> hw_output)
  .unroll(oc, k_oc)
  .unroll(ic, k_ic)
  .loopnest(MEM:[(fx 5), (fy 5), (oc 1), (ic 1)],
            GLB:[(x 56), (y 56), (ic 4)],
            host:[(oc 32)]);

accelerator({hw_input, hw_kernel} -> hw_output)
  .loopnest(MEM:[u(r.ic 16) u(oc 8) (x 28) (y 28) (r.x 3) (r.y 3) (r.ic 2))],
            GLB:[(oc 4) (x 2) (y 2)],
            host:[]);
// from this loopnest for the output_cgra, the other loop orders should be determined

            

*/
            
          } else {
            //const int inputSize = 64;
            //const int outputSize = inputSize-blockSize+1;
            const int outputSize = 62;
            const int tileSize = outputSize; // single tile
            
            output.bound(x, 0, outputSize);
            output.bound(y, 0, outputSize);

            //output.bound(x, 0, tileSize);
            //output.bound(y, 0, tileSize);

            hw_output.compute_root();

            hw_output
              //.tile(x, y, xo, yo, xi, yi, outputSize, outputSize)
              .tile(x, y, xo, yo, xi, yi, tileSize, tileSize)
              .hw_accelerate(xi, xo);

            blur_unnormalized.update()
              .unroll(win.x, blockSize)
              .unroll(win.y, blockSize);

            blur_unnormalized.compute_at(hw_output, xo);
            blur.compute_at(hw_output, xo);

            hw_input.stream_to_accelerator();//.compute_root();//at(output, xo);
            //hw_input.compute_root();
          
            //hw_input.compute_at(hw_output, xo);
            //input_copy.stream_to_accelerator();
            //input_copy.compute_root();

          }
          
        } else {    // schedule to CPU
          if (schedule == 1 || schedule == 2 || schedule == 3) {
            output
              .split(y, y, yi, 32)
              .parallel(y)
              .vectorize(x, 16);
            blur
              .compute_at(output, y)
              .split(y, y, yi, 32)
              .vectorize(x, 16);
          }
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GaussianBlur, gaussian)

