#include "demosaic_soc_mini.h"

#include <iostream>

#include "HalideBuffer.h"

using namespace std;

// g++ lesson_10*run.cpp lesson_10_halide.a -o lesson_10_run -I ../include
int main() {
  //halide_buffer_t input;
  //halide_buffer_t output;

  //Halide::Runtime::Buffer<uint16_t> input(10, 10), output(8, 8, 3);
  //Halide::Runtime::Buffer<uint16_t> input(200, 200), output(200 - 8, 200 - 8, 3);
  Halide::Runtime::Buffer<uint16_t> input(200, 200), output(10, 10, 3);
  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = 23;
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

  cout << "Done" << endl;
  return 0;
}
