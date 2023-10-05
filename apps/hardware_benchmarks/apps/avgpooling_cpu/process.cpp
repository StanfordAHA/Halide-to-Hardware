#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "avgpooling_cpu.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "avgpooling_cpu_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<int16_t> processor("avgpooling_cpu", {"input.mat", "filter_avg.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        avgpooling_cpu(proc.inputs["input.mat"], proc.inputs["filter_avg.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input.mat"], proc.inputs["filter_avg.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          avgpooling_cpu_clockwork(proc.inputs["input.mat"], proc.inputs["filter_avg.mat"], proc.output);
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

    auto in_img = OX ? atoi(OX) : 7;
    auto ksize = KX ? atoi(KX) : 7;
    auto stride = S ? atoi(S) : 7;
    auto n_ic = IC ? atoi(IC) : 512;

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int C = n_ic;
  
    // input.mat loading
    processor.inputs["input.mat"] = Buffer<int16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto input_copy_stencil = processor.inputs["input.mat"];
    int i=1; (void) i;
    int max_rand = pow(2,8) - 1;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
          if (rand() % 100 < 60) { // 60% zero, else rand
            input_copy_stencil(c, x, y) = 0;
          } else {
            input_copy_stencil(c, x, y) = (rand() % (max_rand));
          }
        } } }

    std::cout << "input has dims: " << processor.inputs["input.mat"].dim(0).extent() << "x"
              << processor.inputs["input.mat"].dim(1).extent() << "x"
              << processor.inputs["input.mat"].dim(2).extent() << "\n";

    
    // filter_avg.mat loading
    processor.inputs["filter_avg.mat"] = Buffer<int16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto avgpooling_copy_stencil = processor.inputs["filter_avg.mat"];
    for (int y = 0; y < avgpooling_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < avgpooling_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < avgpooling_copy_stencil.dim(0).extent(); c++) {
          avgpooling_copy_stencil(c, x, y) = 1;
        } } }

    std::cout << "filter_avg has dims: " << processor.inputs["filter_avg.mat"].dim(0).extent() << "x"
              << processor.inputs["filter_avg.mat"].dim(1).extent() << "x"
              << processor.inputs["filter_avg.mat"].dim(2).extent() << "\n";

    // output.mat
    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Buffer<int16_t>(C, imgsize_x, imgsize_y);

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";
    
    bool write_mat = true;
    if (write_mat) {
      std::cout << "Writing input.mat to bin folder" << std::endl;
      save_image(processor.inputs["input.mat"], "bin/input.mat");
      save_image(processor.inputs["filter_avg.mat"], "bin/filter_avg.mat");
    }

    return processor.process_command(argc, argv);
}  
