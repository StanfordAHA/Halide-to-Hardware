#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  auto input   = Buffer<uint8_t>(64, 64);
  auto kernel = Buffer<uint8_t>(3, 3);
  auto output  = Buffer<uint8_t>(62, 62);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = (1 + x + 2*y) % 70;
    }
  }
  std::cout << "created input buffer\n";

  kernel(0,0) = 0;
  kernel(0,1) = 10;
  kernel(0,2) = 11;
  kernel(1,0) = 13;
  kernel(1,1) = 14;
  kernel(1,2) = 16;
  kernel(2,0) = 17;
  kernel(2,1) = 18;
  kernel(2,2) = 19;

  // weights are transposed
  for (int y = 0; y < output.height(); y++) {
      for (int x = 0; x < output.width(); x++) {
          output(x, y) = 0;
          for (int j = 0; j < 3; j++) {
              for (int i = 0; i < 3; i++) {
                  output(x, y) += input(x + i, y + j) * kernel(i, j);
              }
          }
      }
  }
  std::cout << "created output buffer\n";

  save_image(input,   "input.png");
  save_image(output,  "bin/output_cpu.png");
  return 0;
}
