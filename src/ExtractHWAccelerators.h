#ifndef HALIDE_EXTRACT_HW_ACCELERATORS_H
#define HALIDE_EXTRACT_HW_ACCELERATORS_H

#include "IR.h"

namespace Halide {
namespace Internal {

Stmt extract_hwaccelerators(Stmt s, const std::map<std::string, Function> &env);

}
}

#endif
