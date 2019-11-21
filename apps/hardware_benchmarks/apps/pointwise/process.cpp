#include <cstdio>

#include "pointwise.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int inImgSize = 64;
int outImgSize = inImgSize;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("pointwise",
                                            {
                                              {"cpu",
                                                  [&]() { pointwise(processor.input, processor.output); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(inImgSize, inImgSize);
  processor.output = Buffer<uint8_t>(outImgSize, outImgSize);
  
  processor.process_command(argc, argv);
  
}
