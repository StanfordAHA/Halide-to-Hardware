#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "pyramid_blur.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "pyramid_blur_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("pyramid_blur");
  //OneInOneOut_ProcessController<uint16_t> processor("pyramid_blur");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        pyramid_blur( proc.input, proc.output );
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
          pyramid_blur_clockwork( proc.input, proc.output );
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
  auto schedule = env_sch ? atoi(env_sch) : 0;
  std::cout << "using scheudle = " << schedule << std::endl;

  //int input_width  = 1242;
  int host_tiling, glb_tiling;
  switch (schedule) {
  case 1:
    host_tiling = 5;
    glb_tiling = 10;
    break;
  case 2:
    host_tiling = 1;
    glb_tiling = 1;
    break;
  default:
    host_tiling = 1;
    glb_tiling = 1;
    break;
  }

  int num_tiles          = host_tiling * glb_tiling;
  int output_tile_width  = 8;
  int output_tile_height = output_tile_width;
  int output_width       = num_tiles * output_tile_width;
  int output_height      = num_tiles * output_tile_height;

  std::cout << "Running with output size: " << output_width << "x" << output_height << std::endl;
  processor.input  = Buffer<uint8_t>(output_width*2*2*2, output_height*2*2*2);
  processor.output = Buffer<uint8_t>(output_width, output_height);
  
  processor.inputs_preset = true;
  for (int y = 0; y < processor.input.dim(1).extent(); y++) {
      for (int x = 0; x < processor.input.dim(0).extent(); x++) {
        processor.input(x, y) = x + y;
      }
  }
        
  
  //processor.input   = Buffer<uint16_t>(64, 64);
  //processor.output  = Buffer<uint16_t>(62, 62);
  
  return processor.process_command(argc, argv);
  
}
