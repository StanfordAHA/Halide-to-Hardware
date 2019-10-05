
#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>

using namespace Halide;
using namespace Halide::Tools;
using namespace std;

void pointwise_test() {
  Halide::Buffer<uint16_t> in(4, 4);

  Var x, y;
  Halide::Func gradient;
  gradient(x, y) = in(x, y) + 10;

  Halide::Buffer<uint16_t> out(4, 4);
  out = gradient.realize(out.width(), out.height());;
}

int main(int argc, char **argv) {

  pointwise_test();

  Halide::Buffer<uint8_t> input = load_image("../../../../tutorial/images/rgb.png");
  cout << "Input rows = " << input.height() << endl;

  Halide::Var x, y, c;
  auto value = input(x, y, c);

  Halide::Func gradient;

  gradient(x, y, c) = value + 10;

  Halide::Buffer<uint8_t> output =
    gradient.realize(input.width(), input.height(), input.channels());

  save_image(output, "brighter_halide_coreir.png");

  printf("All tests passed!\n");

  return 0;
}
