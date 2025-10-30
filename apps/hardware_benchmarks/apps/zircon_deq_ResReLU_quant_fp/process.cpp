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
   #include "zircon_deq_ResReLU_quant_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "zircon_deq_ResReLU_quant_fp_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("zircon_deq_ResReLU_quant_fp", {"mu_hw_input_stencil.mat", "hw_residual_input_stencil.raw"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        zircon_deq_ResReLU_quant_fp( proc.inputs["mu_hw_input_stencil.mat"], proc.inputs["hw_residual_input_stencil.raw"], proc.output );
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
          zircon_deq_ResReLU_quant_fp_clockwork( proc.inputs["mu_hw_input_stencil.mat"], proc.inputs["hw_residual_input_stencil.raw"], proc.output );
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
  auto n_oc = OC ? atoi(OC) : 64;

  auto DEQUANT_SCALE = getenv("DEQUANT_SCALE");
  const float dequant_scale = DEQUANT_SCALE ? atof(DEQUANT_SCALE) : 0.5f;
  printf("Using dequant_scale of %f\n", dequant_scale);
  auto QUANT_SCALE = getenv("QUANT_SCALE");
  const float quant_scale = QUANT_SCALE ? atof(QUANT_SCALE) : 0.5f;
  printf("Using quant_scale of %f\n", quant_scale);

  processor.inputs["mu_hw_input_stencil.mat"]               = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.inputs["hw_residual_input_stencil.raw" ]        = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.output  = Buffer<uint16_t>(int(n_oc / 2), out_img, out_img);

  // Random input
  for (int c = 0; c < processor.inputs["mu_hw_input_stencil.mat"].dim(0).extent(); c++) {
    for (int y = 0; y < processor.inputs["mu_hw_input_stencil.mat"].dim(2).extent(); y++) {
      for (int x = 0; x < processor.inputs["mu_hw_input_stencil.mat"].dim(1).extent(); x++) {
        // Generate random integer in range [-128, 127]
        int random_int = (rand() % 256) - 128;
        processor.inputs["mu_hw_input_stencil.mat"](c, x, y) = float_to_bfloat16_process(static_cast<float>(random_int));
      }
    }
  }

  // Random residual input
  for (int c = 0; c < processor.inputs["hw_residual_input_stencil.raw"].dim(0).extent(); c++) {
    for (int y = 0; y < processor.inputs["hw_residual_input_stencil.raw"].dim(2).extent(); y++) {
      for (int x = 0; x < processor.inputs["hw_residual_input_stencil.raw"].dim(1).extent(); x++) {
        // Generate random integer in range [-128, 127]
        int random_int = (rand() % 256) - 128;
        processor.inputs["hw_residual_input_stencil.raw"](c, x, y) = float_to_bfloat16_process(static_cast<float>(random_int));
        // processor.inputs["hw_residual_input_stencil.raw"](c, x, y) = 0.0f;
      }
    }
  }

  // Gold output
  for (int y = 0; y < processor.output.dim(2).extent(); y++) {
    for (int x = 0; x < processor.output.dim(1).extent(); x++) {
      for (int c = 0; c < processor.inputs["mu_hw_input_stencil.mat"].dim(0).extent(); c++) {
          float sum = bfloat16_to_float_process(processor.inputs["mu_hw_input_stencil.mat"](c, x, y)) * dequant_scale + bfloat16_to_float_process(processor.inputs["hw_residual_input_stencil.raw"](c, x, y));
          float relu_output = std::max(sum, 0.0f);
          // multiply by quantize_scale and cast to int8 for all value
          int8_t result_int8 = static_cast<int8_t>(std::round(relu_output * quant_scale));
          // pack every two int8 channels into one 16-bit word
          processor.output(int(c / 2), x, y) = (c & 1) ? bit8_pack(result_int8, processor.output(int(c / 2), x, y)) : result_int8;
        }
      }
    }


  // Copy the pre-existing raw file into the bin folder for now
  int ret = system("cp hw_residual_input_stencil.raw bin/hw_residual_input_stencil.raw");
  if (ret != 0) {
    std::cerr << "Error: Failed to copy hw_residual_input_stencil.raw to bin folder. "
                 "hw_residual_input_stencil.raw should have been generated by /aha/voyager/scripts/aha_flow.parse_dnnLayer_tensors.py "
                 "(system call returned " << ret << ")" << std::endl;
    return 1;
  }
  std::cout << "Copying pre-existing hw_residual_input_stencil.raw to bin folder" << std::endl;

  std::cout << "Writing mu_hw_input_stencil.mat to bin folder" << std::endl;
  save_image(processor.inputs["mu_hw_input_stencil.mat"], "bin/mu_hw_input_stencil.mat");

  // std::cout << "Writing hw_residual_input_stencil.raw to bin folder" << std::endl;
  // save_halide_buffer_to_raw(processor.inputs["hw_residual_input_stencil.raw"], "bin/hw_residual_input_stencil.raw");


  std::cout << "Writing output to bin folder" << std::endl;
  save_image(processor.output, "bin/hw_output.mat");

  return processor.process_command(argc, argv);

}
