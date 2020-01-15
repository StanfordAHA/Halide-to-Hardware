#ifndef HALIDE_UBUFFER_REWRITES_H
#define HALIDE_UBUFFER_REWRITES_H


#include "ExtractHWBuffers.h"
#include "HWUtils.h"
#include "IR.h"

namespace Halide {
  namespace Internal {
    void synthesize_hwbuffers(const Stmt& stmt, const std::map<std::string, Function>& env);
  }
}

#endif
