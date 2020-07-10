#include <utility>

#include "CodeGen_RDAI.h"
#include "Closure.h"
#include "Substitute.h"

namespace Halide::Internal {

using std::string;
using std::ostream;
using std::vector;
using std::pair;

class RDAI_Closure : public Closure {
public:
    RDAI_Closure(Stmt s, const string& output_fn_name) : output_name(output_fn_name) {
        s.accept(this);
    }

    vector<HW_Arg> arguments(const Scope<HW_Stencil_Type>& streams_scope) {
        vector<HW_Arg> result;
        internal_assert(buffers.empty()) << "we expect no references to buffers in a hardware pipeline\n";
        for(const pair<string, Type> &v : vars) {
            if(ends_with(v.first, ".stencil")) {
                HW_Stencil_Type stencil_type = streams_scope.get(v.first);
                if(starts_with(v.first, output_name)) {
                    result.push_back({v.first, true, true, Type(), stencil_type});
                } else {
                    result.push_back({v.first, true, false, Type(), stencil_type});
                }
            } else {
                // it is a scalar variable
                result.push_back({v.first, false, true, v.second, HW_Stencil_Type()});
            }
        }
        result.push_back({"hw_output_stencil", true, true, Type(), HW_Stencil_Type()});
        return result;
    }

protected:
    const string& output_name;
};

CodeGen_RDAI::CodeGen_RDAI(ostream& pipeline_stream, const Target& target, const string& pipeline_name)
    : CodeGen_C(pipeline_stream, target, CPlusPlusImplementation, ""),
      target(target),
      pipeline_name(pipeline_name)
{
    stream  << "// [GED_DEBUG] CodeGen_RDAI constructor called\n";
}

CodeGen_RDAI::~CodeGen_RDAI() {
    stream << "// [GED_DEBUG] CodeGen_RDAI destructor called\n";
}

void CodeGen_RDAI::set_output_folder(const string& out_folder) {
    RDAI_TargetGenLike *target_codegen = get_target_codegen();
    target_codegen->set_output_folder(out_folder);
}

void CodeGen_RDAI::visit(const Call *op) {
    if(op->is_intrinsic() && ends_with(op->name, ".stencil")) {
        stream << "// [GED_DEBUG] Absorbed call to " << op->name << "\n";
    } else {
        CodeGen_C::visit(op); 
    }
}

void CodeGen_RDAI::visit(const ProducerConsumer *op) {
    string target_prefix = "hw_output";
    if(starts_with(op->name, target_prefix) && op->is_producer) {
        
        Stmt hw_body = substitute_in_all_letstmts(op->body);

        // Create HW closure
        debug(1) << "[RDAI] compute closure for <" << op->name << ">\n";
        string output_name = op->name.substr(target_prefix.length());
        RDAI_Closure closure(hw_body, output_name);
        vector<HW_Arg> args = closure.arguments(stencils);

        // Launch target codegen
        string hw_ip_name = unique_name("clockwork_target_" + pipeline_name);
        RDAI_TargetGenLike *cg_target = get_target_codegen();
        cg_target->add_kernel(hw_body, hw_ip_name, args);

        // Emit RDAI API
        internal_assert(args.size() > 0) << "no input/output argumnets found for the accelerator\n";

        stream << "\n";
        do_indent();
        stream << "RDAI_PlatfomType platform_type = RDAI_PlatformType::RDAI_CLOCKWORK_PLATFORM;\n";
        do_indent();
        stream << "RDAI_Platform **platforms = RDAI_get_platforms_with_type(&platform_type);\n";
        do_indent();
        stream << "assert(platforms && platforms[0]);\n";
        do_indent();
        stream << "RDAI_VLNV device_vlnv = {{\"aha\"}, {\"halide_hardware\"}, {\"" << pipeline_name << "\"}, 1};\n";
        do_indent();
        stream << "RDAI_Device **devices = RDAI_get_devices_with_vlnv(platforms[0], &device_vlnv);\n";
        do_indent();
        stream << "assert(devices && devices[0]);\n";
        do_indent();
        stream << "RDAI_MemObject **mem_obj_list = (RDAI_MemObject **) malloc(sizeof(RDAI_MemObject*) * " << args.size() + 1 << ";\n";
    } else {
        CodeGen_C::visit(op);
    }
}

void CodeGen_RDAI::visit(const Provide *op) {
    stream << "// [GED_DEBUG] Absorbed Provide [" << op->name << "]\n";
    if(op->values.size() > 0) {
        Expr first = op->values[0];
        if(const Load *cnt = first.as<Load>()) {
            stream << "// [GED_DEBUG] found a Load from " << cnt->name << "\n";
        } else if(const Call *cnt = first.as<Call>()) {
            stream << "// [GED_DEBUG] found a call to " << cnt->name << "\n";
        } else {
            stream << "// [GED_DEBUG] found a NON-VIEW for this PROVIDE node\n";
        }
    }
}

void CodeGen_RDAI::visit(const Realize *op) {
    if(ends_with(op->name, ".stencil")) {
        // create a stencil type
        internal_assert(op->types.size() == 1);
        allocations.push(op->name, {op->types[0]});
        HW_Stencil_Type stype({HW_Stencil_Type::StencilContainerType::Stencil, op->types[0], op->bounds, 1});
        stencils.push(op->name, stype);
        stream << "// [GED_DEBUG] Absorbed Realize[" << op->name << "]\n";
        op->body.accept(this);
        allocations.pop(op->name);
        stencils.pop(op->name);
    } else {
        CodeGen_C::visit(op);
    }
}


} // namespace Halide::Internal
