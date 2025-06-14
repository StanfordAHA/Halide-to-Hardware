#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "coreir.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "relu_layer_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "relu_layer_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("relu_layer_fp", {"hw_input_stencil.mat", "hw_bias_stencil.raw"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        relu_layer_fp(proc.inputs["hw_input_stencil.mat"], proc.inputs["hw_bias_stencil.raw"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif

  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["hw_input_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif

  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          relu_layer_fp_clockwork(proc.inputs["hw_input_stencil.mat"], proc.inputs["hw_bias_stencil.raw"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  auto OX = getenv("out_img");
  auto OC = getenv("n_oc");

  auto out_img = OX ? atoi(OX) : 32;
  auto n_oc = OC ? atoi(OC) : 128;

  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["hw_input_stencil.mat"]        = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.inputs["hw_bias_stencil.raw" ]        = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.output                                = Buffer<uint16_t>(n_oc, out_img, out_img);

  processor.inputs_preset = true;

  for (int y = 0; y < processor.inputs["hw_input_stencil.mat"].dim(2).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_input_stencil.mat"].dim(1).extent(); x++) {
      for (int w = 0; w < processor.inputs["hw_input_stencil.mat"].dim(0).extent(); w++) {
        processor.inputs["hw_input_stencil.mat"](w, x, y) = float_to_bfloat16_process(
          // [-7, 7]
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  }

  for (int y = 0; y < processor.inputs["hw_bias_stencil.raw"].dim(2).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_bias_stencil.raw"].dim(1).extent(); x++) {
      for (int w = 0; w < processor.inputs["hw_bias_stencil.raw"].dim(0).extent(); w++) {
        processor.inputs["hw_bias_stencil.raw"](w, x, y) = float_to_bfloat16_process(
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  }

  // Gold output
  for (int w = 0; w < processor.output.dim(0).extent(); w++) {
    for (int x = 0; x < processor.output.dim(1).extent(); x++) {
      for (int y = 0; y < processor.output.dim(2).extent(); y++) {
        float sum = bfloat16_to_float_process(processor.inputs["hw_input_stencil.mat"](w, x, y)) + bfloat16_to_float_process(processor.inputs["hw_bias_stencil.raw"](w, x, y));
        float result =std::max(sum, 0.0f);
        processor.output(w, x, y) = float_to_bfloat16_process(result);
      }
    }
  }

  std::cout << "Writing hw_input_stencil.mat to bin folder" << std::endl;
  save_image(processor.inputs["hw_input_stencil.mat"], "bin/hw_input_stencil.mat");

  std::cout << "Writing hw_bias_stencil.raw to bin folder" << std::endl;
  saveHalideBufferToRawBigEndian(processor.inputs["hw_bias_stencil.raw"], "bin/hw_bias_stencil.raw");

  std::cout << "Writing output to bin folder" << std::endl;
  save_image(processor.output, "bin/hw_output.mat");

  return processor.process_command(argc, argv);

}
