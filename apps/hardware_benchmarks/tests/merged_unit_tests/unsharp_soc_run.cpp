#include "unsharp_soc_mini.h"

#include <iostream>

#include "HalideBuffer.h"
#include "halide_image_io.h"

using namespace std;

#include "test_utils.h"

// g++ lesson_10*run.cpp lesson_10_halide.a -o lesson_10_run -I ../include
int main() {
  Halide::Runtime::Buffer<uint8_t> input(11, 11, 3), output(5, 5, 3);
  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      for (int b = 0; b < input.channels(); b++) {
        input(x, y, b) = x + y;
      }
    }
  }

  cout << "SoC Input..." << endl;
  printBuffer(input, cout);

  hw_output(input, output);

  cout << "C++ Output..." << endl;
  printBuffer(output, cout);
  Halide::Tools::save_image(output, "unsharp.ppm");
  cout << "Done" << endl;
  return 0;
}
