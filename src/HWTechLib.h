#ifndef HALIDE_HWTECHLIB_H
#define HALIDE_HWTECHLIB_H

#include <map>
#include <string>

namespace Halide {
namespace Internal {

  enum HWInterfacePolicy {
    HW_INTERFACE_POLICY_TOP,
    HW_INTERFACE_POLICY_COMPUTE_UNIT
  };

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
      HWInterfacePolicy interfacePolicy;

      HardwareInfo() : interfacePolicy(HW_INTERFACE_POLICY_TOP) {}

      int criticalPath(const std::string& opname) const {
        if (techlib.criticalPath.count(opname) > 0) {
          return techlib.criticalPath.at(opname);
        }
        return 0;
      }
  };



}
}

#endif
