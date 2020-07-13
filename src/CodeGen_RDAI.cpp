#include <utility>

#include "Closure.h"
#include "CodeGen_RDAI.h"
#include "Simplify.h"
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
        result.push_back({"hw_output_stencil", true, true, Type(), streams_scope.get("hw_output.stencil")});
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
    stream << "#include \"rdai_api.h\"\n";
}

CodeGen_RDAI::~CodeGen_RDAI() {
}

void CodeGen_RDAI::set_output_folder(const string& out_folder) {
    RDAI_TargetGenLike *target_codegen = get_target_codegen();
    target_codegen->set_output_folder(out_folder);
}

string CodeGen_RDAI::print_name(const string& name) {
    std::ostringstream oss;

    // prefix an underscore to avoid reserved words
    if(isalpha(name[0])) {
        oss << '_';
    }
    for(size_t i = 0; i < name.size(); i++) {
        if(!isalnum(name[i])) {
            oss << "_";
        } else {
            oss << name[i];
        }
    }

    return oss.str();
}

void CodeGen_RDAI::visit(const Call *op) {
    if(op->is_intrinsic() && ends_with(op->name, ".stencil")) {
        HW_Stencil_Type stencil = stencils.get(op->name);
        Region dim_bounds = stencil.bounds;
        internal_assert(dim_bounds.size() == op->args.size());
        
        Expr temp_arg = op->args[0];
        Expr temp_stride = 1;
        for (size_t i = 1; i < dim_bounds.size(); i++) {
            temp_stride = temp_stride * dim_bounds[i-1].extent;
            temp_arg = temp_arg + (op->args[i] * temp_stride);
        }
        temp_arg = simplify(temp_arg);
        stream << print_expr(temp_arg) << ";\n";

        string elt_type = print_type(stencil.elemType);
        string op_name = print_name(op->name);

        do_indent();
        stream << elt_type << " *" << op_name << "_host = (" << elt_type << " *) " << op_name << "->host_ptr;\n";
        print_assignment(op->type, op_name + "_host[" + id + "]");
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
        stream << "RDAI_PlatformType platform_type = RDAI_PlatformType::RDAI_CLOCKWORK_PLATFORM;\n";
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
        stream << "RDAI_MemObject *mem_obj_list["<< args.size() + 1 <<"] = {\n";
        for(size_t i = 0; i < args.size(); i++) {
            do_indent(); do_indent();
            stream << print_name(args[i].name) << ",\n";
        }
        do_indent(); do_indent();
        stream << "NULL\n";
        do_indent();
        stream << "};\n";
        do_indent();
        stream << "RDAI_Status status = RDAI_device_run(devices[0], mem_obj_list);\n";
        stream << "\n";

    } else {
        CodeGen_C::visit(op);
    }
}

void CodeGen_RDAI::visit(const Provide *op) {

    internal_assert(ends_with(op->name, ".stencil"));

    string elt_type = print_type(op->values[0].type());
    string op_name = print_name(op->name);

    do_indent();
    stream << "// Provision buffer " << op_name << "\n";
    stream << print_expr(op->values[0]) << ";\n";
    string id0 = id;
    do_indent();
    stream << elt_type << " *" << op_name << "_host = (" << elt_type << " *) " << op_name << "->host_ptr;\n";

    Region dim_bounds = stencils.get(op->name).bounds;
    internal_assert(dim_bounds.size() == op->args.size());

    Expr temp_arg = op->args[0];
    Expr temp_stride = 1;
    for (size_t i = 1; i < dim_bounds.size(); i++) {
        temp_stride = temp_stride * dim_bounds[i-1].extent;
        temp_arg = temp_arg + (op->args[i] * temp_stride);
    }
    temp_arg = simplify(temp_arg);

    stream << print_expr(temp_arg) << ";\n";
    string id1 = id;

    do_indent();
    stream << op_name << "_host["<< id1 <<"] = " << id0 << ";\n";
}

void CodeGen_RDAI::visit(const Realize *op) {
    if(ends_with(op->name, ".stencil")) {
        // create a stencil type
        internal_assert(op->types.size() == 1);
        //allocations.push(op->name, {op->types[0]});
        HW_Stencil_Type stype({HW_Stencil_Type::StencilContainerType::Stencil, op->types[0], op->bounds, 1});
        stencils.push(op->name, stype);
        
        Expr buf_size = op->types[0].bytes();
        for(size_t i = 0; i < op->bounds.size(); i++) {
            Range r = op->bounds[i];
            buf_size = buf_size * r.extent;
        }

        buf_size = simplify(buf_size);

        do_indent();
        stream << "// Allocate shared buffer for " << print_name(op->name) << "\n";
        do_indent();
        stream << "RDAI_MemObject *" << print_name(op->name) << " = RDAI_mem_shared_allocate(" << buf_size << ");\n";
        op->body.accept(this);
        do_indent();
        stream << "// Free shared buffer for " << op->name << "\n";
        do_indent();
        stream << "RDAI_mem_free(" << print_name(op->name) << ");\n";
        //allocations.pop(op->name);
        stencils.pop(op->name);
    } else {
        CodeGen_C::visit(op);
    }
}


} // namespace Halide::Internal
