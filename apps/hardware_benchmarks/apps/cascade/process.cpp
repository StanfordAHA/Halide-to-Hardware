#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "cascade.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "cascade_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("cascade");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        cascade( proc.input, proc.output );
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
          cascade_clockwork( proc.input, proc.output );
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
  auto schedule = env_sch ? atoi(env_sch) : 3;
  std::cout << "using schedule = " << schedule << std::endl;;

  int host_tiling_x, host_tiling_y, glb_tiling, output_tile_width, output_tile_height;
  switch (schedule) {
  case 3:
  case 50 ... 70:
    processor.inputs_preset = true;
    glb_tiling = 1;
    host_tiling_x = 9;  host_tiling_y = 5;
    output_tile_width = 666;
    output_tile_height = 799;
    break;
  default:
    processor.inputs_preset = false;
    host_tiling_x = 1;  host_tiling_y = 1;
    output_tile_width = 60;
    output_tile_height = 60;
    glb_tiling = 1;
    break;
  }

  int num_tiles_x        = host_tiling_x * glb_tiling;
  int num_tiles_y        = host_tiling_y * glb_tiling;
  int output_width       = num_tiles_x * output_tile_width;
  int output_height      = num_tiles_y * output_tile_height;

  std::cout << "Running with output size: " << output_width << "x" << output_height << std::endl;
  processor.input  = Buffer<uint8_t>(output_width+4, output_height+4);
  processor.output = Buffer<uint8_t>(output_width, output_height);

  if (schedule == 3 || schedule >= 50) {
    // load this 6000x4000 image
    std::cout << "Using a big tern image" << std::endl;
    processor.input = load_and_convert_image("../../images/tern_biggray.png");
  }
  
  return processor.process_command(argc, argv);
  
}
