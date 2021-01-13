#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "conv_layer_3D.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "conv_layer_3D_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<int16_t> processor("conv_layer_3D", {"input.png", "kernel.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        conv_layer_3D(proc.inputs["input.png"], proc.inputs["kernel.png"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input.png"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          conv_layer_3D_clockwork(proc.inputs["input.png"], proc.inputs["kernel.png"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    int OX = 16;
    int OY = 24;
    int K_X = 3;
    int K_Y = 3;
    int X = OX + K_X - 1;
    int Y = OY + K_Y - 1;
    int Z = 4; // input channel 
    int W = 6; // output channel

    ///// INPUT IMAGE /////
    processor.inputs["input.png"] = Buffer<int16_t>(Z, X, Y);
    auto input_copy_stencil = processor.inputs["input.png"];
    //int i=1;
    int max_rand = pow(2,8) - 1;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          //input_copy_stencil(z, x, y) = z + x + y;      // diagonal
          //input_copy_stencil(z, x, y) = 1;              // all ones
          //input_copy_stencil(z, x, y) = i;    i = i+1;  // increasing
          if (rand() % 100 < 60) { // 60% zero, else rand
            input_copy_stencil(z, x, y) = 0;
          } else {
            input_copy_stencil(z, x, y) = (rand() % (max_rand));
          }
    } } }

    std::cout << "input has dims: " << processor.inputs["input.png"].dim(0).extent() << "x"
              << processor.inputs["input.png"].dim(1).extent() << "x"
              << processor.inputs["input.png"].dim(2).extent() << "\n";

    bool write_images = false;


    ///// KERNEL WEIGHTS /////  
    processor.inputs["kernel.png"] = Buffer<int16_t>(Z, W, K_X, K_Y);
    auto kernel_copy_stencil = processor.inputs["kernel.png"];
    //int j=1;
    //bool first = true;
    for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
        for (int w = 0; w < kernel_copy_stencil.dim(1).extent(); w++) {
          for (int z = 0; z < kernel_copy_stencil.dim(0).extent(); z++) {
            //kernel_copy_stencil(z, w, x, y) = z + w + x + y;
            //kernel_copy_stencil(z, w, x, y) = 1;
            
            //kernel_copy_stencil(z, w, x, y) = j;
            //if (first && j==Z) { first = false; j=0; }
            //j = j+1;

            if (rand() % 100 < 60) { // 60% zero, else rand
              kernel_copy_stencil(z, w, x, y) = 0;
            } else {
              kernel_copy_stencil(z, w, x, y) = (rand() % (2*max_rand)) - max_rand;
            }
            
            //std::cout << "kernel " << z << "," << w << "," << x << "," << y << " = " << +kernel_copy_stencil(z,w,x,y) << std::endl;
    } } } }
    //std::cout << "kernel 2,1 = 6 ?= " << +kernel_copy_stencil(0,0,2,1) << std::endl;
  
    std::cout << "kernel has dims: " << processor.inputs["kernel.png"].dim(0).extent() << "x"
              << processor.inputs["kernel.png"].dim(1).extent() << "x"
              << processor.inputs["kernel.png"].dim(2).extent() << "x"
              << processor.inputs["kernel.png"].dim(3).extent() << "\n";
    processor.inputs_preset = true;
    
    int imgsize_x = OX;//std::floor( (X + 2*P_X - K_X) / stride ) + 1;
    int imgsize_y = OY;//std::floor( (Y + 2*P_Y - K_Y) / stride ) + 1;
    processor.output = Buffer<int16_t>(imgsize_x, imgsize_y, W);

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";


    auto output = processor.process_command(argc, argv);
    //std::cout << "output 0 0 ?= " << +processor.output(8,4,0) << std::endl;

    Buffer<int16_t> oned_output(imgsize_x * imgsize_y, W);
    std::vector<Buffer<int16_t>> outputs;
    for (int i=0; i<W; ++i) {
      outputs.emplace_back(Buffer<int16_t>(imgsize_x, imgsize_y));
    }

    for (int w = 0; w < processor.output.dim(2).extent(); w++) {
      for (int y = 0; y < processor.output.dim(1).extent(); y++) {
        for (int x = 0; x < processor.output.dim(0).extent(); x++) {
          oned_output(x + y*imgsize_x, w) = processor.output(x,y,w);
          outputs[w](x,y) = processor.output(x,y,w);
          //std::cout << x << "," << y << "," << w << " = " << +processor.output(x,y,w) << " = " << std::hex << +processor.output(x,y,w) << std::dec << std::endl;
        } } }
    if (write_images) {//if (processor.output(0,0,0,0) == 198) {
      save_image(oned_output, "bin/output_gold.png");
      //save_image(processor.output, "bin/output_gold.png");
      for (size_t i=0; i<outputs.size(); ++i) {
        save_image(outputs[i], "bin/output_" + std::to_string(i) + ".png");
      }
    }

    
    return output;
}  
