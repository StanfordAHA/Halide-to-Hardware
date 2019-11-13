#ifndef HALIDE_HW_UTILS_H
#define HALIDE_HW_UTILS_H

#include <string>
#include <vector>

#include "IR.h"

namespace Halide {
  namespace Internal {

    std::vector<std::string> get_tokens(const std::string &line, const std::string &delimiter);
    int id_const_value(const Expr e);
    std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec);
    bool is_parallelized(const For *op);
    std::string first_for_name(Stmt s);
  }
}
#endif

