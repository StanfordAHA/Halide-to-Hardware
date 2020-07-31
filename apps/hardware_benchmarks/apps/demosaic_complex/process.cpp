#include <cstdio>

#include "demosaic_complex.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("demosaic_complex",
                                            {
                                              {"cpu",
                                                  [&]() { demosaic_complex(processor.input, processor.output); }
                                              },
                                              {"coreir",
                                                  [&]() { run_multi_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                            "self.in_arg_0_0_0", {"self.out_0_0_0","self.out_0_0_1","self.out_0_0_2"}); }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(64-2, 64-2, 3);
  
  return processor.process_command(argc, argv);
  
}
