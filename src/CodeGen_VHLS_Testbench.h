#ifndef HALIDE_CODEGEN_VHLS_TESTBENCH_H
#define HALIDE_CODEGEN_VHLS_TESTBENCH_H

/** \file
 *
 * Defines the code-generator for producing Vivado HLS testbench code
 */
#include <sstream>

#include "CodeGen_VHLS_Base.h"
#include "CodeGen_VHLS_Target.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

namespace Internal {

/** A code generator that emits Xilinx Vivado HLS compatible C++ testbench code.
 */
class CodeGen_VHLS_Testbench : public CodeGen_VHLS_Base {
public:
  CodeGen_VHLS_Testbench(std::ostream &tb_stream, Target target);
  ~CodeGen_VHLS_Testbench();
   void set_output_folder(std::string folderpath) {
     cg_target.set_output_folder(folderpath);
   }

protected:
    using CodeGen_VHLS_Base::visit;

    void visit(const ProducerConsumer *);
    void visit(const Call *);
    void visit(const Realize *);
    void visit(const Block *);

private:
    CodeGen_VHLS_Target cg_target;
};

}
}

#endif
