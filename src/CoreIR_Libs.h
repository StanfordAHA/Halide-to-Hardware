#ifndef HALIDE_COREIR_LIBS_H
#define HALIDE_COREIR_LIBS_H

#include "coreir.h"

#define COREMK(ctx, v) CoreIR::Const::make((ctx), (v))

namespace Halide {
  namespace Internal {

    static CoreIR::Context* active_ctx;
    static CoreIR::ModuleDef* active_def;

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

    void coreir_builder_set_context(CoreIR::Context* context);

    void coreir_builder_set_def(CoreIR::ModuleDef* def);

    CoreIR::Instance* build_counter(CoreIR::ModuleDef* def,
        const int width,
        const int min_val,
        const int max_val,
        const int inc_val);

    CoreIR::Wireable* sub(CoreIR::Wireable* a, CoreIR::Wireable* b);

    CoreIR::Wireable* add(CoreIR::Wireable* a, CoreIR::Wireable* b);

    CoreIR::Wireable* mul(CoreIR::Wireable* a, const int c);
    CoreIR::Wireable* smod(CoreIR::Wireable* a, const int c);

    CoreIR::Wireable* udiv(CoreIR::Wireable* a, const int c);
    CoreIR::Wireable* add(CoreIR::Wireable* a, const int c);
    CoreIR::Wireable* eq(CoreIR::Wireable* a, const int c);

    CoreIR::Wireable* geq(CoreIR::Wireable* a, const int c);
  }
}


#endif
