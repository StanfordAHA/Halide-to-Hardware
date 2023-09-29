#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "image_blend.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "image_blend_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("image_blend", {"in_apple.png", "in_orange.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        image_blend( proc.inputs["in_apple.png"], proc.inputs["in_orange.png"], proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["in_apple.png"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          image_blend_clockwork( proc.inputs["in_apple.png"], proc.inputs["in_orange.png"], proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  //int ksize = 23;
  int ksize = 1;

  processor.inputs["in_apple.png"]  = Buffer<uint8_t>(128, 128, 3);
  processor.inputs["in_orange.png"] = Buffer<uint8_t>(128, 128, 3);
  processor.output                  = Buffer<uint8_t>(128-ksize+1, 128-ksize+1, 3);
  
  auto cmd_output = processor.process_command(argc, argv);

  return cmd_output;
}
