#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "mem_filter_test.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "mem_filter_test_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t> processor("mem_filter_test");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        mem_filter_test( proc.input, proc.output );
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
          mem_filter_test_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  auto in_img_x_fake = getenv("in_img_x_fake") ? atoi(getenv("in_img_x_fake")) : 64;
  auto in_img_y_fake = getenv("in_img_y_fake") ? atoi(getenv("in_img_y_fake")) : 32;
  auto in_img_x = getenv("in_img_x") ? atoi(getenv("in_img_x")) : 64;
  auto in_img_y = getenv("in_img_y") ? atoi(getenv("in_img_y")) : 32;
  auto write_stride = getenv("write_stride") ? atoi(getenv("write_stride")) : 4;

  // Create random input
  processor.input   = Buffer<uint16_t>(in_img_x_fake, in_img_y_fake);
  processor.output  = Buffer<uint16_t>(in_img_x_fake, in_img_y_fake-1);

  processor.inputs_preset = true;

  // Set the input image
  auto real_input = Buffer<uint16_t>(in_img_x, in_img_y);
  for (int y = 0; y < in_img_y; y++) {
    for (int x = 0; x < in_img_x; x++) {
      real_input(x, y) = (uint16_t)(rand() % 65536);
    }
  }

  // Set the real output which should have stride of 4
  int out_w = in_img_x / write_stride;
  auto real_output = Buffer<uint16_t>(out_w, in_img_y);

  for (int y = 0; y < in_img_y; y++) {
      for (int x = 0; x < out_w; x++) {
          real_output(x, y) = real_input(x * write_stride, y);
      }
  }

  save_halide_buffer_to_raw(real_input, "bin/hw_input_stencil.raw");
  save_halide_buffer_to_raw(real_output, "bin/hw_output.raw");

  return processor.process_command(argc, argv);

}
