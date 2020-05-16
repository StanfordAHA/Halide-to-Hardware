#include <cstdio>

#include "fp_conv_1_1.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("fp_conv_1_1",
                                            {
                                              {"cpu",
                                                  [&]() { fp_conv_1_1(processor.input, processor.output); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_2_0_0", "self.out_0_0",
                                                                                      /*has_float_input=*/true, /*has_float_output=*/true); }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(64, 64);
  
  return processor.process_command(argc, argv);
  
}
