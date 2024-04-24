#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "exposure_fusion.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "exposure_fusion_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("exposure_fusion");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        exposure_fusion( proc.input, proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          exposure_fusion_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  int ksize = 23;

  //processor.input = Buffer<uint8_t>(64, 64, 3);
  processor.input = Buffer<uint8_t>(1096, 1112, 3);
  processor.input = load_and_convert_image("../camera_pipeline_2x2/bin/output_cpu.png");
  //processor.output = Buffer<uint8_t>(64-ksize+1, 64-ksize+1, 3);
  processor.output = Buffer<uint8_t>(1096, 1112, 3);
  

  processor.inputs_preset = true;
  auto cmd_output = processor.process_command(argc, argv);

  return cmd_output;
}
