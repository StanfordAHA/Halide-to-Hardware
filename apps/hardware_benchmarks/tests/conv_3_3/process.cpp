#include <cstdio>
#include <chrono>

#include "conv_3_3.h"

#include "halide_benchmark.h"
#include "HalideBuffer.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    Buffer<int16_t> input(66, 66);

    for (int y = 0; y < input.height(); y++) {
      for (int x = 0; x < input.width(); x++) {
        input(x, y) = rand();
      }
    }

    Buffer<int16_t> output(64, 64);

    conv_3_3(input, output);

    // Timing code
    double min_t_manual = benchmark(10, 10, [&]() {
        conv_3_3(input, output);
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    return 0;
}
