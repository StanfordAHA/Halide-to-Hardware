#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "demosaic_complex.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "demosaic_complex_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("demosaic_complex");
  //OneInOneOut_ProcessController<uint16_t> processor("demosaic_complex");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        demosaic_complex( proc.input, proc.output );
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
          demosaic_complex_clockwork( proc.input, proc.output );
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
  processor.output  = Buffer<uint8_t>(62, 62, 3);
  //processor.output  = Buffer<uint8_t>(3, 62, 62);
  

  auto ret_value = processor.process_command(argc, argv);
  //std::cout << "input:" << std::endl;
  //for (int y=0; y<5; ++y) {
  //  for (int x=0; x<8; ++x) {
  //    std::cout << "y=" << y << ",x=" << x << " : " << std::hex << +processor.input(x, y) << std::endl;
  //  }
  //}
  //
  //std::cout << "output:" << std::endl;
  //for (int y=0; y<3; ++y) {
  //  for (int x=0; x<6; ++x) {
  //    std::cout << "y=" << y << ",x=" << x << " : " << std::hex << +processor.output(x, y, 0) << std::endl;
  //  }
  //}

  bool save_output = false;
  bool save_separate_output = true;
  if (save_output) {
    Buffer<uint8_t> twod_output(3, 62*62);
    for (int y=0; y<62; ++y) {
      for (int x=0; x<62; ++x) {
        for (int c=0; c<3; ++c) {
          twod_output(c, x + y*62) = processor.output(c, x, y);
        }
      }
    }
    save_image(twod_output, "bin/linear_output.png");
    
  } else if (save_separate_output) {
    for (int c=0; c<3; ++c) {
      Buffer<uint8_t> channel_output(62, 62);
      for (int y=0; y<62; ++y) {
        for (int x=0; x<62; ++x) {
          channel_output(x, y) = processor.output(x, y, c);
        }
      }
      std::string filename = "bin/output" + std::to_string(c) + ".png";
      save_image(channel_output, filename);
    }
  }

  return ret_value;
  //return processor.process_command(argc, argv);
  
}
