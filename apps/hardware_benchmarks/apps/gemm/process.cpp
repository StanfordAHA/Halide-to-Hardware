#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "gemm.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "gemm_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<int16_t> processor("gemm", {"input.mat","kernel.met"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        gemm(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
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
          gemm_clockwork(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  auto env_width = getenv("imgsize");
  auto width = env_width ? atoi(env_width) : 64;
  (void) width;
      
  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["input.mat"] = Buffer<int16_t>(width, width);
  processor.inputs["kernel.mat"] = Buffer<int16_t>(width, width);
  processor.output  = Buffer<int16_t>(width, width);
  processor.inputs_preset = true;


  for (int y = 0; y < processor.output.dim(1).extent(); y++) {
    for (int x = 0; x < processor.output.dim(0).extent(); x++) {
      processor.inputs["input.mat"](x, y) = x*-3 + y*2;
    }
  }
  for (int y = 0; y < processor.output.dim(1).extent(); y++) {
    for (int x = 0; x < processor.output.dim(0).extent(); x++) {
      processor.inputs["kernel.mat"](x, y) = x*5 - y;
    }
  }

  
 return processor.process_command(argc, argv);
  
}
