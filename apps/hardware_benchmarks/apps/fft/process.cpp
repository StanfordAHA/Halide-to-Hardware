#include <cstdio>
#include <chrono>
#include <math.h>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "fft.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "fft_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t> processor("fft");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        fft(proc.input, proc.output);
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
          fft_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;
  processor.input = Buffer<uint16_t>(16, 2);

  for (int i = 0; i < 16; i++)
  {
	  processor.input(i, 0) = rand() % int(pow(2, 16)) - 1;
	  processor.input(i, 1) = rand() % int(pow(2, 16)) - 1;

	  std::cout << processor.input(i, 0) << "+" << processor.input(i, 1) << "j" << "   " << i << std::endl;
  }
  
  std::cout << std::endl << std::endl << std::endl;
  
  processor.inputs_preset = true;
  processor.output = Buffer<uint16_t>(16, 2);
  
  
  auto start = std::chrono::steady_clock::now();
  auto out = processor.process_command(argc, argv);
  auto end = std::chrono::steady_clock::now();
  
  
  auto output = processor.output;
  for (int i = 0; i < 16; i++)
  {
	  std::cout << output(i, 0) << "+" << output(i, 1) << "j" << "   " << i << std::endl;
  }
  
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
  
  return out;
}
