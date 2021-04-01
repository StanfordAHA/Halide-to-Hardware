#include <cstdio>
#include "hardware_process_helper.h"
#include "halide_image_io.h"

#if defined(WITH_CPU)
   #include "conv_chain.h"
#endif

#if defined(WITH_COREIR)
    #include "coreir_interpret.h"
#endif

#if defined(WITH_CLOCKWORK)
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
    #include "conv_chain_clockwork.h"
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv ) {
  std::map<std::string, std::function<void()>> functions;
  OneInOneOut_ProcessController<uint8_t> processor("conv_chain");

  #if defined(WITH_CPU)
      auto cpu_process = [&]( auto &proc ) {
        conv_chain( proc.input, proc.output );
      };
      functions["cpu"] = [&](){ cpu_process( processor ); } ;
  #endif
  
  #if defined(WITH_COREIR)
      auto coreir_process = [&]( auto &proc ) {
          run_coreir_on_interpreter<>( "bin/design_top.json",
                                       proc.input, proc.output,
                                       "self.in_arg_0_0_0", "self.out_0_0" );
      };
      functions["coreir"] = [&](){ coreir_process( processor ); };
  #endif
  
  #if defined(WITH_CLOCKWORK)
      auto clockwork_process = [&]( auto &proc ) {
        RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
        if ( rdai_platform ) {
          printf( "[RUN_INFO] found an RDAI platform\n" );
          conv_chain_clockwork( proc.input, proc.output );
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
  //  HALIDE_GEN_ARGS="ksize=3 stride=2 is_upsample=true"

  auto K = getenv("ksize");
  auto C = getenv("nconv");
  auto S = getenv("stride");
  auto U = getenv("is_upsample");
  
  auto ksize = K ? atoi(K) : 3;
  auto nconv = C ? atoi(C) : 3;
  auto stride = S ? atoi(S) : 1;
  auto is_upsample = U ? std::string(U)=="true" : false;

  
  processor.input   = Buffer<uint8_t>(64, 64);
  int input_size = 64;
  int imgsize = input_size;
  for (int i=0; i<nconv; ++i) {
    imgsize = is_upsample ?
      (imgsize - ksize + 1)*stride :
      (imgsize - ksize + 1)/stride;
  }

  std::cout << "ksize=" << ksize << " nconv=" << nconv << " stride=" << stride
            << " size=" << imgsize << std::endl;
  processor.output  = Buffer<uint8_t>(imgsize, imgsize);
  
 return processor.process_command(argc, argv);
  
}
