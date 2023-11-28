#ifndef HALIDE_CODEGEN_PONO_H
#define HALIDE_CODEGEN_PONO_H

/** \file
 *
 * Defines an IRPrinter that emits C++ code equivalent to a halide stmt
 */

#include "IRPrinter.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

struct Argument;

namespace Internal {

/** This class emits C++ code equivalent to a halide Stmt. It's
 * mostly the same as an IRPrinter, but it's wrapped in a function
 * definition, and some things are handled differently to be valid
 * C++.
 */
class CodeGen_Pono : public IRPrinter {
public:

    CodeGen_Pono(std::ostream &dest,
              Target target);
    ~CodeGen_Pono() override;

    /** Emit the declarations contained in the module as C code. */
    void compile(const Halide::Module &module);

    /** The target we're generating code for */
    const Target &get_target() const { return target; }

    static void test();

protected:

    /** Emit a declaration. */
    // @{
    virtual void compile(const LoweredFunc &func);
    virtual void compile(const Buffer<> &buffer);
    // @}

    /** An ID for the most recently generated ssa variable */
    std::string id;

    /** The target being generated for. */
    Target target;


    using IRPrinter::visit;

    void visit(const Add *) override;
    void visit(const Mul *) override;
};

}  // namespace Internal
}  // namespace Halide

#endif
