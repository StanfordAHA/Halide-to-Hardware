#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  auto input   = Buffer<uint8_t>(16, 16);
  auto output  = Buffer<uint8_t>(6, 6);
  auto kernel = Buffer<uint16_t>(2, 2);
  kernel(0,0) = 0;
  kernel(1,0) = 11;
  kernel(0,1) = 13;
  kernel(1,1) = 14;

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = (1 + x + 2*y) % 70;
    }
  }
  std::cout << "created input buffer\n";
  
  for (int y = 0; y < output.height(); y++) {
      for (int x = 0; x < output.width(); x++) {
          output(x, y) = input(2 * x,  2 * y) * kernel(0, 0) + input(2 * x + 2, 2 * y) * kernel(1, 0) + input(2 * x, 2 * y + 2) * kernel(0, 1) + input(2 * x + 2, 2 * y + 2) * kernel(1, 1);
      }
  }
  std::cout << "created output buffer\n";

  save_image(output,  "bin/output_cpu.png");
  save_image(input, "input.png");
  return 0;
}
