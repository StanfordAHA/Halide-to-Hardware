#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "up_sample.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "up_sample_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("up_sample");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        up_sample( proc.input, proc.output );
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
          up_sample_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  processor.input   = Buffer<uint8_t>(64, 64);
  //processor.output  = Buffer<uint8_t>(128, 128);
  processor.output  = Buffer<uint8_t>(124, 124);

  auto return_value = processor.process_command(argc, argv);
  std::cout << "input is size " << processor.input.dim(1).extent() << " x "
            << processor.input.dim(0).extent() << std::endl;

  bool write_large_input = true;
  if (write_large_input) {
    Buffer<uint8_t> large_in(128, 128);
    for (int y=0; y<large_in.dim(1).extent(); ++y) {
      for (int x=0; x<large_in.dim(0).extent(); ++x) {
        if (y%2==0 && x%2==0) {
          large_in(x, y) = processor.input(x/2, y/2);
        } else {
          large_in(x, y) = 0;
        }
        //std::cout << "y=" << y << " x=" << x << " : " << +large_in(x, y) << std::endl;
      }
    }
    save_image(large_in, "bin/input_large.png");
  }

  return return_value;
}
