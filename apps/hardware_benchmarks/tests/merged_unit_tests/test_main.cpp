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

std::string GREEN = "\033[32m";
std::string RED = "\033[31m";
std::string RESET = "\033[0m";

template<typename T>
class CoordinateVector {
  public:

    std::vector<T> values;
    std::vector<std::string> names;
    std::vector<T> bounds;

    bool finished;

    CoordinateVector(vector<std::string> names_, vector<T> bounds_) : names(names_), bounds(bounds_), finished(false) {
      values.resize(names.size());
      for (int i = 0; i < (int) bounds.size(); i++) {
        values[i] = 0;
      }
    }

    CoordinateVector(vector<std::string>& names_, vector<T>& bounds_) : names(names_), bounds(bounds_), finished(false) {
      values.resize(names.size());
      for (int i = 0; i < (int) bounds.size(); i++) {
        values[i] = 0;
      }
    }

    int coord(const std::string& str) {
      for (int i = 0; i < (int) names.size(); i++) {
        auto cN = names[i];
        if (cN == str) {
          return values[i];
        }
      }

      assert(false);
    }

    std::string coordString() const {
      std::string str = "{";
      for (int i = 0; i < ((int) bounds.size()); i++) {
        str += std::to_string(values[i]) + " : " + std::to_string(bounds[i]);
        if (i < ((int) bounds.size()) - 1) {
          str += ", ";
        }
      }
      str += "}";
      return str;
    }
    bool allLowerAtMax(const int level) const {
      if (level == ((int) bounds.size()) - 1) {
        return true;
      }

      for (int i = level + 1; i < (int) bounds.size(); i++) {
        if (!atMax(i)) {
          return false;
        }
      }

      return true;
    }

    bool atMax(const int level) const {
      bool atM = bounds[level] == values[level];
      //cout << "atM = " << atM << " for level: " << level << ", bounds = " << bounds[level] << ", value = " << values[level] << endl;
      return atM;
    }

    bool allAtMax() const {
      return atMax(0) && allLowerAtMax(0);
    }

    bool allDone() const {
      return finished && atMax(0) && allLowerAtMax(0);
    }
    
    void increment() {
      if (allAtMax() && !allDone()) {
        finished = true;
      }

      if (allDone()) {
        return;
      }

      for (int i = 0; i < (int) bounds.size(); i++) {
        if (allLowerAtMax(i)) {
          values[i]++;

          for (int j = i + 1; j < (int) bounds.size(); j++) {
            values[j] = 0;
          }
          break;
        }
      }
    }

};

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

template<typename T>
void run_for_cycle(CoordinateVector<int>& writeIdx,
    CoordinateVector<int>& readIdx,

    Halide::Runtime::Buffer<T> input,
    Halide::Runtime::Buffer<T> output,
    string input_name,
    string output_name,

    CoreIR::SimulatorState& state
    ) {

  const int x = writeIdx.coord("x");
  const int y = writeIdx.coord("y");
  const int c = writeIdx.coord("c");

  if (!writeIdx.allDone()) {

    state.setValue("self.in_en", BitVector(1, true));

    state.setValue(input_name, BitVector(16, input(x,y,c)));
    //std::cout << "y=" << y << ",x=" << x << " " << hex << "in=" << (int) input(x, y, c) << endl;
    std::cout << "y=" << y << ",x=" << x << " " << "in=" << (int) input(x, y, c) << endl;

    writeIdx.increment();
  } else {
    state.setValue("self.in_en", BitVector(1, false));
  }
  // propogate to all wires
  state.exeCombinational();

  // read output wire
  //std::cout << "using valid\n";
  bool valid_value = state.getBitVec("self.valid").to_type<bool>();
  //std::cout << "got my valid\n";
  //cout << "output_bv_n = " << output_bv_n << endl;
  if (valid_value) {
    auto output_bv = state.getBitVec(output_name);

    std::cout << "this one is valid = " << output_bv.to_type<int>() << endl;
    // bitcast to float if it is a float
    T output_value;
    output_value = output_bv.to_type<T>();

    //coreir_img_writer.write(output_value);

    const int xr = readIdx.coord("x");
    const int yr = readIdx.coord("y");
    const int cr = readIdx.coord("c");
    output(xr, yr, cr) = output_value;
    //std::cout << "y=" << y << ",x=" << x << " " << hex << "in=" << (state.getBitVec(input_name)) << " out=" << +output_value << " based on bv=" << state.getBitVec(output_name) << dec << endl;
    readIdx.increment();
  }


  //std::cout << "y=" << y << ",x=" << x << " " << "in=" << (state.getBitVec(input_name)) << " out=" << +output_value << " based on bv=" << state.getBitVec(output_name).to_type<int>() << dec << endl;
  //std::cout << "y=" << y << ",x=" << x << " " << hex << "in=" << (state.getBitVec(input_name)) << " out=" << +output_value << " based on bv=" << state.getBitVec(output_name).to_type<int>() << dec << endl;

  // give another rising edge (execute seq)
  state.exeSequential();
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


  // Creating input data
  Halide::Buffer<uint8_t> inputBuf(4, 4);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(4, 4, 1);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int b = 0; b < 1; b++) {
        inputBuf(i, j, b) = 12;
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
  cout << "CPU output..." << endl;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      cout << (int) cpuOutput(i, j) << " ";
    }
    cout << endl;
  }
  
  //Buffer<uint8_t> paramBuf(4, 4, 1);
  //Buffer<uint8_t> outParamBuf(2, 2, 1);
  Halide::Runtime::Buffer<uint8_t> outputBuf(2, 2, 1);
  //ParamMap rParams;
  //rParams.set(input, paramBuf);
  //rParams.set(output, outParamBuf);
  //Target t;
  //Buffer<uint8_t> cpuOut = hw_output.realize(t, rParams);

  //assert(false);

  
  int tileSize = 4;
  hw_output.tile(x,y, xo,yo, xi,yi, tileSize-2, tileSize-2)
    .hw_accelerate(xi, xo);

  conv.update()
    .unroll(r.x, 3)
    .unroll(r.y, 3);
  conv.linebuffer();

  hw_input.stream_to_accelerator();

  // Generate CoreIR
  auto context = hwContext();
  //Context* context = newContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_conv_3_3", args, "conv_3_3", hw_output);
  cout << "Module = " << endl;
  m->print();

  SimulatorState state(m);
  state.setValue("self.in_arg_0_0_0", BitVector(16, 0));
  state.setValue("self.in_en", BitVector(1, 0));
  state.setClock("self.clk", 0, 1);
  state.setValue("self.reset", BitVector(1, 1));

  state.resetCircuit();

  state.setValue("self.reset", BitVector(1, 0));

  int maxCycles = 100;
  int cycles = 0;
  

  std::string inputName = "self.in_arg_0_0_0";
  std::string outputName = "self.out_0_0";
  CoordinateVector<int> writeIdx({"y", "x", "c"}, {hwInputBuf.height() - 1, hwInputBuf.width() - 1, hwInputBuf.channels() - 1});
  CoordinateVector<int> readIdx({"y", "x", "c"}, {outputBuf.height() - 1, outputBuf.width() - 1, outputBuf.channels() - 1});
  
  while (cycles < maxCycles && !readIdx.allDone()) {
    cout << "Read index = " << readIdx.coordString() << endl;
    cout << "Cycles     = " << cycles << endl;


    run_for_cycle(writeIdx, readIdx,
        hwInputBuf, outputBuf,
        inputName, outputName,
        state);
    cycles++;
  }

  cout << "final buffer" << endl;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int b = 0; b < 1; b++) {
        cout << (int) outputBuf(i, j, b) << " ";
        assert(outputBuf(i, j, b) == cpuOutput(i, j, b));
      }
    }
    cout << endl;
  }
  deleteContext(context);
 
  cout << GREEN << "Conv 3x3 test passed" << RESET << endl;
  ////Target t;
  ////t = t.with_feature(Target::Feature::CoreIR);
  ////auto halideMod = hw_output.compile_to_module({input}, "coreir_brighter", t);

  ////cout << "Module = " << endl;
  ////cout << halideMod << endl;

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

  cout << GREEN << "All tests passed" << RESET << endl;
  return 0;
}
