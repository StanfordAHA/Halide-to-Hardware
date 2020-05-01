#ifndef HALIDE_CODEGEN_CLOCKWORK_TARGET_H
#define HALIDE_CODEGEN_CLOCKWORK_TARGET_H

/** \file
 *
 * Defines an IRPrinter that emits Clockwork code.
 */

#include "CodeGen_Clockwork_Base.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

namespace Internal {

struct Clockwork_Argument {
    std::string name;

    bool is_stencil;

    Type scalar_type;

    CodeGen_Clockwork_Base::Stencil_Type stencil_type;
};

/** This class emits Xilinx Vivado HLS compatible C++ code.
 */
class CodeGen_Clockwork_Target {
public:
    /** Initialize a C code generator pointing at a particular output
     * stream (e.g. a file, or std::cout) */
    CodeGen_Clockwork_Target(const std::string &name, Target target);
    virtual ~CodeGen_Clockwork_Target();

    void init_module();

    void add_kernel(Stmt stmt,
                    const std::string &name,
                    const std::vector<Clockwork_Argument> &args);

    void dump();
    void set_output_folder(std::string folderpath) {
      output_base_path = folderpath;
      hdrc.set_output_path(folderpath);
      srcc.set_output_path(folderpath);
    }


protected:
    class CodeGen_Clockwork_C : public CodeGen_Clockwork_Base {
    public:
 CodeGen_Clockwork_C(std::ostream &s, Target target, OutputKind output_kind) :
  CodeGen_Clockwork_Base(s, target, output_kind) { }

      void set_output_path(std::string pathname) {
        output_base_path = pathname;
      }
  
      void add_kernel(Stmt stmt,
                      const std::string &name,
                      const std::vector<Clockwork_Argument> &args);

    protected:
      std::string print_stencil_pragma(const std::string &name);
      std::string output_base_path;
        
      using CodeGen_Clockwork_Base::visit;

      void visit(const For *op);
      void visit(const Allocate *op);
    };

    /** A name for the Clockwork target */
    std::string target_name;
    std::string output_base_path;
      
    /** String streams for building header and source files. */
    // @{
    std::ostringstream hdr_stream;
    std::ostringstream src_stream;
    // @}

    /** Code generators for Clockwork target header and the source. */
    // @{
    CodeGen_Clockwork_C hdrc;
    CodeGen_Clockwork_C srcc;
    // @}
};

}
}

#endif
