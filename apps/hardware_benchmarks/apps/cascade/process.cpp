#include <cstdio>

#include "cascade.h"

#include "hardware_process_helper.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

    OneInOneOut_ProcessController processor("cascade",
                                            {
                                              {"cpu",
                                                  [&]() { cascade(processor.input, processor.output); }
                                              }
                                            });

  processor.input = Buffer<uint16_t>(64, 64);
  processor.output = Buffer<uint16_t>(60, 60);

  
  processor.process_command(argc, argv);
  
}
