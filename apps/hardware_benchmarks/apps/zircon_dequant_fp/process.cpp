#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "coreir.h"
#include "hw_support_utils.h"
#include "cstdlib"

#if defined(WITH_CPU)
   #include "zircon_dequant_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "zircon_dequant_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t> processor("zircon_dequant_fp");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        zircon_dequant_fp( proc.input, proc.output );
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
          zircon_dequant_fp_clockwork( proc.input, proc.output );
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


  auto DEQUANT_SCALE = getenv("DEQUANT_SCALE");
  const float dequant_scale = DEQUANT_SCALE ? atof(DEQUANT_SCALE) : 0.5f;
  printf("Using dequant_scale of %f\n", dequant_scale);

  processor.input   = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.output  = Buffer<uint16_t>(n_oc, out_img, out_img);

  // Fill input with random values
  for (int c = 0; c < processor.input.dim(0).extent(); c++) {
    for (int y = 0; y < processor.input.dim(2).extent(); y++) {
      for (int x = 0; x < processor.input.dim(1).extent(); x++) {
        processor.input(c, x, y) = float_to_bfloat16_process(
            // [-7, 7]
            ((float)rand() / RAND_MAX) * 14.0 - 7.0
          );
      }
    }
  }

  // Gold output
  for (int y = 0; y < processor.output.dim(2).extent(); y++) {
    for (int x = 0; x < processor.output.dim(1).extent(); x++) {
      for (int c = 0; c < processor.input.dim(0).extent(); c++) {
          float result = bfloat16_to_float_process(processor.input(c, x, y)) * dequant_scale;
          processor.output(c, x, y) = float_to_bfloat16_process(result);
        }
      }
    }

  std::cout << "Writing mu_hw_input_stencil.mat to bin folder" << std::endl;
  save_image(processor.input, "bin/mu_hw_input_stencil.mat");

  std::cout << "Writing output to bin folder" << std::endl;
  save_image(processor.output, "bin/hw_output.mat");

  return processor.process_command(argc, argv);

}
