#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "lensblur.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "lensblur_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("lensblur");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        lensblur( proc.input, proc.output );
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
          lensblur_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  auto env_sch = getenv("schedule");
  auto env_width = getenv("mywidth");
  auto schedule = env_sch ? atoi(env_sch) : 3;
  auto width = env_width ? atoi(env_width) : 122;
  (void) width;
  std::cout << "using schedule = " << schedule << std::endl;


  int output_width       = 192;
  int output_height      = 320;


  std::cout << "Running with output size: " << output_width << "x" << output_height << std::endl;
  processor.input  = Buffer<uint8_t>(output_width, output_height, 3);
  processor.output = Buffer<uint8_t>(output_width, output_height, 3);
  
  if (schedule == 2 || schedule == 3) {
    // load this 1536x2560 image
    std::cout << "Using a small parrot image" << std::endl;
    processor.input = load_and_convert_image("../../../images/rgb_small.png");
    
  } else {
    for (int y = 0; y < processor.input.dim(1).extent(); y++) {
      for (int x = 0; x < processor.input.dim(0).extent(); x++) {
        for (int c = 0; c < processor.input.dim(2).extent(); c++) {
          processor.input(x, y, c) = x + y + 30*c;
        }
      }
    }
  }
  
  return processor.process_command(argc, argv);
}
