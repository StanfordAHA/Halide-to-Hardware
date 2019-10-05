#include "coreir.h"
#include "coreir/simulator/interpreter.h"

#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>

using namespace CoreIR;
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
    
    Target t;
    t = t.with_feature(Target::Feature::CoreIR);
    hwOutput.compile_to_coreir("coreir_brighter", {input}, "brighter", t);

    Context* context = newContext();
    if (!loadFromFile(context, "./conv_3_3_app.json")) {
      cout << "Error: Could not load json for unit test!" << endl;
      context->die();
    }
    context->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});
    CoreIR::Module* m = context->getNamespace("global")->getModule("DesignTop");
    cout << "Module..." << endl;
    m->print();
    SimulatorState state(m);

    state.setValue("self.in_en", BitVector(1, 1));
    state.setValue("self.in_arg_0_0_0", BitVector(16, 123));
    
    state.resetCircuit();
    cout << "Starting to execute" << endl;

    state.exeCombinational();

    assert(state.getBitVec("self.out_0_0") == BitVec(16, 123 + 10));
    assert(state.getBitVec("self.valid") == BitVec(1, 1));

    deleteContext(context);

    cout << "Test passed!" << endl;
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
