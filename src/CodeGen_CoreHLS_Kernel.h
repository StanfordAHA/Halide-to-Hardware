#ifndef HALIDE_CODEGEN_COREHLS_KERNEL_H
#define HALIDE_CODEGEN_COREHLS_KERNEL_H

#include <sstream>

#include "IRVisitor.h"
#include "Module.h"
#include "Scope.h"
#include "StencilType.h"

namespace Halide {
namespace Internal {

  class CodeGen_CoreHLS_Kernel : public IRGraphVisitor {
    public:
      std::string outName;

      Scope<Stencil_Type> stencils;  // scope of stencils and streams of stencils
      CodeGen_CoreHLS_Kernel(const std::string& outFileName) : outName(outFileName) {}
    protected:

      void visit(const ProducerConsumer *);
      void visit(const Call *);
      void visit(const Realize *);
      void visit(const Block *);
  };


}
}

#endif
