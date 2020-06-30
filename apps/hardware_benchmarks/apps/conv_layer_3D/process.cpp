#include <cstdio>
#include <chrono>

#include "conv_layer_3D.h"
//#include "conv_layer_auto_schedule.h"

#include "halide_benchmark.h"
#include "HalideBuffer.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    Buffer<uint8_t> input(69, 69);
    Buffer<uint8_t> filter(5, 5);

      for (int y = 0; y < input.height(); y++) {
        for (int x = 0; x < input.width(); x++) {
          input(x, y) = (uint8_t)rand();
        }
      }

      for (int y = 0; y < filter.height(); y++) {
        for (int x = 0; x < filter.width(); x++) {
          filter(x, y) = (uint8_t)rand();
        }
      }

    Buffer<uint8_t> output(64, 64);

    conv_layer(input, filter, output);

    // Timing code

    // Manually-tuned version
    double min_t_manual = benchmark(10, 10, [&]() {
        conv_layer(input, filter, output);
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
    // double min_t_auto = benchmark(10, 10, [&]() {
    //    conv_layer_auto_schedule(input, filter, output);
    // });
    // printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);

    return 0;
}
