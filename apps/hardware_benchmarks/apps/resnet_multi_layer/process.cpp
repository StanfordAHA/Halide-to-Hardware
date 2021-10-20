#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "resnet_multi_layer.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "resnet_multi_layer_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("resnet_multi_layer", {"input.png", "kernel.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        resnet_multi_layer(proc.inputs["input.png"], proc.inputs["kernel.png"], proc.output);
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
          resnet_multi_layer_clockwork(proc.inputs["input.png"], proc.inputs["kernel.png"], proc.output);
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
    //  HALIDE_GEN_ARGS="in_img=28 pad=1 ksize=3 stride=1 k_ic=8 k_oc=6"

    auto OX = getenv("in_img");
    auto P = getenv("pad");
    auto K = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");
    auto OC = getenv("n_oc");
    //auto NL = getenv("n_layers");

    auto in_img = OX ? atoi(OX) : 28;
    auto pad = P ? atoi(P) : 1;
    auto ksize = K ? atoi(K) : 3;
    auto stride = S ? atoi(S) : 1;
    auto n_ic = IC ? atoi(IC) : 128;
    auto n_oc = OC ? atoi(OC) : 512;

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int P_X = pad;
    int P_Y = P_X;
    int Z = n_ic; // input channel 
    int W = n_oc; // output channel
  
    processor.inputs["input.png"] = Buffer<uint8_t>(Z, X, Y);
    auto input_copy_stencil = processor.inputs["input.png"];
    int i=1;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          //input_copy_stencil(z, x, y) = z + x + y;
          //input_copy_stencil(z, x, y) = 1;
          input_copy_stencil(z, x, y) = i;
          i = i+1;
        } } }

    std::cout << "input has dims: " << processor.inputs["input.png"].dim(0).extent() << "x"
              << processor.inputs["input.png"].dim(1).extent() << "x"
              << processor.inputs["input.png"].dim(2).extent() << "\n";

    bool write_images = false;

    Buffer<uint8_t> full_input(Z, X + pad*2, Y + pad*2);
    Buffer<uint8_t> oned_input(Z *(X + pad*2), Y + pad*2);
    std::vector<Buffer<uint8_t>> inputs;
    for (int i=0; i<Z; ++i) {
      inputs.emplace_back(Buffer<uint8_t>(X + pad*2, Y + pad*2));
    }
    
    for (int y = 0; y < full_input.dim(2).extent(); y++) {
      for (int x = 0; x < full_input.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          auto x_coord = x-pad<0 ? 0 : x-pad >= X ? X-1 : x-pad;
          auto y_coord = y-pad<0 ? 0 : y-pad >= Y ? Y-1 : y-pad;
          full_input(z, x, y) = input_copy_stencil(z, x_coord, y_coord);
          oned_input(z + Z*x, y) = input_copy_stencil(z, x_coord, y_coord);
          
          inputs[z](x, y) = input_copy_stencil(z, x_coord, y_coord);
          //std::cout << z << "," << x << "," << y << " = " << +full_input(z,x,y) << std::endl;
        } } }
    std::cout << "input 3,2 = 31 ?= " << +full_input(3,2) << std::endl;
    //save_image(full_input, "bin/input.png");
    if (write_images) {
      save_image(oned_input, "bin/input.png");
      for (size_t i=0; i<inputs.size(); ++i) {
        save_image(inputs[i], "bin/input_" + std::to_string(i) + ".png");
      }
    }

  
    processor.inputs["kernel.png"] = Buffer<uint8_t>(Z, W, K_X, K_Y);
    auto kernel_copy_stencil = processor.inputs["kernel.png"];
    int j=1;
    bool first = true;
    for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
        for (int w = 0; w < kernel_copy_stencil.dim(1).extent(); w++) {
          for (int z = 0; z < kernel_copy_stencil.dim(0).extent(); z++) {
            //kernel_copy_stencil(z, w, x, y) = z + w + x + y;
            //kernel_copy_stencil(z, w, x, y) = 1;
            kernel_copy_stencil(z, w, x, y) = j;
            if (first && j==Z) { first = false; j=0; }
            j = j+1;
            //std::cout << "kernel " << z << "," << w << "," << x << "," << y << " = " << +kernel_copy_stencil(z,w,x,y) << std::endl;
          } } } }
    std::cout << "kernel 2,1 = 6 ?= " << +kernel_copy_stencil(0,0,2,1) << std::endl;
  
    std::cout << "kernel has dims: " << processor.inputs["kernel.png"].dim(0).extent() << "x"
              << processor.inputs["kernel.png"].dim(1).extent() << "x"
              << processor.inputs["kernel.png"].dim(2).extent() << "x"
              << processor.inputs["kernel.png"].dim(3).extent() << "\n";
    processor.inputs_preset = true;
    
    int imgsize_x = std::floor( (X + 2*P_X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y + 2*P_Y - K_Y) / stride ) + 1;
    processor.output = Buffer<uint8_t>(imgsize_x, imgsize_y, W);

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";


    auto output = processor.process_command(argc, argv);
    //std::cout << "output 0 0 ?= " << +processor.output(8,4,0) << std::endl;

    Buffer<uint8_t> oned_output(imgsize_x * imgsize_y, W);
    std::vector<Buffer<uint8_t>> outputs;
    for (int i=0; i<W; ++i) {
      outputs.emplace_back(Buffer<uint8_t>(imgsize_x, imgsize_y));
    }

    for (int w = 0; w < processor.output.dim(2).extent(); w++) {
      for (int y = 0; y < processor.output.dim(1).extent(); y++) {
        for (int x = 0; x < processor.output.dim(0).extent(); x++) {
          oned_output(x + y*imgsize_x, w) = processor.output(x,y,w);
          outputs[w](x,y) = processor.output(x,y,w);
          //std::cout << x << "," << y << "," << w << " = " << +processor.output(x,y,w) << " = " << std::hex << +processor.output(x,y,w) << std::dec << std::endl;
        } } }
    if (write_images) {//if (processor.output(0,0,0,0) == 198) {
      //save_image(oned_output, "bin/output_gold.png");
      save_image(processor.output, "bin/output_gold.png");
      for (size_t i=0; i<outputs.size(); ++i) {
        save_image(outputs[i], "bin/output_" + std::to_string(i) + ".png");
      }
    }

    
    return output;
}  
