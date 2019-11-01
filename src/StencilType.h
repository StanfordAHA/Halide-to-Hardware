#ifndef HALIDE_STENCIL_TYPE_H
#define HALIDE_STENCIL_TYPE_H

#include "IR.h"

namespace Halide {

namespace Internal {

    // Moved from CoreIR_CodeGen_Base
    struct Stencil_Type {
      typedef enum {Stencil, Stream, AxiStream} StencilContainerType;
      StencilContainerType type;
      Type elemType;  // type of the element
      Region bounds;  // extent of each dimension
      int depth;      // FIFO depth if it is a Stream type
    };
  }
}
#endif
