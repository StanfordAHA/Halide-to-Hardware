#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  auto input   = Buffer<uint8_t>(16, 16);
  auto output  = Buffer<uint8_t>(8, 8);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      input(x, y) = (1 + x + 2*y) % 70;
    }
  }
  std::cout << "created input buffer\n";
  
  for (int y = 0; y < output.height(); y++) {
      for (int x = 0; x < output.width(); x++) {
          output(x,y) = std::max(std::max(input(2*x,2*y), input(2*x+1,2*y)), std::max(input(2*x,2*y+1),input(2*x+1,2*y+1)));
      }
  }
  std::cout << "created output buffer\n";

  save_image(output,  "bin/output_cpu.png");
  save_image(input, "input.png");
  return 0;
}
