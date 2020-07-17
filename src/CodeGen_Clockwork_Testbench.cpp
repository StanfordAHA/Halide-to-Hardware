#include "CodeGen_Clockwork_Testbench.h"

namespace Halide::Internal {

using std::ostream;
using std::string;
using std::vector;

CodeGen_Clockwork_Testbench::CodeGen_Clockwork_Testbench(ostream& output_stream,
                                                         const Target& target,
                                                         const std::string& pipeline_name
                                                        ) : CodeGen_RDAI(output_stream, target, pipeline_name),
                                                            tgt_codegen(pipeline_name, target),
                                                            tgt_codegen_like(this)
{
    tgt_codegen.init_module();
}

CodeGen_Clockwork_Testbench::~CodeGen_Clockwork_Testbench() {
}

RDAI_TargetGenLike *CodeGen_Clockwork_Testbench::get_target_codegen() {
    return &tgt_codegen_like;
}

} // namespace Halide::Internal
