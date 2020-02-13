#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <math.h>

#include "halide_image.h"
#include "halide_image_io.h"

#include "example_vhls.h"
#include "example.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char *argv[]) {
    Image<uint8_t> input = load_image(argv[1]);
    Image<uint8_t> out_native(input.width() - 8, input.height() - 8, 3);
    Image<uint8_t> out_hls(64, 64, 3);  // cropped

    printf("[APP]  Starting...\n");

    example(input, out_native);
    save_image(out_native, "out_native.png");
    printf("[APP] Finished running native code\n");

    example_vhls(input, out_hls);
    printf("[APP] Finished running hls code\n");

    bool success = true;
    for (int y = 0; y < out_hls.height(); y++) {
        for (int x = 0; x < out_hls.width(); x++) {
            for (int c = 0; c < out_hls.channels(); c++) {
                if (out_native(x, y, c) != out_hls(x, y, c)) {
                    success = false;
                }
            }
        }

    }

    if(success) {
        printf("[APP] Test Passed!\n");
    } else {
        printf("[APP] Test Failed!\n");
        return 1;
    }

    return 0;
}