#include <cstdio>

#include "conv_3_1.h"

#include "hardware_process_helper.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

    OneInOneOut_ProcessController processor("conv_3_1",
                                            {
                                              {"cpu",
                                                  [&]() { conv_3_3(processor.input, processor.output); }
                                              }
                                            });

  processor.input = Buffer<uint16_t>(64, 64);
  processor.output = Buffer<uint16_t>(64, 62);

  
  processor.process_command(argc, argv);
  
}
