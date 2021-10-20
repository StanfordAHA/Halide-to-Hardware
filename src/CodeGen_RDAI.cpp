#include <utility>
#include <fstream>

#include "Closure.h"
#include "CodeGen_RDAI.h"
#include "Simplify.h"
#include "Substitute.h"
#include "Module.h"

namespace Halide::Internal {

using std::string;
using std::ostream;
using std::vector;
using std::pair;
using std::ofstream;
using std::ostringstream;

class RDAI_Closure : public Closure {
public:
    RDAI_Closure(Stmt s, const string& output_fn_name) : output_name(output_fn_name) {
        s.accept(this);
    }

    vector<HW_Arg> arguments(const Scope<HW_Stencil_Type>& streams_scope) {
        vector<HW_Arg> result;
        if (!buffers.empty()) {
          std::cout << "buffers include: ";
          for (auto buffer : buffers) {
            std::cout << buffer.first << ", ";
          }
          std::cout << std::endl;
        }
        internal_assert(buffers.empty()) << "we expect no references to buffers in a hardware pipeline\n";
        for (const pair<string, Type> &v : vars) {
            if (ends_with(v.first, ".stencil")) {
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

        //result.push_back({output_name + "_stencil", true, true, Type(), streams_scope.get("hw_output.stencil")});
        //result.push_back({output_name + "_stencil", true, true, Type(), streams_scope.get("cim.stencil")});
        
        //std::cout << "output name is " << output_name << std::endl;        
        string out_scope_name = output_name.substr(1); // removes the preceding .
        result.push_back({output_name + "_stencil", true, true, Type(), streams_scope.get(out_scope_name + ".stencil")});
        return result;
    }

protected:
    const string& output_name;
};

CodeGen_RDAI::CodeGen_RDAI(ostream& pipeline_stream, const Target& target, string pipeline_name)
    : CodeGen_C(pipeline_stream, target, CPlusPlusImplementation, ""),
      target(target),
      pipeline_name(pipeline_name) {
    stream << "#include \"rdai_api.h\"\n";
}

CodeGen_RDAI::~CodeGen_RDAI() {}

namespace {
    void render_rdai_data(const string& output_folder, const RDAI_Info& info, const string& pipeline_name) {

        if(info.devices.empty()) return;

        string out_path = output_folder.empty() ? "rdai_clockwork_platform.h" :
            output_folder + "/" + "rdai_clockwork_platform.h";
        
        std::ofstream rdai_stream(out_path, std::ofstream::out);

        rdai_stream << "#ifndef RDAI_CLOCKWORK_PLATFORM_H\n"
                    << "#define RDAI_CLOCKWORK_PLATFORM_H\n";

        rdai_stream << "\n";
        rdai_stream << "#include \"rdai_api.h\"\n";
        rdai_stream << "\n";

        ostringstream oss;
        oss << info.devices[0].vendor << "_" << info.devices[0].library <<
            "_" << info.devices[0].name << "_" << info.devices[0].version;

        string device_name = oss.str();

        rdai_stream << "extern RDAI_Platform " << info.platform_name << ";\n";
        rdai_stream << "\n";

        rdai_stream << "static RDAI_Device " << device_name << " = {\n"
                    << "\t{ 1 },\n"
                    << "\t{\n"
                    << "\t\t{ \"" << info.devices[0].vendor  << "\" },\n"
                    << "\t\t{ \"" << info.devices[0].library << "\"  },\n"
                    << "\t\t{ \"" << info.devices[0].name  << "\" },\n"
                    << "\t\t" << info.devices[0].version << ",\n"
                    << "\t},\n"
                    << "\t&" << info.platform_name << ",\n"
                    << "\tNULL,\n"
                    << "\t" << info.devices[0].num_inputs << "\n"
                    << "};\n";
        rdai_stream << "\n";

        rdai_stream << "static RDAI_Device *" << info.platform_name << "_devices[2] = { &" << device_name << ", NULL };\n";
        rdai_stream << "\n";

        rdai_stream << "RDAI_Platform " << info.platform_name << " = {\n"
                    << "\tRDAI_PlatformType::RDAI_CLOCKWORK_PLATFORM,\n"
                    << "\t{ 0 },\n"
                    << "\tNULL,\n"
                    << "\t" << info.platform_name << "_devices\n"
                    << "};\n";
                    
        rdai_stream << "\n\n";

        rdai_stream << "#endif // RDAI_CLOCKWORK_PLATFORM_H\n";
    }
}

void CodeGen_RDAI::set_output_folder(const string& out_folder) {
    output_directory = out_folder;
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
        
        Expr temp_arg = op->args[0] - dim_bounds[0].min;
        Expr temp_stride = 1;
        for (size_t i = 1; i < dim_bounds.size(); i++) {
            temp_stride = temp_stride * dim_bounds[i-1].extent;
            temp_arg = temp_arg + ((op->args[i] - dim_bounds[i].min) * temp_stride);
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
    string target_prefix = "_hls_target";
    if (starts_with(op->name, target_prefix) && op->is_producer) {
        
        Stmt hw_body = substitute_in_all_letstmts(op->body);

        // Create HW closure
        debug(1) << "[RDAI] compute closure for <" << op->name << ">\n";
        string output_name = op->name.substr(target_prefix.length());
        RDAI_Closure closure(hw_body, output_name);
        vector<HW_Arg> args = closure.arguments(stencils);

        // Launch target codegen
        string hw_ip_name = unique_name("clockwork_target_" + pipeline_name);
        RDAI_TargetGenLike *cg_target = get_target_codegen();
        std::cout << op->body << std::endl; // show the accelerated section

        std::cout << "xcel for " << output_name << " out of " << num_xcels
                  << " xcels in " << pipeline_name << std::endl;
        cg_target->add_kernel(hw_body, num_xcels>1 ? output_name : pipeline_name, args);

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
        do_indent();
        stream << "RDAI_free_device_list(devices);\n";
        do_indent();
        stream << "RDAI_free_platform_list(platforms);\n";
        stream << "\n";

        // Emit RDAI Info
        rdai_info.platform_type = "RDAI_PlatformType::RDAI_CLOCKWORK_PLATFORM";
        rdai_info.platform_name = "rdai_clockwork_platform";
        rdai_info.devices.push_back({"aha", "halide_hardware", pipeline_name, 1, args.size() - 1});

        render_rdai_data(output_directory, rdai_info, pipeline_name);

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

    Expr temp_arg = op->args[0] - dim_bounds[0].min;
    Expr temp_stride = 1;
    for (size_t i = 1; i < dim_bounds.size(); i++) {
        temp_stride = temp_stride * dim_bounds[i-1].extent;
        temp_arg = temp_arg + ((op->args[i] - dim_bounds[i].min) * temp_stride);
    }
    temp_arg = simplify(temp_arg);

    stream << print_expr(temp_arg) << ";\n";
    string id1 = id;

    do_indent();
    stream << op_name << "_host["<< id1 <<"] = " << id0 << ";\n";
}

void CodeGen_RDAI::visit(const Realize *op) {
    if(ends_with(op->name, ".stencil")) {
        // possibly insert _halide_buffer_get_host calls
        if(!inserted_host_buf_calls && (func_args.size() > 0)) {
            do_indent();
            stream << "// get host pointers for input/output halide buffers\n";
            for(const LoweredArgument& arg : func_args) {
                Expr var_expr = Variable::make(type_of<struct halide_buffer_t *>(), arg.name + ".buffer");
                vector<Expr> call_args = {var_expr};
                Expr call_expr = Call::make(Handle(), "_halide_buffer_get_host", call_args, Call::CallType::Extern);
                const Call *call_ptr = call_expr.as<Call>();
                do_indent();
                stream << "void *" << print_name(arg.name) << " = " << print_extern_call(call_ptr) << ";\n";
            }
            stream << "\n";

            inserted_host_buf_calls = true;
        }

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

class CountAccelerators : public IRVisitor {
    using IRVisitor::visit;
    void visit(const ProducerConsumer *op) {
      string target_prefix = "_hls_target";
      if (starts_with(op->name, target_prefix) && op->is_producer) {
        xcels.insert(op->name);
      } else {
        IRVisitor::visit(op);
      }
    }

public:
  std::set<string> xcels;

  CountAccelerators() {}
};

int accelerator_count(Stmt s) {
    CountAccelerators ca;
    s.accept(&ca);
    return ca.xcels.size();
}

void CodeGen_RDAI::compile(const LoweredFunc &func) {
    func_args = func.args;
    num_xcels = accelerator_count(func.body);
    CodeGen_C::compile(func);
}

} // namespace Halide::Internal
