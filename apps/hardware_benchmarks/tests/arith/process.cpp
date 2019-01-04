#include <cstdio>

#include "absolute.h"

#include "hardware_process_helper.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

    OneInOneOut_ProcessController processor("absolute",
                                            {
                                              {"cpu",
                                                  [&]() { absolute(processor.input, processor.output); }
                                              }
                                            });

  processor.input = Buffer<uint16_t>(64, 64);
  processor.output = Buffer<uint16_t>(62, 62);

  
  processor.process_command(argc, argv);
  
}
