#include <cassert>
#include <cstdint>
#include <iostream>

#include "cgra_wrapper.h"

#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"
using namespace std;

CGRAWrapper::CGRAWrapper() {
  c = CoreIR::newContext();

  CoreIRLoadLibrary_commonlib(c);
  CoreIRLoadLibrary_float(c);
  if (!loadFromFile(c, "./conv_3_3_app.json")) {
    cout << "Error: Could not load json for CGRA accelerator!" << endl;
    c->die();
  }
  c->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});
  m = c->getNamespace("global")->getModule("DesignTop");
  cout << "Module..." << endl;
  m->print();
  state = new CoreIR::SimulatorState(m);
  cout << "Initialized simulator..." << endl;
}

CGRAWrapper::~CGRAWrapper() {
  delete state;
  CoreIR::deleteContext(c);
}

void CGRAWrapper::subimage_to_stream(halide_buffer_t* buf,
    int stride_0, int subimage_extend_0,
    int stride_1, int subimage_extend_1,
    int stride_2, int subimage_extend_2,
    int stride_3, int subimage_extend_3) {

}

void CGRAWrapper::stream_to_subimage(halide_buffer_t* buf,
    int stride_0, int subimage_extend_0,
    int stride_1, int subimage_extend_1,
    int stride_2, int subimage_extend_2,
    int stride_3, int subimage_extend_3) {

}
