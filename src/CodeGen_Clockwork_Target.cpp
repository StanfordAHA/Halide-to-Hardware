#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>

#include "CodeGen_Clockwork_Target.h"
#include "CodeGen_Internal.h"
#include "Substitute.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Param.h"
#include "Var.h"
#include "Lerp.h"
#include "Simplify.h"

namespace Halide {
namespace Internal {

using std::ostream;
using std::endl;
using std::string;
using std::vector;
using std::ostringstream;
using std::ofstream;

namespace {

class ContainForLoop : public IRVisitor {
    using IRVisitor::visit;
    void visit(const For *op) {
        found = true;
        return;
    }

public:
    bool found;

    ContainForLoop() : found(false) {}
};

bool contain_for_loop(Stmt s) {
    ContainForLoop cfl;
    s.accept(&cfl);
    return cfl.found;
}

}

CodeGen_Clockwork_Target::CodeGen_Clockwork_Target(const string &name, Target target)
  : target_name(name),
    hdrc(hdr_stream, target, CodeGen_Clockwork_C::CPlusPlusHeader), 
    srcc(src_stream, target, CodeGen_Clockwork_C::CPlusPlusImplementation),
    clkc(clk_stream, target, CodeGen_Clockwork_C::CPlusPlusImplementation) { clkc.is_clockwork = true; }
    //clkc(std::cout, target, CodeGen_Clockwork_C::CPlusPlusImplementation) { clkc.is_clockwork = true; }



CodeGen_Clockwork_Target::~CodeGen_Clockwork_Target() {
    hdr_stream << "#endif\n";
    clkc.memory_stream << endl
                       << "  return prg;" << endl
                       << "}";

    std::cout << "outputting clockwork target named " << target_name << std::endl;

    // write the header and the source streams into files
    string src_name = output_base_path + target_name + ".cpp";
    string hdr_name = output_base_path + target_name + ".h";
    string clk_debug_name = output_base_path + target_name + "_debug.cpp";
    string clk_memory_name = output_base_path + target_name + "_memory.cpp";
    string clk_compute_name = output_base_path + target_name + "_compute.h";

    ofstream src_file(src_name.c_str());
    ofstream hdr_file(hdr_name.c_str());
    ofstream clk_debug_file(clk_debug_name.c_str());
    ofstream clk_memory_file(clk_memory_name.c_str());
    ofstream clk_compute_file(clk_compute_name.c_str());

    src_file << src_stream.str() << endl;
    hdr_file << hdr_stream.str() << endl;
    clk_debug_file << clk_stream.str() << endl;
    clk_memory_file << clkc.memory_stream.str() << endl;
    clk_compute_file << clkc.compute_stream.str() << endl;
    
    src_file.close();
    hdr_file.close();
    clk_debug_file.close();
    clk_memory_file.close();
    clk_compute_file.close();
}

namespace {
const string clockwork_header_includes =
    "#include <assert.h>\n"
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <hls_stream.h>\n"
    "#include \"Stencil.h\"\n";

const string local_halide_helpers =
    "\n"
    "template <typename T>\n"
    "static inline T halide_cpp_min(const T&a, const T&b) { return (a < b) ? a : b; }\n\n"
    "template <typename T>\n"
    "static inline T halide_cpp_max(const T&a, const T&b) { return (a > b) ? a : b; }\n\n";
}

void CodeGen_Clockwork_Target::init_module() {
    debug(1) << "CodeGen_Clockwork_Target::init_module\n";

    // wipe the internal streams
    hdr_stream.str("");
    hdr_stream.clear();
    src_stream.str("");
    src_stream.clear();
    clk_stream.str("");
    clk_stream.clear();

    // initialize the header file
    string module_name = "HALIDE_CODEGEN_CLOCKWORK_TARGET_" + target_name + "_H";
    std::transform(module_name.begin(), module_name.end(), module_name.begin(), toupper);
    hdr_stream << "#ifndef " << module_name << '\n'
               << "#define " << module_name << "\n\n"
               << clockwork_header_includes << '\n';

    // initialize the source file
    src_stream << "#include \"" << target_name << ".h\"\n\n"
               << "#include \"Linebuffer.h\"\n"
               << "#include \"halide_math.h\"\n"
               << local_halide_helpers;

    // initialize the clockwork compute file
    clkc.compute_stream << "#pragma once" << endl
                        << "#include \"hw_classes.h\"" << endl << endl;

    // initialize the clockwork memory file
    clkc.memory_stream << "#include \"app.h\"\n"
                       << "#include \"ubuffer.h\"\n"
                       << "#include \"codegen.h\"\n"
                       << "#include \"prog.h\"\n\n";

}

void CodeGen_Clockwork_Target::add_kernel(Stmt s,
                                          const string &name,
                                          const vector<Clockwork_Argument> &args) {
    debug(1) << "CodeGen_Clockwork_Target::add_kernel " << name << "\n";

    //hdrc.add_kernel(s, name, args);
    //srcc.add_kernel(s, name, args);
    clkc.add_kernel(s, name, args);
}

void CodeGen_Clockwork_Target::dump() {
    std::cerr << src_stream.str() << std::endl;
}


string CodeGen_Clockwork_Target::CodeGen_Clockwork_C::print_stencil_pragma(const string &name) {
    ostringstream oss;
    internal_assert(stencils.contains(name));
    Stencil_Type stype = stencils.get(name);
    if (stype.type == Stencil_Type::StencilContainerType::Stream ||
        stype.type == Stencil_Type::StencilContainerType::AxiStream) {
        oss << "#pragma HLS STREAM variable=" << print_name(name) << " depth=" << stype.depth << "\n";
        if (stype.depth <= 100) {
            // use shift register implementation when the FIFO is shallow
            oss << "#pragma HLS RESOURCE variable=" << print_name(name) << " core=FIFO_SRL\n\n";
        }
    } else if (stype.type == Stencil_Type::StencilContainerType::Stencil) {
        oss << "#pragma HLS ARRAY_PARTITION variable=" << print_name(name) << ".value complete dim=0\n\n";
    } else {
        internal_error;
    }
    return oss.str();
}


void CodeGen_Clockwork_Target::CodeGen_Clockwork_C::add_kernel(Stmt stmt,
                                                               const string &name,
                                                               const vector<Clockwork_Argument> &args) {

    if (is_clockwork) {
      //stream << "prog " << name << "() {" << std::endl;
      memory_stream << "prog " << name << "() {" << endl
                    << "  prog prg;" << std::endl
                    << "  prg.compute_unit_file = \"" << name << "_compute.h\"" << std::endl
                    << "  prg.name = \"" << name << "\";" << endl
                    << std::endl;
      mem_bodyname = "prg";
    }
  
    // Emit the function prototype
    //stream << "void " << name << "(\n";
    
    for (size_t i = 0; i < args.size(); i++) {
        string arg_name = "arg_" + std::to_string(i);
        if (args[i].is_stencil) {
            CodeGen_Clockwork_Base::Stencil_Type stype = args[i].stencil_type;
            internal_assert(args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream ||
                            args[i].stencil_type.type == Stencil_Type::StencilContainerType::Stencil);
            //stream << print_stencil_type(args[i].stencil_type) << " ";
            if (args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream) {
              //stream << "&";  // hls_stream needs to be passed by reference
            }
            //stream << arg_name;
            allocations.push(args[i].name, {args[i].stencil_type.elemType});
            stencils.push(args[i].name, args[i].stencil_type);
        } else {
          //stream << print_type(args[i].scalar_type) << " " << arg_name;
        }

        if (is_clockwork && args[i].is_output) {
          memory_stream << "  prg.add_output(\"" << arg_name << "\");" << std::endl;
          memory_stream << "  prg.buffer_port_widths[\"" << arg_name << "\"] = 16;" << std::endl;
        } else if (is_clockwork && !args[i].is_output) {
          memory_stream << "  prg.add_input(\"" << arg_name << "\");" << std::endl;
          memory_stream << "  prg.buffer_port_widths[\"" << arg_name << "\"] = 16;" << std::endl;
        }

        //if (i < args.size()-1) stream << ",\n";
    }
    
    memory_stream << std::endl;
    
    if (is_header()) {
      //stream << ");\n";
    } else {
        stream << ")\n";
        open_scope();

        // add HLS pragma at function scope
        stream << "#pragma HLS DATAFLOW\n"
               << "#pragma HLS INLINE region\n"
               << "#pragma HLS INTERFACE s_axilite port=return bundle=config\n";
        for (size_t i = 0; i < args.size(); i++) {
            string arg_name = "arg_" + std::to_string(i);
            if (args[i].is_stencil) {
                if (ends_with(args[i].name, ".stream")) {
                    // stream arguments use AXI-stream interface
                    stream << "#pragma HLS INTERFACE axis register "
                           << "port=" << arg_name << " name=" << arg_name << "\n";
                } else {
                    // stencil arguments use AXI-lite interface
                    stream << "#pragma HLS INTERFACE s_axilite "
                           << "port=" << arg_name
                           << " bundle=config\n";
                    stream << "#pragma HLS ARRAY_PARTITION "
                           << "variable=" << arg_name << ".value complete dim=0\n";
                }
            } else {
                // scalar arguments use AXI-lite interface
                stream << "#pragma HLS INTERFACE s_axilite "
                       << "port=" << arg_name << " bundle=config\n";
            }
        }
        stream << "\n";
        //return;

        // create alias (references) of the arguments using the names in the IR
        do_indent();
        stream << "// alias the arguments\n";
        for (size_t i = 0; i < args.size(); i++) {
            string arg_name = "arg_" + std::to_string(i);
            do_indent();
            if (args[i].is_stencil) {
                CodeGen_Clockwork_Base::Stencil_Type stype = args[i].stencil_type;
                stream << print_stencil_type(args[i].stencil_type) << " &"
                       << print_name(args[i].name) << " = " << arg_name << ";\n";
            } else {
                stream << print_type(args[i].scalar_type) << " &"
                       << print_name(args[i].name) << " = " << arg_name << ";\n";
            }
        }
        //stream << "\n";

        // print body
        print(stmt);

        close_scope("kernel hls_target" + print_name(name));
    }
    //stream << "\n";

    for (size_t i = 0; i < args.size(); i++) {
        // Remove buffer arguments from allocation scope
        if (args[i].stencil_type.type == Stencil_Type::StencilContainerType::Stream) {
            allocations.pop(args[i].name);
            stencils.pop(args[i].name);
        }
    }
}

class Compute_Closure : public Closure {
public:
  Compute_Closure(Stmt s, std::string output_string)  {
        s.accept(this);
        output_name = output_string;
    }

  vector<Clockwork_Argument> arguments();

protected:
    using Closure::visit;
    std::string output_name;

  void found_buffer_ref(const string &name, Type type, bool read, bool written, Halide::Buffer<> image);
};

void Compute_Closure::found_buffer_ref(const string &name, Type type,
                               bool read, bool written, Halide::Buffer<> image) {
  //std::cout << "looking at " << name << " in closure where ignore=" <<
  //ignore.contains(name) << "\n";
  //if (!ignore.contains(name)) {
  if (true) {
        std::cout << "Adding buffer " << name << " to closure\n";
        Buffer &ref = buffers[name];
        ref.type = type.element_of(); // TODO: Validate type is the same as existing refs?
        ref.read = ref.read || read;
        ref.write = ref.write || written;

        // If reading an image/buffer, compute the size.
        if (image.defined()) {
            ref.size = image.size_in_bytes();
            ref.dimensions = image.dimensions();
        }
    } else {
        debug(3) << "Not adding " << name << " to closure\n";
    }
}


vector<Clockwork_Argument> Compute_Closure::arguments() {
    vector<Clockwork_Argument> res;
    for (const std::pair<string, Closure::Buffer> &i : buffers) {
        std::cout << "buffer: " << i.first << " " << i.second.size;
        if (i.second.read) std::cout << " (read)";
        if (i.second.write) std::cout << " (write)";
        std::cout << "\n";
        res.push_back({i.first, true, true, Type(), CodeGen_Clockwork_Base::Stencil_Type()});
    }
    //internal_assert(buffers.empty()) << "we expect no references to buffers in a hw pipeline.\n";
    for (const std::pair<string, Type> &i : vars) {
        std::cout << "var: " << i.first << "\n";
        if(ends_with(i.first, ".stream") ||
           ends_with(i.first, ".stencil") ) {
            if (starts_with(i.first, output_name)) {
              res.push_back({i.first, true, true, Type(), CodeGen_Clockwork_Base::Stencil_Type()});
            } else {
              res.push_back({i.first, true, false, Type(), CodeGen_Clockwork_Base::Stencil_Type()});              
            }

        } else if (ends_with(i.first, ".stencil_update")) {
          //internal_error << "we don't expect to see a stencil_update type in Compute_Closure.\n";
        } else {
            // it is a scalar variable
          res.push_back({i.first, false, true, i.second, CodeGen_Clockwork_Base::Stencil_Type()});
        }
    }
    return res;
}

void CodeGen_Clockwork_Target::CodeGen_Clockwork_C::visit(const Provide *op) {
  // output the memory
  memory_stream << "//store is: " << Stmt(op) << std::endl;
  Compute_Closure c(Stmt(op), op->name);
  vector<Clockwork_Argument> compute_args = c.arguments();
  memory_stream << "  " << func_name << "->add_function(\"" << func_name << "\");" << endl;
    
  for (auto arg : compute_args) {
    memory_stream << "  " << func_name << "->add_load(\""
                  << arg.name << "\""
                  << ");\n";
  }
  
  memory_stream << "  " << func_name << "->add_store(\""
                << op->name << "\"";
  for (auto arg : op->args) {
    memory_stream << ", \"" << arg << "\"";
  }
  memory_stream << ");\n";

  // output the compute
  compute_stream << "//store is: " << Stmt(op) << std::endl;
  compute_stream << "int " << func_name << "(";
  for (size_t i=0; i<compute_args.size(); ++i) {
    if (i != 0) { compute_stream << ", "; }
    compute_stream << "int " << compute_args[i].name;
  }
  compute_stream << ") {\n";
  
  compute_stream << "  return "
                 << op->values[0] << ";" << endl
                 << "}" << endl;
      
  CodeGen_Clockwork_Base::visit(op);
}

void CodeGen_Clockwork_Target::CodeGen_Clockwork_C::visit(const ProducerConsumer *op) {
  if (op->is_producer) {
    //memory_stream << "////producing " << op->name << endl;
  } else {
    //memory_stream << "//consuming " << op->name << endl;

    func_name = "compute_" + op->name;
    memory_stream << "  auto " << func_name
                  << " = "
                  << mem_bodyname << "->add_op(\""
                  << func_name << "\");" << endl;

    //if (starts_with(op->name, "hw_output")) {
    //  memory_stream << "//let's find a closure\n";
    //  std::cout <<  "//let's find a closure\n";
    //  std::cout << op->body << std::endl;
    //  Compute_Closure c(op->body, op->name);
    //  vector<Clockwork_Argument> args = c.arguments();
    //}
  }
    
  CodeGen_Clockwork_Base::visit(op);
}

// almost that same as CodeGen_C::visit(const For *)
// we just add a 'HLS PIPELINE' pragma after the 'for' statement
void CodeGen_Clockwork_Target::CodeGen_Clockwork_C::visit(const For *op) {
    internal_assert(op->for_type == ForType::Serial)
        << "Can only emit serial for loops to HLS C\n";

    string id_min = print_expr(op->min);
    string id_extent = print_expr(op->extent);

    do_indent();
    stream << "for (int "
           << print_name(op->name)
           << " = " << id_min
           << "; "
           << print_name(op->name)
           << " < " << id_min
           << " + " << id_extent
           << "; "
           << print_name(op->name)
           << "++)\n";

    string loopname = "loop" + print_name(op->name);
    string bodyname = mem_bodyname;
    memory_stream << "  auto " << loopname << " = "
                  << bodyname << ".add_loop("
                  << "\"" << print_name(op->name) << "\""
                  << ", " << id_min
                  << ", " << id_extent
                  << ");\n";
    
    open_scope();
    // add a 'PIPELINE' pragma if it is an innermost loop
    if (!contain_for_loop(op->body)) {
        //stream << "#pragma HLS DEPENDENCE array inter false\n"
        //       << "#pragma HLS LOOP_FLATTEN off\n";
        stream << "#pragma HLS PIPELINE II=1\n";
    }

    mem_bodyname = loopname;
    op->body.accept(this);
    mem_bodyname = bodyname;
    
    close_scope("for " + print_name(op->name));
}

class RenameAllocation : public IRMutator {
    const string &orig_name;
    const string &new_name;

    using IRMutator::visit;

    Expr visit(const Load *op) {
        if (op->name == orig_name ) {
            Expr index = mutate(op->index);
            return Load::make(op->type, new_name, index, op->image, op->param, op->predicate, ModulusRemainder());
        } else {
            return IRMutator::visit(op);
        }
    }

    Stmt visit(const Store *op) {
        if (op->name == orig_name ) {
            Expr value = mutate(op->value);
            Expr index = mutate(op->index);
            return Store::make(new_name, value, index, op->param, op->predicate, ModulusRemainder());
        } else {
            return IRMutator::visit(op);
        }
    }

    Stmt visit(const Free *op) {
        if (op->name == orig_name) {
            return Free::make(new_name);
        } else {
            return IRMutator::visit(op);
        }
    }

public:
    RenameAllocation(const string &o, const string &n)
        : orig_name(o), new_name(n) {}
};

void CodeGen_Clockwork_Target::CodeGen_Clockwork_C::visit(const Call *op) {
  //std::cout << " looking at " << op->name << endl;
  CodeGen_Clockwork_Base::visit(op);
}

// most code is copied from CodeGen_C::visit(const Allocate *)
// we want to check that the allocation size is constant, and
// add a 'HLS ARRAY_PARTITION' pragma to the allocation
void CodeGen_Clockwork_Target::CodeGen_Clockwork_C::visit(const Allocate *op) {
    // We don't add scopes, as it messes up the dataflow directives in HLS compiler.
    // Instead, we rename the allocation to a unique name
    //open_scope();

    internal_assert(!op->new_expr.defined());
    internal_assert(!is_zero(op->condition));
    int32_t constant_size;
    constant_size = op->constant_allocation_size();
    if (constant_size > 0) {

    } else {
        internal_error << "Size for allocation " << op->name
                       << " is not a constant.\n";
    }

    // rename allocation to avoid name conflict due to unrolling
    string alloc_name = op->name + unique_name('a');
    Stmt new_body = RenameAllocation(op->name, alloc_name).mutate(op->body);

    Allocation alloc;
    alloc.type = op->type;
    allocations.push(alloc_name, alloc);

    do_indent();
    stream << print_type(op->type) << ' '
           << print_name(alloc_name)
           << "[" << constant_size << "];\n";
    // add a 'ARRAY_PARTITION" pragma
    //stream << "#pragma HLS ARRAY_PARTITION variable=" << print_name(op->name) << " complete dim=0\n\n";

    new_body.accept(this);

    // Should have been freed internally
    internal_assert(!allocations.contains(alloc_name))
        << "allocation " << alloc_name << " is not freed.\n";

    //close_scope("alloc " + print_name(op->name));

}

}
}
