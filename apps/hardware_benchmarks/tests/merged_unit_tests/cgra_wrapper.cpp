#include <cassert>
#include <cstdint>
#include <iostream>

#include "cgra_wrapper.h"

#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"

using namespace CoreIR;

using namespace std;

#include "test_utils.h"

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
  resetSim("self.in_arg_3_0_0", m, *state);
  cout << "Initialized simulator..." << endl;
}

CGRAWrapper::~CGRAWrapper() {
  delete state;
  CoreIR::deleteContext(c);
}

// Q: How do we start to stream from buffer? Note that this function must
// write to the simulator and read out and buffer its data to a temporary
// buffer in order to save it
void CGRAWrapper::subimage_to_stream(halide_buffer_t* buf,
    int stride_0, int subimage_extent_0,
    int stride_1, int subimage_extent_1,
    int stride_2, int subimage_extent_2,
    int stride_3, int subimage_extent_3) {

  cout << "subimage to stream on buffer with extents..." << endl;
  cout << "\t" << subimage_extent_0 << endl;
  cout << "\t" << subimage_extent_1 << endl;
  cout << "\t" << subimage_extent_2 << endl;
  cout << "\t" << subimage_extent_3 << endl;
}

void CGRAWrapper::stream_to_subimage(halide_buffer_t* buf,
    int stride_0, int subimage_extent_0,
    int stride_1, int subimage_extent_1,
    int stride_2, int subimage_extent_2,
    int stride_3, int subimage_extent_3) {

}
