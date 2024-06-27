#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "dirent.h"
#include <iostream>
#include <fstream>
#include <sstream>
//#include <libraw/libraw.h>
//#include <opencv4/opencv2/opencv.hpp>




#if defined(WITH_CPU)
   #include "hdr_plus_kernel_1.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "hdr_plus_kernel_1_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;
using std::string;
using std::vector;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<int16_t, uint8_t> processor("hdr_plus_kernel_1");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        hdr_plus_kernel_1( proc.input, proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          hdr_plus_kernel_1_clockwork( proc.input, proc.output );
          printf( "[RUN_INFO] finished running hdr_plus_kernel_1_clockwork\n" );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;


  // Input is alignPyramid[4]
  int im_width = 3;
  int im_height = 3;
  int num_frames = 3; 

  // Dims are tx, ty, xy, n
  //processor.input = Buffer<int16_t>(im_width, im_height, 2, num_frames);
  processor.input = Buffer<int16_t>(im_width, im_height, num_frames);


  for (int x = 0; x < im_width; x++) {
    for (int y = 0; y < im_height; y++){
      //for (int xy = 0; xy < 2; xy++){
        for (int n = 0; n < num_frames; n++){
          //processor.input(x, y, xy, n) = int16_t(0);
          processor.input(x, y, n) = int16_t(0);
        }
      //}
    }
  }

 
  processor.inputs_preset = true;
  printf("Successfully load input images!\n");

   processor.output = Buffer<uint8_t>(5, 5, 3);
  //processor.output = Buffer<uint8_t>(32, 32, 3);


  auto cmd_output = processor.process_command(argc, argv);
  printf("Ran process command!");

  return cmd_output;
}
