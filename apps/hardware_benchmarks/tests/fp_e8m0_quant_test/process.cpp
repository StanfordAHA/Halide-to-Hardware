#include <cstdio>
#include <math.h>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "fp_e8m0_quant_test.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "fp_e8m0_quant_test_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("fp_e8m0_quant_test", {"input", "e8m0"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        fp_e8m0_quant_test( proc.inputs["input"], proc.inputs["e8m0"], proc.output );
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
          fp_e8m0_quant_test_clockwork( proc.inputs["input"], proc.inputs["e8m0"], proc.output );
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
  processor.inputs["input"]   = Buffer<uint16_t>(64, 64);
  processor.inputs["e8m0"]   = Buffer<uint16_t>(64, 64);
  processor.output  = Buffer<uint16_t>(64, 64);

  // Initialize input buffer
  auto input_copy = processor.inputs["input"];
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      if (rand() % 100 < 10) {
        // Set some values to zero for corner cases
        input_copy(x, y) = 0;
      } else {
        float random_val = (static_cast<float>(rand()) / RAND_MAX) * 512.0f - 256.0f;
        input_copy(x, y) = float_to_bfloat16_process(random_val);
      }
    }
  }

  auto e8m0_copy = processor.inputs["e8m0"];
  for (int y = 0; y < 64; y++) {
    float abs_max = 0.0f;
    for (int x = 0; x < 64; x++) {
      if (abs(bfloat16_to_float_process(input_copy(x, y))) > abs_max) {
        abs_max = abs(bfloat16_to_float_process(input_copy(x, y)));
      }
    }
    for (int x = 0; x < 64; x++) {
      if (abs_max > 0.0f) {
        e8m0_copy(x, y) = uint16_t(floor(log2(abs_max))) + 127 - 6;
      } else {
        e8m0_copy(x, y) = uint16_t(127);
      }
    }
  }

  // Generate reference output based on the algorithm in fp_e8m0_quant_test_generator.cpp
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      processor.output(x, y) =
      static_cast<uint16_t>(
        static_cast<uint8_t>(
          static_cast<int8_t>(
            round(bfloat16_to_float_process(input_copy(x, y)) / float(pow(2, e8m0_copy(x, y) - 127)))
          )
        )
      );
    }
  }

  // Save buffer to raw
  save_halide_buffer_to_raw(processor.inputs["input"], "bin/hw_input_stencil.raw");
  save_halide_buffer_to_raw(processor.inputs["e8m0"], "bin/hw_e8m0_stencil.raw");
  save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

  //processor.tolerance = 1;
  std::cout << "Running with tolerance=" << processor.tolerance << std::endl;

  return processor.process_command(argc, argv);

}
