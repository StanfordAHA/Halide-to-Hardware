
#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>

using namespace Halide;
using namespace Halide::Tools;
using namespace std;

void pointwise_test() {

    Func brighter;
    Var x, y;
    Var xo, yo, xi, yi;

    ImageParam input(type_of<uint8_t>(), 2);

    brighter(x, y) = input(x, y) + 10;

    brighter.compile_to_static_library("cpu_brighter", {input}, "brighter");

    brighter.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, yi);
    brighter.compile_to_coreir("coreir_brighter", {input}, "brighter");
  //Input<Buffer<uint8_t>>  input{"input", 2};
  //Output<Buffer<uint8_t>> output{"output", 2};
  
  //Var x, y;
  //Halide::Func gradient;
  //gradient(x, y) = input(x, y) + 10;

  //Halide::Buffer<uint16_t> in(4, 4);

  //Halide::Buffer<uint16_t> out(4, 4);
  //out = gradient.realize(out.width(), out.height());;
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
