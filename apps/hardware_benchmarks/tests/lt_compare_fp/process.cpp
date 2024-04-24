#include <iostream>
#include <math.h>
#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "lt_compare_fp.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "lt_compare_fp_clockwork.h"
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
  ManyInOneOut_ProcessController<uint16_t> processor("lt_compare_fp", {"hw_input_stencil.mat"});

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        lt_compare_fp(proc.inputs["hw_input_stencil.mat"], proc.output);
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.inputs["hw_input_stencil.mat"], proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          lt_compare_fp_clockwork(proc.inputs["hw_input_stencil.mat"], proc.output);
          RDAI_unregister_platform( rdai_platform );
        } else {
          printf("[RUN_INFO] failed to register RDAI platform!\n");
        }
      };
      functions["clockwork"] = [&](){ clockwork_process( processor ); };
  #endif

  auto OX = getenv("out_img");
  auto OC = getenv("n_oc");
  auto out_img = OX ? atoi(OX) : 10;
  auto n_oc = OC ? atoi(OC) : 8;
      
  // Add all defined functions
  processor.run_calls = functions;

  processor.inputs["hw_input_stencil.mat"]        = Buffer<uint16_t>(n_oc, out_img, out_img);
  processor.output                                = Buffer<uint16_t>(n_oc, out_img, out_img);

  processor.inputs_preset = true;
  
  for (int y = 0; y < processor.inputs["hw_input_stencil.mat"].dim(2).extent(); y++) {
    for (int x = 0; x < processor.inputs["hw_input_stencil.mat"].dim(1).extent(); x++) {
      for (int w = 0; w < processor.inputs["hw_input_stencil.mat"].dim(0).extent(); w++) {
        processor.inputs["hw_input_stencil.mat"](w, x, y) = float_to_bfloat16_process((static_cast<float>(rand()) / RAND_MAX) * 7.0f - 3.5f);
      }
    }
  }

  auto input_copy = processor.inputs["hw_input_stencil.mat"];
  for (int y = 0; y < processor.output.dim(2).extent(); y++) {
    for (int x = 0; x < processor.output.dim(1).extent(); x++) {
      for (int w = 0; w < processor.output.dim(0).extent(); w++) {
        // processor.output(w, x, y) = float_to_bfloat16_process(std::min(std::max(bfloat16_to_float_process(input_copy(w, x, y)), 0.0f), 6.0f));
        processor.output(w, x, y) = float_to_bfloat16_process(std::min(bfloat16_to_float_process(input_copy(w, x, y)), 6.0f));
      }
    }
  }

  save_image(processor.inputs["bin/hw_input_stencil.mat"], "bin/hw_input_stencil.mat");
  save_image(processor.output, "bin/hw_output.mat");
  
  return processor.process_command(argc, argv);
  
}
