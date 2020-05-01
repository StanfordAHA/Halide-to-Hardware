#ifndef HALIDE_CODEGEN_CLOCKWORK_TESTBENCH_H
#define HALIDE_CODEGEN_CLOCKWORK_TESTBENCH_H

/** \file
 *
 * Defines the code-generator for producing Clockwork testbench code
 */
#include <sstream>

#include "CodeGen_Clockwork_Base.h"
#include "CodeGen_Clockwork_Target.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

namespace Internal {

/** A code generator that emits Xilinx Vivado HLS compatible C++ testbench code.
 */
class CodeGen_Clockwork_Testbench : public CodeGen_Clockwork_Base {
public:
  CodeGen_Clockwork_Testbench(std::ostream &tb_stream, Target target);
  ~CodeGen_Clockwork_Testbench();
  void set_output_folder(std::string folderpath) {
    cg_target.set_output_folder(folderpath);
  }

protected:
    using CodeGen_Clockwork_Base::visit;

    void visit(const ProducerConsumer *);
    void visit(const Call *);
    void visit(const Realize *);
    void visit(const Block *);

private:
    CodeGen_Clockwork_Target cg_target;
};

}
}

#endif
