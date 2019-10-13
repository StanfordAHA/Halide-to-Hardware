#pragma once

#include "coreir/ir/dynamic_bit_vector.h"
#include "coreir.h"
#include "coreir/simulator/interpreter.h"

using namespace std;

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

template<typename T>
std::ostream& operator<<(std::ostream& out, const Point<T>& pt) {
  out << "(";
  for (int i = 0; i < (int) pt.values.size(); i++) {
    out << pt.values[i];
    if (i < ((int) pt.values.size()) - 1) {
      out << ", ";
    }
  }
  out << ")";
  return out;
}

template<typename T>
class CoordinateVector {
  public:

    std::vector<T> values;
    std::vector<std::string> names;
    std::vector<T> bounds;
    std::vector<T> increments;

    bool finished;

    CoordinateVector(vector<std::string> names_, vector<T> bounds_) : names(names_), bounds(bounds_), finished(false) {
      values.resize(names.size());
      increments.resize(names.size());
      for (int i = 0; i < (int) bounds.size(); i++) {
        values[i] = 0;
        increments[i] = 1;
      }
    }

    CoordinateVector(vector<std::string>& names_, vector<T>& bounds_) : names(names_), bounds(bounds_), finished(false) {
      values.resize(names.size());
      for (int i = 0; i < (int) bounds.size(); i++) {
        values[i] = 0;
      }
    }

    int indexOf(const std::string& name) const {
      for (int i = 0; i < (int) names.size(); i++) {
        auto cN = names[i];
        if (cN == name) {
          return i;
        }
      }

      assert(false);
    }

    void setIncrement(const std::string& name, const int inc) {
      int val = indexOf(name);
      increments[val] = inc;
    }

    int coord(const std::string& str) const {
      for (int i = 0; i < (int) names.size(); i++) {
        auto cN = names[i];
        if (cN == str) {
          return values[i];
        }
      }

      assert(false);
    }

    int intervalEnd(const std::string& name) const {
      int ind = indexOf(name);
      return intervalEnd(ind);
    }

    int intervalEnd(const int ind) const {
      return std::min(values[ind] + increments[ind] - 1, bounds[ind]);
    }

    std::string coordString() const {
      std::string str = "{";
      for (int i = 0; i < ((int) bounds.size()); i++) {
        str += "[" + std::to_string(values[i]) + ", " + std::to_string(intervalEnd(i)) + "] : " + std::to_string(bounds[i]);
        if (i < ((int) bounds.size()) - 1) {
          str += ", ";
        }
      }
      str += "}";
      return str;
    }

    std::vector<Point<T> > allPoints(const std::vector<Point<T> >& startingPts, const int i) const {
      assert(startingPts.size() > 0);
      assert(i < ((int) values.size()));

      std::vector<Point<T> > newPoints;
      for (auto pt : startingPts) {
        for (int b = values[i]; b <= intervalEnd(i); b++) {
          newPoints.push_back(pt.appendDim(names[i], b));
        }
      }

      //cout << "Newpoints at " << i << " has size " << newPoints.size() << endl;
      if (i == ((int) values.size()) - 1) {
        return newPoints;
      }

      auto finalPoints = allPoints(newPoints, i + 1);
      return finalPoints;
    }

    int intervalMin(const std::string& name) const {
      return values[indexOf(name)];
    }

    int intervalWidth(const std::string& name) const {
      return increments[indexOf(name)];
    }
    
    Point<T> relativeWindowPosition(const Point<T>& pt) const {
      Point<T> rel = pt;
      for (auto dim : pt.dimensionNames()) {
        rel.setCoord(dim, pt.coord(dim) % intervalWidth(dim));
      }
      return rel;
    }

    std::vector<Point<T> > currentWindowRelative() const {
      auto window = currentWindow();
      vector<Point<T> > relWindow;
      for (auto pt : window) {
        relWindow.push_back(relativeWindowPosition(pt));
      }

      return relWindow;
    }
    
    std::vector<Point<T> > currentWindow() const {
      std::vector<Point<T> > window{{}};
      assert(window.size() == 1);
      auto finalWindow = allPoints(window, 0);
      return finalWindow;
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
      return intervalEnd(level) == bounds[level];
      //bool atM = bounds[level] == values[level];
      ////cout << "atM = " << atM << " for level: " << level << ", bounds = " << bounds[level] << ", value = " << values[level] << endl;
      //return atM;
    }

    bool allAtMax() const {
      return atMax(0) && allLowerAtMax(0);
    }

    bool allDone() const {
      return finished && atMax(0) && allLowerAtMax(0);
    }
 
    void increment(const int index) {
      assert(index < ((int) values.size()));
      values[index] = std::min(bounds[index], values[index] + increments[index]);
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
          increment(i);
          //values[i]++;

          for (int j = i + 1; j < (int) bounds.size(); j++) {
            values[j] = 0;
          }
          break;
        }
      }
    }

};

template<typename T>
static inline
std::string outputForCoord2D(Point<T>& point) {
  return "self.out_0_0";
}

template<typename T>
static inline
std::string outputForCoord3D(Point<T>& point) {
  assert(point.coord("x") == 0);
  assert(point.coord("y") == 0);

  return "self." + std::string("out_") + std::to_string(point.coord("c")) + std::string("_0_0");
}

