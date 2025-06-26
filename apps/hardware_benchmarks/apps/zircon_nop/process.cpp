#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "zircon_nop.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "zircon_nop_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("zircon_nop");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        zircon_nop( proc.input, proc.output );
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
          zircon_nop_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs_preset = true;


  auto OX = getenv("out_img");
  auto OC = getenv("n_oc");
  auto out_img = OX ? atoi(OX) : 14;
  auto n_oc = OC ? atoi(OC) : 256;

  processor.input   = Buffer<uint8_t>(n_oc, out_img, out_img);
  processor.output  = Buffer<uint8_t>(n_oc, out_img, out_img);

  int max_rand = 256 - 1;
  // Fill input with random values
  for (int c = 0; c < processor.input.dim(0).extent(); c++) {
    for (int y = 0; y < processor.input.dim(2).extent(); y++) {
      for (int x = 0; x < processor.input.dim(1).extent(); x++) {
        processor.input(c, x, y) = (rand() % (max_rand));
      }
    }
  }

  save_image(processor.input, "bin/mu_hw_input_stencil.mat");

  return processor.process_command(argc, argv);

}
