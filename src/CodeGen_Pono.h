#ifndef HALIDE_CODEGEN_PONO_H
#define HALIDE_CODEGEN_PONO_H

/** \file
 *
 * Defines an IRPrinter that emits C++ code equivalent to a halide stmt
 */
#include "CodeGen_C.h"
#include "IRPrinter.h"
#include "Module.h"
#include "Scope.h"
#include "Bounds.h"

namespace Halide {

struct Argument;

namespace Internal {

struct HW_Stencil_Type_Pono
{
    enum class StencilContainerType { Stencil, Stream, AxiStream };
    
    StencilContainerType type;
    Type elemType;
    Region bounds;
    int depth;
};

struct HW_Arg_Pono
{
    std::string name;
    bool is_stencil;
    bool is_output;
    Type scalar_type;
    HW_Stencil_Type_Pono stencil_type;
    std::vector<Expr> args;
    Box box;
};



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


    /** The target we're generating code for */
    const Target &get_target() const { return target; }

    /** emit an expression on the output stream */
    void print(Expr);


    /** emit a statement on the output stream */
    void print(Stmt);

    /** emit a comma delimited list of exprs, without any leading or
     * trailing punctuation. */
    void print_list(const std::vector<Expr> &exprs);

    void print_arg_list(const std::vector<Expr> &exprs);

    static void test();

    // list of strings
    std::vector<std::string> buffers;

protected:


    /** An ID for the most recently generated ssa variable */
    std::string id;

    /** The target being generated for. */
    Target target;


    using IRPrinter::visit;

    /** The current indentation level, useful for pretty-printing
     * statements */
    int indent;

    Scope<Expr> scope;

    bool python_type;

    /** Emit spaces according to the current indentation level */
    void do_indent();
    std::string print_name(const std::string &name);
    std::string print_type(const Halide::Type &t, const std::string &val);
    std::string print_type(const Halide::Type &t);

    void visit(const IntImm *) override;
    void visit(const UIntImm *) override;
    void visit(const FloatImm *) override;
    void visit(const StringImm *) override;
    void visit(const Cast *) override;
    void visit(const Variable *) override;
    void visit(const Add *) override;
    void visit(const Sub *) override;
    void visit(const Mul *) override;
    void visit(const Div *) override;
    void visit(const Mod *) override;
    void visit(const Min *) override;
    void visit(const Max *) override;
    void visit(const EQ *) override;
    void visit(const NE *) override;
    void visit(const LT *) override;
    void visit(const LE *) override;
    void visit(const GT *) override;
    void visit(const GE *) override;
    void visit(const And *) override;
    void visit(const Or *) override;
    void visit(const Not *) override;
    void visit(const Select *) override;
    void visit(const Load *) override;
    void visit(const Ramp *) override;
    void visit(const Broadcast *) override;
    void visit(const Call *) override;
    void visit(const Let *) override;
    void visit(const LetStmt *) override;
    void visit(const AssertStmt *) override;
    void visit(const ProducerConsumer *) override;
    void visit(const For *) override;
    void visit(const Acquire *) override;
    void visit(const Store *) override;
    void visit(const Provide *) override;
    void visit(const Allocate *) override;
    void visit(const Free *) override;
    void visit(const Realize *) override;
    void visit(const Block *) override;
    void visit(const Fork *) override;
    void visit(const IfThenElse *) override;
    void visit(const Evaluate *) override;
    void visit(const Shuffle *) override;
    void visit(const Prefetch *) override;
};


class CodeGen_Pono_Testbench : public IRVisitor {
public:
    CodeGen_Pono_Testbench(std::ostream &s, std::ostream &ts, Target t, std::string target_filename);
    void compile(const Halide::Module &module);

protected:
    using IRVisitor::visit;
    Scope<Expr> scope;
    Scope<HW_Stencil_Type_Pono> stencils;

    virtual void compile(const LoweredFunc &func);
    virtual void compile(const Buffer<> &buffer);

    std::string print_name(const std::string &name);

    std::ostream &stream;
    std::ostream &testbench_stream;
    CodeGen_Pono codegen;
    CodeGen_Pono codegen_testbench;
    void visit(const ProducerConsumer *op) override;
    void visit(const Realize *op) override;
};

}  // namespace Internal
}  // namespace Halide

#endif
