#ifndef HALIDE_CODEGEN_CLOCKWORK_TESTBENCH_H
#define HALIDE_CODEGEN_CLOCKWORK_TESTBENCH_H

/** \file
 *
 * Defines the code-generator for producing Clockwork testbench code
 */
#include "CodeGen_Clockwork_Target.h"
#include "CodeGen_RDAI.h"
#include "Module.h"

namespace Halide::Internal {

using std::string;
using std::vector;

/** A code generator that emits Xilinx Vivado HLS compatible C++ testbench code.
 */
class CodeGen_Clockwork_Testbench : public CodeGen_RDAI {
public:
    CodeGen_Clockwork_Testbench(std::ostream& output_stream, const Target& target, const std::string& pipeline_name);

    ~CodeGen_Clockwork_Testbench();

    RDAI_TargetGenLike *get_target_codegen()  override;

private:

    struct Clockwork_TargetGenLike : public RDAI_TargetGenLike
    {
        Clockwork_TargetGenLike(CodeGen_Clockwork_Testbench *parent) : parent(parent) {}

        void add_kernel(Stmt stmt, const string& kernel_name, const vector<HW_Arg>& args) {
            parent->tgt_codegen.add_kernel(stmt, kernel_name, args);
        }

        void set_output_folder(const string& out_dir) {
            parent->tgt_codegen.set_output_folder(out_dir);
        }

    private:
        CodeGen_Clockwork_Testbench *parent;
    };

    CodeGen_Clockwork_Target tgt_codegen;

    Clockwork_TargetGenLike tgt_codegen_like;
};

} // namespace Halide::Internal

#endif
