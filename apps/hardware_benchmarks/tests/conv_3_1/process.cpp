#include <cstdio>
#include <chrono>

#include "conv_3_1.h"

#include "halide_benchmark.h"
#include "HalideBuffer.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
  int kernel_width  = 1;
  int kernel_height = 3;
  int image_size = 10;
  
  Buffer<int16_t> input(image_size + kernel_width-1,
                        image_size + kernel_height-1);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = rand();
    }
  }

  Buffer<int16_t> output(image_size, image_size);

  conv_3_1(input, output);

  // Timing code
  double min_t_manual = benchmark(10, 10, [&]() {
      conv_3_1(input, output);
    });
  printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

  return 0;
}
