#include <cstdio>

#include "conv_3_1.h"
#include "conv_3_1_vhls.h"
#include "vhls_target.h"

#include "hardware_process_helper.h"
#include "halide_image_io.h"
#include "halide_old_image.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

template< typename T >
using Image = Buffer<T>;

int main(int argc, char **argv) {
  OldImage<uint16_t> in_hls(64, 64, 1);
  OldImage<uint16_t> out_hls(in_hls.width(), in_hls.height()-2, in_hls.channels());
  
  OneInOneOut_ProcessController processor("conv_3_1",
                                          {
                                            {"cpu",
                                                [&]() {
                                                conv_3_1(processor.input,
                                                         processor.output);}},
                                            {"hls",
                                                [&]() {
                                                conv_3_1_old_buffer_t(in_hls,
                                                                      out_hls);}}
                                          });

  int kernel_width  = 1; int kw = kernel_width;
  int kernel_height = 3; int kh = kernel_height;
  int image_size = 10;   int N = image_size;
  
  processor.input = Buffer<uint16_t>(N, N);
  processor.output = Buffer<uint16_t>(N - kw, N - kh);

  
  processor.process_command(argc, argv);
  
}
