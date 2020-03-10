#include "accel_soc_soc_mini.h"

#include <iostream>

#include "HalideBuffer.h"
#include "halide_image_io.h"

using namespace std;

#include "test_utils.h"

// g++ lesson_10*run.cpp lesson_10_halide.a -o lesson_10_run -I ../include
int main() {
  Halide::Runtime::Buffer<uint16_t> input(2, 2), output(2, 2, 3);
  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = y*input.width() + x;
    }
  }

  hw_output(input, output);
  Halide::Tools::save_image(output, "accel_soc.ppm");
  return 0;
}
