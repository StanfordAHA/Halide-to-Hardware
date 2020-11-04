#include <cmath>
#include <iostream>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "hologram_wgs.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "hologram_wgs_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("hologram_wgs", {"x.png", "y.png", "z.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        hologram_wgs(proc.inputs["x.png"], proc.inputs["y.png"], proc.inputs["z.png"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["x.png"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          hologram_wgs_clockwork(proc.inputs["x.png"], proc.inputs["y.png"], proc.inputs["z.png"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    int num_traps = 100;
 
    processor.inputs["x.png"] = Buffer<float>(num_traps);
    processor.inputs["y.png"] = Buffer<float>(num_traps);
    processor.inputs["z.png"] = Buffer<float>(num_traps);
    auto input_x = processor.inputs["x.png"];
    auto input_y = processor.inputs["y.png"];
    auto input_z = processor.inputs["z.png"];
    for (int i = 0; i < num_traps; i++) {
        input_x(i) = (float(rand())/RAND_MAX-0.5)*100.0;
        input_y(i) = (float(rand())/RAND_MAX-0.5)*100.0;
        input_z(i) = (float(rand())/RAND_MAX-0.5)*10.0;
    }

    processor.inputs_preset = true;
    processor.output = Buffer<float>(512, 512);

    return processor.process_command(argc, argv);
}
