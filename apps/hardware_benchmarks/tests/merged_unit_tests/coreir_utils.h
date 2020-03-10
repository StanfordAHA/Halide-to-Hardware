#pragma once

#include "coreir.h"
#include "coreir/simulator/interpreter.h"
#include "coreir/libs/float.h"

#include "coreir/ir/dynamic_bit_vector.h"
#include "coreir.h"
#include "coreir/simulator/interpreter.h"

using namespace CoreIR;
using namespace Halide;
using namespace Halide::Tools;
using namespace std;

static
CoreIR::Module* buildModule(Halide::Internal::HardwareInfo& info, bool useUbuffer, CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Target t;
  t = t.with_feature(Target::Feature::CoreIR);
  if (!useUbuffer) {
    //t = t.with_feature(Target::Feature::UseExtractHWKernel);
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

static
CoreIR::Module* buildModule(const bool useUbuffer, CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Halide::Internal::HardwareInfo info;
  info.hasCriticalPathTarget = false;
  return buildModule(info, useUbuffer, context, name, args, fName, hwOutput);
}

static
CoreIR::Module* buildModule(CoreIR::Context* context, const std::string& name, std::vector<Argument>& args, const std::string& fName, Func& hwOutput) {
  Halide::Internal::HardwareInfo info;
  info.hasCriticalPathTarget = false;
  return buildModule(info, false, context, name, args, fName, hwOutput);
}


