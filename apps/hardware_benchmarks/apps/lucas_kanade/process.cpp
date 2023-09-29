#include <cmath>
#include <iostream>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "lucas_kanade.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "lucas_kanade_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t,uint8_t> processor("lucas_kanade", {"input0.png", "input1.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        lucas_kanade(proc.inputs["input0.png"], proc.inputs["input1.png"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input0.png"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          lucas_kanade_clockwork(proc.inputs["input0.png"], proc.inputs["input1.png"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    processor.inputs["input0.png"] = Buffer<uint8_t>(64,64);
    processor.inputs["input1.png"] = Buffer<uint8_t>(64,64);

    processor.inputs_preset = true;
    processor.inputs["input0.png"] = load_and_convert_image("input0.png");
    processor.inputs["input1.png"] = load_and_convert_image("input1.png");
    auto input = processor.inputs["input0.png"];
    std::cout << "input is: " << input.dim(0).extent() << " x " << input.dim(1).extent() << std::endl;
    processor.output = Buffer<uint8_t>(input.dim(0).extent(), input.dim(1).extent(), 3);

    // Below is a method that converts a color image to a gray image.
    Buffer<uint8_t> seagull = load_and_convert_image("../../images/seagull4.png");
    Buffer<uint8_t> new_seagull = Buffer<uint8_t>(128, 128);
    for (int y = 0; y < seagull.dim(1).extent(); y++) {
      for (int x = 0; x < seagull.dim(0).extent(); x++) {
        new_seagull(x, y) = (seagull(x, y, 0)*77 + 150*seagull(x, y, 1) + seagull(x, y, 2)*29) / 256;
      }
    }
    std::string output_filename = "bin/gray_seagull.png";
    convert_and_save_image(new_seagull, output_filename);
    std::cout << "gray bird written" << std::endl;
    

    return processor.process_command(argc, argv);
}
