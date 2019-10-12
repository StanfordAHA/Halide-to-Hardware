#include <cassert>
#include <cstdint>
#include <iostream>

#include "cgra_wrapper.h"

#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"

using namespace CoreIR;

using namespace std;

#include "test_utils.h"

extern "C" {
uint8_t* _halide_buffer_get_host(const halide_buffer_t* buf);
}

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

  pixelOutputs.clear();
  assert(pixelOutputs.size() == 0);

  // TODO: Perform dummy computation where we just write the value 23 to
  // every pixel of the storage buffer and then write it back out?
  for (int i = 0; i < subimage_extent_0; i++) {
    for (int j = 0; j < subimage_extent_1; j++) {
      for (int k = 0; k < subimage_extent_2; k++) {
        for (int m = 0; m < subimage_extent_3; m++) {
          pixelOutputs.push_back(23);
          pixelOutputs.push_back(23);
          pixelOutputs.push_back(23);
        }
      }
    }
  }
}

void CGRAWrapper::stream_to_subimage(halide_buffer_t* buf,
    int stride_0, int subimage_extent_0,
    int stride_1, int subimage_extent_1,
    int stride_2, int subimage_extent_2,
    int stride_3, int subimage_extent_3) {

  cout << "subimage to stream on buffer with extents..." << endl;
  cout << "\t" << subimage_extent_0 << endl;
  cout << "\t" << subimage_extent_1 << endl;
  cout << "\t" << subimage_extent_2 << endl;
  cout << "\t" << subimage_extent_3 << endl;
  
  uint16_t* dataPtr = (uint16_t*)_halide_buffer_get_host(buf);
  int nextPix = 0;
  for (int i = 0; i < subimage_extent_0; i++) {
    for (int j = 0; j < subimage_extent_1; j++) {
      for (int k = 0; k < subimage_extent_2; k++) {
        for (int m = 0; m < subimage_extent_3; m++) {
          int offset = stride_0 * i +
            stride_1 * j + 
            stride_2 * k + 
            stride_3 * m;
          assert(nextPix < ((int) pixelOutputs.size()));
          dataPtr[offset] = pixelOutputs[nextPix];
          nextPix++;
        }
      }
    }
  }

  cout << "nextPix = " << nextPix << endl;
  cout << "pixels  = " << pixelOutputs.size() << endl;
  //assert(nextPix == ((int) pixelOutputs.size()));
}
