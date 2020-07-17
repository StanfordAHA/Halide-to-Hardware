#include <cstdio>

// Halide helpers
#include "hardware_process_helper.h"
#include "halide_image_io.h"

// CoreIR interpretor header
#include "coreir_interpret.h"

// Clockwork - RDAI
#ifdef WITH_CLOCKWORK
    #include "rdai_api.h"
    #include "clockwork_sim_platform.h"
#endif // WITH_CLOCKWORK

// Pipeline header
#include "unsharp.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main( int argc, char **argv )
{
    auto cpu_process = [&]( auto &proc ) {
        unsharp( proc.input, proc.output );
    };

    auto coreir_process = [&]( auto &proc ) {
        run_coreir_on_interpreter<>( "bin/design_top.json",
                                     proc.input,
                                     proc.output,
                                     "self.in_arg_0_0_0",
                                     "self.out_0_0"
                                   );
    };

    auto clockwork_process = [&]( auto &proc ) {
        #ifdef WITH_CLOCKWORK
            RDAI_Platform *rdai_platform = RDAI_register_platform( &rdai_clockwork_sim_ops );
            if( rdai_platform ) {
                printf( "[RUN_INFO] found an RDAI platform\n" );
                unsharp( proc.input, proc.output ); 
                RDAI_unregister_platform( rdai_platform );
            } else {
                printf("[RUN_INFO] failed to register RDAI platform!\n");
            }
        #else
            printf("[RUN_INFO] Please run the run-clockwork target instead!\n");
        #endif // WITH_CLOCKWORK
    };

  OneInOneOut_ProcessController<uint8_t> processor( "unsharp",
                                                    {
                                                      { "cpu",          [&](){ cpu_process( processor ); } },
                                                      { "coreir",       [&](){ coreir_process( processor ); } },
                                                      { "clockwork",    [&](){ clockwork_process( processor ); } }
                                                    }
                                                  );

  processor.input   = Buffer<uint8_t>(64, 64, 3);
  processor.output  = Buffer<uint8_t>(60, 60);
  
  processor.process_command(argc, argv);
  
}
