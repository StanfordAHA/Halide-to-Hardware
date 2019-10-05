#include "coreir.h"
#include "coreir/simulator/interpreter.h"
#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"

#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>

using namespace CoreIR;
using namespace Halide;
using namespace Halide::Tools;
using namespace std;

CoreIR::Context* hwContext() {
  CoreIR::Context* context = newContext();
  CoreIRLoadLibrary_commonlib(context);
  CoreIRLoadLibrary_float(context);
  return context;
}

CoreIR::Module* buildModule(CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Target t;
  t = t.with_feature(Target::Feature::CoreIR);
  hwOutput.compile_to_coreir(name, args, fName, t);
  //hwOutput.compile_to_coreir("coreir_brighter", {input}, "brighter", t);

  //Context* context = newContext();
  if (!loadFromFile(context, "./conv_3_3_app.json")) {
    cout << "Error: Could not load json for unit test!" << endl;
    context->die();
  }
  context->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});
  CoreIR::Module* m = context->getNamespace("global")->getModule("DesignTop");
  cout << "Module..." << endl;
  m->print();
  return m;
}

void small_conv_3_3_test() {
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  Var x("x"), y("y");

  Func kernel("kernel");
  Func conv("conv");
  RDom r(0, 3,
      0, 3);

  kernel(x,y) = 0;
  kernel(0,0) = 11;      kernel(0,1) = 12;      kernel(0,2) = 13;
  kernel(1,0) = 14;      kernel(1,1) = 0;       kernel(1,2) = 16;
  kernel(2,0) = 17;      kernel(2,1) = 18;      kernel(2,2) = 19;

  conv(x, y) = 0;

  Func hw_input("hw_input");
  hw_input(x, y) = cast<uint16_t>(input(x, y));
  conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint8_t>(conv(x, y));
  output(x, y) = hw_output(x,y);

  Var xi,yi, xo,yo;

  hw_input.compute_root();
  hw_output.compute_root();

  int tileSize = 4;
  hw_output.tile(x,y, xo,yo, xi,yi, tileSize-2, tileSize-2)
    .hw_accelerate(xi, xo);

  conv.update()
    .unroll(r.x, 3)
    .unroll(r.y, 3);
  conv.linebuffer();

  hw_input.stream_to_accelerator();

  auto context = hwContext();
  //Context* context = newContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_conv_3_3", args, "conv_3_3", hw_output);
  SimulatorState state(m);

  deleteContext(context);
  //// What I would like:
  //// Be able to run either the CPU implementation or the mixed coreir / cpu
  //// implementation on any given function
  //// OR for simple tests:
  //// Be able to run the entire thing in coreir and just check equivalence at the end
  ////
  //// I also want to be able to run just coreir synthesis on individual kernels with
  //// different settings for hardware parameters (operation latencies / IIs)
  //Target t;
  //t = t.with_feature(Target::Feature::CoreIR);
  //auto halideMod = hw_output.compile_to_module({input}, "coreir_brighter", t);

  //cout << "Module = " << endl;
  //cout << halideMod << endl;

}

void pointwise_add_test() {

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
    
    Context* context = newContext();
    vector<Argument> args{input};
    auto m = buildModule(context, "coreir_brighter", args, "brighter", hwOutput);
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

  pointwise_add_test();
  small_conv_3_3_test();
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
