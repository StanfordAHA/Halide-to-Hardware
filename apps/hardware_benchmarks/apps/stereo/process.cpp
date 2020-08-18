#include <iostream>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "stereo.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "stereo_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("stereo", {"left.png", "right.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        stereo(proc.inputs["left.png"], proc.inputs["right.png"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["left.png"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          stereo_clockwork(proc.inputs["left.png"], proc.inputs["right.png"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    int R_X = 128;
    int R_Y = 128;
    int L_X = 128;
    int L_Y = 128;
  
    processor.inputs["right.png"] = Buffer<uint8_t>(R_X, R_Y);
    // auto right_copy_stencil = processor.inputs["right.png"];
    // for (int y = 0; y < right_copy_stencil.dim(1).extent(); y++) {
    //   for (int x = 0; x < right_copy_stencil.dim(0).extent(); x++) {
    //       right_copy_stencil(x, y) = x + y;
    //       //input_copy_stencil(z, x, y) = 1;
    //     } }

    // std::cout << "right has dims: " << processor.inputs["right.png"].dim(0).extent() << "x"
    //           << processor.inputs["right.png"].dim(1).extent() << "\n";

  
    processor.inputs["left.png"] = Buffer<uint8_t>(L_X, L_Y);
    // auto left_copy_stencil = processor.inputs["left.png"];
    // for (int y = 0; y < left_copy_stencil.dim(1).extent(); y++) {
    //   for (int x = 0; x < left_copy_stencil.dim(0).extent(); x++) {
    //         left_copy_stencil(x, y) = x + y;
    //         //kernel_copy_stencil(z, w, x, y) = 1;
    //       } }
  
    // std::cout << "left has dims: " << processor.inputs["left.png"].dim(0).extent() << "x"
    //           << processor.inputs["left.png"].dim(1).extent() << "\n";
    // processor.inputs_preset = true;
    processor.output = Buffer<uint8_t>(128,128);
  
  return processor.process_command(argc, argv);
  
}
