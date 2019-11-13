#ifndef HALIDE_HW_STENCIL_H
#define HALIDE_HW_STENCIL_H

namespace Halide {
  namespace Internal {

struct Stride {
  int stride;
  bool is_inverse;
  Stride(int stride=0, bool is_inverse=false) :
    stride(stride), is_inverse(is_inverse) {}

};

  }
}

#endif
