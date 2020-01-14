#include "coreir.h"
#include "coreir/simulator/interpreter.h"
#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"

#include "Halide.h"

#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>

#include <fstream>
#include "test_utils.h"
#include "coreir_utils.h"

using namespace CoreIR;
using namespace Halide;
using namespace Halide::Tools;
using namespace std;

void ubuffer_small_conv_3_3_test() {
  ImageParam input(type_of<uint8_t>(), 2);
  Func output;

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

  int inTileSize = 4;
  int outTileSize = inTileSize - 2;

  hw_output.bound(x, 0, outTileSize);
  hw_output.bound(y, 0, outTileSize);

  output.bound(x, 0, outTileSize);
  output.bound(y, 0, outTileSize);

  // Creating input data
  Halide::Buffer<uint8_t> inputBuf(4, 4);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(4, 4, 1);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int b = 0; b < 1; b++) {
        inputBuf(i, j, b) = i + j*2;
        hwInputBuf(i, j, b) = inputBuf(i, j, b);
      }
    }
  }
 
  // Creating CPU reference output
  Halide::Buffer<uint8_t> cpuOutput(2, 2);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);
  
  Halide::Runtime::Buffer<uint8_t> outputBuf(2, 2, 1);
  
  int tileSize = 4;
  hw_output.tile(x,y, xo,yo, xi,yi, tileSize-2, tileSize-2)
    .hw_accelerate(xi, xo);

  kernel.compute_at(hw_output, xo)
    .unroll(x).unroll(y);

  conv.update()
    .unroll(r.x, 3)
    .unroll(r.y, 3);
  conv.linebuffer();

  hw_input.stream_to_accelerator();

  // Generate CoreIR
  auto context = hwContext();
  vector<Argument> args{input};
  buildModule(context, "coreir_conv_3_3", args, "conv_3_3", hw_output);

  // Get unified buffer fie
  if (!loadFromFile(context, "./ubuffers.json")) {
    cout << "Error: Could not load json for ubuffer test!" << endl;
    context->die();
  }
  context->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});
  CoreIR::Module* m = context->getNamespace("global")->getModule("hw_input_ubuffer");
  cout << "hw_input_ubuffer..." << endl;
  m->print();

  SimulatorState state(m);
  state.setValue("self.write_port_0", BitVector(16, 0));
  state.setValue("self.write_port_0_en", BitVector(1, 0));
  state.setClock("self.clk", 0, 1);
  state.setValue("self.reset", BitVector(1, 1));
  state.resetCircuit();
  state.setValue("self.reset", BitVector(1, 0));

  assert(state.getBitVec("self.read_port_0_valid") == BitVec(1, 0));

  state.setValue("self.write_port_0", BitVector(16, 1));
  state.setValue("self.write_port_0_en", BitVector(1, 1));
  
  state.execute();

  //assert(state.getBitVec("self.read_port_0_valid") == BitVec(1, 0));
  
  deleteContext(context);
 
  cout << GREEN << "UBuffer to linebuffer for conv 3x3 test passed" << RESET << endl;
  //assert(false);
}

