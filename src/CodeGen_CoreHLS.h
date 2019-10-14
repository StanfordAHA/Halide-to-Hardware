#ifndef HALIDE_CODEGEN_COREHLS_H
#define HALIDE_CODEGEN_COREHLS_H

#include "coreir.h"

namespace Halide {
  namespace Internal {

    void loadHalideLib(CoreIR::Context* context);
    
    CoreIR::Module* createCoreIRForStmt(CoreIR::Context* context,
        Stmt stmt,
        const std::string& name,
        const std::vector<CoreIR_Argument>& args);
  }
}
#endif
