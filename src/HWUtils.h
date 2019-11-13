#ifndef HALIDE_HW_UTILS_H
#define HALIDE_HW_UTILS_H

#include <string>
#include <vector>

namespace Halide {
  namespace Internal {

    std::vector<std::string> get_tokens(const std::string &line, const std::string &delimiter);
  }
}
#endif

