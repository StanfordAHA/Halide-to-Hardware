#include "demosaic_soc_mini.h"

#include <iostream>

#include "HalideBuffer.h"

using namespace std;

// g++ lesson_10*run.cpp lesson_10_halide.a -o lesson_10_run -I ../include
int main() {
  //halide_buffer_t input;
  //halide_buffer_t output;

  Halide::Runtime::Buffer<uint16_t> input(10, 10), output(8, 8, 3);
  cout << "Starting demosaic" << endl;
  hw_demosaic(input, output);
  //hw_demosaic(&input, &output);

  cout << "Done" << endl;
  return 0;
}
