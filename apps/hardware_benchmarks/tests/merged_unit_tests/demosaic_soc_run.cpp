#include "demosaic_soc_mini.h"

#include <iostream>

#include "HalideBuffer.h"
#include "halide_image_io.h"

using namespace std;

#include "test_utils.h"

// g++ lesson_10*run.cpp lesson_10_halide.a -o lesson_10_run -I ../include
int main() {
  Halide::Runtime::Buffer<uint16_t> input(200, 200), output(10, 10, 3);
  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = y + x;
      //input(x, y) = 0;
    }
  }

  cout << "After setting all inputs to 23..." << endl;
  halide_buffer_t* inputBuf = input;
  void* inputHost = inputBuf->host;
  printf("Input Buffer location = %lld\n", (unsigned long long ) inputBuf);
  printf("Input host location = %lld\n", (unsigned long long ) &inputHost);
  for (int i = 0; i < 10; i++) {
    printf("input[%d] = %d\n", i, ((uint16_t*)(inputBuf->host))[i]);
  }
  cout << "Starting demosaic with input set to all 23" << endl;
  hw_demosaic(input, output);
  //hw_demosaic(&input, &output);

  cout << "C++ Output..." << endl;
  printBuffer(output, cout);
  Halide::Tools::save_image(output, "demosaic.ppm");
  cout << "Done" << endl;
  return 0;
}
