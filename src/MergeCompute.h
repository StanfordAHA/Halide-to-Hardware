#ifndef HALIDE_MERGE_COMPUTE_H
#define HALIDE_MERGE_COMPUTE_H

#include "IR.h"

namespace Halide {
namespace Internal {

Stmt merge_compute(Stmt s, const std::map<std::string, Function> &env);

}
}

#endif
