#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "conv2D_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "conv2D_fp_clockwork.h"
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
  ManyInOneOut_ProcessController<uint16_t> processor("conv2D_fp", {"input_host_stencil.mat", "kernel_host_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        conv2D_fp(proc.inputs["input_host_stencil.mat"], proc.inputs["kernel_host_stencil.mat"], proc.output);
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
          conv2D_fp_clockwork(proc.inputs["input_host_stencil.mat"], proc.inputs["kernel_host_stencil.mat"], proc.output);
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
    auto K = getenv("ksize");
    auto S = getenv("stride");
    auto IC = getenv("n_ic");
    auto OC = getenv("n_oc");

    auto in_img = OX ? atoi(OX) : 56;
    auto ksize = K ? atoi(K) : 3;
    auto stride = S ? atoi(S) : 1;
    auto n_ic = IC ? atoi(IC) : 16;
    auto n_oc = OC ? atoi(OC) : 8;

    int X = in_img;
    int Y = X;
    int K_X = ksize;
    int K_Y = K_X;
    int Z = n_ic; // input channel 
    int W = n_oc; // output channel

    if (true) {//OX || P || K || S || IC || OC) {
      std::cout << "using inputs set within process.cpp" << std::endl;
      processor.inputs_preset = true;
    } else {
      std::cout << "reading input_host_stencil.mat and kernel_host_stencil.mat" << std::endl;
      processor.inputs_preset = false;
    }
    
    ///// INPUT IMAGE /////
    processor.inputs["input_host_stencil.mat"] = Halide::Runtime::Buffer<uint16_t>(Z, X, Y);
    auto input_copy_stencil = processor.inputs["input_host_stencil.mat"];
    int in_sparsity = 0;
    for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
      for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
        for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
          if (rand() % 100 < in_sparsity) {
            input_copy_stencil(z, x, y) = float_to_bfloat16_process(0.0f);
          } else {
            input_copy_stencil(z, x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 6.0f);
          }
          // std::cout << "input: " << " z: " << z << " x: " << x << " y: " << y << " val: " << bfloat16_to_float_process(input_copy_stencil(z, x, y)) << std::endl;
        }
      }
    }

    std::cout << "input has dims: " << processor.inputs["input_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["input_host_stencil.mat"].dim(2).extent() << "\n";

    ///// KERNEL WEIGHTS /////  
    processor.inputs["kernel_host_stencil.mat"] = Halide::Runtime::Buffer<uint16_t>(W, Z, K_X, K_Y);
    auto kernel_copy_stencil = processor.inputs["kernel_host_stencil.mat"];
    int kernel_sparsity = 0;
    for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
      for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
        for (int z = 0; z < kernel_copy_stencil.dim(1).extent(); z++) {
          for (int w = 0; w < kernel_copy_stencil.dim(0).extent(); w++) {
            if (rand() % 100 < kernel_sparsity) {
              kernel_copy_stencil(w, z, x, y) = float_to_bfloat16_process(0.0f);
            } else {
              kernel_copy_stencil(w, z, x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX));
            }
            // std::cout << "kernel: " << " w: " << w << " z: " << z << " x: " << x << " y: " << y << " val: " << bfloat16_to_float_process(kernel_copy_stencil(w, z, x, y)) << std::endl;
          } 
        } 
      } 
    }

    std::cout << "kernel has dims: " << processor.inputs["kernel_host_stencil.mat"].dim(0).extent() << "x"
              << processor.inputs["kernel_host_stencil.mat"].dim(1).extent() << "x"
              << processor.inputs["kernel_host_stencil.mat"].dim(2).extent() << "x"
              << processor.inputs["kernel_host_stencil.mat"].dim(3).extent() << "\n";

    ///// GOLD OUTPUTS /////
    int imgsize_x = std::floor( (X - K_X) / stride ) + 1;
    int imgsize_y = std::floor( (Y - K_Y) / stride ) + 1;
    processor.output = Halide::Runtime::Buffer<uint16_t>(W, imgsize_x, imgsize_y);

    // Conv2D operation
    for (int w = 0; w < W; w++) { // For each output channel
      for (int x = 0; x < imgsize_x; x++) {
        for (int y = 0; y < imgsize_y; y++) {
          float sum = 0.0f;
          for (int z = 0; z < Z; z++) { // For each input channel
            for (int kx = 0; kx < K_X; kx++) { // Kernel height
              for (int ky = 0; ky < K_Y; ky++) { // Kernel width
                int src_x = x * stride + kx;
                int src_y = y * stride + ky;
                if (src_x >= 0 && src_x < X && src_y >= 0 && src_y < Y) {
                    float val = bfloat16_to_float_process(input_copy_stencil(z, src_x, src_y));
                    float kernel_val = bfloat16_to_float_process(kernel_copy_stencil(w, z, kx, ky));
                    sum += val * kernel_val;
                }
              }
            }
          }
          processor.output(w, x, y) = float_to_bfloat16_process(sum);
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

      std::cout << "Writing kernel_host_stencil.mat to bin folder" << std::endl;
      save_image(processor.inputs["kernel_host_stencil.mat"], "bin/kernel_host_stencil.mat");

      std::cout << "Writing hw_output.mat to bin folder" << std::endl;
      save_image(processor.output, "bin/hw_output.mat");
    }

    return processor.process_command(argc, argv);
}  
