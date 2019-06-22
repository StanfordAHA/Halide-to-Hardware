#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  auto input   = Buffer<uint8_t>(8, 128);
  auto weights = Buffer<uint8_t>(8, 128);
  auto output  = Buffer<uint8_t>(4, 128);
  auto interleaved = Buffer<uint8_t>(16, 128);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = (1 + x + 2*y) % 70;
      interleaved(2*((x / 2) * 2) + (x % 2), y) = input(x, y);
    }
  }
  std::cout << "created input buffer\n";
  
  for (int y = 0; y < weights.height(); y++) {
    for (int x = 0; x < weights.width(); x++) {
      weights(x, y) = (1 + x + y);
      interleaved(2*((x / 2) * 2) + (x % 2) + 2, y) = weights(x, y);
    }
  }
  std::cout << "created weights buffer\n";
    
  for (int y = 0; y < output.height(); y++) {
    output(0, y) = input(0, y) * weights(0, y) + input(1, y) * weights(1, y) + input(2, y) * weights(2, y) + input(3, y) * weights(3, y);
    output(1, y) = input(0, y) * weights(4, y) + input(1, y) * weights(5, y) + input(2, y) * weights(6, y) + input(3, y) * weights(7, y);
    output(2, y) = input(4, y) * weights(0, y) + input(5, y) * weights(1, y) + input(6, y) * weights(2, y) + input(7, y) * weights(3, y);
    output(3, y) = input(4, y) * weights(4, y) + input(5, y) * weights(5, y) + input(6, y) * weights(6, y) + input(7, y) * weights(7, y);
  }
  std::cout << "created output buffer\n";

  save_image(interleaved, "input.png");
  save_image(weights,     "weights.png");
  save_image(output,      "bin/output_cpu.png");
  return 0;
}
