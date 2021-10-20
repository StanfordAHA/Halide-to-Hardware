#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "camera_pipeline.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "camera_pipeline_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint16_t, uint8_t> processor("camera_pipeline");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        camera_pipeline( proc.input, proc.output );
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
          camera_pipeline_clockwork( proc.input, proc.output );
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  // Add all defined functions
  processor.run_calls = functions;

  int ksize = 9;
  //int ksize = 5;
  //processor.input   = Buffer<uint8_t>(64, 64);
  //processor.input   = Buffer<uint16_t>(90, 90);
  //processor.output  = Buffer<uint8_t>(64, 64, 3);
  processor.input   = Buffer<uint16_t>(64, 64);
  processor.output  = Buffer<uint8_t>(64-ksize+1, 64-ksize+1, 3);

  //Halide::Runtime::Buffer<uint16_t> output(64, 64, 3);
  //Halide::Runtime::Buffer<uint8_t> image(64, 64, 3);
  //image = load_and_convert_image("bison_bayer.png");
  //for (int y=0; y<64; ++y) {
  //  for (int x=0; x<64; ++x) {
  //    if (image(x, y, 0) != 0) {
  //      assert(image(x, y, 1) == 0 && image(x, y, 2) == 0);
  //      output(x, y) = image(x, y, 0);
  //
  //    } else if (image(x, y, 1) != 0) {
  //      assert(image(x, y, 0) == 0 && image(x, y, 2) == 0);
  //      output(x, y) = image(x, y, 1);
  //
  //    } else {
  //      assert(image(x, y, 0) == 0 && image(x, y, 1) == 0);
  //      output(x, y) = image(x, y, 2);
  //
  //    }
  //  }
  //}
  //std::cout << "fist pixel = " << +image(0,0,0) << "," << +image(0,0,1) << "," << +image(0,0,2) << std::endl
  //          << "sec  pixel = " << +image(0,1,0) << "," << +image(0,1,1) << "," << +image(0,1,2) << std::endl;
  //save_image(output, "bin/bison_bayer.png");
  
  auto cmd_output = processor.process_command(argc, argv);

  //Halide::Runtime::Buffer<uint8_t> output8(64, 64, 3);
  //for (int y=0; y<64; ++y) {
  //  for (int x=0; x<64; ++x) {
  //    for (int c=0; c<3; ++c) {
  //      output8(x, y, c) = processor.output(x, y, c) % 256;
  //      //std::cout << "x=" << x << ",y=" << y << ",c=" << c << " : " << +(processor.output(x,y,c)) << std::endl;
  //    }
  //  }
  //}
  //convert_and_save_image(output8, "bin/output64.png");
  
  //for (int c=0; c<3; ++c) {
  //  Buffer<uint8_t> output2d(64-ksize+1, 64-ksize+1);
  //  for (int y=0; y<64-ksize+1; ++y) {
  //    for (int x=0; x<64-ksize+1; ++x) {
  //      output2d(x, y) = processor.output(x, y, c);
  //    }
  //  }
  //  std::string filename = "bin/output2d_" + std::to_string(c) + ".png";
  //  convert_and_save_image(output2d, filename);
  //}
  //
  //std::cout << std::hex;
  //for (int y=0; y<3; ++y) {
  //  for (int x=0; x<10; ++x) {
  //    std::cout << "input(" << y << "," << x << ") is 0x" << +processor.input(x, y) << std::endl;
  //  }
  //}
  //std::cout << std::dec << std::endl;

  //// Print the output bytes to cout
  //std::cout << std::hex;
  //for (int y=0; y<3; ++y) {
  //  for (int x=0; x<10; ++x) {
  //    std::cout << "denoised(" << y << "," << x << ") is ";
  //    for (int c=0; c<1; ++c) {
  //      std::cout << (c!=0 ? "," : "") << +processor.output(x, y, c);
  //    }
  //    std::cout << std::endl;
  //  }
  //}
  //std::cout << std::dec << std::endl;

  int max = 0;
  for (int y=0; y<64; ++y) {
    for (int x=0; x<64; ++x) {
      max = max < processor.input(x, y) ? processor.input(x, y) : max;
    }
  }
  //std::cout << "max is " << max << std::endl;
  //std::cout << "output(0,0) is " << std::hex
  //          << +processor.output(0, 0, 0) << ","
  //          << +processor.output(0, 0, 1) << ","
  //          << +processor.output(0, 0, 2) << std::dec << std::endl;


  return cmd_output;
}
