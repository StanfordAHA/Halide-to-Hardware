#include <cstdio>
#include <math.h>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "fp_arith.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "fp_arith_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t> processor("fp_arith");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        fp_arith( proc.input, proc.output );
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
          fp_arith_clockwork( proc.input, proc.output );
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
  processor.input   = Buffer<uint16_t>(64, 64);
  processor.output  = Buffer<uint16_t>(64, 64);

  // Initialize input buffer with random values in range [-255.0, 255.0]
  auto input_copy = processor.input;
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      float random_val = (static_cast<float>(rand()) / RAND_MAX) * 510.0f - 255.0f;
      input_copy(x, y) = float_to_bfloat16_process(random_val);
    }
  }


  // Generate reference output based on the algorithm in fp_arith_generator.cpp
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      // Convert input to float for calculations
      float input_val = bfloat16_to_float_process(input_copy(x, y));

      // Apply the algorithm operations
      float mult = input_val * 0.02f;
      float sub = mult - 1.0f;
      float expo = expf(sub);

      float mult2 = input_val * 0.1f;

      float absolute_max = (fabsf(expo) > fabsf(mult2)) ? fabsf(expo) : fabsf(mult2);

      float add = absolute_max + 3.0f;
      float div = 20.0f / add;
      float sub2 = div - add;
      float mult3 = sub2 * 5.0f;

      // Store the result in output buffer
      processor.output(x, y) = float_to_bfloat16_process(mult3);
    }
  }

  // Save buffer to raw
  saveHalideBufferToRawBigEndian(processor.input, "bin/hw_input_stencil.raw");
  saveHalideBufferToRawBigEndian(processor.output, "bin/hw_output.raw");

  //processor.tolerance = 1;
  std::cout << "Running with tolerance=" << processor.tolerance << std::endl;

  return processor.process_command(argc, argv);

}
