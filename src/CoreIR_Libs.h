#ifndef HALIDE_COREIR_LIBS_H
#define HALIDE_COREIR_LIBS_H

#include "coreir.h"

#define COREMK(ctx, v) CoreIR::Const::make((ctx), (v))

namespace Halide {
  namespace Internal {
    void loadHalideLib(CoreIR::Context* context);
  }
}

#endif
