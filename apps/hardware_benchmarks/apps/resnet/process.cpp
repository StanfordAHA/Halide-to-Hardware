#include <iostream>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "resnet.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "resnet_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("unet_resnet", {"input.png", "kernel.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        resnet(proc.inputs["input.png"], proc.inputs["kernel.png"], proc.output);
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
          resnet_clockwork(proc.inputs["input.png"], proc.inputs["kernel.png"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    int X = 30;
    int Y = 30;
    int K_X = 3;
    int K_Y = 3;
    //int Z = 8; // input channel
    int Z = 4;
    int W = 3; // output channel (should be 8)
  
    processor.inputs["input.png"] = Buffer<uint8_t>(Z, X, Y);
    auto input_copy_stencil = processor.inputs["input.png"];
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          input_copy_stencil(z, x, y) = z + x + y;
          //input_copy_stencil(z, x, y) = 1;
        } } }

    std::cout << "input has dims: " << processor.inputs["input.png"].dim(0).extent() << "x"
              << processor.inputs["input.png"].dim(1).extent() << "x"
              << processor.inputs["input.png"].dim(2).extent() << "\n";

  
    processor.inputs["kernel.png"] = Buffer<uint8_t>(Z, W, K_X, K_Y);
    auto kernel_copy_stencil = processor.inputs["kernel.png"];
    for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
        for (int w = 0; w < kernel_copy_stencil.dim(1).extent(); w++) {
          for (int z = 0; z < kernel_copy_stencil.dim(0).extent(); z++) {
            kernel_copy_stencil(z, w, x, y) = z + w + x + y;
            //kernel_copy_stencil(z, w, x, y) = 1;
          } } } }
  
    std::cout << "kernel has dims: " << processor.inputs["kernel.png"].dim(0).extent() << "x"
              << processor.inputs["kernel.png"].dim(1).extent() << "x"
              << processor.inputs["kernel.png"].dim(2).extent() << "x"
              << processor.inputs["kernel.png"].dim(3).extent() << "\n";
    processor.inputs_preset = true;
    processor.output = Buffer<uint8_t>(X - K_X + 1, Y - K_Y + 1, W);

    return processor.process_command(argc, argv);
}  

/*
#include <cstdio>

#include "resnet.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"
#include "clockwork_testscript.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
  int x = 28;
  int y = 28;
  int k_x = 3;
  int k_y = 3;
  int z = 8; // output channel
  int w = 16; // input channel

  ManyInOneOut_ProcessController<uint8_t> processor("resnet", {"input.png", "kernel.png"},
                                            {
                                              {"cpu",
                                                  [&]() { resnet(processor.inputs["input.png"], processor.inputs["kernel.png"], processor.output); }
                                              },
                                              {"rewrite",
                                                  [&]() { run_coreir_rewrite_on_interpreter<>("bin/design_top.json", "bin/ubuffers.json",
                                                                                       processor.inputs["input.png"], processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json",
                                                                                       processor.inputs["input.png"], processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"clockwork",
                                                  [&]() { run_clockwork_program<>(processor.inputs["input.png"], processor.inputs["kernel.png"], processor.output); }
                                                  //[&]() { run_clockwork_program<>(processor.inputs["input.png"], processor.output); }
                                              }
                                            });


  processor.inputs["input.png"] = Buffer<uint8_t>(z, x, y);
  auto input_copy_stencil = processor.inputs["input.png"];
  for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
    for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
      for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
        input_copy_stencil(z, x, y) = z + x + y;
        //input_copy_stencil(z, x, y) = 1;
  } } }

  std::cout << "input has dims: " << processor.inputs["input.png"].dim(0).extent() << "x"
            << processor.inputs["input.png"].dim(1).extent() << "x"
            << processor.inputs["input.png"].dim(2).extent() << "\n";

  
  processor.inputs["kernel.png"] = Buffer<uint8_t>(z, w, k_x, k_y);
  auto kernel_copy_stencil = processor.inputs["kernel.png"];
  for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
    for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
      for (int w = 0; w < kernel_copy_stencil.dim(1).extent(); w++) {
        for (int z = 0; z < kernel_copy_stencil.dim(0).extent(); z++) {
          kernel_copy_stencil(z, w, x, y) = z + w + x + y;
          //kernel_copy_stencil(z, w, x, y) = 1;
   } } } }
  
  std::cout << "kernel has dims: " << processor.inputs["kernel.png"].dim(0).extent() << "x"
            << processor.inputs["kernel.png"].dim(1).extent() << "x"
            << processor.inputs["kernel.png"].dim(2).extent() << "x"
            << processor.inputs["kernel.png"].dim(3).extent() << "\n";
  processor.inputs_preset = true;
  processor.output = Buffer<uint8_t>(x - k_x + 1, y - k_y + 1, w);

  return processor.process_command(argc, argv);
    
}

*/
