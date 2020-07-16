#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

#include "conv_3_3.h"
#include "rdai_api.h"
#include "clockwork_sim_platform.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("conv_3_3",
                                            {
                                              {"cpu",
                                                  [&]() { conv_3_3(processor.input, processor.output); }
                                              },
                                              {"rewrite",
                                                  [&]() { run_coreir_rewrite_on_interpreter<>("bin/design_top.json", "bin/ubuffers.json",
                                                                                       processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json",
                                                                                       processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"clockwork",
                                                  [&]() { 
                                                    RDAI_Platform *rdai_platform = RDAI_register_platform(&rdai_clockwork_sim_ops);
                                                    if(rdai_platform) {
                                                        printf("[INFO]: found an RDAI platform\n");
                                                        conv_3_3(processor.input, processor.output); 
                                                        RDAI_unregister_platform(rdai_platform);
                                                    } else {
                                                        printf("failed to register RDAI platform!\n");
                                                    }

                                                  }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(62, 62);
  
  return processor.process_command(argc, argv);

}
