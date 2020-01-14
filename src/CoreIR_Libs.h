#ifndef HALIDE_COREIR_LIBS_H
#define HALIDE_COREIR_LIBS_H

#include "coreir.h"

#define COREMK(ctx, v) CoreIR::Const::make((ctx), (v))

namespace Halide {
  namespace Internal {

    template<typename TOut, typename T>
      TOut* sc(T* p) {
        return static_cast<TOut*>(p);
      }

    void loadHalideLib(CoreIR::Context* context);

    std::string coreStr(const CoreIR::Type* w);
    std::string coreStr(const CoreIR::Wireable* w);

    std::vector<int> arrayDims(CoreIR::Type* tp);
    std::vector<int> arrayDims(CoreIR::Wireable* w);

    CoreIR::Values getGenArgs(CoreIR::Wireable* p);
    CoreIR::Instance* mkConst(CoreIR::ModuleDef* def, const std::string& name, const int width, const int val);

    CoreIR::Wireable* andVals(CoreIR::ModuleDef* def, CoreIR::Wireable* a, CoreIR::Wireable* b);

    CoreIR::Wireable* andList(CoreIR::ModuleDef* def, const std::vector<CoreIR::Wireable*>& vals);

  }
}


#endif
