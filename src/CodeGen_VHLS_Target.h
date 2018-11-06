#ifndef HALIDE_CODEGEN_VHLS_TARGET_H
#define HALIDE_CODEGEN_VHLS_TARGET_H

/** \file
 *
 * Defines an IRPrinter that emits Vivado HLS C++ code.
 */

#include "CodeGen_VHLS_Base.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

namespace Internal {

struct VHLS_Argument {
    std::string name;

    bool is_stencil;

    Type scalar_type;

    CodeGen_VHLS_Base::Stencil_Type stencil_type;
};

/** This class emits Xilinx Vivado HLS compatible C++ code.
 */
class CodeGen_VHLS_Target {
public:
    /** Initialize a C code generator pointing at a particular output
     * stream (e.g. a file, or std::cout) */
  CodeGen_VHLS_Target(const std::string &name, Target target);
    virtual ~CodeGen_VHLS_Target();

    void init_module();

    void add_kernel(Stmt stmt,
                    const std::string &name,
                    const std::vector<VHLS_Argument> &args);

    void dump();

protected:
    class CodeGen_VHLS_C : public CodeGen_VHLS_Base {
    public:
 CodeGen_VHLS_C(std::ostream &s, Target target, OutputKind output_kind) :
  CodeGen_VHLS_Base(s, target, output_kind) {}

        void add_kernel(Stmt stmt,
                        const std::string &name,
                        const std::vector<VHLS_Argument> &args);

    protected:
        std::string print_stencil_pragma(const std::string &name);

        using CodeGen_VHLS_Base::visit;

        void visit(const For *op);
        void visit(const Allocate *op);
    };

    /** A name for the VHLS target */
    std::string target_name;

    /** String streams for building header and source files. */
    // @{
    std::ostringstream hdr_stream;
    std::ostringstream src_stream;
    // @}

    /** Code generators for VHLS target header and the source. */
    // @{
    CodeGen_VHLS_C hdrc;
    CodeGen_VHLS_C srcc;
    // @}
};

}
}

#endif
