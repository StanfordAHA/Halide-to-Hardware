#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "depthwise_conv.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "depthwise_conv_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("depthwise_conv", {"input.mat", "kernel.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        depthwise_conv(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          depthwise_conv_clockwork(proc.inputs["input.mat"], proc.inputs["kernel.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    // Set enviroment variable to set these:
    //  HALIDE_GEN_ARGS="in_img=28 pad=1 ksize=3 stride=1 n_ic=8 k_oc=8"

    auto OX = getenv("in_img");
    auto KX = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");

    auto in_img = OX ? atoi(OX) : 64;
    auto ksize = KX ? atoi(KX) : 3;
    auto stride = S ? atoi(S) : 1;
    auto n_ic = IC ? atoi(IC) : 4;

    int X = in_img;
    int Y = in_img;
    int K_X = ksize;
    int K_Y = K_X;
    int C = n_ic;
  
    // input image
    processor.inputs["input.mat"] = Buffer<uint8_t>(C, X, Y);
    processor.inputs_preset = true;
    auto input_copy_stencil = processor.inputs["input.mat"];
    int i=1;
    int max_rand = pow(2,8) - 1;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
          if (rand() % 100 < 60) { // 60% zero, else rand
            // input_copy_stencil(c, x, y) = 0;
            input_copy_stencil(c, x, y) = i;
          } else {
            // input_copy_stencil(c, x, y) = (rand() % (max_rand));
            input_copy_stencil(c, x, y) = i;
          }
        }
        i++;
      }
    }

    std::cout << "input has dims: " << processor.inputs["input.mat"].dim(0).extent() << "x"
              << processor.inputs["input.mat"].dim(1).extent() << "x"
              << processor.inputs["input.mat"].dim(2).extent() << "\n";

    // kernel weights
    processor.inputs["kernel.mat"] = Buffer<uint8_t>(C, K_X, K_Y);
    processor.inputs_preset = true;
    auto kernel_copy_stencil = processor.inputs["kernel.mat"];
    i=1;
    for (int y = 0; y < kernel_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < kernel_copy_stencil.dim(0).extent(); c++) {
          if (rand() % 100 < 60) { // 60% zero, else rand
            // kernel_copy_stencil(c, x, y) = 0;
            kernel_copy_stencil(c, x, y) = i;
          } else {
            // kernel_copy_stencil(c, x, y) = (rand() % (max_rand));
            kernel_copy_stencil(c, x, y) = i;
          }
        }
        i++;
      }
    }

    std::cout << "kernel has dims: " << processor.inputs["kernel.mat"].dim(0).extent() << "x"
              << processor.inputs["kernel.mat"].dim(1).extent() << "x"
              << processor.inputs["kernel.mat"].dim(2).extent() << "\n";

    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Buffer<uint8_t>(C, imgsize_x, imgsize_y);

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";
    
    bool write_mat = true;
    if (write_mat) {
      std::cout << "Writing input.mat to bin folder" << std::endl;
      std::cout << "Writing kernel.mat to bin folder" << std::endl;
      std::cout << "Writing output_gold.mat to bin folder" << std::endl;
      save_image(processor.inputs["input.mat"], "bin/input.mat");
      save_image(processor.inputs["kernel.mat"], "bin/kernel.mat");
      save_image(processor.output, "bin/output_gold.mat");
    }

    return processor.process_command(argc, argv);
}  
