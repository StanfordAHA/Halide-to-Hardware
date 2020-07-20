#ifndef HALIDE_CODEGEN_CLOCKWORK_BASE_H
#define HALIDE_CODEGEN_CLOCKWORK_BASE_H

/** \file
 *
 * Defines an base class of the Vivado HLS C code-generator
 */
#include "CodeGen_C.h"
#include "CodeGen_RDAI.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

namespace Internal {

/** This class emits C++ code from given Halide stmt that contains
 * stream and stencil types.
 */
class CodeGen_Clockwork_Base : public CodeGen_C {
public:
    /** Initialize a C code generator pointing at a particular output
     * stream (e.g. a file, or std::cout) */
  CodeGen_Clockwork_Base(std::ostream &dest,
                         Target target,
                         OutputKind output_kind = CPlusPlusImplementation,
                         const std::string &include_guard = "")
      : CodeGen_C(dest, target, output_kind, include_guard) {}

protected:
    Scope<HW_Stencil_Type> stencils;  // scope of stencils and streams of stencils

    virtual std::string print_stencil_type(HW_Stencil_Type s);
    virtual std::string print_name(const std::string &name);
    virtual std::string print_stencil_pragma(const std::string &name);

    using CodeGen_C::visit;

    void visit(const Call *);
    void visit(const Provide *);
    void visit(const Realize *);
};

}
}

#endif
