#ifndef HALIDE_CODEGEN_COREIR_TESTBENCH_H
#define HALIDE_CODEGEN_COREIR_TESTBENCH_H

/** \file
 *
 * Defines the code-generator for producing HLS testbench code
 */
#include <sstream>

#include "CodeGen_CoreIR_Base.h"
#include "CodeGen_CoreIR_Target.h"
#include "Module.h"
#include "Scope.h"

namespace Halide {

namespace Internal {

class CoreIR_Closure : public Closure {
public:
  CoreIR_Closure(Stmt s, std::string output_string)  {
        s.accept(this);
        output_name = output_string;
    }

  std::vector<CoreIR_Argument> arguments(const Scope<CodeGen_CoreIR_Base::Stencil_Type> &scope);

protected:
    using Closure::visit;
    std::string output_name;

};


/** A code generator that emits Xilinx Vivado HLS compatible C++ testbench code.
 */
class CodeGen_CoreIR_Testbench : public CodeGen_CoreIR_Base {
public:
  CodeGen_CoreIR_Testbench(std::ostream &tb_stream, Target target);
    ~CodeGen_CoreIR_Testbench();
    void set_output_folder(std::string folderpath) {
      cg_target.set_output_folder(folderpath);
    }
    
protected:
    using CodeGen_CoreIR_Base::visit;

    void visit(const ProducerConsumer *);
    void visit(const Call *);
    void visit(const Realize *);
    void visit(const Block *);

private:
    CodeGen_CoreIR_Target cg_target;


};

}
}

#endif
