#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  auto input   = Buffer<uint8_t>(4, 4);
  auto weights = Buffer<uint8_t>(4, 4);
  auto output  = Buffer<uint8_t>(4, 4);

  auto interleaved = Buffer<uint8_t>(8, 4);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = (1 + x + 2*y) % 70;
      interleaved(2 * x, y) = input(x, y);
    }
  }
  std::cout << "created input buffer\n";
  
  for (int y = 0; y < weights.height(); y++) {
    for (int x = 0; x < weights.width(); x++) {
      weights(x, y) = (1 + x + y);
      interleaved(2 * x + 1, y) = weights(x, y);
    }
  }
  std::cout << "created weights buffer\n";

  // weights are transposed
  for (int y = 0; y < output.height(); y++) {
      for (int x = 0; x < output.width(); x++) {
          output(x, y) = input(x, 0) * weights(y, 0) + input(x, 1) * weights(y, 1) + input(x, 2) * weights(y, 2) + input(x, 3) * weights(y, 3);
      }
  }
  std::cout << "created output buffer\n";

  save_image(input,   "bin/input.png");
  save_image(weights, "bin/weights.png");
  save_image(output,  "bin/output_cpu.png");
  save_image(interleaved, "interleaved.png");
  return 0;
}
