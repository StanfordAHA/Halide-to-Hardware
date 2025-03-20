#include <iostream>
#include <math.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstdint>
#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "hw_support_utils.h"

#if defined(WITH_CPU)
   #include "maxpooling.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "maxpooling_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("maxpooling", {"input_host_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        maxpooling(proc.inputs["input_host_stencil.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif

  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input_host_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif

  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          maxpooling_clockwork(proc.inputs["input_host_stencil.mat"], proc.output);
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

    auto in_img = OX ? atoi(OX) : 57;
    auto ksize = KX ? atoi(KX) : 3;
    auto stride = S ? atoi(S) : 2;
    auto n_ic = IC ? atoi(IC) : 56;

    int X = in_img;
    int Y = in_img;
    int K_X = ksize;
    int K_Y = K_X;
    int C = n_ic;

    // input image processor
    processor.inputs["input_host_stencil.mat"] = Halide::Runtime::Buffer<uint16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto input_copy_stencil = processor.inputs["input_host_stencil.mat"];
    int i=1; (void) i;
    int sparsity = 0;

    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
        for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
            for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
                if (rand() % 100 < sparsity) {
                    input_copy_stencil(c, x, y) = float_to_bfloat16_process(0.0f);
                } else {
                    input_copy_stencil(c, x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 6.0f);
                }
            }
        }
    }

    std::cout << "input has dims: " << processor.inputs["input_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(2).extent() << "\n";

    // output image processor
    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Halide::Runtime::Buffer<uint16_t>(C, imgsize_x, imgsize_y);

    // Max pooling operation
    for (int c = 0; c < C; c++) { // For each channel
        for (int x = 0; x < imgsize_x; x++) {
            for (int y = 0; y < imgsize_y; y++) {
                // std::cout << "c: " << c << " x: " << x << " y: " << y << " input: " << bfloat16_to_float_process(input_copy_stencil(c, x, y)) << std::endl;
                float max_val = -std::numeric_limits<float>::infinity();
                for (int kx = 0; kx < K_X; kx++) { // Kernel height
                    for (int ky = 0; ky < K_Y; ky++) { // Kernel width
                        int src_x = x * stride + kx;
                        int src_y = y * stride + ky;
                        if (src_x < X && src_y < Y) {
                            float val = bfloat16_to_float_process(input_copy_stencil(c, src_x, src_y));
                            max_val = std::max(max_val, val); // Keep the max value
                        }
                    }
                }
                processor.output(c, x, y) = float_to_bfloat16_process(max_val);
                // std::cout << "c: " << c << " x: " << x << " y: " << y << " output: " << bfloat16_to_float_process(processor.output(c, x, y)) << std::endl;
            }
        }
    }

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";

    bool write_mat = true;
    if (write_mat) {
      std::cout << "Writing input_host_stencil.mat to bin folder" << std::endl;
      save_image(processor.inputs["input_host_stencil.mat"], "bin/input_host_stencil.mat");

      std::cout << "Writing hw_output.mat to bin folder" << std::endl;
      save_image(processor.output, "bin/hw_output.mat");
    }

    return processor.process_command(argc, argv);
}
