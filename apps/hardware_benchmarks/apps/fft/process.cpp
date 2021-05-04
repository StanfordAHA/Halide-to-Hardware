#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include <chrono>

#if defined(WITH_CPU)
   #include "fft.h"
#endif

// #if defined(WITH_COREIR)
    // #include "coreir_interpret.h"
// #endif

// #if defined(WITH_CLOCKWORK)
    // #include "rdai_api.h"
    // #include "clockwork_sim_platform.h"
    // #include "brighten_and_blur_clockwork.h"
// #endif

using namespace std;
using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<float> processor("fft", {"in1.png", "in2.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        fft(proc.inputs["in1.png"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          brighten_and_blur_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;
  processor.inputs["in1.png"] = Buffer<float>(16, 2);
  
  auto in1 = processor.inputs["in1.png"];

  for (int i = 0; i < 16; i++)
  {
	  in1(i, 0) = (float(rand()) / RAND_MAX - 0.5) * 2000;
	  in1(i, 1) = (float(rand()) / RAND_MAX - 0.5) * 2000;

	  cout << in1(i, 0) << "+" << in1(i, 1) << "i" << "   " << i << endl;
  }
  
  cout << endl << endl << endl;
  
  //for (int i = 0; i < 16; i++)
  //{
	//  in2(i, 0) = (float(rand()) / RAND_MAX - 0.5) * 2000;
	//  in2(i, 1) = (float(rand()) / RAND_MAX - 0.5) * 2000;
  //}
  
  processor.inputs_preset = true;
  processor.output = Buffer<float>(16, 2);
  
  
  
  auto start = chrono::steady_clock::now();
  auto out = processor.process_command(argc, argv);
  auto end = chrono::steady_clock::now();
  
  
  auto output = processor.output;
  for (int i = 0; i < 16; i++)
  {
	  cout << output(i, 0) << "+" << output(i, 1) << "i" << "   " << i << endl;
  }
  
  cout << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
  
  return out;
}
