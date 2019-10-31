#ifndef HALIDE_PREPROCESS_HW_LOOPS_H
#define HALIDE_PREPROCESS_HW_LOOPS_H

#include "IR.h"

namespace Halide {
  namespace Internal {

    bool isInnermostLoop(const For* f);
    Stmt preprocessHWLoops(Stmt& stmt);

  }
}

#endif

