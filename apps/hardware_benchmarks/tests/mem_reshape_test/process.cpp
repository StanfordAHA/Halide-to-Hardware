#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "mem_reshape_test.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "mem_reshape_test_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t> processor("mem_reshape_test");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        mem_reshape_test( proc.input, proc.output );
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
          mem_reshape_test_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  auto in_img = getenv("in_img") ? atoi(getenv("in_img")) : 64;

  // Create random input
  processor.input   = Buffer<uint16_t>(in_img, in_img);
  processor.output  = Buffer<uint16_t>(in_img, in_img-1);

  processor.inputs_preset = true;

  // Set the input image
  for (int y = 0; y < processor.input.dim(1).extent(); y++) {
    for (int x = 0; x < processor.input.dim(0).extent(); x++) {
      processor.input(x, y) = (uint16_t)(rand() % 65536);
    }
  }

  // Set the real output which should be the input size
  auto real_output = Buffer<uint16_t>(in_img, in_img);
  for (int y = 0; y < processor.input.dim(1).extent(); y++) {
    for (int x = 0; x < processor.input.dim(0).extent(); x++) {
      real_output(x, y) = processor.input(y, x);
    }
  }

  saveHalideBufferToRawBigEndian(processor.input, "bin/hw_input_stencil.raw");
  saveHalideBufferToRawBigEndian(real_output, "bin/hw_output.raw");

  return processor.process_command(argc, argv);

}
