#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "mobilenet.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "mobilenet_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint8_t> processor("mobilenet", {"input.png", "filter_dw.png", "filter_pw.png"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        mobilenet(proc.inputs["input.png"], proc.inputs["filter_dw.png"], proc.inputs["filter_pw.png"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["input.png"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          mobilenet_clockwork(proc.inputs["input.png"], proc.inputs["filter_dw.png"], proc.inputs["filter_pw.png"], proc.output);
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
    //  HALIDE_GEN_ARGS="in_img=28 pad=1 ksize=3 stride=1 k_ic=8 k_oc=8"

    auto OX = getenv("in_img");
    auto P = getenv("pad");
    auto KX = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("k_ic");
    auto OC = getenv("k_oc");

    auto in_img = OX ? atoi(OX) : 28;
    auto pad = P ? atoi(P) : 1;
    auto ksize = KX ? atoi(KX) : 3;
    auto stride = S ? atoi(S) : 1;
    auto k_ic = IC ? atoi(IC) : 4;
    auto k_oc = OC ? atoi(OC) : 3;

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int P_X = pad;
    int P_Y = P_X;
    int C = k_ic; // input channel 
    int K = k_oc; // output channel
  
    processor.inputs["input.png"] = Buffer<uint8_t>(C, X, Y);
    auto input_copy_stencil = processor.inputs["input.png"];
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
          input_copy_stencil(c, x, y) = c + x + y;
          //input_copy_stencil(z, x, y) = 1;
        } } }

    std::cout << "input has dims: " << processor.inputs["input.png"].dim(0).extent() << "x"
              << processor.inputs["input.png"].dim(1).extent() << "x"
              << processor.inputs["input.png"].dim(2).extent() << "\n";

  
    processor.inputs["filter_dw.png"] = Buffer<uint8_t>(C, K_X, K_Y);
    auto filter_dw_copy_stencil = processor.inputs["filter_dw.png"];
    for (int y = 0; y < filter_dw_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < filter_dw_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < filter_dw_copy_stencil.dim(0).extent(); c++) {
          //filter_dw_copy_stencil(x, y, c) = x + y + c;
          filter_dw_copy_stencil(c, x, y) = x + y + c;
          //filter_dw_copy_stencil(x, y, c) = 1;
        } } }

    std::cout << "filter_dw has dims: " << processor.inputs["filter_dw.png"].dim(0).extent() << "x"
              << processor.inputs["filter_dw.png"].dim(1).extent() << "x"
              << processor.inputs["filter_dw.png"].dim(2).extent() << "\n";
    
    processor.inputs["filter_pw.png"] = Buffer<uint8_t>(K, C);
    auto filter_pw_copy_stencil = processor.inputs["filter_pw.png"];
      for (int k = 0; k < filter_pw_copy_stencil.dim(0).extent(); k++) {
        for (int c = 0; c < filter_pw_copy_stencil.dim(1).extent(); c++) {
        filter_pw_copy_stencil(k, c) = k + c;
        std::cout << "k=" <<k << ", c=" << c << " := " << +filter_pw_copy_stencil(k, c) << std::endl;
        //filter_pw_copy_stencil(k, c) = 1;
      } }

    std::cout << "filter_pw has dims: " << processor.inputs["filter_pw.png"].dim(0).extent() << "x"
              << processor.inputs["filter_pw.png"].dim(1).extent() << "\n";
    
    processor.inputs_preset = true;

    bool gen_interleaved_input = true;
    if (gen_interleaved_input) {
      int input_size = 32*32;
      int num_inputs = 20;
      Buffer<uint8_t> input_interleaved(num_inputs, input_size);
      for (int i=0; i<num_inputs; ++i) {
        for (int j=0; j<input_size; ++j) {
          input_interleaved(i, j) = i*17 + 3*j;
        }
      }
      save_image(input_interleaved, "bin/input_interleaved.png");
    }
    
    int imgsize_x = std::floor( (X + 2*P_X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y + 2*P_Y - K_Y) / stride ) + 1;
    processor.output = Buffer<uint8_t>(K, imgsize_x, imgsize_y);

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";

    int return_value =  processor.process_command(argc, argv);
    for (int y=0; y<5; ++y) {
      for (int x=0; x<5; ++x) {
        for (int k=0; k<3; ++k) {
          std::cout << "hw_out(" << k << "," << x << "," << y << ") = "
                    << std::hex << +processor.output(k, x, y) << std::dec << std::endl;
        }
      }
    }

    bool write_output = true;
    if (write_output) {
      int output_size = imgsize_x*imgsize_y;
      int num_oc = K;
      Buffer<uint8_t> output_flat(num_oc, output_size);
      for (int i=0; i<imgsize_x; ++i) {
        for (int j=0; j<imgsize_y; ++j) {
          for (int k=0; k<num_oc; ++k) {
            output_flat(k, i+ imgsize_x*j) = processor.output(k, i, j);
          }
        }
      }
      save_image(output_flat, "bin/output_flat.png");
    }
    return return_value;
}  
