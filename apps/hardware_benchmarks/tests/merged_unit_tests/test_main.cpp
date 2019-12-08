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

#define PRINT_PASSED(msg) std::cout << GREEN << msg << " test passed." << RESET << std::endl;

using namespace CoreIR;
using namespace Halide;
using namespace Halide::Tools;
using namespace std;

std::string GREEN = "\033[32m";
std::string RED = "\033[31m";
std::string RESET = "\033[0m";

template<typename T>
bool is2D(T& buf) {
  return (buf.dimensions() == 2) || (buf.dimensions() == 3 && buf.channels() == 1);
}

template<typename T0, typename T1>
void indexTestPatternRandom(T0& inputBuf, T1& hwInputBuf) {

  if (is2D(inputBuf)) {
    for (int i = 0; i < inputBuf.width(); i++) {
      for (int j = 0; j < inputBuf.height(); j++) {
        hwInputBuf(i, j, 0) = rand() % 255;
        inputBuf(i, j) = hwInputBuf(i, j);
      }
    }
  } else {
    assert(inputBuf.dimensions() == 3);
    for (int i = 0; i < inputBuf.width(); i++) {
      for (int j = 0; j < inputBuf.height(); j++) {
        for (int b = 0; b < inputBuf.channels(); b++) {
          hwInputBuf(i, j, b) = rand() % 255;
          inputBuf(i, j, b) = hwInputBuf(i, j, b);
        }
      }
    }
  }
}

template<typename T0, typename T1>
void indexTestPattern2D(T0& inputBuf, T1& hwInputBuf) {
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      hwInputBuf(i, j, 0) = i + j*inputBuf.width();
      inputBuf(i, j) = hwInputBuf(i, j);
    }
  }
}

std::string getInputAlias(const std::string& jsonFile) {
  json j;
  ifstream inFile(jsonFile);
  //"accel_interface_info.json");
  inFile >> j;
  cout << "JSON..." << endl;
  cout << j << endl;

  auto aliasMap = j["aliasMap"];
  cout << "Alias map = " << aliasMap << endl;
  assert(aliasMap.size() == 1);

  string inS = begin(aliasMap)->get<string>();
  cout << "inS = " << inS << endl;
  string outS = "hw_output.stencil.stream";
  string accelName = "self.in_" + inS + "_0_0";
  return accelName;
}

template<typename T, typename OT>
Halide::Buffer<OT> realizeCPU(Func hw_output, ImageParam& input, Halide::Buffer<T>& inputBuf, Halide::Runtime::Buffer<OT>& outputBuf) {
  if (outputBuf.dimensions() == 3) {
    cout << "Doing 3d realization... for " << outputBuf.channels() << " channels" << endl;
    Halide::Buffer<OT> cpuOutput(outputBuf.width(), outputBuf.height(), outputBuf.channels());
    ParamMap rParams;
    rParams.set(input, inputBuf);
    Target t;
    hw_output.realize(cpuOutput, t, rParams);
    return cpuOutput;
  } else {
    assert(outputBuf.dimensions() == 2);
    
    Halide::Buffer<OT> cpuOutput(outputBuf.width(), outputBuf.height());
    ParamMap rParams;
    rParams.set(input, inputBuf);
    Target t;
    hw_output.realize(cpuOutput, t, rParams);
    return cpuOutput;
  }
}

template<typename T>
void runHWKernel(const std::string& inputName, CoreIR::Module* m, Halide::Runtime::Buffer<T>& hwInputBuf, Halide::Runtime::Buffer<T>& outputBuf) {

  SimulatorState state(m);
  resetSim(inputName, m, state);

  assert(is2D(hwInputBuf));

  int maxCycles = 100;
  int cycles = 0;

  std::string outputName = "self.out_0_0";
  CoordinateVector<int> writeIdx({"y", "x", "c"}, {hwInputBuf.height() - 1, hwInputBuf.width() - 1, hwInputBuf.channels() - 1});
  CoordinateVector<int> readIdx({"y", "x", "c"}, {outputBuf.height() - 1, outputBuf.width() - 1, outputBuf.channels() - 1});
  if (!is2D(outputBuf)) {
    readIdx.setIncrement("c", 3);
  }

  while (cycles < maxCycles && !readIdx.allDone()) {
    cout << "Read index = " << readIdx.coordString() << endl;
    cout << "Cycles     = " << cycles << endl;

    run_for_cycle(writeIdx, readIdx,
        hwInputBuf, outputBuf,
        inputName, outputName,
        state);
    cycles++;
  }

  assert(readIdx.allDone());

  // Just to be sure, run for a few more cycles and check that no extra valids come out
  const int NUM_BUFFER_CYCLES = 5;
  int i = 0;
  while (i < NUM_BUFFER_CYCLES) {

    state.setValue("self.in_en", BitVector(1, false));
    state.exeCombinational();
    bool valid_value = state.getBitVec("self.valid").to_type<bool>();
    assert(!valid_value);
    state.exeSequential();
    i++;
  }


}

template<typename T>
void runHWKernel(CoreIR::Module* m, Halide::Runtime::Buffer<T>& hwInputBuf, Halide::Runtime::Buffer<T>& outputBuf) {
  runHWKernel("self.in_arg_0_0_0", m, hwInputBuf, outputBuf);
}

CoreIR::Context* hwContext() {
  CoreIR::Context* context = newContext();
  CoreIRLoadLibrary_commonlib(context);
  CoreIRLoadLibrary_float(context);
  return context;
}

CoreIR::Module* buildModule(Halide::Internal::HardwareInfo& info, bool useUbuffer, CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Target t;
  t = t.with_feature(Target::Feature::CoreIR);
  if (!useUbuffer) {
    t = t.with_feature(Target::Feature::UseExtractHWKernel);
  }
  auto hm = hwOutput.compile_to_module(args, name, t);
  cout << "Compiled to module..." << endl;
  cout << hm << endl;
  for (auto f : hm.functions()) {
    cout << "Generating coreir for function " << f.name << endl;
    Halide::Internal::CodeGen_CoreHLS_Kernel gen("conv_3_3_app.json");
    gen.info = info;
    f.body.accept(&gen);
  }

  if (!loadFromFile(context, "./conv_3_3_app.json")) {
    cout << "Error: Could not load json for unit test!" << endl;
    context->die();
  }
  context->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});
  CoreIR::Module* m = context->getNamespace("global")->getModule("DesignTop");
  cout << "Module after wiring clocks ..." << endl;
  m->print();
  return m;
}

CoreIR::Module* buildModule(const bool useUbuffer, CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Halide::Internal::HardwareInfo info;
  info.hasCriticalPathTarget = false;
  return buildModule(info, useUbuffer, context, name, args, fName, hwOutput);
}

CoreIR::Module* buildModule(CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Halide::Internal::HardwareInfo info;
  info.hasCriticalPathTarget = false;
  return buildModule(info, false, context, name, args, fName, hwOutput);
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
    //state.setValue("self.in_en", BitVector(1, false));
    state.setValue(input_name, BitVector(16, input(x,y,c)));
    std::cout << "y=" << y << ",x=" << x << " " << dec << "in=" << (int) input(x, y, c) << endl;
    //std::cout << "y=" << y << ",x=" << x << " " << "in=" << (int) input(x, y, c) << endl;

    writeIdx.increment();
  } else {
    cout << "Writes to accelerator done, now just processing output" << endl;
    state.setValue("self.in_en", BitVector(1, false));
    state.setValue(input_name, BitVector(16, 0));
  }
  // propogate to all wires
  state.exeCombinational();

  // read output wire
  bool valid_value = state.getBitVec("self.valid").to_type<bool>();

  //cout << "Read window with " << readIdx.currentWindowRelative().size() << " points..." << endl;
  for (auto point : readIdx.currentWindowRelative()) {
    assert(point.numDims() > 0);
  }
  
  if (valid_value) {
    cout << "\tOutput window is valid" << endl;
    for (auto point : readIdx.currentWindow()) {
      assert(point.numDims() > 0);
      auto windowPos = readIdx.relativeWindowPosition(point);
      cout << "\t\tReadIndex                   = " << readIdx.coordString() << endl;
      cout << "\t\tRelative position in window = " << windowPos << endl;
      BitVector output_bv;
      if (!is2D(output)) {
        output_bv = state.getBitVec(outputForCoord3D(windowPos));
      } else {
        cout << "\t\t2D Output coord for " << windowPos << " = " << outputForCoord3D(windowPos) << endl;
        output_bv = state.getBitVec(outputForCoord2D(windowPos));
        cout << "\t\t2D Output bv = "  << output_bv << endl;
      }
      T output_value;
      output_value = output_bv.to_type<T>();
      //std::cout << "\tthis one is valid = " << output_bv << ", int = " << output_bv.to_type<int>() << endl;

      const int xr = point.coord("x");
      const int yr = point.coord("y");
      const int cr = point.coord("c");
      output(xr, yr, cr) = output_value;
    }
    readIdx.increment();
  }


  //std::cout << "y=" << y << ",x=" << x << " " << "in=" << (state.getBitVec(input_name)) << " out=" << +output_value << " based on bv=" << state.getBitVec(output_name).to_type<int>() << dec << endl;
  //std::cout << "y=" << y << ",x=" << x << " " << hex << "in=" << (state.getBitVec(input_name)) << " out=" << +output_value << " based on bv=" << state.getBitVec(output_name).to_type<int>() << dec << endl;

  // give another rising edge (execute seq)
  state.exeSequential();
}

template<typename T>
void compare_buffers(Halide::Runtime::Buffer<T>& outputBuf, Halide::Buffer<T>& cpuOutput) {
  cout << "Comparing buffers..." << endl;
  cout << "Hardware output" << endl;
  printBuffer(outputBuf, cout);
  
  cout << endl;
  cout << "CPU Output" << endl;
  printBuffer(cpuOutput, cout);

  for (int i = 0; i < outputBuf.width(); i++) {
    for (int j = 0; j < outputBuf.height(); j++) {
      for (int b = 0; b < outputBuf.channels(); b++) {
        if (outputBuf(i, j, b) != cpuOutput(i, j, b)) {
          cout << "Error: Buffers do not match at " << i << ", " << j << ", " << b << endl;
        }
        assert(outputBuf(i, j, b) == cpuOutput(i, j, b));
      }
    }
    cout << endl;
  }

}
using namespace Halide::Internal;

class AcceleratorCallConsolidator : public IRMutator {
  public:
    std::vector<const Call*> accelCalls;
    bool inHWRegion;

    AcceleratorCallConsolidator() : inHWRegion(false) {}
  
    Stmt visit(const ProducerConsumer* p) {
      if (inHWRegion && p->is_producer) {
        // Consolidate calls in to one larger call
        cout << "Accelerator calls above " << p->name << "...." << endl;
        for (auto c : accelCalls) {
          cout << "\t" << "Call" << endl;
        }
        assert(accelCalls.size() == 2);
        auto imageToStream = accelCalls[0];
        auto streamToImage = accelCalls[1];

        vector<Expr> args;
        for (uint i = 1; i < imageToStream->args.size(); i++) {
          args.push_back(imageToStream->args[i]);
        }
        // add padding
        int numDims = (((int) imageToStream->args.size()) - 1 - 2) / 2;
        numDims = 4 - numDims;
        cout << "num dims in arg list = " << numDims << endl;
        for (int i = 0; i < numDims; i++) {
          args.push_back(IntImm::make(Int(32), 1));
          args.push_back(IntImm::make(Int(32), 1));
        }
        
        {
          for (uint i = 1; i < streamToImage->args.size(); i++) {
            args.push_back(streamToImage->args[i]);
          }
          int numDims = (((int) streamToImage->args.size()) - 1 - 2) / 2;
          numDims = 4 - numDims;
          cout << "num dims in arg list = " << numDims << endl;
          for (int i = 0; i < numDims; i++) {
            args.push_back(IntImm::make(Int(32), 1));
            args.push_back(IntImm::make(Int(32), 1));
          }
        }        
        auto accelCall = Call::make(Handle(), "call_accelerator", args, Call::CallType::ExternCPlusPlus);
        auto eval = Evaluate::make(accelCall);
        auto newBody = Block::make(eval, p);
        return newBody;
        //auto newBody = Block::make(eval, p->body);
        //Stmt newPc = ProducerConsumer::make(p->name + ".accel", p->is_producer, newBody);

        //cout << "New pc = " << newPc << endl;
        //return newPc;
        //p;
      } else {
        return IRMutator::visit(p);
      }
    }
    
    Stmt visit(const Realize* p) {

      //cout << "Visiting realize " << p->name << endl;

      bool justEntered = inHWRegion == false;
      if (justEntered) {
        inHWRegion = true;
      }

      auto res = IRMutator::visit(p);
      //auto res = p->body.accept(this);

      if (justEntered) {
        inHWRegion = false;
      }

      return res;
    }

    Expr visit(const Call* c) {
      //cout << "Visiting call " << c->name << endl;
      if (c->name == "stream_subimage") {
        accelCalls.push_back(c);
      }
      //cout << "Done with call" << endl;
      return c;
    }
};

class CodeGen_SoC_Test : public CodeGen_C {
  public:

    bool inHWRegion;

    CodeGen_SoC_Test(std::ostream &dest,
              Target target,
              OutputKind output_kind = CImplementation,
              const std::string &include_guard = "") :
      CodeGen_C(dest, target, output_kind, include_guard) {
        inHWRegion = false;
      }

    ~CodeGen_SoC_Test() {
      //std::cout << "Calling destructor for printer" << endl;
      CodeGen_C::~CodeGen_C();
    }

    void compileForCGRA(const Halide::Module& input) {
      stream << "// CGRA Specific wrapper code" << endl;
      stream << "#include \"cgra_wrapper.h\"" << endl;
      stream << endl;
      CodeGen_SoC_Test::compile(input);
      stream.flush();
    }

    void visit(const Provide* p) {
      stream << "// Found a provide, must be start of something hardware related!" << endl;
    }

    void visit(const Call* c) {
      if (c->name == "stream_subimage") {

        stream << "// Call to stream_subimage, we need to do commands here?..." << endl;
        const StringImm *direction = c->args[0].as<StringImm>();

        string a1 = print_expr(c->args[1]);
        //string a2 = print_expr(c->args[2]);
        //string a3 = print_expr(c->args[3]);

        cout << "args->[3] = " << c->args[3] << endl;
        cout << "args->[3]->args[0] = " << c->args[3].as<Call>()->args[0] << endl;
        string offset = print_expr(c->args[3].as<Call>()->args[0].as<Load>()->index);
        cout << "offset = " << offset << endl;
        //assert(false);
        //string val = print_expr(c->args[3].as<Call>()->args[0]);
        //cout << "Offset value = " << val << endl;

        if (direction->value == "buffer_to_stream") {
          stream << "accelerator.subimage_to_stream(";
        } else {
          assert(direction->value == "stream_to_buffer");
          stream << "accelerator.stream_to_subimage(";
        }
        
        //stream << a1 << ", " << a2 << ", " << a3;
        stream << a1 << ", " << offset;
        for (size_t i = 4; i < c->args.size(); i++) {
            stream << ", " << print_expr(c->args[i]);
        }

        stream << ");" << endl;
      } else if (c->name == "call_accelerator") {
        stream << "// Call accelerator..." << endl;

        // # args = 2*( call_name + buf_name + buf_offset + strides )
        vector<string> sourceIndexing;
        vector<string> destIndexing;
        string sourceBuffer;
        string destBuffer;
        string sourceOffset;
        string destOffset;
        {
          int argBase = 0;
          sourceBuffer = print_expr(c->args[argBase]);
          sourceOffset = print_expr(c->args[argBase + 2].as<Call>()->args[0].as<Load>()->index);
          for (size_t i = argBase + 3; i < argBase + 3 + 2*4; i++) {
            sourceIndexing.push_back(print_expr(c->args[i]));
          }
        }
        
        {
          int argBase = 3 + 2*4;
          destBuffer = print_expr(c->args[argBase]);
          destOffset = print_expr(c->args[argBase + 2].as<Call>()->args[0].as<Load>()->index);
          for (size_t i = argBase + 3; i < argBase + 3 + 2*4; i++) {
            destIndexing.push_back(print_expr(c->args[i]));
          }
        }
        
        stream << "accelerator.produce_subimage(";
        stream << sourceBuffer << ", " << sourceOffset;
        for (auto ind : sourceIndexing) {
          stream << ", " << ind;
        }
        stream << ", ";
        stream << destBuffer << ", " << destOffset;
        for (auto ind : destIndexing) {
          stream << ", " << ind;
        }
        stream << ");" << endl;

      }else {
        CodeGen_C::visit(c);
      }
    }

    void visit(const ProducerConsumer* p) {
      if (inHWRegion && p->is_producer) {
        stream << "// Should generate CoreIR for produce: " << p->name << endl;
        // Generate coreir module for this statement...
      } else {
        CodeGen_C::visit(p);
      }
    }

    void visit(const Evaluate* op) {
      if (inHWRegion) {
        stream << "// Evaluating... = " << op->value << endl;
        op->value.accept(this);
      } else {
        CodeGen_C::visit(op);
      }
    }

    void visit(const For* f) {
      stream << "// Entering for loop" << endl;
      CodeGen_C::visit(f);
      stream << "// Leaving for loop" << endl;
    }

    void visit(const Realize* p) {

      //stream << "// Found a realize of " << p->name << ", entering outermost hardware region" << endl;
      //stream << "CGRAWrapper accelerator;" << std::endl;
      
      bool justEntered = inHWRegion == false;
      if (justEntered) {
        stream << "// Found a realize of " << p->name << ", entering outermost hardware region" << endl;
        stream << "std::cout << \"Computing output tile with accelerator\" << std::endl;\n";
        stream << "CGRAWrapper accelerator;" << std::endl;
        inHWRegion = true;
      }

      p->body.accept(this);

      if (justEntered) {
        stream << "// Done with hardware region" << endl;
        inHWRegion = false;
      }
    }
};

class HWRegionFinder : public IRGraphVisitor {
  public:
    bool foundRegion;
    const ProducerConsumer* region;

    HWRegionFinder() : foundRegion(false) {}

    //void visit(const Realize* p) {
    void visit(const ProducerConsumer* p) {
      if (starts_with(p->name, "_hls_target")) {
        region = p;
        foundRegion = true;
      } else {
        IRGraphVisitor::visit(p);
      }
    }

};

void offset_window_test() {
  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 2);

  Var x("x"), y("y"), c("c");
  Var xi,yi,zi, xo,yo,zo;

  Func hw_input, hw_output;
  hw_input(x, y) = cast<uint16_t>(input(x+1,y+1));
  hw_output(x, y) = hw_input(x, y);
  output(x, y) = hw_output(x, y);

  hw_input.compute_root();
  hw_output.compute_root();

  Halide::Buffer<uint16_t> inputBuf(15, 15);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(8, 8);
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      inputBuf(i, j) = j*inputBuf.width() + i;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

  //Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(outputBuf.width(), outputBuf.height());
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  
  hw_output.tile(x, y, xo, yo, xi, yi, 8, 8);
  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
 
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "hw_offset_window", args, "offset_window", hw_output);

  {
    {
      Target t;
      t = t.with_feature(Target::Feature::CoreIR);
      t = t.with_feature(Target::Feature::UseExtractHWKernel);
      auto mod = hw_output.compile_to_module(args, "hw_output", t);

      //cout << "Module before consolidation..." << endl;
      //cout << mod << endl;

      for (auto& f : mod.functions()) {
        //cout << "Consolidating function" << f << endl;
        AcceleratorCallConsolidator cons;
        f.body = cons.mutate(f.body);
      }

      cout << "Compiled to module" << endl;
      cout << mod << endl;
      ofstream outFile("offset_soc_mini.cpp");
      CodeGen_SoC_Test testPrinter(outFile, t, CodeGen_C::OutputKind::CPlusPlusImplementation);
      testPrinter.compileForCGRA(mod);
      
      cout << "Compiled cpp code" << endl;
    }
    cout << "Done with compiling for CGRA" << endl;
    runCmd("g++ -std=c++11 offset_soc_run.cpp offset_soc_mini.cpp cgra_wrapper.cpp -I ../../../../tools `libpng-config --cflags --ldflags` -ljpeg -lHalide -lcoreir-float -lcoreir -lcoreir-commonlib -lcoreirsim -L ../../../../bin");
    cout << "Compiled c++ executable..." << endl;
    runCmd("./a.out");

    cout << "Ran executable" << endl;
    Halide::Runtime::Buffer<uint16_t> cppRes = load_image("offset.pgm");
    cout << "C++ res from pgm..." << endl;
    printBuffer(cppRes, cout);
    cout << "CPU output" << endl;
    printBuffer(cpuOutput, cout);
    compare_buffers(cppRes, cpuOutput);
  }

  //assert(false);

  //auto m = buildModule(context, "demosaic_coreir", args, "demosaic", hw_output);

  //runHWKernel("self.in_arg_3_0_0", m, hwInputBuf, outputBuf);
  //cout << "Input buffer" << endl;
  //printBuffer(hwInputBuf, cout);
  
  cout << GREEN << "Small offset test passed" << RESET << endl;
}
void small_demosaic_test() {

  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 3);

  Var x("x"), y("y"), c("c");
  Var xi,yi,zi, xo,yo,zo;

  Func hw_input;
  hw_input(x,y) = cast<uint16_t>(input(x+1,y+1));

  // common patterns: average of four surrounding pixels
  Func neswNeighbors, diagNeighbors;
  // Should this actually be /4 outside of the hw input?
  neswNeighbors(x, y) = (hw_input(x-1, y)   + hw_input(x+1, y) +
      hw_input(x,   y-1) + hw_input(x,   y+1)) / 4;
  diagNeighbors(x, y) = (hw_input(x-1, y-1) + hw_input(x+1, y-1) +
      hw_input(x-1, y+1) + hw_input(x+1, y+1)) / 4;

  // common patterns: average of two adjacent pixels
  Func vNeighbors, hNeighbors;
  vNeighbors(x, y) = (hw_input(x, y-1) + hw_input(x, y+1))/2;
  hNeighbors(x, y) = (hw_input(x-1, y) + hw_input(x+1, y))/2;

  int phase = 0;

  // output pixels depending on image layout.
  // Generally, image looks like
  //    R G R G R G R G
  //    G B G B G B G B
  //    R G R G R G R G
  //    G B G B G B G B
  Func green, red, blue;
  green(x, y) = select((y % 2) == (phase / 2),
      select((x % 2) == (phase % 2), neswNeighbors(x, y), hw_input(x, y)), // First row, RG
      select((x % 2) == (phase % 2), hw_input(x, y),      neswNeighbors(x, y))); // Second row, GB

  red(x, y) = select((y % 2) == (phase / 2),
      select((x % 2) == (phase % 2), hw_input(x, y),   hNeighbors(x, y)), // First row, RG
      select((x % 2) == (phase % 2), vNeighbors(x, y), diagNeighbors(x, y))); // Second row, GB

  blue(x, y) = select((y % 2) == (phase / 2),
      select((x % 2) == (phase % 2), diagNeighbors(x, y), vNeighbors(x, y)), // First row, RG
      select((x % 2) == (phase % 2), hNeighbors(x, y),    hw_input(x, y))); // Second row, GB

  // output all channels
  Func demosaic, hw_output;
  demosaic(x,y,c) = cast<uint16_t>(select(c == 0, red(x, y),
        c == 1, green(x, y),
        blue(x, y)));

  hw_output(x,y,c) = demosaic(x,y,c);
  output(x,y,c) = hw_output(x,y,c);

  // Schedule common
  demosaic.bound(c, 0, 3);
  hw_output.bound(c, 0, 3);
  //output.bound(c, 0, 3);

  hw_input.compute_root();
  hw_output.compute_root();

  Halide::Buffer<uint16_t> inputBuf(200, 200);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  //Halide::Runtime::Buffer<uint16_t> outputBuf(inputBuf.width() - 8, inputBuf.height() - 8, 3);
  Halide::Runtime::Buffer<uint16_t> outputBuf(10, 10, 3);
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      //inputBuf(i, j) = rand() % 255;
      //inputBuf(i, j) = 23;
      inputBuf(i, j) = i + j;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

  //Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(outputBuf.width(), outputBuf.height(), outputBuf.channels());
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  
  hw_output.tile(x, y, xo, yo, xi, yi, 10, 10)
    .reorder(c, xi, yi, xo, yo);

  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
  hw_output.unroll(c);

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "hw_demosaic", args, "demosaic", hw_output);
  {
    {
      Target t;
      t = t.with_feature(Target::Feature::CoreIR);
      t = t.with_feature(Target::Feature::UseExtractHWKernel);
      auto mod = hw_output.compile_to_module(args, "hw_demosaic", t);

      for (auto& f : mod.functions()) {
        AcceleratorCallConsolidator cons;
        cout << "Starting consolidate of calls" << endl;
        f.body = cons.mutate(f.body);
        cout << "Consolidated calls for " << f.body << endl;
        //f.body.accept(&cons);
      }

      cout << "Compiled to module" << endl;
      cout << mod << endl;
      //assert(false);
      ofstream outFile("demosaic_soc_mini.cpp");
      CodeGen_SoC_Test testPrinter(outFile, t, CodeGen_C::OutputKind::CPlusPlusImplementation);
      testPrinter.compileForCGRA(mod);
      
      cout << "Compiled cpp code" << endl;
    }
    cout << "Done with compiling for CGRA" << endl;
    runCmd("g++ -std=c++11 demosaic_soc_run.cpp demosaic_soc_mini.cpp cgra_wrapper.cpp -I ../../../../tools `libpng-config --cflags --ldflags` -ljpeg -lHalide -lcoreir-float -lcoreir -lcoreir-commonlib -lcoreirsim -L ../../../../bin");
    cout << "Compiled c++ executable..." << endl;
    runCmd("./a.out");

    cout << "Ran executable" << endl;
    Halide::Runtime::Buffer<uint16_t> cppRes = load_image("demosaic.ppm");
    cout << "CPU output" << endl;
    printBuffer(cpuOutput, cout);

    compare_buffers(cppRes, cpuOutput);
  }
  
  cout << GREEN << "Demosaic test passed" << RESET << endl;
}

void multi_channel_conv_test() {
  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 3);

  Var x("x"), y("y"), z("z");
  Var xi,yi,zi, xo,yo,zo;

  Func kernel("kernel");
  kernel(x) = 0;
  kernel(0) = 0;
  kernel(1) = 1;
  kernel(2) = 2;

  Func conv("conv");
  Func hw_input("hw_input");
  hw_input(x, y) = cast<uint16_t>(input(x, y));
  conv(x, y, z) = hw_input(x, y) + kernel(z);
  
  Func hw_output("hw_output");
  hw_output(x, y, z) = cast<uint16_t>(conv(x, y, z));
  output(x, y, z) = hw_output(x, y, z);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

  // Creating input data
  Halide::Buffer<uint16_t> inputBuf(4, 4);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(inputBuf.width(), inputBuf.height(), 3);
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      inputBuf(i, j) = rand() % 255;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

  //Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(outputBuf.width(), outputBuf.height(), outputBuf.channels());
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  cout << "CPU output" << endl;
  printBuffer(cpuOutput, cout);
  
  // Hardware schedule
  int tileSize = 8;
  hw_output.bound(z, 0, 3);
  conv.bound(z, 0, 3);

  hw_output.reorder(z, x, y).tile(x, y, xo, yo, xi, yi, tileSize, tileSize).
    unroll(z).
    hw_accelerate(xi, xo);
  //conv.unroll(z, 3);
  hw_input.stream_to_accelerator();

  cout << "Loop nest.." << endl;
  hw_output.print_loop_nest();
  
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "mc_conv_coreir", args, "mc_conv", hw_output);

  runHWKernel(m, hwInputBuf, outputBuf);

  cout << "Input buffer" << endl;
  printBuffer(hwInputBuf, cout);
  compare_buffers(outputBuf, cpuOutput);

  cout << GREEN << "Multi channel conv test passed" << RESET << endl;
}

void clamped_grad_x_test() {

  // Build the app
  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input("hw_input");
  hw_input(x, y) = input(x, y);
  Func padded16;
  padded16(x, y) = cast<uint16_t>(0);
  padded16(x, y) = hw_input(x + 3, y + 3);
  // Sobel filter
  Func grad_x_unclamp, grad_x;
  grad_x_unclamp(x, y) = cast<int16_t>(-padded16(x - 1, y - 1) + padded16(x + 1, y - 1)
      -2*padded16(x - 1, y) + 2*padded16(x + 1, y)
      -padded16(x-1, y+1) + padded16(x + 1, y + 1));

  grad_x(x, y) = clamp(grad_x_unclamp(x, y), -255, 255);

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint16_t>(grad_x(x, y));
  output(x, y) = hw_output(x,y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

  // Creating input data
  Halide::Buffer<uint16_t> inputBuf(32, 32);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(16, 16, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = rand() % 255;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

  {
    Halide::Buffer<uint16_t> paddingOut(4, 4);
    paddingOut.set_min(-3, -3);
    cout << "Padding y range = " << paddingOut.bottom() << ", " << paddingOut.top() << endl;
    ParamMap rParams;
    rParams.set(input, inputBuf);
    Target t;
    padded16.realize(paddingOut, t, rParams);

    cout << "Original" << endl;
    printBuffer(inputBuf, cout);

    cout << "Padded" << endl;
    printBuffer(paddingOut, cout);
  }
  assert(false);
  
  // Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(4, 4);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Hardware schedule
  padded16.compute_root();
  int tileSize = 16;
  hw_output.tile(x, y, xo, yo, xi, yi, tileSize, tileSize).accelerate({padded16}, xi, xo);
  hw_input.stream_to_accelerator();
  grad_x_unclamp.linebuffer();
  grad_x.linebuffer();
  
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_harris", args, "harris", hw_output);

  runHWKernel(m, hwInputBuf, outputBuf);
  cout << "Input buf" << endl;
  printBuffer(inputBuf, cout);

  compare_buffers(outputBuf, cpuOutput);

  cout << GREEN << "Harris test passed" << RESET << endl;
}

void control_path_test() {
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<int16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input, hw_output, clamped;
  hw_input(x, y) = input(x, y);
  clamped(x, y) = select(x % 2 == 0, hw_input(x, y), hw_input(x, y) + 1);

  hw_output(x, y) = cast<int16_t>(clamped(x, y));
  output(x, y) = hw_output(x, y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

   // Creating input data
   int nRows = 4;
   int nCols = 2;
  Halide::Buffer<int16_t> inputBuf(nCols, nRows);
  Halide::Runtime::Buffer<int16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  Halide::Runtime::Buffer<int16_t> outputBuf(inputBuf.width(), inputBuf.height(), 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(j, i) = 0;
      hwInputBuf(j, i, 0) = inputBuf(j, i);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<int16_t> cpuOutput(outputBuf.width(), outputBuf.height());
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Create HW schedule
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, xo);
  clamped.linebuffer();
  hw_input.stream_to_accelerator();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_harris", args, "harris", hw_output);

  auto accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  
  //runHWKernel("self.in_arg_2_0_0", m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "Control path test passed" << RESET << endl;
}

void control_path_xy_test() {
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<int16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input, hw_output, clamped;
  hw_input(x, y) = input(x, y);
  clamped(x, y) = select(x % 2 == 0, select(y % 2 == 0, hw_input(x, y), hw_input(x, y) + 1),
      select(y % 2 == 0, hw_input(x, y) + 2, hw_input(x, y) + 3));

  hw_output(x, y) = cast<int16_t>(clamped(x, y));
  output(x, y) = hw_output(x, y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

   // Creating input data
   int nRows = 4;
   int nCols = 2;
  Halide::Buffer<int16_t> inputBuf(nCols, nRows);
  Halide::Runtime::Buffer<int16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  Halide::Runtime::Buffer<int16_t> outputBuf(inputBuf.width(), inputBuf.height(), 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(j, i) = 0;
      hwInputBuf(j, i, 0) = inputBuf(j, i);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<int16_t> cpuOutput(outputBuf.width(), outputBuf.height());
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Create HW schedule
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, xo);
  clamped.linebuffer();
  hw_input.stream_to_accelerator();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_harris", args, "harris", hw_output);

  //runHWKernel("self.in_arg_3_0_0", m, hwInputBuf, outputBuf);
  auto accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "Control path xy test passed" << RESET << endl;
}
void mod2_test() {
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<int16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input, hw_output, clamped;
  hw_input(x, y) = input(x, y);
  clamped(x, y) = hw_input(x, y) % 2;

  hw_output(x, y) = cast<int16_t>(clamped(x, y));
  output(x, y) = hw_output(x, y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

   // Creating input data
  Halide::Buffer<int16_t> inputBuf(2, 2);
  Halide::Runtime::Buffer<int16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<int16_t> outputBuf(2, 2, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = i + j;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<int16_t> cpuOutput(2, 2);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Create HW schedule
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, xo);
  clamped.linebuffer();
  hw_input.stream_to_accelerator();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_harris", args, "harris", hw_output);

  runHWKernel(m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "mod2 test passed" << RESET << endl;
}

void shiftRight_test() {
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<int16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input, hw_output, clamped;
  hw_input(x, y) = input(x, y);
  clamped(x, y) = hw_input(x, y) >> 7;

  hw_output(x, y) = cast<int16_t>(clamped(x, y));
  output(x, y) = hw_output(x, y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

   // Creating input data
  Halide::Buffer<int16_t> inputBuf(2, 2);
  Halide::Runtime::Buffer<int16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<int16_t> outputBuf(2, 2, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = rand();
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<int16_t> cpuOutput(2, 2);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Create HW schedule
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, xo);
  clamped.linebuffer();
  hw_input.stream_to_accelerator();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_harris", args, "harris", hw_output);

  runHWKernel(m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "shiftRight test passed" << RESET << endl;
}

void rom_read_test() {
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<int16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input, hw_output, clamped, curve;
  {
    curve(x) = 0;
    curve(0) = 1;
    curve(1) = 2;
    curve(2) = 234;
    curve(3) = 15;
  }

  curve.bound(x, 0, 4);

  hw_input(x, y) = input(x, y);

  hw_output(x, y) = cast<int16_t>(curve(x) + hw_input(x, y));
  output(x, y) = hw_output(x, y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

   // Creating input data
  Halide::Buffer<int16_t> inputBuf(2, 2);
  Halide::Runtime::Buffer<int16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<int16_t> outputBuf(2, 2, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = rand();
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<int16_t> cpuOutput(2, 2);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Create HW schedule
  curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, xo);
  hw_input.stream_to_accelerator();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_curve", args, "curve", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");
  //runHWKernel("self.in_arg_2_0_0", m, hwInputBuf, outputBuf);
  runHWKernel(accelName, m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "ROM read test passed" << RESET << endl;
}

void clamp_test() {
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<int16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func hw_input, hw_output, clamped;
  hw_input(x, y) = input(x, y);
  clamped(x, y) = clamp(hw_input(x, y), cast<int16_t>(-255), cast<int16_t>(255));

  hw_output(x, y) = cast<int16_t>(clamped(x, y));
  output(x, y) = hw_output(x, y);

  // Create common elements of the CPU and hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

   // Creating input data
  Halide::Buffer<int16_t> inputBuf(2, 2);
  Halide::Runtime::Buffer<int16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<int16_t> outputBuf(2, 2, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = rand();
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<int16_t> cpuOutput(2, 2);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Create HW schedule
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2).hw_accelerate(xi, xo);
  clamped.linebuffer();
  hw_input.stream_to_accelerator();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_harris", args, "harris", hw_output);

  runHWKernel(m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "clamp test passed" << RESET << endl;
}

void small_harris_test() {

  int blockSize = 3;

  // k is a sensitivity parameter for detecting corners.
  // k should vary from 0.04 to 0.15 according to literature.
  int shiftk = 4; // equiv to k = 0.0625

  // Threshold for cornerness measure.
  int threshold = 1;
  // Build the app
  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;

  Func padded16, padded;
  padded16(x, y) = input(x + 3, y + 3);

  // Sobel filter
  Func grad_x_unclamp, grad_y_unclamp, grad_x, grad_y;
  grad_x_unclamp(x, y) = cast<int16_t>(-padded16(x - 1, y - 1) + padded16(x + 1, y - 1)
      -2*padded16(x - 1, y) + 2*padded16(x + 1, y)
      -padded16(x-1, y+1) + padded16(x + 1, y + 1));

  grad_y_unclamp(x, y) = cast<int16_t>(padded16(x - 1, y + 1) - padded16(x - 1, y - 1) +
      2*padded16(x, y + 1) - 2*padded16(x, y - 1) + 
      padded16(x + 1, y + 1) - padded16(x + 1, y - 1));

  grad_x(x, y) = clamp(grad_x_unclamp(x, y), -255, 255);
  grad_y(x, y) = clamp(grad_y_unclamp(x, y), -255, 255);

  // Product of gradients
  Func grad_xx, grad_yy, grad_xy;
  grad_xx(x, y) = grad_x(x, y) * grad_x(x, y);
  grad_yy(x, y) = grad_y(x, y) * grad_y(x, y);
  grad_xy(x, y) = grad_x(x, y) * grad_y(x, y);

  // Shift gradients
  Func lxx, lyy, lxy;
  lxx(x, y) = grad_xx(x, y) >> 7;
  lyy(x, y) = grad_yy(x, y) >> 7;
  lxy(x, y) = grad_xy(x, y) >> 7;

  // Box filter
  Func lgxx, lgyy, lgxy;
  RDom box(-blockSize / 2, blockSize, -blockSize/2, blockSize);
  lgxx(x, y) += lxx(x + box.x, y + box.y);
  lgyy(x, y) += lyy(x + box.x, y + box.y);
  lgxy(x, y) += lxy(x + box.x, y + box.y);

  Expr lgxx8 = lgxx(x, y) >> 6;
  Expr lgyy8 = lgyy(x, y) >> 6;
  Expr lgxy8 = lgxy(x, y) >> 6;

  // calculate Cim
  // int scale = (1 << (Ksize - 1)) * blockSize
  Func cim;
  Expr det = lgxx8*lgyy8 - lgxy8*lgxy8;
  Expr trace = lgxx8 + lgyy8;
  cim(x, y) = det - (trace*trace >> shiftk);

  // Perform non-maximal suppression
  Func hw_output("hw_output");
  Expr is_max = cim(x, y) > cim(x - 1, y - 1) && cim(x, y) > cim(x, y - 1) &&
    cim(x, y) > cim(x + 1, y - 1) && cim(x, y) > cim(x - 1, y) &&
    cim(x, y) > cim(x + 1, y) && cim(x, y) > cim(x - 1, y  + 1) &&
    cim(x, y) > cim(x, y + 1) && cim(x, y) > cim(x + 1, y + 1);
  hw_output(x, y) = cast<uint16_t>(select(is_max && (cim(x, y) >= threshold), 255, 0));
  output(x, y) = hw_output(x,y);

  // Create common elements of the CPU and hardware schedule

  //hw_input.compute_root();
  hw_output.compute_root();

  // Creating input data
  Halide::Buffer<uint16_t> inputBuf(16, 16);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(4, 4, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = rand() % 255;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }
 
  // Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(4, 4);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);
  cout << "CPU output..." << endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      cout << (int) cpuOutput(i, j) << " ";
    }
    cout << endl;
  }

 compare_buffers(outputBuf, cpuOutput);
 cout << GREEN << "Harris test passed" << RESET << endl;
}

void small_cascade_test() {

  cout << "Starting cascade test" << endl;

  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 2);

  Var x("x"), y("y");

  Var xi,yi, xo,yo;
  
  Func kernel("kernel");
  Func conv1("conv1"), conv2("conv2");
  RDom r(0, 3,
      0, 3);

  RDom r0(0, 3,
      0, 3);
  kernel(x,y) = 0;
  kernel(0,0) = 1;      kernel(0,1) = 2;      kernel(0,2) = 1;
  kernel(1,0) = 2;      kernel(1,1) = 4;      kernel(1,2) = 3;
  kernel(2,0) = 1;      kernel(2,1) = 2;      kernel(2,2) = 1;

  conv1(x, y) = 0;

  Func hw_input("hw_input");
  hw_input(x, y) = cast<uint16_t>(input(x, y));
  conv1(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
  conv2(x, y)  += kernel(r0.x, r0.y) * conv1(x + r0.x, y + r0.y);
  
  kernel.compute_root();

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint16_t>(conv2(x, y));
  output(x, y) = hw_output(x,y);

  hw_input.compute_root();
  hw_output.compute_root();

  // Creating input data
  Halide::Buffer<uint16_t> inputBuf(8, 8);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(8, 8, 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(4, 4, 1);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      inputBuf(i, j) = 11;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }
 
  // Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(4, 4);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);
  cout << "CPU output..." << endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      cout << (int) cpuOutput(i, j) << " ";
    }
    cout << endl;
  }

  //int tileSize = 8;
  hw_output.tile(x,y, xo,yo, xi,yi, 8-4, 8-4)
    .hw_accelerate(xi, xo);

  kernel.compute_at(hw_output, xo);

  conv1.update()
    .unroll(r.x)
    .unroll(r.y);
  conv1.linebuffer();

  conv2.update()
    .unroll(r0.x)
    .unroll(r0.y);
  conv2.linebuffer();

  hw_input.stream_to_accelerator();
  
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_cascade", args, "cascade", hw_output);
  //cout << "Module = " << endl;
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

  //cout << "final buffer" << endl;
  for (int i = 0; i < outputBuf.height(); i++) {
    for (int j = 0; j < outputBuf.width(); j++) {
      for (int b = 0; b < outputBuf.channels(); b++) {
        //cout << (int) outputBuf(i, j, b) << " ";
        //cout << (int) cpuOutput(i, j, b) << " ";
        assert(outputBuf(i, j, b) == cpuOutput(i, j, b));
      }
    }
    cout << endl;
  }
  cout << GREEN << "Cascade test passed" << RESET << endl;
}

void small_conv_3_3_not_unrolled_test() {
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
  Halide::Buffer<uint8_t> inputBuf(tileSize + 2, tileSize + 2);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  indexTestPatternRandom(inputBuf, hwInputBuf);
  Halide::Runtime::Buffer<uint8_t> outputBuf(tileSize, tileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);
  
  hw_output.tile(x,y, xo,yo, xi,yi, tileSize-2, tileSize-2)
    .hw_accelerate(xi, xo);

  kernel.compute_at(hw_output, xo)
    .unroll(x).unroll(y);

  conv.update();
  conv.linebuffer();

  hw_input.stream_to_accelerator();

  hw_output.print_loop_nest();

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "coreir_curve", args, "curve", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  cout << GREEN << "conv 3x3 not unrolled test passed" << RESET << endl;
  assert(false);
}

void small_conv_3_3_critical_path_test() {
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
  Halide::Internal::HardwareInfo info;
  info.techlib.criticalPath["mul"] = 100;
  info.techlib.criticalPath["add"] = 100;
  info.hasCriticalPathTarget = true;
  info.criticalPathTarget = 150;
  auto m = buildModule(info, false, context, "coreir_conv_3_3", args, "conv_3_3", hw_output);
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

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);
 
  PRINT_PASSED("Conv 3x3 with critical path test passed");
}

void small_conv_3_3_test() {
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
  auto m = buildModule(context, "coreir_conv_3_3", args, "conv_3_3", hw_output);
  //auto m = buildModule(true, context, "coreir_conv_3_3", args, "conv_3_3", output);
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
}

void pointwise_add_test() {

    ImageParam input(type_of<uint8_t>(), 2);
    Func output;
    //ImageParam output(type_of<uint8_t>(), 2);

    Var x, y;

    Func hw_input, hw_output;
    Func mult("mult");
    hw_input(x, y) = cast<uint16_t>(input(x, y));

    mult(x, y) = hw_input(x,y) + 10;
    hw_output(x, y) = cast<uint8_t>(mult(x, y));
    output(x, y) = hw_output(x, y);

    Var xi,yi, xo,yo;

    hw_input.compute_root();
    hw_output.compute_root();

    int tileSize = 1;
    hw_output.tile(x,y, xo,yo, xi,yi, 1, 1)
      .hw_accelerate(xi, xo);
    hw_output.bound(x, 0, 1);
    hw_output.bound(y, 0, 1);

    output.bound(x, 0, 1);
    output.bound(y, 0, 1);
    hw_input.stream_to_accelerator();
    
    Context* context = newContext();
    vector<Argument> args{input};
    //auto m = buildModule(true, context, "coreir_brighter", args, "brighter", dummyOut);
    auto m = buildModule(true, context, "coreir_brighter", args, "brighter", output);
    SimulatorState state(m);

    state.setValue("self.reset", BitVector(1, 1));
    state.setValue("self.in_en", BitVector(1, 1));
    state.setValue("self.in_arg_0_0_0", BitVector(16, 123));
    
    state.resetCircuit();
    cout << "Starting to execute" << endl;

    state.setValue("self.reset", BitVector(1, 0));
    state.exeCombinational();

    assert(state.getBitVec("self.out_0_0") == BitVec(16, 123 + 10));
    assert(state.getBitVec("self.valid") == BitVec(1, 1));

    deleteContext(context);

    cout << GREEN << "Pointwise add passed!" << RESET << endl;
}

void runSoC(Func hw_output, vector<Argument>& args, const std::string& name) {
  auto context = hwContext();
  auto m = buildModule(context, name + "_coreir", args, name, hw_output);

  // Note: You cannot remove this scope bracket bc CodeGen_C (which CodeGen_SoC_Test inherits from)
  // does some stream output in its destructor
  {
    Target t;
    t = t.with_feature(Target::Feature::CoreIR);
    t = t.with_feature(Target::Feature::UseExtractHWKernel);
    auto mod = hw_output.compile_to_module(args, "hw_output", t);

    //cout << "Module before consolidation..." << endl;
    //cout << mod << endl;

    for (auto& f : mod.functions()) {
      //cout << "Consolidating function" << f << endl;
      AcceleratorCallConsolidator cons;
      f.body = cons.mutate(f.body);
    }

    //cout << "Compiled to module" << endl;
    //cout << mod << endl;
    ofstream outFile(name + "_soc_mini.cpp");
    CodeGen_SoC_Test testPrinter(outFile, t, CodeGen_C::OutputKind::CPlusPlusImplementation);
    testPrinter.compileForCGRA(mod);

    ofstream headerOut(name + "_soc_mini.h");
    headerOut << "extern \"C\" {\n";
    headerOut << "\tint hw_output(struct halide_buffer_t *_input_buffer, struct halide_buffer_t *_hw_output_buffer);\n";
    headerOut << "}\n";
    headerOut.close();
  }
  cout << "Compiled cpp code" << endl;
  cout << "Done with compiling for CGRA" << endl;
  runCmd("g++ -std=c++11 " + name + "_soc_run.cpp " + name + "_soc_mini.cpp cgra_wrapper.cpp -I ../../../../tools `libpng-config --cflags --ldflags` -ljpeg -lHalide -lcoreir-float -lcoreir -lcoreir-commonlib -lcoreirsim -L ../../../../bin");
  cout << "Compiled c++ executable..." << endl;
  runCmd("./a.out");
  cout << "Ran executable" << endl;
}

void accel_soc_test() {
  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 3);

  Var x("x"), y("y"), c("c");
  Var xi,yi,zi, xo,yo,zo;

  Func hw_input, hw_output;
  hw_input(x,y) = input(x, y);
  hw_output(x,y,c) = cast<uint16_t>(hw_input(x, y) + c);
  output(x,y,c) = hw_output(x,y,c);

  // Schedule common
  hw_output.bound(c, 0, 3);
  hw_input.compute_root();
  hw_output.compute_root();

  Halide::Buffer<uint16_t> inputBuf(2, 2);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  indexTestPattern2D(inputBuf, hwInputBuf);
  Halide::Runtime::Buffer<uint16_t> outputBuf(2, 2, 3);

  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  // Hardware schedule
  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2)
    .reorder(c, xi, yi, xo, yo);
  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
  hw_output.unroll(c);
 
  // Run on SoC
  vector<Argument> args{input};
  runSoC(hw_output, args, "accel_soc");

  Halide::Runtime::Buffer<uint16_t> cppRes = load_image("accel_soc.ppm");
  compare_buffers(cppRes, cpuOutput);
  
  cout << GREEN << "Accel soc test passed" << RESET << endl;
  //assert(false);
}

void accel_interface_test() {
  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 3);

  Var x("x"), y("y"), c("c");
  Var xi,yi,zi, xo,yo,zo;

  Func hw_input, hw_output;
  hw_input(x,y) = input(x, y);
  hw_output(x,y,c) = cast<uint16_t>(hw_input(x, y) + c);
  output(x,y,c) = hw_output(x,y,c);

  // Schedule common
  hw_output.bound(c, 0, 3);
  hw_input.compute_root();
  hw_output.compute_root();

  Halide::Buffer<uint16_t> inputBuf(2, 2);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(2, 2, 3);
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      hwInputBuf(i, j, 0) = i + j*inputBuf.width();
      inputBuf(i, j) = hwInputBuf(i, j);
    }
  }

  //Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(outputBuf.width(), outputBuf.height(), outputBuf.channels());
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  // Creating tiles
  hw_output.tile(x, y, xo, yo, xi, yi, 10, 10)
    .reorder(c, xi, yi, xo, yo);
  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
  hw_output.unroll(c);

  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "accel_coreir", args, "accel", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");
  //json j;
  //ifstream inFile("accel_interface_info.json");
  //inFile >> j;
  //cout << "JSON..." << endl;
  //cout << j << endl;

  //auto aliasMap = j["aliasMap"];
  //cout << "Alias map = " << aliasMap << endl;
  //assert(aliasMap.size() == 1);

  //string inS = begin(aliasMap)->get<string>();
  //cout << "inS = " << inS << endl;
  //string outS = "hw_output.stencil.stream";
  //string accelName = "self.in_" + inS + "_0_0";

  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  compare_buffers(outputBuf, cpuOutput);

  cout << GREEN << "Accelerator interface test passed" << RESET << endl;
}

Func hot_pixel_suppression(Func input) {
  Var x("x"), y("y");
  Func denoised("denoised");
  Expr max_value = max(max(input(x-2, y), input(x+2, y)),
      max(input(x, y-2), input(x, y+2)));
  Expr min_value = min(min(input(x-2, y), input(x+2, y)),
      min(input(x, y-2), input(x, y+2)));

  denoised(x, y) = clamp(input(x,y), min_value, max_value);
  //denoised(x, y) = input(x, y);
  //denoised(x, y) = clamp(input(x,y), 5, 200);
  return denoised;
}

Func interleave_x(Func a, Func b) {
  Var x, y;
  Func out;
  out(x, y) = select((x%2)==0, a(x, y), b(x-1, y));
  return out;
}

Func interleave_y(Func a, Func b) {
  Var x, y;
  Func out;
  out(x, y) = select((y%2)==0, a(x, y), b(x, y-1));
  return out;
}

Expr avg(Expr a, Expr b) {
  return (a + b + 1) >> 1;
}

Func demosaic(Func raw) {
  Var x("x"), y("y"), c("c");
  // The demosaic algorithm is optimized for HLS schedule
  // such that the bound analysis can derive a constant window
  // and shift step without needed to unroll 'demosaic' into
  // a 2x2 grid.
  //
  // The changes made from the original is that there is no
  // explict downsample and upsample in 'deinterleave' and
  // 'interleave', respectively.
  // All the intermediate functions are the same size as the
  // raw image although only pixels at even coordinates are used.

  // These are the values we already know from the input
  // x_y = the value of channel x at a site in the input of channel y
  // gb refers to green sites in the blue rows
  // gr refers to green sites in the red rows

  // Give more convenient names to the four channels we know
  Func r_r, g_gr, g_gb, b_b;
  g_gr(x, y) = raw(x, y);//deinterleaved(x, y, 0);
  r_r(x, y)  = raw(x+1, y);//deinterleaved(x, y, 1);
  b_b(x, y)  = raw(x, y+1);//deinterleaved(x, y, 2);
  g_gb(x, y) = raw(x+1, y+1);//deinterleaved(x, y, 3);

  // These are the ones we need to interpolate
  Func b_r, g_r, b_gr, r_gr, b_gb, r_gb, r_b, g_b;

  // First calculate green at the red and blue sites
  // Try interpolating vertically and horizontally. Also compute
  // differences vertically and horizontally. Use interpolation in
  // whichever direction had the smallest difference.
  Expr gv_r  = avg(g_gb(x, y-1), g_gb(x, y));
  Expr gvd_r = absd(g_gb(x, y-1), g_gb(x, y));
  Expr gh_r  = avg(g_gr(x+1, y), g_gr(x, y));
  Expr ghd_r = absd(g_gr(x+1, y), g_gr(x, y));

  g_r(x, y)  = select(ghd_r < gvd_r, gh_r, gv_r);

  Expr gv_b  = avg(g_gr(x, y+1), g_gr(x, y));
  Expr gvd_b = absd(g_gr(x, y+1), g_gr(x, y));
  Expr gh_b  = avg(g_gb(x-1, y), g_gb(x, y));
  Expr ghd_b = absd(g_gb(x-1, y), g_gb(x, y));

  g_b(x, y)  = select(ghd_b < gvd_b, gh_b, gv_b);

  // Next interpolate red at gr by first interpolating, then
  // correcting using the error green would have had if we had
  // interpolated it in the same way (i.e. add the second derivative
  // of the green channel at the same place).
  Expr correction;
  correction = g_gr(x, y) - avg(g_r(x, y), g_r(x-1, y));
  r_gr(x, y) = correction + avg(r_r(x-1, y), r_r(x, y));

  // Do the same for other reds and blues at green sites
  correction = g_gr(x, y) - avg(g_b(x, y), g_b(x, y-1));
  b_gr(x, y) = correction + avg(b_b(x, y), b_b(x, y-1));

  correction = g_gb(x, y) - avg(g_r(x, y), g_r(x, y+1));
  r_gb(x, y) = correction + avg(r_r(x, y), r_r(x, y+1));

  correction = g_gb(x, y) - avg(g_b(x, y), g_b(x+1, y));
  b_gb(x, y) = correction + avg(b_b(x, y), b_b(x+1, y));

  // Now interpolate diagonally to get red at blue and blue at
  // red. Hold onto your hats; this gets really fancy. We do the
  // same thing as for interpolating green where we try both
  // directions (in this case the positive and negative diagonals),
  // and use the one with the lowest absolute difference. But we
  // also use the same trick as interpolating red and blue at green
  // sites - we correct our interpolations using the second
  // derivative of green at the same sites.
  correction = g_b(x, y)  - avg(g_r(x, y), g_r(x-1, y+1));
  Expr rp_b  = correction + avg(r_r(x, y), r_r(x-1, y+1));
  Expr rpd_b = absd(r_r(x, y), r_r(x-1, y+1));

  correction = g_b(x, y)  - avg(g_r(x-1, y), g_r(x, y+1));
  Expr rn_b  = correction + avg(r_r(x-1, y), r_r(x, y+1));
  Expr rnd_b = absd(r_r(x-1, y), r_r(x, y+1));

  r_b(x, y)  = select(rpd_b < rnd_b, rp_b, rn_b);

  // Same thing for blue at red
  correction = g_r(x, y)  - avg(g_b(x, y), g_b(x+1, y-1));
  Expr bp_r  = correction + avg(b_b(x, y), b_b(x+1, y-1));
  Expr bpd_r = absd(b_b(x, y), b_b(x+1, y-1));

  correction = g_r(x, y)  - avg(g_b(x+1, y), g_b(x, y-1));
  Expr bn_r  = correction + avg(b_b(x+1, y), b_b(x, y-1));
  Expr bnd_r = absd(b_b(x+1, y), b_b(x, y-1));

  b_r(x, y)  =  select(bpd_r < bnd_r, bp_r, bn_r);

  // Interleave the resulting channels
  Func r = interleave_y(interleave_x(r_gr, r_r),
      interleave_x(r_b, r_gb));
  Func g = interleave_y(interleave_x(g_gr, g_r),
      interleave_x(g_b, g_gb));
  Func b = interleave_y(interleave_x(b_gr, b_r),
      interleave_x(b_b, b_gb));

  Func demosaicked("demosaicked");
  demosaicked(x, y, c) = select(c == 0, r(x, y),
      c == 1, g(x, y),
      b(x, y));
  return demosaicked;
}

// Applies a color correction matrix to redefine rgb values.
//Func color_correct(Func input, int32_t matrix[3][4]) {
  //Var x, y;
  //Expr ir = cast<int32_t>(input(x, y, 0));
  //Expr ig = cast<int32_t>(input(x, y, 1));
  //Expr ib = cast<int32_t>(input(x, y, 2));

  //Expr r = matrix[0][3] + matrix[0][0] * ir + matrix[0][1] * ig + matrix[0][2] * ib;
  //Expr g = matrix[1][3] + matrix[1][0] * ir + matrix[1][1] * ig + matrix[1][2] * ib;
  //Expr b = matrix[2][3] + matrix[2][0] * ir + matrix[2][1] * ig + matrix[2][2] * ib;

  //r = cast<int16_t>(r/256);
  //g = cast<int16_t>(g/256);
  //b = cast<int16_t>(b/256);

  //Func corrected;
  //corrected(x, y, c) = select(c == 0, r,
      //c == 1, g,
      //b);

  //return corrected;
//}

Func simple_demosaic(Func input) {
  Var x("x"), y("y"), c("c");
  Func r, g, b;
  // Fake demosaic
  r(x, y) = input(x - 1, y + 1);
  g(x, y) = input(x, y);
  b(x, y) = (input(x - 1, y + 1) + input(x, y)) / 2;
  Func demosaicked("demosaicked");
  demosaicked(x, y, c) = select(c == 0, r(x, y),
      c == 1, g(x, y),
      b(x, y));
  return demosaicked;
}

Func color_correct(Func input) {
  Var x("x"), y("y"), c("c");
  Expr ir = cast<int16_t>(input(x, y, 0));
  Expr ig = cast<int16_t>(input(x, y, 1));
  Expr ib = cast<int16_t>(input(x, y, 2));

  int16_t matrix[3][4] = {{ 200, -44,  17, -3900},
    {-38,  159, -21, -2541},
    {-8, -73,  228, -2008}};

  Expr r = matrix[0][3] + matrix[0][0] * ir + matrix[0][1] * ig + matrix[0][2] * ib;
  Expr g = matrix[1][3] + matrix[1][0] * ir + matrix[1][1] * ig + matrix[1][2] * ib;
  Expr b = matrix[2][3] + matrix[2][0] * ir + matrix[2][1] * ig + matrix[2][2] * ib;

  r = cast<int16_t>(r/256);
  g = cast<int16_t>(g/256);
  b = cast<int16_t>(b/256);

  Func corrected;
  corrected(x, y, c) = select(c == 0, r,
      c == 1, g,
      b);

  return corrected;
}

void curve_16_lookup_test() {
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  ImageParam input(type_of<int16_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  float gamma = 1.0;
  float contrast = 1.0;

  Func hw_input, hw_output;
  hw_input(x, y) = input(x, y);

  Func curve;
  curve(x) = cast<uint8_t>(Expr(x));

  hw_output(x, y) = curve(clamp(cast<uint8_t>(hw_input(x, y)), 0, 255));

  int tileSize = 4;
  Halide::Buffer<int16_t> inputBuf(tileSize, tileSize);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  indexTestPattern2D(inputBuf, hwInputBuf);
  inputBuf(0, 0) = (int16_t) (1 << 12) | 1;
  hwInputBuf(0, 0, 0) = inputBuf(0, 0);

  inputBuf(1, 0) = -32;
  hwInputBuf(1, 0, 0) = inputBuf(1, 0);
  Halide::Runtime::Buffer<uint8_t> outputBuf(tileSize, tileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  printBuffer(cpuOutput, cout);
  hw_input.compute_root();
  hw_output.compute_root();

  hw_output.tile(x, y, xo, yo, xi, yi, tileSize, tileSize);

  curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
  hw_output.accelerate({hw_input}, xi, xo, {});
  
  cout << "After hw schedule..." << endl;
  hw_output.print_loop_nest();
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "camer_pipe_coreir", args, "camera_pipeline", hw_output);

  cout << "Module..." << endl;
  cout << m << endl;

  string accelName = "self.in_arg_0_0_0";
  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  compare_buffers(outputBuf, cpuOutput);

  cout << GREEN << "Curve 16 lookup test passed" << RESET << endl;
}
void curve_lookup_test() {
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  float gamma = 1.0;
  float contrast = 1.0;

  Func hw_input, hw_output;
  hw_input(x, y) = input(x, y);

  Func curve;
  {
    Expr xf = x/1024.0f;
    Expr g = pow(xf, 1.0f/gamma);
    Expr b = 2.0f - (float) pow(2.0f, contrast/100.0f);
    Expr a = 2.0f - 2.0f*b;
    Expr val = select(g > 0.5f,
        1.0f - (a*(1.0f-g)*(1.0f-g) + b*(1.0f-g)),
        a*g*g + b*g);
    curve(x) = cast<uint8_t>(clamp(val*256.0f, 0.0f, 255.0f));
  }

  hw_output(x, y) = curve(clamp(hw_input(x, y), 0, cast<uint8_t>(1023)));

  int tileSize = 4;
  // TODO: Extract to template function
  Halide::Buffer<uint8_t> inputBuf(tileSize, tileSize);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      hwInputBuf(i, j, 0) = i + j*inputBuf.width();
      inputBuf(i, j) = hwInputBuf(i, j);
    }
  }

  Halide::Runtime::Buffer<uint8_t> outputBuf(tileSize, tileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  printBuffer(cpuOutput, cout);
  // Hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

  hw_output.tile(x, y, xo, yo, xi, yi, tileSize, tileSize);

  curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
  hw_output.accelerate({hw_input}, xi, xo, {});
  
  cout << "After hw schedule..." << endl;
  hw_output.print_loop_nest();
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "camer_pipe_coreir", args, "camera_pipeline", hw_output);

  cout << "Module..." << endl;
  cout << m << endl;

  string accelName = "self.in_arg_0_0_0";
  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  compare_buffers(outputBuf, cpuOutput);

  cout << GREEN << "Curve lookup test passed" << RESET << endl;
}


void hot_pixel_suppression_test() {
  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  Func hw_input, hw_output, denoised;
  hw_input(x, y) = input(x + 3, y + 3);
  denoised = hot_pixel_suppression(hw_input);
  hw_output(x, y) = denoised(x, y);

  hw_output.compute_root();
  hw_input.compute_root();

  int outTileSize = 2;
  Halide::Buffer<uint8_t> inputBuf(8, 8);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  indexTestPattern2D(inputBuf, hwInputBuf);
  Halide::Runtime::Buffer<uint8_t> outputBuf(outTileSize, outTileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  // Hardware schedule
  hw_output.tile(x, y, xo, yo, xi, yi, outTileSize, outTileSize)
    .reorder(xi, yi, xo, yo);
  hw_input.stream_to_accelerator();
  denoised.linebuffer();
  //unroll(x).unroll(y);
  hw_output.hw_accelerate(xi, xo);
 
  vector<Argument> args{input};
  runSoC(hw_output, args, "hot_pixel_suppression");

  Halide::Runtime::Buffer<uint8_t> cppRes = load_image("hot_pixel_suppression.pgm");
  compare_buffers(cppRes, cpuOutput);
  
  cout << GREEN << "Hot pixel suppression test passed" << RESET << endl;
}

void camera_pipeline_test() {

  Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi");
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 3);

  Func hw_input, hw_output, denoised, demosaicked, color_corrected;
  hw_input(x, y) = input(x + 3, y + 3);
  denoised = hot_pixel_suppression(hw_input);
  //demosaicked = simple_demosaic(denoised);
  demosaicked = demosaic(denoised);
  color_corrected = color_correct(demosaicked);

  Func curve;
  {
    float gamma = 1.0;
    float contrast = 1.0;

    Expr xf = x/1024.0f;
    Expr g = pow(xf, 1.0f/gamma);
    Expr b = 2.0f - (float) pow(2.0f, contrast/100.0f);
    Expr a = 2.0f - 2.0f*b;
    Expr val = select(g > 0.5f,
        1.0f - (a*(1.0f-g)*(1.0f-g) + b*(1.0f-g)),
        a*g*g + b*g);
    curve(x) = cast<uint8_t>(clamp(val*256.0f, 0.0f, 255.0f));
  }

  //hw_output(x, y, c) = 0;
  //hw_output(x, y, c) = curve(clamp(color_corrected(x, y, c), 0, 1023));
  //hw_output(x, y, c) = curve(clamp(color_corrected(x, y, c), 0, 255));
  //hw_output(x, y, c) = curve(clamp(cast<uint8_t>(color_corrected(x, y, c)), 0, cast<uint8_t>(1023)));

  Func hw_output_8;
  //hw_output_8(x, y, c) = cast<uint8_t>(clamp(color_corrected(x, y, c), 0, 1023));
  //
  // Note: Passes unit test
  //hw_output_8(x, y, c) = cast<uint8_t>(clamp(color_corrected(x, y, c), 0, 10));
  // Note: This fails one unit test
  //hw_output_8(x, y, c) = cast<uint8_t>(clamp(color_corrected(x, y, c), 0, 100));
  //hw_output_8(x, y, c) = cast<uint8_t>(clamp(color_corrected(x, y, c), 0, 50));
  //hw_output(x, y, c) = hw_output_8(x, y, c);
  //hw_output(x, y, c) = curve(hw_output_8(x, y, c));
  // Note: Passes one unit test
  //hw_output(x, y, c) = cast<uint8_t>(clamp(color_corrected(x, y, c), 0, 1023));
  // Note: Passes one unit test
  //hw_output(x, y, c) = cast<uint8_t>(color_corrected(x, y, c));
  //curve(clamp(color_corrected(x, y, c), 0, 1023));
  hw_output(x, y, c) = cast<uint8_t>(clamp(cast<uint8_t>(color_corrected(x, y, c)), 0, 255));
  hw_output.bound(c, 0, 3);
  
  Halide::Buffer<uint8_t> inputBuf(8, 8);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  Halide::Runtime::Buffer<uint8_t> outputBuf(2, 2, 3);
  for (int i = 0; i < inputBuf.width(); i++) {
    for (int j = 0; j < inputBuf.height(); j++) {
      hwInputBuf(i, j, 0) = i + j*inputBuf.width();
      inputBuf(i, j) = hwInputBuf(i, j);
    }
  }

  Halide::Buffer<uint8_t> cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  // Hardware schedule
  hw_input.compute_root();
  hw_output.compute_root();

  hw_output.tile(x, y, xo, yo, xi, yi, 2, 2)
    .reorder(c, xi, yi, xo, yo);

  denoised.linebuffer()
    .unroll(x).unroll(y);
  demosaicked.linebuffer()
    .unroll(c).unroll(x).unroll(y);

  curve.compute_at(hw_output, xo).unroll(x);  // synthesize curve to a ROM
  hw_output.accelerate({hw_input}, xi, xo, {});
  hw_output.unroll(c);
  
  cout << "After hw schedule..." << endl;
  hw_output.print_loop_nest();

  vector<Argument> args{input};
  runSoC(hw_output, args, "camera_pipeline");
  Halide::Runtime::Buffer<uint8_t> cppRes = load_image("camera_pipeline.ppm");
  compare_buffers(cppRes, cpuOutput);
  cout << GREEN << "Camera pipeline test passed" << RESET << endl;
}


void double_unsharp_test() {
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  Var x("x"), y("y");

  Func kernel("kernel");
  Func blurred("blurred"), blurred1("blurred1");
  Func diff("diff");
  RDom r(0, 3,
      0, 3);

  kernel(x,y) = 1;

  Func hw_input("hw_input");
  hw_input(x, y) = cast<uint16_t>(input(x, y));
  
  blurred(x, y) = 0;
  blurred(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
  
  blurred1(x, y) = 0;
  blurred1(x, y)  += kernel(r.x, r.y) * blurred(x + r.x, y + r.y);

  diff(x, y) = hw_input(x, y) - blurred1(x, y);

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint8_t>(diff(x, y));
  output(x, y) = hw_output(x,y);

  Var xi, yi, xo, yo;

  hw_input.compute_root();
  hw_output.compute_root();

  // Hardware schedule
  int outTileSize = 4;
  Halide::Buffer<uint8_t> inputBuf(outTileSize + 4, outTileSize + 4);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  //indexTestPattern2D(inputBuf, hwInputBuf);
  indexTestPatternRandom(inputBuf, hwInputBuf);
  Halide::Runtime::Buffer<uint8_t> outputBuf(outTileSize, outTileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  hw_output.tile(x, y, xo, yo, xi, yi, outTileSize, outTileSize)
    .reorder(xi, yi, xo, yo);

  // Unrolling by xi or r.x / r.y causes an error, but I dont understand
  // why this is the case. Unrolling in x / y unrolls the outermost loop,
  // but not the inner loops. What is the relationship between the outer loop
  // that computes blur(x, y) = 0 and the use of kernel to accumulate blur values?
  blurred.linebuffer().update().unroll(r.x).unroll(r.y);
  blurred1.linebuffer().update().unroll(r.x).unroll(r.y);
  diff.linebuffer();
  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
  
  hw_output.print_loop_nest();
  
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "hw_double_unsharp", args, "double_unsharp", hw_output);

  //json j;
  //ifstream inFile("accel_interface_info.json");
  //inFile >> j;
  //cout << "JSON..." << endl;
  //cout << j << endl;

  //auto aliasMap = j["aliasMap"];
  //cout << "Alias map = " << aliasMap << endl;
  //assert(aliasMap.size() == 1);

  //string inS = begin(aliasMap)->get<string>();
  //cout << "inS = " << inS << endl;
  //string outS = "hw_output.stencil.stream";
  //string accelName = "self.in_" + inS + "_0_0";

  auto accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  compare_buffers(outputBuf, cpuOutput);
  
  cout << GREEN << "Double unsharp test passed" << RESET << endl;
  //assert(false);
}

void simple_unsharp_test() {
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  Var x("x"), y("y");

  Func kernel("kernel");
  Func blurred("blurred");
  Func diff("diff");
  RDom r(0, 3,
      0, 3);

  kernel(x,y) = 1;

  Func hw_input("hw_input");
  hw_input(x, y) = cast<uint16_t>(input(x, y));
  
  blurred(x, y) = 0;
  blurred(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
  
  diff(x, y) = hw_input(x, y) - blurred(x, y);

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint8_t>(diff(x, y));
  output(x, y) = hw_output(x,y);

  Var xi, yi, xo, yo;

  hw_input.compute_root();
  hw_output.compute_root();

  // Hardware schedule
  int outTileSize = 8;
  Halide::Buffer<uint8_t> inputBuf(outTileSize + 2, outTileSize + 2);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  indexTestPatternRandom(inputBuf, hwInputBuf);
  Halide::Runtime::Buffer<uint8_t> outputBuf(outTileSize, outTileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  hw_output.tile(x, y, xo, yo, xi, yi, outTileSize, outTileSize)
    .reorder(xi, yi, xo, yo);

  blurred.linebuffer().update().unroll(r.x).unroll(r.y);
  diff.linebuffer();
  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
  
  hw_output.print_loop_nest();
  
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "hw_simple_unsharp", args, "simple_unsharp", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  compare_buffers(outputBuf, cpuOutput);
  
  cout << GREEN << "Simple unsharp test passed" << RESET << endl;
}


void different_latency_kernels_test() {
  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);

  Var x("x"), y("y");
  Var xi, yi, xo, yo;

  Func lut("lut");
  Func translated("kernel");
  Func brightened("brightened");
  Func diff("diff");
  Func hw_input("hw_input");
  Func hw_output("hw_output");
  
  hw_input(x, y) = cast<uint8_t>(input(x, y));
  lut(x) = cast<uint8_t>(Expr(x));
  translated(x, y) = lut(hw_input(x, y));
  brightened(x, y) = hw_input(x, y) + 10;
  diff(x, y) = brightened(x, y) - translated(x, y);
  hw_output(x, y) = diff(x, y);

  int outTileSize = 5;
  Halide::Buffer<uint8_t> inputBuf(outTileSize, outTileSize);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 1);
  indexTestPatternRandom(inputBuf, hwInputBuf);
  Halide::Runtime::Buffer<uint8_t> outputBuf(outTileSize, outTileSize);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  printBuffer(cpuOutput, cout);
  hw_output.compute_root();
  hw_input.compute_root();
 
  hw_output.tile(x, y, xo, yo, xi, yi, outTileSize, outTileSize)
    .reorder(xi, yi, xo, yo);

  lut.compute_at(hw_output, xo).unroll(x);
  translated.linebuffer();
  brightened.linebuffer();
  diff.linebuffer();
  hw_input.stream_to_accelerator();
  hw_output.hw_accelerate(xi, xo);
  
  hw_output.print_loop_nest();
  
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "hw_different_latencies", args, "different_latencies", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");

  runHWKernel(accelName, m, hwInputBuf, outputBuf);
  compare_buffers(outputBuf, cpuOutput);
  PRINT_PASSED("Different latency kernels");
}

void real_unsharp_test() {
  ImageParam input(type_of<uint8_t>(), 3);
  ImageParam output(type_of<uint8_t>(), 3);

  const int blockSize = 5;

  Var c("c"), x("x"), y("y");
  Var xo("xo"), yo("yo"), xi("xi"), yi("yi");

  // create the gaussian kernel
  Func kernel_f;
  float sigma = 1.5f;
  kernel_f(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);

  // create a normalized set of 8bit weights
  Func kernel;
  Expr sum_kernel[blockSize];
  for (int i=0; i<blockSize; ++i) {
    if (i==0) {
      sum_kernel[i] = kernel_f(i-blockSize/2);
    } else {
      sum_kernel[i] = kernel_f(i-blockSize/2) + sum_kernel[i-1];
    }
  }
  kernel(x) = cast<uint16_t>(kernel_f(x) * 255 / sum_kernel[blockSize-1]);

  // create the input
  Func hw_input;
  hw_input(x, y, c) = cast<uint16_t>(input(x+blockSize/2, y+blockSize/2, c));

  // create a grayscale image
  Func gray;
  gray(x, y) = cast<uint16_t>((77 * cast<uint16_t>(hw_input(x, y, 0))
        + 150 * cast<uint16_t>(hw_input(x, y, 1))
        + 29 * cast<uint16_t>(hw_input(x, y, 2))) >> 8);

  // Use a 2D filter to blur the input
  Func blur_unnormalized, blur;
  RDom win(-blockSize/2, blockSize, -blockSize/2, blockSize);
  //RDom win(0, blockSize, 0, blockSize);
  blur_unnormalized(x, y) += cast<uint16_t>( kernel(win.x) * gray(x+win.x, y+win.y) );
  blur(x, y) = blur_unnormalized(x, y) / 8;

  // sharpen the image by subtracting the blurred image
  Func sharpen;
  sharpen(x, y) = cast<uint16_t>(clamp(2 * cast<uint16_t>(gray(x, y)) - blur(x, y), 0, 255));

  // find the ratio of sharpened and original image
  Func ratio;
  ratio(x, y) = cast<uint16_t>(clamp(cast<uint16_t>(sharpen(x, y)) * 32 / max(gray(x, y), 1), 0, 255));

  // Use the ratio to sharpen the input image.
  Func hw_output;
  hw_output(x, y, c) = cast<uint8_t>(clamp(cast<uint16_t>(ratio(x, y)) * hw_input(x, y, c) / 32, 0, 255));
  // Note: Passes unit test;
  //hw_output(x, y, c) = cast<uint8_t>(hw_input(x, y, c));
  //hw_output(x, y, c) = cast<uint8_t>(blur_unnormalized(x, y) + hw_input(x, y, c));

  hw_output.bound(c, 0, 3);

  output(x, y, c) = hw_output(x, y, c);
  
  int outTileSize = 5;
  Halide::Buffer<uint8_t> inputBuf(outTileSize + blockSize + 1, outTileSize + blockSize + 1, 3);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.width(), inputBuf.height(), 3);
  for (int y = 0; y < inputBuf.height(); y++) {
    for (int x = 0; x < inputBuf.width(); x++) {
      for (int b = 0; b < inputBuf.channels(); b++) {
        inputBuf(x, y, b) = x + y;
        hwInputBuf(x, y, b) = inputBuf(x, y, b);
      }
    }
  }
  //indexTestPatternRandom(inputBuf, hwInputBuf);
  cout << "CPU Input..." << endl;
  printBuffer(inputBuf, cout);
  Halide::Runtime::Buffer<uint8_t> outputBuf(outTileSize, outTileSize, 3);
  auto cpuOutput = realizeCPU(hw_output, input, inputBuf, outputBuf);

  printBuffer(cpuOutput, cout);
  hw_output.compute_root();
  hw_input.compute_root();
  
  hw_output.tile(x, y, xo, yo, xi, yi, outTileSize, outTileSize).
    reorder(c, xi, yi, xo, yo);
    //.reorder(xi, yi, xo, yo);
  hw_output.unroll(c);  // hw output bound
  
  blur_unnormalized.linebuffer()
    .update().unroll(win.x).unroll(win.y);
  gray.linebuffer();
  ratio.linebuffer();
  
  hw_output.hw_accelerate(xi, xo);
  hw_input.stream_to_accelerator();
  
  hw_output.print_loop_nest();
  
  vector<Argument> args{input};
  runSoC(hw_output, args, "unsharp");
  Halide::Runtime::Buffer<uint8_t> cppRes = load_image("unsharp.ppm");
  compare_buffers(cppRes, cpuOutput);
  
  PRINT_PASSED("Real unsharp");

  //assert(false);
}

class ProducerFinder : public IRGraphVisitor {
  public:
    std::string target;
    bool foundTarget;
    const ProducerConsumer* result;

    ProducerFinder() : foundTarget(false) {}

    using IRGraphVisitor::visit;

    void visit(const ProducerConsumer* pc) {
      if (pc->is_producer && pc->name == target) {
        foundTarget = true;
        result = pc;
      } else {
        IRGraphVisitor::visit(pc);
      }
    }
};

const ProducerConsumer* findProducer(const std::string& name, Stmt& stmt) {
  ProducerFinder f;
  f.target = name;
  stmt.accept(&f);

  assert(f.foundTarget);

  return f.result;
}

class PolyStmt {
  public:

    std::vector<const For*> surroundingLoops;
    bool isAssign;
    const Store* store;
    std::string varName;
    Expr varVal;
};

std::ostream& operator<<(std::ostream& out, const PolyStmt& s) {
  for (auto lp : s.surroundingLoops) {
    out << lp->name << " : " << lp->min << ", " << lp->extent << "; ";
  }
  if (s.isAssign) {
    out << s.varName << " = " << s.varVal;
  } else {
    auto store = s.store;
    out << store->name << "[" << store->index << "] = " << store->value << " if " << store->predicate;
  }
  return out;
}

class PolyhedralStmts : public IRGraphVisitor {
  public:

    using IRGraphVisitor::visit;

    std::vector<const For*> activeLoops;
    std::vector<PolyStmt> stmts;

    void visit(const For* lp) override {
      activeLoops.push_back(lp);

      lp->body.accept(this);

      activeLoops.pop_back();
    }

    void visit(const LetStmt* lt) override {
      stmts.push_back({activeLoops, true, nullptr, lt->name, lt->value});
      lt->body.accept(this);
    }

    void visit(const Store* st) override {
      stmts.push_back({activeLoops, false, st});
    }


};

void conv_layer_mobile_test() {
  ImageParam input(type_of<int8_t>(), 3);
  ImageParam output(type_of<int8_t>(), 3);
  Func hw_input("hw_input");
  Func hw_output("hw_output");
  Var x("x"), y("y"), c("c"), k("k");

  Func filter_dw, filter_pw;
  Func bias_dw, bias_pw;

  filter_dw(x, y, c) = 1;
  filter_pw(c, k) = 3;
  bias_dw(c) = 0;
  bias_pw(k) = 0;

  Func dw_conv("dw_conv");
  Func pw_conv("pw_conv");
  Func pw_conv_reduction;

  RDom r_dw(0, 3, 0, 3);
  RVar pw_c("c");
  RDom r_pw(0, 4);

  hw_input(x, y, c) = input(x, y, c);

  //depthwise ConvolutionLayer
  dw_conv(x, y, c) = cast<int16_t>(bias_dw(c));
  dw_conv(x, y, c) += cast<int16_t>(filter_dw(r_dw.x, r_dw.y, c) *
      hw_input(x + r_dw.x, y + r_dw.y, c));

  //pointwise ConvolutionLayer
  pw_conv(x, y, c, k) = cast<int16_t>(bias_pw(k));
  pw_conv(x, y, c, k) += cast<int16_t>(filter_pw(c, k) * dw_conv(x, y, c));
  pw_conv_reduction(x, y, k) = 0;
  pw_conv_reduction(x, y, k) += cast<int16_t>(pw_conv(x, y, r_pw.x, k));
  hw_output(x, y, k) = cast<int8_t>(max(0, pw_conv_reduction(x, y, k)));

  output(x, y, k) = cast<uint8_t>(hw_output(x, y, k));


  hw_output.bound(x, 0, 14);
  hw_output.bound(y, 0, 14);
  hw_output.bound(k, 0, 4);

  bias_dw.bound(c, 0, 4);
  bias_pw.bound(k, 0, 4);

  filter_dw.bound(c, 0, 4);
  filter_pw.bound(c, 0, 4);
  filter_pw.bound(k, 0, 4);

  pw_conv.bound(x, 0, 14);
  pw_conv.bound(y, 0, 14);
  pw_conv.bound(k, 0, 4);

  // Blocking spatially on X Y dim
  Var xo("xo"), xi("xi"), yo("yo"), yi("yi");

  hw_input.compute_root();
  hw_output.compute_root();

  filter_dw.compute_at(hw_output, xo)
    .unroll(x).unroll(y).unroll(c);
  filter_pw.compute_at(hw_output, xo)
    .unroll(c).unroll(k);
  bias_dw.compute_at(hw_output, xo)
    .unroll(c);
  bias_pw.compute_at(hw_output, xo)
    .unroll(k);

  hw_output.tile(x, y, xo, yo, xi, yi, 14, 14)
    .reorder(xi, yi, k, xo, yo)
    .reorder_storage(k, x, y)
    .accelerate({hw_input}, xi, xo);

  //schedule pw conv reduction
  pw_conv_reduction.update()
    //.unroll(k)
    .reorder(k, x, y, r_pw.x);

  pw_conv_reduction.compute_at(hw_output, xo).store_at(hw_output, xo);

  //schedule pw conv
  pw_conv.compute_at(hw_output, xo).store_at(hw_output, xo)
    .reorder(c, x, y, k).unroll(c);

  //schedule dw conv
  dw_conv.compute_at(pw_conv, x).store_at(hw_output, xo)
    .reorder(x, y, c);

  dw_conv.update()
    .reorder(r_dw.x, r_dw.y, x, y, c)
    .unroll(r_dw.x)
    .unroll(r_dw.y);


  hw_input.stream_to_accelerator().reorder_storage(c, x, y);
  hw_output.print_loop_nest();

  Target t;
  //t = t.with_feature(Target::Feature::CoreIR);
  vector<Argument> args{input};
  auto mod = hw_output.compile_to_module(args, "hw_output", t);

  cout << "Module before consolidation..." << endl;
  cout << mod << endl;

  for (auto f : mod.functions()) {
    cout << "Preprocessed body for " << f.name << endl;
    HWRegionFinder finder;
    f.body.accept(&finder);
    if (finder.foundRegion) {
      Stmt s = preprocessHWLoops(finder.region->body);
      cout << "---- Found hw region..." << endl;
      cout << s << endl;

      MemoryInfoCollector mic;
      s.accept(&mic);

      cout << "----- Memory info..." << endl;
      for (auto op : mic.memOps) {
        cout << "\t" << op << endl;
      }

      cout << "----- ROMS..." << endl;
      for (auto r : mic.roms()) {
        cout << r << endl;
      }
    } else {

    }

    const ProducerConsumer* hwRegion =
      findProducer("hw_output", f.body);
    cout << "HWRegion..." << endl;
    cout << hwRegion->body << endl;

    Closure c(hwRegion->body);
    cout << "Closure variables..." << endl;
    for (auto vars : c.vars) {
      cout << "\t" << vars.first << endl;
    }
    cout << "Closure buffers..." << endl;
    for (auto buffers : c.buffers) {
      cout << "\t" << buffers.first << endl;
    }

    MemoryInfoCollector mic;
    hwRegion->body.accept(&mic);
    cout << "----- Memory info..." << endl;
    for (auto op : mic.memOps) {
      cout << "\t" << op << endl;
    }

    cout << "----- ROMS..." << endl;
    for (auto r : mic.roms()) {
      cout << r << endl;
    }

    PolyhedralStmts ps;
    hwRegion->body.accept(&ps);

    cout << "Polyhedral statements..." << endl;
    for (auto stmt : ps.stmts) {
      cout << "\t" << stmt << endl;
    }
  }

  //cout << "Postprocessed module" << endl;
  //cout << 
  //auto context = hwContext();
  PRINT_PASSED("Conv layer mobile");
  //assert(false);
}

void ushift_test() {

  ImageParam input(type_of<uint16_t>(), 2);
  ImageParam output(type_of<uint16_t>(), 2);
  
  Var x("x"), y("y");

  Func hw_input("hw_input");
  hw_input(x, y) = cast<uint16_t>(input(x, y));

  Func res;
  res(x, y) = hw_input(x, y) >> 9;

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint16_t>(res(x, y));
  output(x, y) = hw_output(x,y);

   // Creating input data
  Halide::Buffer<uint16_t> inputBuf(4, 4);
  Halide::Runtime::Buffer<uint16_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<uint16_t> outputBuf(4, 4, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = 0xffff - i - j;
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<uint16_t> cpuOutput(4, 4);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  /* THE HARDWARE SCHEDULE */
  Var xi,yi, xo,yo;

  hw_input.compute_root();
  hw_output.compute_root();

  hw_output.bound(x, 0, 4);
  hw_output.bound(y, 0, 4);

  hw_output.tile(x,y, xo,yo, xi,yi, 4, 4)
    .hw_accelerate(xi, xo);

  hw_input.stream_to_accelerator();
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "arith_coreir", args, "arith", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  PRINT_PASSED("ushift test passed");
  //assert(false);
}

void arith_test() {

  ImageParam input(type_of<uint8_t>(), 2);
  ImageParam output(type_of<uint8_t>(), 2);
  
  Var x("x"), y("y");

  Func hw_input("hw_input");
  hw_input(x, y) = cast<int16_t>(input(x, y));

  Func mult, div, add, sub, mod;
  mult(x,y) = hw_input(x,y) * 13;
  div(x,y) = hw_input(x,y) / 4;
  mod(x,y) = hw_input(x,y) % 16;
  add(x,y) = div(x,y) + mod(x,y);
  //add(x,y) = div(x,y);
  sub(x,y) = mult(x,y) - add(x,y);
  //sub(x,y) = mult(x,y) + add(x,y);

  Func hw_output("hw_output");
  hw_output(x, y) = cast<uint8_t>(sub(x, y));
  output(x, y) = hw_output(x,y);

   // Creating input data
  Halide::Buffer<uint8_t> inputBuf(4, 4);
  Halide::Runtime::Buffer<uint8_t> hwInputBuf(inputBuf.height(), inputBuf.width(), 1);
  Halide::Runtime::Buffer<uint8_t> outputBuf(4, 4, 1);
  for (int i = 0; i < inputBuf.height(); i++) {
    for (int j = 0; j < inputBuf.width(); j++) {
      inputBuf(i, j) = rand();
      hwInputBuf(i, j, 0) = inputBuf(i, j);
    }
  }

   //Creating CPU reference output
  Halide::Buffer<uint8_t> cpuOutput(4, 4);
  ParamMap rParams;
  rParams.set(input, inputBuf);
  Target t;
  hw_output.realize(cpuOutput, t, rParams);

  /* THE HARDWARE SCHEDULE */
  Var xi,yi, xo,yo;

  hw_input.compute_root();
  hw_output.compute_root();

  hw_output.bound(x, 0, 4);
  hw_output.bound(y, 0, 4);

  hw_output.tile(x,y, xo,yo, xi,yi, 4, 4)
    .hw_accelerate(xi, xo);

  hw_input.stream_to_accelerator();
  auto context = hwContext();
  vector<Argument> args{input};
  auto m = buildModule(context, "arith_coreir", args, "arith", hw_output);

  string accelName = getInputAlias("accel_interface_info.json");
  runHWKernel(accelName, m, hwInputBuf, outputBuf);

  compare_buffers(outputBuf, cpuOutput);
  deleteContext(context);

  PRINT_PASSED("Arith test passed");
}

int main(int argc, char **argv) {
  //small_conv_3_3_not_unrolled_test();
  small_conv_3_3_critical_path_test();
  control_path_test();
  control_path_xy_test();
  different_latency_kernels_test();
  shiftRight_test();
  ushift_test();
  arith_test();
  small_conv_3_3_test();

  pointwise_add_test();
  mod2_test();
  clamp_test();
  
  small_cascade_test();
 
  // Experimenta tests
  //conv_layer_mobile_test();
  
  double_unsharp_test();
  real_unsharp_test();
  
  rom_read_test();
  curve_16_lookup_test();
  camera_pipeline_test();
  simple_unsharp_test();
  hot_pixel_suppression_test();
  accel_interface_test();
  accel_soc_test();
  curve_lookup_test();
  
  offset_window_test();  
  small_demosaic_test();
  multi_channel_conv_test();
  
  cout << GREEN << "All tests passed" << RESET << endl;
  return 0;
}
