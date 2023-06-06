#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "resnet_output_stationary.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "resnet_output_stationary_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<int16_t> processor("resnet_output_stationary", {"input.mat", "kernel.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        resnet_output_stationary(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          resnet_output_stationary_clockwork(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    // Set enviroment variable to set these:
    //  HALIDE_GEN_ARGS="in_img=28 pad=1 ksize=3 stride=1 n_ic=8 n_oc=6"

    auto OX = getenv("in_img");
    auto P = getenv("pad");
    auto K = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");
    auto OC = getenv("n_oc");

    auto in_img = OX ? atoi(OX) : 56;
    auto pad = P ? atoi(P) : 1;
    auto ksize = K ? atoi(K) : 3;
    auto stride = S ? atoi(S) : 1;
    auto n_ic = IC ? atoi(IC) : 16;
    auto n_oc = OC ? atoi(OC) : 8;

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int P_X = pad;
    int P_Y = P_X;
    int Z = n_ic; // input channel 
    int W = n_oc; // output channel

    if (true) {//OX || P || K || S || IC || OC) {
      std::cout << "using inputs set within process.cpp" << std::endl;
      processor.inputs_preset = true;
    } else {
      std::cout << "reading input.mat and kernel.mat" << std::endl;
      processor.inputs_preset = false;
    }
    
    ///// INPUT IMAGE /////
    processor.inputs["input.mat"] = Buffer<int16_t>(Z, X, Y);
    auto input_copy_stencil = processor.inputs["input.mat"];
    int i=1; (void) i;
    int max_rand = pow(2,8) - 1;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          //input_copy_stencil(z, x, y) = z + x + y;      // diagonal
          //input_copy_stencil(z, x, y) = 1;              // all ones
          //input_copy_stencil(z, x, y) = i;    i = i+1;  // increasing
          //continue;
          if (rand() % 100 < 60) { // 60% zero, else rand
            input_copy_stencil(z, x, y) = 0;
          } else {
            input_copy_stencil(z, x, y) = (rand() % (max_rand));
          }
    } } }

    std::cout << "input has dims: " << processor.inputs["input.mat"].dim(0).extent() << "x"
              << processor.inputs["input.mat"].dim(1).extent() << "x"
              << processor.inputs["input.mat"].dim(2).extent() << "\n";

    bool write_images = false;

    Buffer<int16_t> full_input(Z, X + pad*2-(stride-1), Y + pad*2-(stride-1));
    Buffer<int16_t> oned_input(Z *(X + pad*2), Y + pad*2);
    Buffer<int16_t> interleaved_input(Z *(X + pad*2) * 2, Y + pad*2);
    std::vector<Buffer<int16_t>> inputs;
    for (int i=0; i<Z; ++i) {
      inputs.emplace_back(Buffer<int16_t>(X + pad*2, Y + pad*2));
    }
    
    for (int y = 0; y < full_input.dim(2).extent(); y++) {
      for (int x = 0; x < full_input.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          auto x_coord = x-pad<0 ? 0 : x-pad >= X ? X-1 : x-pad;
          auto y_coord = y-pad<0 ? 0 : y-pad >= Y ? Y-1 : y-pad;
          full_input(z, x, y) = input_copy_stencil(z, x_coord, y_coord);
          oned_input(z + Z*x, y) = input_copy_stencil(z, x_coord, y_coord);
          interleaved_input(2*(z + Z*x) + 0, y) = input_copy_stencil(z, x_coord, y_coord);
          interleaved_input(2*(z + Z*x) + 1, y) = input_copy_stencil(z, x_coord, y_coord);
          
          inputs[z](x, y) = input_copy_stencil(z, x_coord, y_coord);
          //std::cout << "input " << z << "," << x << "," << y << " = " << std::hex << +full_input(z,x,y) << std::dec << std::endl;
        } } }
    //std::cout << "input 3,2 = 31 ?= " << +full_input(3,2) << std::endl;
    std::cout << "full input has dims: " << full_input.dim(0).extent() << "x"
              << full_input.dim(1).extent() << "x"
              << full_input.dim(2).extent() << "\n";

    std::cout << "num_dims=" << oned_input.dimensions() << std::endl
              << oned_input.dim(0).extent() << "x" << oned_input.dim(1).extent()
              << std::endl;
    if (write_images) {
      save_image(oned_input, "bin/input.pgm");
      std::cout << "wrote an image\n";
      save_image(interleaved_input, "bin/input_interleaved.png");
      for (size_t i=0; i<inputs.size(); ++i) {
        save_image(inputs[i], "bin/input_" + std::to_string(i) + ".mat");
      }
    }

    ///// KERNEL WEIGHTS /////  
    processor.inputs["kernel.mat"] = Buffer<int16_t>(W, Z, K_X, K_Y);
    auto kernel_copy_stencil = processor.inputs["kernel.mat"];
    //int j=1;
    //bool first = true;
    for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
        for (int z = 0; z < kernel_copy_stencil.dim(1).extent(); z++) {
          for (int w = 0; w < kernel_copy_stencil.dim(0).extent(); w++) {
            //kernel_copy_stencil(z, w, x, y) = z + w + x + y;
            //continue;
            //kernel_copy_stencil(z, w, x, y) = 1;
            
            //kernel_copy_stencil(z, w, x, y) = j;
            //if (first && j==Z) { first = false; j=0; }
            //j = j+1;

            if (rand() % 100 < 60) { // 60% zero, else rand
              kernel_copy_stencil(w, z, x, y) = 0;
            } else {
              kernel_copy_stencil(w, z, x, y) = (rand() % (2*max_rand)) - max_rand;
            }
            
            //std::cout << "kernel " << z << "," << w << "," << x << "," << y << " = " << +kernel_copy_stencil(z,w,x,y) << std::endl;
    } } } }

    bool write_mat = true;
    if (write_mat) {
      std::cout << "Writing input.mat and kernel.mat to bin folder" << std::endl;
      save_image(processor.inputs["input.mat"], "bin/input.mat");
      save_image(processor.inputs["kernel.mat"], "bin/kernel.mat");
      save_image(full_input, "bin/input_padded.mat");

      //Buffer<int16_t> inin = load_and_convert_image("input.mat");
      //std::cout << "loaded input has dims: " << inin.dim(0).extent() << "x"
      //          << inin.dim(1).extent() << "x"
      //          << inin.dim(2).extent() << "\n";
      //
      //Buffer<int16_t> kerker = load_and_convert_image("kernel.mat");
      //std::cout << "loaded kernel has dims: " << kerker.dim(0).extent() << "x"
      //          << kerker.dim(1).extent() << "x"
      //          << kerker.dim(2).extent() << "x"
      //          << kerker.dim(3).extent() << "\n";

    }
    
    //std::cout << "kernel 2,1 = 6 ?= " << +kernel_copy_stencil(0,0,2,1) << std::endl;
  
    std::cout << "kernel has dims: " << processor.inputs["kernel.mat"].dim(0).extent() << "x"
              << processor.inputs["kernel.mat"].dim(1).extent() << "x"
              << processor.inputs["kernel.mat"].dim(2).extent() << "x"
              << processor.inputs["kernel.mat"].dim(3).extent() << "\n";

    
    int imgsize_x = std::floor( (X + 2*P_X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y + 2*P_Y - K_Y) / stride ) + 1;
    processor.output = Buffer<int16_t>(W, imgsize_x, imgsize_y);

    std::cout << "1st value is " << std::hex << processor.output(0, 0, 0) << std::dec << std::endl;
    std::cout << "2nd value is " << std::hex << processor.output(1, 0, 0) << std::dec << std::endl;
    std::cout << "3rd value is " << std::hex << processor.output(2, 0, 0) << std::dec << std::endl;

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

    for (int w = 0; w < processor.output.dim(0).extent(); w++) {
      for (int y = 0; y < processor.output.dim(2).extent(); y++) {
        for (int x = 0; x < processor.output.dim(1).extent(); x++) {
          oned_output(x + y*imgsize_x, w) = processor.output(w,x,y);
          outputs[w](x,y) = processor.output(w,x,y);
          //std::cout << x << "," << y << "," << w << " = " << +processor.output(x,y,w) << " = " << std::hex << +processor.output(x,y,w) << std::dec << std::endl;
        } } }
    if (write_images) {//if (processor.output(0,0,0,0) == 198) {
      //save_image(oned_output, "bin/output_gold.mat");
      save_image(processor.output, "bin/output_gold.mat");
      for (size_t i=0; i<outputs.size(); ++i) {
        save_image(outputs[i], "bin/output_" + std::to_string(i) + ".mat");
      }
    }

    return output;
}  
