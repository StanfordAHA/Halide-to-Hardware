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

union {
  uint32_t val;
  float f;
} union_var;

uint16_t round_to_even_process(float a) {
  //uint32_t e = reinterpret_cast<uint32_t&>(a);
  union_var.f = a;
  uint32_t e = union_var.val;
  
  // round float to even, comment out this codeblock for truncation
  uint32_t half = 0x00008000;
  uint32_t sum = e + half;
  
  // check if bottom bits are all zero
  uint32_t mantissa_mask = 0x0000ffff;
  bool is_zeroed = (sum & mantissa_mask) == 0;
  
  // clear last bit (round even) on tie
  uint32_t clear_mask = ~( ((uint32_t)is_zeroed) << 16);
  e = sum & clear_mask;

  // clear bottom bits
  e = e >> 16;

  //return bfloat16_t::make_from_bits(float_to_bfloat16( expf(bfloat16_to_float(a.to_bits())) ));
  //return bfloat16_t::make_from_bits( (uint16_t)e );
  return (uint16_t)e;
}

// Similar routines for bfloat. It's somewhat simpler.
uint16_t float_to_bfloat16_process(float f) {
//    uint16_t ret[2];
//    memcpy(ret, &f, sizeof(float));
//    // Assume little-endian floats
//    return ret[1];
  return round_to_even_process(f);
}

float bfloat16_to_float_process(uint16_t b) {
    // Assume little-endian floats
    uint16_t bits[2] = {0, b};
    float ret;
    memcpy(&ret, bits, sizeof(float));
    return ret;
}

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  ManyInOneOut_ProcessController<uint16_t> processor("depthwise_conv", {"input_host_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        depthwise_conv(proc.inputs["input_host_stencil.mat"], proc.output);
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
          depthwise_conv_clockwork(proc.inputs["input_host_stencil.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

    // Add all defined functions
    processor.run_calls = functions;

    auto OX = getenv("in_img");
    auto KX = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");

    auto in_img = OX ? atoi(OX) : 40;
    auto ksize = KX ? atoi(KX) : 5;
    auto stride = S ? atoi(S) : 2;
    auto n_ic = IC ? atoi(IC) : 24;

    int X = in_img;
    int Y = in_img;
    int K_X = ksize;
    int K_Y = K_X;
    int C = n_ic;
  
    // input image
    processor.inputs["input_host_stencil.mat"] = Buffer<uint16_t>(C, X, Y);
    processor.inputs_preset = true;
    auto input_copy_stencil = processor.inputs["input_host_stencil.mat"];
    int sparsity=0;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int c = 0; c < input_copy_stencil.dim(0).extent(); c++) {
          if (rand() % 100 < sparsity) {
            input_copy_stencil(c, x, y) = 0;
          } else {
            input_copy_stencil(c, x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 6.0f);
          }
        }
      }
    }

    std::cout << "input has dims: " << processor.inputs["input_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(2).extent() << "\n";


    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Buffer<uint16_t>(C, imgsize_x, imgsize_y);

    // Kernel generation similar to the preload kernel in the Halide generator
    int block_size = ksize;
    float total_range = 2.0f; // From -1 to 1
    int total_elements = block_size * block_size;
    float step = total_range / (total_elements - 1);
    // Assuming the kernel buffer dimensions are (C, block_size, block_size)
    Buffer<uint16_t> kernel_stencil(C, block_size, block_size);
    // Populate the kernel buffer
    for (int c = 0; c < C; ++c) {
        for (int y = 0; y < block_size; ++y) {
            for (int x = 0; x < block_size; ++x) {
                float value = 0.0f + step * (y * block_size + x);
                kernel_stencil(c, x, y) = float_to_bfloat16_process(value);
            }
        }
    }
    // Depthwise Convolution Operation using the generated kernel
    for (int c = 0; c < C; ++c) {
        for (int y = 0; y < imgsize_y; ++y) {
            for (int x = 0; x < imgsize_x; ++x) {
                float sum = 0.0f;
                for (int ky = 0; ky < K_Y; ++ky) {
                    for (int kx = 0; kx < K_X; ++kx) {
                        int ix = x * stride + kx;
                        int iy = y * stride + ky;
                        if (ix < X && iy < Y) {
                            float input_val = bfloat16_to_float_process(input_copy_stencil(c, ix, iy));
                            float kernel_val = bfloat16_to_float_process(kernel_stencil(c, kx, ky)); // Use the generated kernel
                            sum += input_val * kernel_val;
                        }
                    }
                }
                processor.output(c, x, y) = float_to_bfloat16_process(sum);
            }
        }
    }

    std::cout << "output has dims: " << processor.output.dim(0).extent() << "x"
              << processor.output.dim(1).extent() << "x"
              << processor.output.dim(2).extent() << "\n";
    
    bool write_mat = true;
    if (write_mat) {
      std::cout << "Writing input_host_stencil.mat to bin folder" << std::endl;
      std::cout << "Writing hw_output.mat to bin folder" << std::endl;
      save_image(processor.inputs["input_host_stencil.mat"], "bin/input_host_stencil.mat");
      save_image(processor.output, "bin/hw_output.mat");
    }

    return processor.process_command(argc, argv);
}  
