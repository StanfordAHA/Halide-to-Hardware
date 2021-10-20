#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "glb_channel_reduction.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "glb_channel_reduction_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("glb_channel_reduction");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        glb_channel_reduction( proc.input, proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.input, proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          glb_channel_reduction_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  int numtiles = 2;
  int numchannels = 8;
  processor.input   = Buffer<uint8_t>(numchannels, 64*numtiles, 64*numtiles);
  processor.output  = Buffer<uint8_t>(64*numtiles,   64*numtiles);
  //processor.input   = Buffer<uint8_t>(514, 514);
  //processor.output  = Buffer<uint8_t>(512, 512);

  int i=0;
  for (int c = 0; c < processor.input.dim(0).extent(); c++) {
    for (int y = 0; y < processor.input.dim(2).extent(); y++) {
      for (int x = 0; x < processor.input.dim(1).extent(); x++) {
        processor.input(c, x, y) = i;
        i = i+1;
      } } }
  processor.inputs_preset = true;
  //save_image(processor.input, "bin/input.mat");
  
 return processor.process_command(argc, argv);
  
}
