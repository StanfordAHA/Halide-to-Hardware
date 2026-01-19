#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "pe_mem_flush_test.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "pe_mem_flush_test_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t> processor("pe_mem_flush_test");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        pe_mem_flush_test( proc.input, proc.output );
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
          pe_mem_flush_test_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  auto input_width_env = getenv("input_width");
  auto input_height_env = getenv("input_height");
  auto input_width = input_width_env ? atoi(input_width_env) : 64;
  auto input_height = input_height_env ? atoi(input_height_env) : 128;

  std::cout << "using inputs set within process.cpp" << std::endl;
  processor.inputs_preset = true;

  processor.input   = Buffer<uint16_t>(input_width, input_height);
  processor.output  = Buffer<uint16_t>(input_width, input_height);

  // Generate random uint16 values for input
  for (int y = 0; y < input_height; y++) {
    for (int x = 0; x < input_width; x++) {
      processor.input(x, y) = (uint16_t)(rand() % 65536);
    }
  }

  // Generate output by copying input
  for (int y = 0; y < input_height; y++) {
    for (int x = 0; x < input_width; x++) {
      processor.output(x, y) = processor.input(x, y);
    }
  }

  int result = processor.process_command(argc, argv);

  // Save input and output buffers to raw files
  save_halide_buffer_to_raw(processor.input, "bin/hw_input_stencil.raw");
  save_halide_buffer_to_raw(processor.output, "bin/hw_output.raw");

  // Create glb bank config
  // inputs, outputs, mu_inputs
  using namespace glb_cfg;
  const config_spec spec = {
    {
      tensor_spec{"hw_input_stencil", {"x_coord"}},
    },
    {
      tensor_spec{"hw_output", {"x_coord"}}
    },
    {}
  };
  write_glb_bank_config(spec);

  return result;

}
