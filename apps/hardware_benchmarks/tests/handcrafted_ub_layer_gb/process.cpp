#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  int input_x = 16;
  int input_y = 16;
  int weight_x = 3;
  int weight_y = 3;
  int in_chan = 64;
  int out_chan = 64;
  auto input   = Buffer<uint8_t>(input_x, input_y, in_chan);
  auto weights = Buffer<uint8_t>(weight_x, weight_y, in_chan, out_chan);
  auto output  = Buffer<uint8_t>(input_x - weight_x + 1, input_y - weight_y + 1, out_chan);

  auto input_flat = Buffer<uint8_t>(in_chan, input_x * input_y);
  auto weights_flat = Buffer<uint8_t>(in_chan * out_chan, weight_x * weight_y);
  auto output_flat = Buffer<uint8_t>(out_chan, output.height() * output.width());

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      for (int w = 0; w < in_chan; w++) {
        input(x, y, w) = (1 + x + 2*y + w) % 70;
        input_flat(w, x + y * input_x) = input(x, y, w);
      }
    }
  }
  std::cout << "created input buffer\n";
  
  for (int y = 0; y < weights.height(); y++) {
    for (int x = 0; x < weights.width(); x++) {
      for (int w = 0; w < in_chan; w++) {
        for (int v = 0; v < out_chan; v++) {
          weights(x, y, w, v) = (1 + x + y + w + v);
          weights_flat(v * in_chan + w, x + y * weight_x) = weights(x, y, w, v);
        }
      }
    }
  }
  std::cout << "created weights buffer\n";

  for (int y = 0; y < output.height(); y++) {
      for (int x = 0; x < output.width(); x++) {
          for (int w = 0; w < out_chan; w++) {
              uint8_t value = 0;

              for (int j = -1; j < weights.height() - 1; j++) {
                  for (int i = -1; i < weights.width() - 1; i++) {
                      for (int k = 0; k < in_chan; k++) {
                          value += input(x + i, j + i, k) * weights(i + 1, j + 1, k, w);
                      }
                  }
              }

              output(x, y, w) = value;
              output_flat(w, x + output.width() * y) = value;
          }
      }
  }
  std::cout << "created output buffer\n";

  save_image(weights_flat, "weights.png");
  save_image(output_flat,  "bin/output_cpu.png");
  save_image(input_flat,   "input.png");
  return 0;
}
