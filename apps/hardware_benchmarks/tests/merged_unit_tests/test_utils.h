#pragma once

#include "coreir/ir/dynamic_bit_vector.h"
#include "coreir.h"
#include "coreir/simulator/interpreter.h"

static
bool hasClock(CoreIR::Module* m) {
  for (auto f : m->getType()->getRecord()) {
    if (isClockOrNestedClockType(f.second, m->getContext()->Named("coreir.clkIn"))) {
      return true;
    }
  }
  return false;
}

static inline
void resetSim(const std::string& inputName, CoreIR::Module* m, CoreIR::SimulatorState& state) {

    state.setValue(inputName, BitVector(16, 0));
    state.setValue("self.in_en", BitVector(1, 0));
    if (hasClock(m)) {
      state.setClock("self.clk", 0, 1);
    }
    state.setValue("self.reset", BitVector(1, 1));

    state.resetCircuit();

    state.setValue("self.reset", BitVector(1, 0));
}

static inline
void runCmd(const std::string& cmd) {
  std::cout << "Running command: " << cmd << std::endl;
  int res = system(cmd.c_str());
  assert(res == 0);
}

template<typename T>
static inline
void printBuffer(T& inputBuf, std::ostream& out) {
  if (inputBuf.dimensions() <= 2 || inputBuf.channels() == 1) {
    for (int i = inputBuf.top(); i <= inputBuf.bottom(); i++) {
      for (int j = inputBuf.left(); j <= inputBuf.right(); j++) {
        out << inputBuf(j, i) << " ";
      }
      out << std::endl;
    }
  } else {
    for (int c = 0; c < inputBuf.channels(); c++) {
      out << "------------------------------------------" << std::endl;
      out << "Channel " << c << std::endl;
      out << "------------------------------------------" << std::endl;
      for (int i = inputBuf.top(); i <= inputBuf.bottom(); i++) {
        for (int j = inputBuf.left(); j <= inputBuf.right(); j++) {
          out << inputBuf(j, i, c) << " ";
        }
        out << std::endl;
      }
    }
  }
}

template<typename T>
class Point {
  public:
    std::vector<T> values;
    std::vector<std::string> names;

    std::vector<std::string> dimensionNames() const { return names; }

    int numDims() { return values.size(); }

    void setCoord(const std::string& name, const T& newVal) {
      auto ind = indexOf(name);
      values[ind] = newVal;
    }

    int indexOf(const std::string& name) const {
      for (int i = 0; i < names.size(); i++) {
        if (names[i] == name) {
          return i;
        }
      }
      assert(false);
    }

    int coord(const std::string& name) const {
      for (int i = 0; i < names.size(); i++) {
        if (names[i] == name) {
          return values[i];
        }
      }
      assert(false);
    }

    Point<T> appendDim(const std::string& name, const int i) {
      Point<T> fresh = *this;
      fresh.names.push_back(name);
      fresh.values.push_back(i);
      return fresh;
    }

};

