#include <cmath>
#include <cstdint>
#include <cstdio>
#ifdef __SSE2__
#include <emmintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

float t;
int res = 512;

#include "hologram_wgs.h"

Buffer<float> wgs_halide(Buffer<float> x, Buffer<float> y, Buffer<float> z) {
    Buffer<float> phases(res, res);

    Buffer<float> pists(100);

    // Call it once to initialize the halide runtime stuff
    hologram_wgs(x, y, z, phases);
    // Copy-out result if it's device buffer and dirty.
    phases.copy_to_host();

    t = benchmark(10, 1, [&]() {
        hologram_wgs(x, y, z, phases);
        // Sync device execution if any.
        phases.device_sync();
    });

    phases.copy_to_host();

    return phases;
}

int main(int argc, char **argv) {
    int num_traps = 100;

    Buffer<float> x(num_traps);
    Buffer<float> y(num_traps);
    Buffer<float> z(num_traps);

    // hacky way for now

    for (int i = 0; i < num_traps; i++) {
        x(i) = (float(rand())/RAND_MAX-0.5)*100.0;
        y(i) = (float(rand())/RAND_MAX-0.5)*100.0;
        z(i) = (float(rand())/RAND_MAX-0.5)*10.0;
    }

    Buffer<float> halide = wgs_halide(x, y, z);
    float halide_time = t;

    printf("time: %f\n", halide_time);

    // no phase_gold
//    for (int x = 0; x < res; x++) {
//        for (int y = 0; y < res; y++) {
//            if (abs(halide(x,y) - phase_gold[y][x]) > 0.05 && abs(halide(x,y)) < 3.14 && abs(phase_gold[y][x]) < 3.14) {
//                printf("(%d,%d) mismatch! got: %f, expected: %f\n", x, y, halide(x,y), phase_gold[y][x]);
//            }
//        }
//     }

    printf("Success!\n");
    return 0;
}
