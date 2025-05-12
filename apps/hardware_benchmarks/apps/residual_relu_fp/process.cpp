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
   #include "residual_relu_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "residual_relu_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

nlohmann::json output_starting_json;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("residual_relu_fp", {"mu_hw_input_stencil.mat", "hw_residual_input_stencil.raw"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        residual_relu_fp(proc.inputs["mu_hw_input_stencil.mat"], proc.inputs["hw_residual_input_stencil.raw"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif

  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["mu_hw_input_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif

  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          residual_relu_fp_clockwork(proc.inputs["mu_hw_input_stencil.mat"], proc.inputs["hw_residual_input_stencil.raw"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  auto OX = getenv("out_img");
  auto OC = getenv("n_oc");

  auto out_img = OX ? atoi(OX) : 56;
  auto n_oc = OC ? atoi(OC) : 32;

  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["mu_hw_input_stencil.mat"]        = Buffer<uint16_t>(out_img, out_img);
  processor.inputs["hw_residual_input_stencil.raw" ]        = Buffer<uint16_t>(out_img, out_img);
  processor.output                                = Buffer<uint16_t>(out_img, out_img);

  processor.inputs_preset = true;

  // for (int y = 0; y < processor.inputs["mu_hw_input_stencil.mat"].dim(2).extent(); y++) {
    for (int y = 0; y < processor.inputs["mu_hw_input_stencil.mat"].dim(1).extent(); y++) {
      for (int x = 0; x < processor.inputs["mu_hw_input_stencil.mat"].dim(0).extent(); x++) {
        processor.inputs["mu_hw_input_stencil.mat"](x, y) = float_to_bfloat16_process(
          // [-7, 7]
          ((float)rand() / RAND_MAX) * 14.0 - 7.0
        );
      }
    }
  // }

  for (int y = 0; y < processor.inputs["hw_residual_input_stencil.raw"].dim(1).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_residual_input_stencil.raw"].dim(0).extent(); x++) {
        processor.inputs["hw_residual_input_stencil.raw"](x, y) = float_to_bfloat16_process(
          0.0
        );
      }
    }

  // Gold output
    for (int y = 0; y < processor.output.dim(1).extent(); y++) {
      for (int x = 0; x< processor.output.dim(0).extent(); x++) {
        float sum = bfloat16_to_float_process(processor.inputs["mu_hw_input_stencil.mat"](x, y)) + bfloat16_to_float_process(processor.inputs["hw_residual_input_stencil.raw"](x, y));
        float result = std::max(sum, 0.0f);
        processor.output(x, y) = float_to_bfloat16_process(result);
      }
    }

  std::cout << "Writing mu_hw_input_stencil.mat to bin folder" << std::endl;
  save_image(processor.inputs["mu_hw_input_stencil.mat"], "bin/mu_hw_input_stencil.mat");

  std::cout << "Writing hw_residual_input_stencil.raw to bin folder" << std::endl;
  saveHalideBufferToRawBigEndian(processor.inputs["hw_residual_input_stencil.raw"], "bin/hw_residual_input_stencil.raw");

  std::cout << "Writing output to bin folder" << std::endl;
  save_image(processor.output, "bin/hw_output.mat");

  return processor.process_command(argc, argv);

}
