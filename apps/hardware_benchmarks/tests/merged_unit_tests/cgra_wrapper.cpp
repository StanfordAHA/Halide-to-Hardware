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
uint32_t _halide_buffer_get_type(const halide_buffer_t *buf);
}

bool starts_with(const string &str, const string &prefix) {
    if (str.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (str[i] != prefix[i]) return false;
    }
    return true;
}

bool ends_with(const string &str, const string &suffix) {
    if (str.size() < suffix.size()) return false;
    size_t off = str.size() - suffix.size();
    for (size_t i = 0; i < suffix.size(); i++) {
        if (str[off+i] != suffix[i]) return false;
    }
    return true;
}

string replace_all(const string &str, const string &find, const string &replace) {
    size_t pos = 0;
    string result = str;
    while ((pos = result.find(find, pos)) != string::npos) {
        result.replace(pos, find.length(), replace);
        pos += replace.length();
    }
    return result;
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
  // Set input name
  auto rtp = m->getType();

  // TODO: Need to create a map from outArgs to points that represent offsets
  vector<string> inArgs;
  int cIndex = 0;
  for (auto field : rtp->getFields()) {
    cout << "Field = " << field << endl;
    if (starts_with(field, "in_arg_")) {
      inArgs.push_back(field);
    } else if (starts_with(field, "out_")) {
      outArgs.push_back(field);
      outArgLocations[field] = Point<int>{{0, 0, cIndex}, {"x", "y", "c"}};
      cIndex++;
    }
  }

  cout << "Size of inArgs = " << inArgs.size() << endl;
  cout << "Size of outArgs = " << outArgs.size() << endl;
  assert(inArgs.size() == 1);
  inputName = "self." + inArgs[0];

  //assert(false);
  state = new CoreIR::SimulatorState(m);
  resetSim(inputName, m, *state);
  cout << "Initialized simulator..." << endl;
}

CGRAWrapper::~CGRAWrapper() {
  delete state;
  CoreIR::deleteContext(c);
}

// Q: How do we start to stream from buffer? Note that this function must
// write to the simulator and read out and buffer its data to a temporary
// buffer in order to save it
void CGRAWrapper::subimage_to_stream(halide_buffer_t* buf, int32_t subImageOffset,
    int stride_0, int subimage_extent_0,
    int stride_1, int subimage_extent_1,
    int stride_2, int subimage_extent_2,
    int stride_3, int subimage_extent_3) {
}

void CGRAWrapper::stream_to_subimage(halide_buffer_t* buf, int32_t subImageOffset,
    int stride_0, int subimage_extent_0,
    int stride_1, int subimage_extent_1,
    int stride_2, int subimage_extent_2,
    int stride_3, int subimage_extent_3) {
}

void CGRAWrapper::produce_subimage(halide_buffer_t* sourceBuf, int32_t sourceOffset,
        int src_stride_0, int src_subimage_extent_0,
        int src_stride_1, int src_subimage_extent_1,
        int src_stride_2, int src_subimage_extent_2,
        int src_stride_3, int src_subimage_extent_3,
        
        halide_buffer_t* destBuf, int32_t destOffset,
        int dest_stride_0, int dest_subimage_extent_0,
        int dest_stride_1, int dest_subimage_extent_1,
        int dest_stride_2, int dest_subimage_extent_2,
        int dest_stride_3, int dest_subimage_extent_3) {

  uint32_t sourceElemType = _halide_buffer_get_type(sourceBuf);
  uint32_t destElemType = _halide_buffer_get_type(destBuf);

  cout << "Source type = " << hex << sourceElemType << endl;
  cout << "Dest type   = " << hex << destElemType << endl;

  uint32_t sourceBits = (sourceElemType >> 8) & 0xff;
  uint32_t destBits = (destElemType >> 8) & 0xff;
 
  cout << dec << endl;
  cout << "Source bits = " << sourceBits << endl;
  cout << "Dest bits   = " << destBits << endl;
  
  cout << "subimage to stream on buffer at offset " << destOffset << " with extents..." << endl;
  cout << "\t" << dest_subimage_extent_0 << endl;
  cout << "\t" << dest_subimage_extent_1 << endl;
  cout << "\t" << dest_subimage_extent_2 << endl;
  cout << "\t" << dest_subimage_extent_3 << endl;

  cout << "dest strides..." << endl;
  cout << "\t" << dest_stride_0 << endl;
  cout << "\t" << dest_stride_1 << endl;
  cout << "\t" << dest_stride_2 << endl;
  cout << "\t" << dest_stride_3 << endl;

  assert(src_stride_3 == 1 && src_subimage_extent_3 == 1);
  assert(dest_stride_3 == 1 && dest_subimage_extent_3 == 1);

  CoordinateVector<int> writeIdx({"y", "x", "c"}, {src_subimage_extent_1 - 1,
      src_subimage_extent_0 - 1,
      src_subimage_extent_2 - 1});

  CoordinateVector<int> readIdx({"y", "x", "c"}, {dest_subimage_extent_1 - 1,
      dest_subimage_extent_0 - 1,
      dest_subimage_extent_2 - 1});
  // TODO: Replace with real increment value
  readIdx.setIncrement("c", outArgs.size());

  pixelOutputs.clear();
  assert(pixelOutputs.size() == 0);

  while (!readIdx.allDone()) {
    cout << "Write index = " << writeIdx.coordString() << endl;
    cout << "Read index  = " << readIdx.coordString() << endl;
    auto i = writeIdx.coord("x");
    auto j = writeIdx.coord("y");
    auto k = writeIdx.coord("c");
    // TODO: Replace with 4th dimension stride when that comes up
    auto m = 0;

    int offset = sourceOffset + 
      src_stride_0 * i +
      src_stride_1 * j + 
      src_stride_2 * k + 
      src_stride_3 * m;

    if (!writeIdx.allDone()) {

      assert(sourceBits == 8 || sourceBits == 16);

      if (sourceBits == 8) {
        uint8_t* hostBuf = (uint8_t*) _halide_buffer_get_host(sourceBuf);
        uint8_t nextInPixel = hostBuf[offset];
        cout << "Next pixel = " << nextInPixel << endl;
        state->setValue("self.in_en", BitVector(1, true));
        // Note: Accelerator promotes everything to 16 bits on input
        state->setValue(inputName, BitVector(16, nextInPixel));
      } else {
        assert(sourceBits == 16);

        uint16_t* hostBuf = (uint16_t*) _halide_buffer_get_host(sourceBuf);
        uint16_t nextInPixel = hostBuf[offset];
        cout << "Next pixel = " << nextInPixel << endl;
        state->setValue("self.in_en", BitVector(1, true));
        state->setValue(inputName, BitVector(16, nextInPixel));
      }
    } else {
      state->setValue("self.in_en", BitVector(1, false));
    }
    state->exeCombinational();

    // We really need the extent of each cube, the stride of each cube
    // and the increment at which data is read. 
    bool valid_value = state->getBitVec("self.valid").to_type<bool>();
    if (valid_value) {

      // For now what can I do to get the two examples I have running?
      assert(outArgs.size() > 0);

      auto x = readIdx.coord("x");
      auto y = readIdx.coord("y");
      auto c = readIdx.coord("c");
      auto m = 0;

      //cout << "Reading pixel " << x << ", " << y << ", " << c << endl;

      // TODO: Replace with real window traversal
      for (auto argPt : outArgLocations) {
        string outArgName = "self." + argPt.first;
        cout << "OutArgName = " << outArgName << ", location = " << argPt.second << endl;
        auto output_bv = state->getBitVec(outArgName);
        int output_value;
        output_value = output_bv.to_type<int>();
        cout << "\tOutput value = " << output_value << endl;

        int i = argPt.second.coord("c");
        int dOffset = destOffset +
          dest_stride_0 * x +
          dest_stride_1 * y +
          dest_stride_2 * (c + i) +
          dest_stride_3 * m;
        cout << "\tDest offset " << dOffset << " set to value " << output_value << endl;
        
       
        assert(destBits == 16 || destBits == 8);
        if (destBits == 8) {
          uint8_t* db = (uint8_t*) _halide_buffer_get_host(destBuf);
          db[dOffset] = output_value;
        } else {
          assert(destBits == 16);
          uint16_t* db = (uint16_t*) _halide_buffer_get_host(destBuf);
          db[dOffset] = output_value;
        }
      }

      //cout << "Set output at offset " << dOffset << endl;
      readIdx.increment();
    }
    
    state->exeSequential();

    writeIdx.increment();
  }

}

