#include <cstdio>

#include "conv_2_1.h"

#include "hardware_process_helper.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController processor("cpu",
                                          "conv_2_1",
                                          [&]() {
                                            conv_2_1(processor.input,
                                                     processor.output);
                                          });

  int kernel_width  = 1; int kw = kernel_width;
  int kernel_height = 3; int kh = kernel_height;
  int image_size = 10;   int N = image_size;
  
  processor.input = Buffer<uint16_t>(N, N);
  processor.output = Buffer<uint16_t>(N - kw, N - kh);

  
  processor.process_command(argc, argv);
  
}
