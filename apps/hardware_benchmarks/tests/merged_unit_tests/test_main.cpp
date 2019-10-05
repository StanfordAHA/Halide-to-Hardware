
#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>
//#include "IRPrinter.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace std;

void pointwise_test() {

    Var x, y;
    Var xo, yo, xi, yi;

    ImageParam input(type_of<uint8_t>(), 2);

    Func hwInput("hw_input");
    Func hwOutput("hw_output");
    Func brighter("brighter");
    
    hwInput(x, y) = input(x, y);
    brighter(x, y) = hwInput(x, y) + 10;
    hwOutput(x, y) = brighter(x, y);

    hwInput.compute_root();
    hwOutput.compute_root();

    hwOutput.tile(x, y, xo, yo, xi, yi, 4, 4).hw_accelerate(xi, xo);
    brighter.linebuffer();
    
    hwInput.stream_to_accelerator();
    
    //cout << "Loop nest" << endl;
    //brighter.print_loop_nest();
    //auto m = brighter.compile_to_module({input}, "brighter");
    //cout << "Module..." << endl;
    //cout << m << endl;

    Target t;
    t = t.with_feature(Target::Feature::CoreIR);
    hwOutput.compile_to_coreir("coreir_brighter", {input}, "brighter", t);


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

  //Halide::Buffer<uint8_t> input = load_image("../../../../tutorial/images/rgb.png");
  //cout << "Input rows = " << input.height() << endl;

  //Halide::Var x, y, c;
  //auto value = input(x, y, c);

  //Halide::Func gradient;

  //gradient(x, y, c) = value + 10;

  //Halide::Buffer<uint8_t> output =
    //gradient.realize(input.width(), input.height(), input.channels());

  //save_image(output, "brighter_halide_coreir.png");

  //printf("All tests passed!\n");

  return 0;
}
