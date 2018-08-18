#include <cstdio>
#include <chrono>

#include "gemm.h"
//#include "gemm_auto_schedule.h"

#include "halide_benchmark.h"
#include "HalideBuffer.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    Buffer<uint8_t> input_A(64, 64);
    Buffer<uint8_t> input_B(64, 64);

      for (int y = 0; y < input_A.height(); y++) {
        for (int x = 0; x < input_A.width(); x++) {
          input_A(x, y) = (uint8_t)rand();
        }
      }

      for (int y = 0; y < input_B.height(); y++) {
        for (int x = 0; x < input_B.width(); x++) {
          input_B(x, y) = (uint8_t)rand();
        }
      }

    Buffer<uint8_t> output(64, 64);

    gemm(input_A, input_B, output);

    // Timing code

    // Manually-tuned version
    double min_t_manual = benchmark(10, 10, [&]() {
        gemm(input_A, input_B, output);
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
    // double min_t_auto = benchmark(10, 10, [&]() {
    //    gemm_auto_schedule(input, filter, output);
    // });
    // printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);

    return 0;
}
