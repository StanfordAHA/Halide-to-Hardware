#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  auto input   = Buffer<uint8_t>(4, 16);
  auto weights = Buffer<uint8_t>(4, 16);
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

  int block_width = 4;
  int block_height = 4;
  int num_blocks = 4;
  
  // input is transposed
  for (int b = 0; b < num_blocks; b++) {
    int y_b = b*block_height;
    for (int y = 0; y < block_height; y++) {
      for (int x = 0; x < block_width; x++) {
        output(x, y) =
          input(y + y_b, 0) * weights(x, 0 + y_b) +
          input(y + y_b, 1) * weights(x, 1 + y_b) +
          input(y + y_b, 2) * weights(x, 2 + y_b) +
          input(y + y_b, 3) * weights(x, 3 + y_b);
      }
    }
  }
  std::cout << "created output buffer\n";

  save_image(input,   "input.png");
  save_image(weights, "weights.png");
  save_image(output,  "bin/output_cpu.png");
  save_image(interleaved, "interleaved.png");
  return 0;
}
