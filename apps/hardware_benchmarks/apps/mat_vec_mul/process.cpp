#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "mat_vec_mul.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "mat_vec_mul_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<int16_t> processor("mat_vec_mul", {"matrix.mat", "vector.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        mat_vec_mul(proc.inputs["matrix.mat"], proc.inputs["vector.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["matrix.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          mat_vec_mul_clockwork(proc.inputs["matrix.mat"], proc.inputs["vector.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  const int X = 32;
  const int Y = 32;
      
  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["matrix.mat"]        = Buffer<int16_t>(X, Y);
  processor.inputs["vector.mat"]        = Buffer<int16_t>(X);
  processor.output                      = Buffer<int16_t>(X);

  processor.inputs_preset = true;
  
  for (int y = 0; y < processor.inputs["matrix.mat"].dim(1).extent(); y++) {
    for (int x = 0; x < processor.inputs["matrix.mat"].dim(0).extent(); x++) { 
      
      processor.inputs["matrix.mat"](x, y) = x;
      
    }

      processor.inputs["vector.mat"](y) = 1;
  }
  
  // save_image(processor.inputs["matrix.mat"], "bin/matrix.mat");
  // save_image(processor.inputs["vector.mat"], "bin/vector.mat");

  return processor.process_command(argc, argv);
  
}
