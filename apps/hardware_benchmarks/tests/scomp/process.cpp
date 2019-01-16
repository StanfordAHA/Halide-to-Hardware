#include <cstdio>

#include "scomp.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<int16_t> processor("scomp",
                                          {
                                            {"cpu",
                                                [&]() { scomp(processor.input, processor.output); }
                                            },
                                            {"coreir",
                                                [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                    "self.in_arg_0_0_0", "self.out_0_0"); }
                                            }
                                          });

  processor.input = Buffer<int16_t>(64, 64);
  processor.output = Buffer<int16_t>(64, 64);
  
  processor.process_command(argc, argv);
  
}
