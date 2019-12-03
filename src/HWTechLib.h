#ifndef HALIDE_HWTECHLIB_H
#define HALIDE_HWTECHLIB_H

#include <map>
#include <string>

namespace Halide {
namespace Internal {

  class Techlib {
    public:
      std::map<std::string, int> cycleLatency;
      std::map<std::string, int> criticalPath;
  };

  class HardwareInfo {
    public:
      bool hasCriticalPathTarget;
      int criticalPathTarget;
      // Technology library
      Techlib techlib;
  };



}
}

#endif
