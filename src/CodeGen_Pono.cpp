#include <iostream>
#include <limits>
#include "CodeGen_Pono.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"
#include "HWBufferUtils.h"
#include "Closure.h"

namespace Halide {
    namespace Internal {

        using std::endl;
        using std::map;
        using std::ostream;
        using std::ostringstream;
        using std::string;
        using std::vector;
        using std::pair;


class AssociatedProvideName : public IRVisitor {
    using IRVisitor::visit;
    void visit(const Provide *op) {
      //std::cout << "this provide is " << Stmt(op);
      for (auto value : op->values) {
        if (contains_call(value, call_name)) {
          internal_assert(!found || (op->name == provide_name)) << call_name << " is called by multiple provides! ("
                                                                << provide_name << " and " << op->name << ")";
          provide_name = op->name;
          found = true;
        }
      }
      IRVisitor::visit(op);
    }

public:
  bool found;
  string call_name;
  string provide_name;

  AssociatedProvideName(string call_name) : found(false), call_name(call_name) {}
};

string associated_provide_name(Stmt s, string call_name) {
    AssociatedProvideName apn(call_name);
    s.accept(&apn);
    internal_assert(apn.found);
    return apn.provide_name;
}

        class Pono_Closure : public Closure {
        public:
            Pono_Closure(Stmt s, const string& output_fn_name) : output_name(output_fn_name) {
                s.accept(this);
            }

            vector<HW_Arg_Pono> arguments(const Scope<HW_Stencil_Type_Pono>& streams_scope) {
                vector<HW_Arg_Pono> result;
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
                        HW_Stencil_Type_Pono stencil_type = streams_scope.get(v.first);
                        if (starts_with(v.first, output_name)) {
                            result.push_back({v.first, true, true, Type(), stencil_type});
                        } else {
                            result.push_back({v.first, true, false, Type(), stencil_type});
                        }
                    } else {
                        // it is a scalar variable
                        result.push_back({v.first, false, false, v.second, HW_Stencil_Type_Pono()});
                    }
                }

                std::cout << "output name is " << output_name << std::endl;        
                string out_scope_name = output_name.substr(1); // removes the preceding .
                string out_halide_name = out_scope_name + ".stencil";

                if (streams_scope.contains(out_scope_name + ".glb.stencil")) {
                    out_halide_name = out_scope_name + ".glb.stencil";
                } else {
                    internal_assert(streams_scope.contains(out_halide_name));
                }
                
                //result.push_back({output_name + "_stencil", true, true, Type(), streams_scope.get(out_scope_name + ".stencil")});
                result.push_back({out_halide_name, true, true, Type(), streams_scope.get(out_halide_name)});
                return result;
            }

        protected:
            const string& output_name;
        };

        CodeGen_Pono_Testbench::CodeGen_Pono_Testbench(ostream & s, ostream & ts, Target t, std::string target_filename): stream(s), testbench_stream(ts), indent(0), codegen(s, t), codegen_testbench(ts, t) {


            testbench_stream << "import sys\n";
            testbench_stream << "import os\n";
            testbench_stream << "import numpy as np\n";
            testbench_stream << "import pono\n";
            testbench_stream << "from verified_agile_hardware.solver import Solver\n";
            testbench_stream << "from " << target_filename << "_pono import run_app\n";

            testbench_stream << "\n";

            stream << "import numpy as np\n";
            stream << "import smt_switch as ss\n";

        }

        void CodeGen_Pono_Testbench::compile(const Module & input) {
            for (const auto & f: input.functions()) {
                compile(f);
            }
        }

        void CodeGen_Pono_Testbench::compile(const LoweredFunc & f) {
            const std::vector<LoweredArgument> &args = f.args;
            for (size_t i = 0; i < args.size(); i++) {
                if (args[i].is_buffer()) {
                    std::cout << print_name(args[i].name) << " buffer\n";
                }
            }
            f.body.accept(this);

        }

        void CodeGen_Pono_Testbench::compile(const Buffer < > & buffer) {

        }

        void CodeGen_Pono_Testbench::do_indent() {
            for (int i = 0; i < indent; i++) {
                testbench_stream << "    ";
            }
        }

        string CodeGen_Pono_Testbench::print_name(const string & name) {
            ostringstream oss;

            for (size_t i = 0; i < name.size(); i++) {
                if (name[i] == '.') {
                    oss << '_';
                } else if (name[i] == '$') {
                    oss << "__";
                } else oss << name[i];
            }
            return oss.str();
        }




        void CodeGen_Pono_Testbench::visit(const Realize * op) {
            if (ends_with(op->name, ".stencil")) {
                HW_Stencil_Type_Pono stype({HW_Stencil_Type_Pono::StencilContainerType::Stencil, op->types[0], op->bounds, 1});
                stencils.push(op->name, stype);
            }

            vector < Expr > realize_mins;
            for (size_t i = 0; i < op -> bounds.size(); i++) {
                realize_mins.emplace_back(op -> bounds[i].min);
            }
            std::cout << "CodeGen_Pono_Testbench shifting " << op->name << " by " << realize_mins << std::endl;

            auto new_body = shift_realize_bounds(op -> body, op -> name, realize_mins, scope);
            new_body.accept(this);
            

            // IRVisitor::visit(op);
        }

        void CodeGen_Pono_Testbench::visit(const ProducerConsumer * op) {
            string target_prefix = "_hls_target";
            if (starts_with(op->name, target_prefix) && op->is_producer) {
                string output_name = op->name.substr(target_prefix.length());
                Pono_Closure closure(op -> body, output_name);
                vector<HW_Arg_Pono> args = closure.arguments(stencils);

                stream << "def run_app(solver, ";
                for (size_t i = 0; i < args.size(); i++) {
                    stream << print_name(args[i].name);

                    if (i < args.size() - 1) {
                        stream << ", ";
                    }
                }
                stream << "):\n";
                stream << "    num_roms = 0\n";

                std::cout << "visiting producer " << op -> name << std::endl;

                codegen.print(op -> body);
                
                // Creating buffer input symbol arrays
                testbench_stream << "def create_app(solver):\n";

                for (size_t i = 0; i < args.size() - 1; i++) {
                    if (args[i].is_stencil) {

                        string provide_name = associated_provide_name(op->body, args[i].name);
                        auto box = box_provided(op->body, provide_name);
                        args[i].box = box;
                        auto extents = extract_extents(box);


                       
                        indent = 1;

                        // Declare array
                        do_indent();
                        testbench_stream << print_name(args[i].name) << " = ";
                        for (size_t i = 0; i < extents.size(); i++) {
                            testbench_stream << "[";
                        }
                        testbench_stream << "0";
                        for (size_t i = 0; i < extents.size(); i++) {
                            testbench_stream << " for _ in range(";
                            testbench_stream << extents[extents.size() - i - 1];
                            testbench_stream << ")]";
                        }
                        testbench_stream << "\n";


                        // Fill array with symbols

                        for (size_t j = 0; j < extents.size(); j++) {
                            do_indent();
                            testbench_stream << "for " << print_name(args[i].name) << "_dim_" << j << " in range(" << extents[j] << "):\n";
                            indent += 1;
                        }
                        do_indent();
                        testbench_stream << print_name(args[i].name) << "[";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "][";
                            }
                        }
                        
                        testbench_stream << "] = solver.create_fts_state_var(\"" << print_name(args[i].name) << "_\" + \"_\".join([str(";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "), str(";
                            }
                        }
                        testbench_stream << ")]), solver.create_bvsort(" << args[i].stencil_type.elemType.bits() << "))\n";


                        // Need to do this next part to ensure input var values don't change 
                        do_indent();

                        testbench_stream << "input_var_init = solver.create_fts_state_var(\"init_" << print_name(args[i].name) << "_\" + \"_\".join([str(";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "), str(";
                            }
                        }
                        testbench_stream << ")]), solver.create_bvsort(" << args[i].stencil_type.elemType.bits() << "))\n";

                        do_indent();
                        testbench_stream << "solver.fts.constrain_init(solver.create_term(solver.ops.Equal, ";

                        testbench_stream << print_name(args[i].name) << "[";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "][";
                            }
                        }
                        testbench_stream << "], input_var_init))\n";

                        do_indent();
                        testbench_stream << "solver.fts.assign_next(" << print_name(args[i].name) << "[";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "][";
                            }
                        }

                        testbench_stream << "], " << print_name(args[i].name) << "[";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "][";
                            }
                        }

                        testbench_stream << "])\n";

                        testbench_stream << "\n";
                        
                        indent -= extents.size();

                    } else {
                        std::cout << "Found scalar input " << args[i].name << std::endl;
                        do_indent();
                        testbench_stream << print_name(args[i].name) << " = 0" << "\n";
                        // testbench_stream << print_name(args[i].name) << " = solver.create_term(0, solver.create_bvsort(" << args[i].stencil_type.elemType.bits() << "))\n";
                    }
                }

                // Need to copy this from H2H clockwork implementation
                // I guess we only allow one output thats called hw_output
                // No idea why this is fixed
                int output_buffer_idx = args.size() - 1;
                // testbench_stream << print_name(args[output_buffer_idx].name) << " = ";
                auto box = box_provided(op->body, args[output_buffer_idx].name);
                args[output_buffer_idx].box = box;
                auto extents = extract_extents(box);

                // Declare array
                do_indent();
                testbench_stream << print_name(args[output_buffer_idx].name) << " = ";
                for (size_t i = 0; i < extents.size(); i++) {
                    testbench_stream << "[";
                }
                testbench_stream << "0";
                for (size_t i = 0; i < extents.size(); i++) {
                    testbench_stream << " for _ in range(";
                    testbench_stream << extents[extents.size() - i - 1];
                    testbench_stream << ")]";
                }
                testbench_stream << "\n";

                // Declare symbol array
                for (size_t j = 0; j < extents.size(); j++) {
                    do_indent();
                    testbench_stream << "for " << print_name(args[output_buffer_idx].name) << "_dim_" << j << " in range(" << extents[j] << "):\n";
                    indent += 1;
                }

                // Fill array with symbols
                do_indent();
                testbench_stream << print_name(args[output_buffer_idx].name) << "[";
                for (size_t j = 0; j < extents.size(); j++) {
                    testbench_stream << print_name(args[output_buffer_idx].name) << "_dim_" << j;
                    if (j < extents.size() - 1) {
                        testbench_stream << "][";
                    }
                }
                testbench_stream << "] = solver.create_fts_state_var(\"" << print_name(args[output_buffer_idx].name) << "_\" + \"_\".join([str(";
                for (size_t j = 0; j < extents.size(); j++) {
                    testbench_stream << print_name(args[output_buffer_idx].name) << "_dim_" << j;
                    if (j < extents.size() - 1) {
                        testbench_stream << "), str(";
                    }
                }
                testbench_stream << ")]), solver.create_bvsort(" << args[output_buffer_idx].stencil_type.elemType.bits() << "))\n";
                indent -= extents.size();
                testbench_stream << "\n";

                // Call run_app and return input output symbols
                do_indent();
                testbench_stream << "run_app(solver, ";
                for (size_t i = 0; i < args.size(); i++) {
                    testbench_stream << print_name(args[i].name);

                    if (i < args.size() - 1) {
                        testbench_stream << ", ";
                    }
                }
                testbench_stream << ")\n";

                do_indent();
                testbench_stream << "return ";
                for (size_t i = 0; i < args.size(); i++) {
                    testbench_stream << print_name(args[i].name);

                    if (i < args.size() - 1) {
                        testbench_stream << ", ";
                    }
                }
                testbench_stream << "\n";
                


                // main() testbench for comparing input/output pixels
                indent = 0;
                testbench_stream << "\ndef main():\n";
                indent += 1;


                // Create solver and call create_app
                do_indent();
                testbench_stream << "solver = Solver()\n";
                do_indent();
                
                for (size_t i = 0; i < args.size(); i++) {
                    testbench_stream << print_name(args[i].name);

                    if (i < args.size() - 1) {
                        testbench_stream << ", ";
                    }
                }
                testbench_stream << " = create_app(solver)\n";

                // read input image(s)
                for (size_t i = 0; i < args.size() - 1; i++) {
                    if (args[i].is_stencil) {

                        string provide_name = associated_provide_name(op->body, args[i].name);
                        auto box = box_provided(op->body, provide_name);
                        args[i].box = box;
                        auto extents = extract_extents(box);

                        // Reading in file to compare pixels values
                        do_indent();
                        testbench_stream << print_name(args[i].name) << "_compare = np.fromfile(";

                        testbench_stream << "\"bin/" << print_name(args[i].name) << ".leraw\", dtype=np." << args[i].stencil_type.elemType << ").reshape((";

                        std::cout << print_name(args[i].name);
                        
                        for (size_t j = 0; j < extents.size(); j++) {
                            std::cout << " extent:" << extents[extents.size() - 1 - j];

                            testbench_stream << extents[extents.size() - 1 - j];

                            if (j < extents.size() - 1) {
                                testbench_stream << ", ";
                            }
                        }
                        std::cout << std::endl;
                        testbench_stream << "))\n";

                        // Is transposed, so need to transpose back
                        do_indent();
                        testbench_stream << print_name(args[i].name) << "_compare = np.transpose(" << print_name(args[i].name) << "_compare, (";
                        
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << extents.size() - 1 - j;
                            if (j < extents.size() - 1) {
                                testbench_stream << ", ";
                            }
                        }
                        testbench_stream << "))\n";

                        do_indent();

                        testbench_stream << print_name(args[i].name) << "_compare = " << print_name(args[i].name) << "_compare.tolist()\n";



                        // Constrain fts state var to be equal to input image
                        for (size_t j = 0; j < extents.size(); j++) {
                            do_indent();
                            testbench_stream << "for " << print_name(args[i].name) << "_dim_" << j << " in range(" << extents[j] << "):\n";
                            indent += 1;
                        }

                        do_indent();
                        testbench_stream << "solver.fts.constrain_init(solver.create_term(solver.ops.Equal, ";
                        testbench_stream << print_name(args[i].name) << "[";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "][";
                            }
                        }
                        
                        testbench_stream << "], solver.create_term(";
                        testbench_stream << print_name(args[i].name) << "_compare[";
                        for (size_t j = 0; j < extents.size(); j++) {
                            testbench_stream << print_name(args[i].name) << "_dim_" << j;
                            if (j < extents.size() - 1) {
                                testbench_stream << "][";
                            }
                        }    
                        testbench_stream << "], solver.create_bvsort(" << args[i].stencil_type.elemType.bits() << "))))\n";

                        
                        indent -= extents.size();
                    } 
                }

                // read output image for comparison
                do_indent();
                testbench_stream << print_name(args[output_buffer_idx].name) << "_compare = np.fromfile(";
                testbench_stream << "\"bin/hw_output.leraw\", dtype=np." << args[output_buffer_idx].stencil_type.elemType << ").reshape((";
                for (size_t j = 0; j < extents.size(); j++) {
                    testbench_stream << extents[extents.size() - 1 - j];
                    if (j < extents.size() - 1) {
                        testbench_stream << ", ";
                    }
                }
                testbench_stream << "))\n";

                do_indent();
                testbench_stream << print_name(args[output_buffer_idx].name) << "_compare = np.transpose(" << print_name(args[output_buffer_idx].name) << "_compare, (";
                
                for (size_t j = 0; j < extents.size(); j++) {
                    testbench_stream << extents.size() - 1 - j;
                    if (j < extents.size() - 1) {
                        testbench_stream << ", ";
                    }
                }
                testbench_stream << "))\n";

                do_indent();
                testbench_stream << print_name(args[output_buffer_idx].name) << "_compare = " << print_name(args[output_buffer_idx].name) << "_compare.tolist()\n";


                // create property term
                do_indent();
                testbench_stream << "property_term = solver.create_term(solver.ops.Equal, solver.create_term(0, solver.create_bvsort(16)), solver.create_term(0, solver.create_bvsort(16)))\n";
                
                for (size_t j = 0; j < extents.size(); j++) {
                    do_indent();
                    testbench_stream << "for " << print_name(args[output_buffer_idx].name) << "_dim_" << j << " in range(" << extents[j] << "):\n";
                    indent += 1;
                }
                
                do_indent();
                testbench_stream << "property_term = solver.create_term(solver.ops.And, property_term, solver.create_term(solver.ops.Equal, ";

                testbench_stream << print_name(args[output_buffer_idx].name) << "[";
                for (size_t j = 0; j < extents.size(); j++) {
                    testbench_stream << print_name(args[output_buffer_idx].name) << "_dim_" << j;
                    if (j < extents.size() - 1) {
                        testbench_stream << "][";
                    }
                }
                testbench_stream << "], solver.create_term(";
                testbench_stream << print_name(args[output_buffer_idx].name) << "_compare[";
                for (size_t j = 0; j < extents.size(); j++) {
                    testbench_stream << print_name(args[output_buffer_idx].name) << "_dim_" << j;
                    if (j < extents.size() - 1) {
                        testbench_stream << "][";
                    }
                }
                testbench_stream << "], solver.create_bvsort(" << args[output_buffer_idx].stencil_type.elemType.bits() << "))))\n";

                indent -= extents.size();

                testbench_stream << "\n";

                // solve
                do_indent();
                testbench_stream << "prop = pono.Property(solver.solver, property_term)" << "\n";
                do_indent();
                testbench_stream << "btor_solver = Solver(solver_name = \"bitwuzla\")\n";
                do_indent();
                testbench_stream << "bmc = pono.Bmc(prop, solver.fts, btor_solver.solver)" << "\n";
                do_indent();
                testbench_stream << "res = bmc.check_until(0)" << "\n";
                do_indent();
                testbench_stream << "if res == None or res:" << "\n";
                indent += 1;
                do_indent();
                testbench_stream << "print(\"Formal check passed\")" << "\n";
                indent -= 1;
                do_indent();
                testbench_stream << "else:" << "\n";
                indent += 1;
                do_indent();
                testbench_stream << "print(\"Formal check failed\")" << "\n";
                indent -= 1;


                indent = 0;

                testbench_stream << "\nif __name__==\"__main__\":\n";
                testbench_stream << "    main()\n";

            } else {
                // std::cout << "printing producer? " << op -> name << std::endl;
                // codegen_testbench.print(op);
                IRVisitor::visit(op);
            }
        }

        CodeGen_Pono::CodeGen_Pono(ostream & s, Target t):
            IRPrinter(s), id("$$ BAD ID $$"), target(t), indent(2), python_type(false)  {

            }

        CodeGen_Pono::~CodeGen_Pono() {

        }

        void CodeGen_Pono::print(Expr ir) {
            ir.accept(this);
        }

        void CodeGen_Pono::print(Stmt ir) {
            ir.accept(this);
        }

        void CodeGen_Pono::print_arg_list(const std::vector < Expr > & exprs) {
            for (size_t i = 0; i < exprs.size(); i++) {
                print(exprs[i]);
                if (i < exprs.size() - 1) {
                    stream << "][";
                }
            }
        }

        void CodeGen_Pono::print_list(const std::vector < Expr > & exprs) {
            for (size_t i = 0; i < exprs.size(); i++) {
                print(exprs[i]);
                if (i < exprs.size() - 1) {
                    stream << ", ";
                }
            }
        }

        void CodeGen_Pono::do_indent() {
            for (int i = 0; i < indent; i++) {
                stream << "  ";
            }
        }

        void CodeGen_Pono::visit(const IntImm * op) {
            stream << print_type(op->type, std::to_string(op->value));
        }

        void CodeGen_Pono::visit(const UIntImm * op) {
            stream << print_type(op->type, std::to_string(op->value));
        }

        void CodeGen_Pono::visit(const FloatImm * op) {
            stream << print_type(op->type, std::to_string(op->value));
        }

        void CodeGen_Pono::visit(const StringImm * op) {
            stream << '"';
            for (size_t i = 0; i < op -> value.size(); i++) {
                unsigned char c = op -> value[i];
                if (c >= ' ' && c <= '~' && c != '\\' && c != '"') {
                    stream << c;
                } else {
                    stream << '\\';
                    switch (c) {
                    case '"':
                        stream << '"';
                        break;
                    case '\\':
                        stream << '\\';
                        break;
                    case '\t':
                        stream << 't';
                        break;
                    case '\r':
                        stream << 'r';
                        break;
                    case '\n':
                        stream << 'n';
                        break;
                    default:
                        string hex_digits = "0123456789ABCDEF";
                        stream << 'x' << hex_digits[c >> 4] << hex_digits[c & 0xf];
                    }
                }
            }
            stream << '"';
        }

        void CodeGen_Pono::visit(const Cast * op) {
            // If casting to higher bitwidth, we need to zero extend
            // If casting to lower bitwidth, we need to truncate
            
            if (python_type) {
                stream << print_type(op->type, "(");
                print(op->value);
                stream << ")";
            } else {
                if (op->type.bits() > op->value.type().bits()) {
                    if (op->type.is_int()) {
                        stream << "solver.create_term(ss.Op(solver.ops.Sign_Extend, " << (op->type.bits() - op->value.type().bits()) << "), ";
                    } else {
                        stream << "solver.create_term(ss.Op(solver.ops.Zero_Extend, " << (op->type.bits() - op->value.type().bits()) << "), ";
                    }
                    print(op->value);
                    stream << ")";
                } else if (op->type.bits() < op->value.type().bits()) {
                    stream << "solver.create_term(ss.Op(solver.ops.Extract, " << op->type.bits()-1 << ", 0), ";
                    print(op->value);
                    stream << ")";
                } else {
                    print(op->value);
                }
            }
        }

        string CodeGen_Pono::print_name(const string & name) {
            ostringstream oss;

            for (size_t i = 0; i < name.size(); i++) {
                if (name[i] == '.') {
                    oss << '_';
                } else if (name[i] == '$') {
                    oss << "__";
                } else oss << name[i];
            }
            return oss.str();
        }

        string CodeGen_Pono::print_type(const Halide::Type &t, const std::string &val) {

            if (python_type) {
                if (t.is_int()) {
                    return "np.int" + std::to_string(t.bits()) + "(" + val + ")";
                } else if (t.is_uint()) {
                    return "np.int" + std::to_string(t.bits()) + "(" + val + ")";
                } else if (t.is_float()) {
                    return "float(" + val + ")";
                } else if (t.is_bfloat()) {
                    return "float(" + val + ")";
                } else if (t.is_handle()) {
                    return "";
                } else {
                    return "";
                }
            } else {
                if (t.is_int()) {
                    return "solver.create_term(" + val + ", solver.create_bvsort(" + std::to_string(t.bits()) + "))";
                } else if (t.is_uint()) {
                    return "solver.create_term(" + val + ", solver.create_bvsort(" + std::to_string(t.bits()) + "))";
                } else if (t.is_float()) {
                    return "print_pono_type needs update";
                } else if (t.is_bfloat()) {
                    return "print_pono_type needs update";
                } else if (t.is_handle()) {
                    return "";
                } else {
                    return "";
                }
            }

        }

        string CodeGen_Pono::print_type(const Halide::Type &t) {

            if (python_type) {
                if (t.is_int()) {
                    return "np.int" + std::to_string(t.bits());
                } else if (t.is_uint()) {
                    return "np.int" + std::to_string(t.bits());
                } else if (t.is_float()) {
                    return "float";
                } else if (t.is_bfloat()) {
                    return "float";
                } else if (t.is_handle()) {
                    return "";
                } else {
                    return "";
                }
            } else {
                return "";
            }
        }


        void CodeGen_Pono::visit(const Variable * op) {
            if (python_type) {
                stream << print_name(op -> name);
            } else {
                // Should be a loop variable used in body of loop
                stream << "solver.create_term(" + print_name(op -> name) + ", solver.create_bvsort(" + std::to_string(op->type.bits()) + "))";
            }
        }

        void CodeGen_Pono::visit(const Mul * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << "*";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.BVMul, ";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";
            }
        }

        void CodeGen_Pono::visit(const Add * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " + ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.BVAdd, ";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";    
            }
        }

        void CodeGen_Pono::visit(const Sub * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " - ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.BVSub, ";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";    
            }
        }

        void CodeGen_Pono::visit(const Div * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << "//";
                print(op -> b);
                stream << ')';
            } else {
                int shift_amt;
                if (is_const_power_of_two_integer(op->b, &shift_amt)) {
                    if (op->type.is_int()) {
                        stream << "solver.create_term(solver.ops.BVAshr, ";
                    } else {
                        stream << "solver.create_term(solver.ops.BVLshr, ";
                    }
                    print(op -> a);
                    stream << ", ";
                    stream << "solver.create_term(" << shift_amt << ", solver.create_bvsort(" << op->type.bits() << "))";
                    stream << ")";    
                } else {
                    stream << "solver.create_term(solver.ops.BVSdiv, ";
                    print(op -> a);
                    stream << ", ";
                    print(op -> b);
                    stream << ")";  
                }  
            }
        }

        void CodeGen_Pono::visit(const Mod * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " % ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.BVSmod, ";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";    
            }
        }

        void CodeGen_Pono::visit(const Min * op) {
            if (python_type) {
                stream << "min(";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";                    
                    stream << "solver.create_term(solver.ops.BVSlt, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", ";
                    print(op -> a);
                    stream << ", ";
                    print(op -> b);
                    stream << ")";
            }
        }

        void CodeGen_Pono::visit(const Max * op) {
            if (python_type) {
                stream << "max(";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.BVSgt, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", ";
                    print(op -> a);
                    stream << ", ";
                    print(op -> b);
                    stream << ")"; 
            }
        }

        void CodeGen_Pono::visit(const EQ * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " == ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.Equal, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", solver.create_term(1, solver.create_bvsort(1)), solver.create_term(0, solver.create_bvsort(1)))";
            }
        }

        void CodeGen_Pono::visit(const NE * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " != ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.Not, solver.create_term(solver.ops.Equal, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << "))";    
                    stream << ", solver.create_term(1, solver.create_bvsort(1)), solver.create_term(0, solver.create_bvsort(1)))";
            }
        }

        void CodeGen_Pono::visit(const LT * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " < ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.BVSlt, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", solver.create_term(1, solver.create_bvsort(1)), solver.create_term(0, solver.create_bvsort(1)))";
            }
        }

        void CodeGen_Pono::visit(const LE * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " <= ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.BVSle, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", solver.create_term(1, solver.create_bvsort(1)), solver.create_term(0, solver.create_bvsort(1)))";
            }
        }

        void CodeGen_Pono::visit(const GT * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " > ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.BVSgt, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", solver.create_term(1, solver.create_bvsort(1)), solver.create_term(0, solver.create_bvsort(1)))";
            }
        }

        void CodeGen_Pono::visit(const GE * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " >= ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.BVSge, ";
                        print(op -> a);
                        stream << ", ";
                        print(op -> b);
                        stream << ")";    
                    stream << ", solver.create_term(1, solver.create_bvsort(1)), solver.create_term(0, solver.create_bvsort(1)))";
            }
        }

        void CodeGen_Pono::visit(const And * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " and ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.BVAnd, ";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";    
            }
        }

        void CodeGen_Pono::visit(const Or * op) {
            if (python_type) {
                stream << '(';
                print(op -> a);
                stream << " or ";
                print(op -> b);
                stream << ')';
            } else {
                stream << "solver.create_term(solver.ops.BVOr, ";
                print(op -> a);
                stream << ", ";
                print(op -> b);
                stream << ")";    
            }
        }

        void CodeGen_Pono::visit(const Not * op) {
            if (python_type) {
                stream << '!';
                print(op -> a);
            } else {
                stream << "solver.create_term(solver.ops.BVNot, ";
                print(op -> a);
                stream << ")";
            }
        }

        void CodeGen_Pono::visit(const Select * op) {
            if (python_type) {
                stream << "(";
                print(op -> true_value);
                stream << " if ";
                print(op -> condition);
                stream << " else ";
                print(op -> false_value);
                stream << ")";
            } else {
                stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.Equal, ";
                        print(op -> condition);
                        stream << ", solver.create_term(1, solver.create_bvsort(1)))";
                    stream << ", ";
                    print(op -> true_value);
                    stream << ", ";
                    print(op -> false_value);
                    stream << ")";
            }
        }

        void CodeGen_Pono::visit(const Ramp * op) {
            stream << "ramp(";
            print(op -> base);
            stream << ", ";
            print(op -> stride);
            stream << ", " << op -> lanes << ")";
        }

        void CodeGen_Pono::visit(const Broadcast * op) {
            stream << "x" << op -> lanes << "(";
            print(op -> value);
            stream << ")";
        }

        void CodeGen_Pono::visit(const Call * op) {

            // add bitand, bitor, bitxor, bitnot
            if (op->is_intrinsic(Call::bitwise_and)) {
                internal_assert(op->args.size() == 2);
                if (python_type) {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << '(';
                    print(a);
                    stream << " & ";
                    print(b);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << "solver.create_term(solver.ops.BVAnd, ";
                    print(a);
                    stream << ", ";
                    print(b);
                    stream << ")";    
                }
            } else if (op->is_intrinsic(Call::bitwise_or)) {
                internal_assert(op->args.size() == 2);
                if (python_type) {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << '(';
                    print(a);
                    stream << " | ";
                    print(b);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << "solver.create_term(solver.ops.BVOr, ";
                    print(a);
                    stream << ", ";
                    print(b);
                    stream << ")";    
                }
            } else if (op->is_intrinsic(Call::bitwise_xor)) {
                internal_assert(op->args.size() == 2);
                if (python_type) {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << '(';
                    print(a);
                    stream << " ^ ";
                    print(b);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << "solver.create_term(solver.ops.BVXor, ";
                    print(a);
                    stream << ", ";
                    print(b);
                    stream << ")";    
                }
            } else if (op->is_intrinsic(Call::bitwise_not)) {
                internal_assert(op->args.size() == 1);
                if (python_type) {
                    Expr a = op->args[0];
                    stream << "(1 << " << op->type.bits() << ") - 1 - ";
                    print(a);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    stream << "solver.create_term(solver.ops.BVNot, ";
                    print(a);
                    stream << ")";    
                }
            } else if (op->is_intrinsic(Call::shift_left)) {
                internal_assert(op->args.size() == 2);
                if (python_type) {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << '(';
                    print(a);
                    stream << " << ";
                    print(b);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << "solver.create_term(solver.ops.BVShl, ";
                    print(a);
                    stream << ", ";
                    print(b);
                    stream << ")";    
                }
            } else if (op->is_intrinsic(Call::shift_right)) {
                internal_assert(op->args.size() == 2);
                if (python_type) {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << '(';
                    print(a);
                    stream << " >> ";
                    print(b);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    // Might be the wrong type of right shift?
                    stream << "solver.create_term(solver.ops.BVAshr, ";
                    print(a);
                    stream << ", ";
                    print(b);
                    stream << ")";    
                }
            } else if (op->is_intrinsic(Call::abs)) {
                internal_assert(op->args.size() == 1);
                if (python_type) {
                    Expr a = op->args[0];
                    stream << "abs(";
                    print(a);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    stream << "solver.create_term(solver.ops.Ite, ";
                    stream << "solver.create_term(solver.ops.BVSlt, ";
                    print(a);
                    stream << ", solver.create_term(0, solver.create_bvsort(" << op->type.bits() << "))), ";
                    stream << "solver.create_term(solver.ops.BVSub, ";
                    stream << "solver.create_term(0, solver.create_bvsort(" << op->type.bits() << ")), ";
                    print(a);
                    stream << "), ";
                    print(a);
                    stream << ")";
                }
            } else if (op->is_intrinsic(Call::absd)) {
                internal_assert(op->args.size() == 2);
                if (python_type) {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << "abs(";
                    print(a);
                    stream << " - ";
                    print(b);
                    stream << ')';
                } else {
                    Expr a = op->args[0];
                    Expr b = op->args[1];
                    stream << "solver.create_term(solver.ops.Ite, ";
                        stream << "solver.create_term(solver.ops.BVSgt, ";
                            print(a);
                            stream << ", ";
                            print(b);
                            stream << ")";    
                        stream << ", ";
                        stream << "solver.create_term(solver.ops.BVSub, ";
                            print(a);
                            stream << ", ";
                            print(b);
                            stream << ")";
                        stream << ", ";
                        stream << "solver.create_term(solver.ops.BVSub, ";
                            print(b);
                            stream << ", ";
                            print(a);
                            stream << ")";
                        stream << ")";
                }
            } else if (op->is_intrinsic(Call::reinterpret)) {
                stream << print_name(op -> name) << "(";
                print_list(op -> args);
                stream << ")";
            } else {
                stream << print_name(op -> name) << "[";
                bool old_python_type = python_type;
                python_type = true;
                print_arg_list(op -> args);
                python_type = old_python_type;
                stream << "]";
            }
        }

        void CodeGen_Pono::visit(const Let * op) {
            print(op -> body);
        }

        void CodeGen_Pono::visit(const LetStmt * op) {
            ScopedBinding < Expr > bind(scope, op -> name, simplify(expand_expr(op -> value, scope)));
            print(op -> body);
        }

        void CodeGen_Pono::visit(const AssertStmt * op) {

        }

        void CodeGen_Pono::visit(const For * op) {

            do_indent();
            python_type = true;
            stream << op -> for_type << op -> device_api << " " << print_name(op -> name) << " in range(";
            print(op -> min);
            stream << ", ";
            print(op -> min);
            stream << "+";
            print(op -> extent);
            stream << "):\n";
            python_type = false;
            indent += 2;
            print(op -> body);
            indent -= 2;

        }

        void CodeGen_Pono::visit(const Acquire * op) {
            stream << "Acquire\n";
            print(op -> body);
        }

        void CodeGen_Pono::visit(const Provide * op) {
            do_indent();
            stream << print_name(op -> name) << "[";
            bool old_python_type = python_type;
            python_type = true;
            print_arg_list(op -> args);
            python_type = old_python_type;
            stream << "] = ";
            if (op -> values.size() > 1) {
                stream << "(";
            }
            print_list(op -> values);
            if (op -> values.size() > 1) {
                stream << ")";
            }

            stream << '\n';
        }

        void CodeGen_Pono::visit(const Load * op) {
            
            stream << "solver.create_term(solver.ops.Select, ";
            stream << print_name(op -> name) << ", ";
            print(op -> index);
            stream << ")";
            
        }

        void CodeGen_Pono::visit(const Store * op) {
            do_indent();
            // ROMs should use theory of arrays

            stream << print_name(op -> name) << " = solver.create_term(solver.ops.Store, ";
            stream << print_name(op -> name) << ", ";
            print(op -> index);
            stream << ", ";
            print(op -> value);
            stream << ")";
            stream << '\n';

        }

        void CodeGen_Pono::visit(const Allocate * op) {
            do_indent();

            // This might happen in a For loop, and we can't use the same FTS state var name each iteration
            // Need to make the pono names unique, doesn't matter what they are
            
            stream << print_name(op -> name) << " = solver.create_fts_state_var(\"" << print_name(op -> name) << "\" + str(num_roms), solver.solver.make_sort(ss.sortkinds.ARRAY, solver.create_bvsort(32), solver.create_bvsort(16)))";
            stream << "\n";
            do_indent();
            stream << "num_roms += 1\n";
            print(op -> body);
        }

        void CodeGen_Pono::visit(const Free * op) {

        }

        void CodeGen_Pono::visit(const Realize * op) {

            //  bool has_variable_min = false;
            vector < Expr > realize_mins;
            for (size_t i = 0; i < op -> bounds.size(); i++) {
                realize_mins.emplace_back(op -> bounds[i].min);
                // if (!is_const(simplify(op->bounds[i].min))) {
                //   has_variable_min = true;
                // }
            }
            //   //auto new_body = op->body;
            auto new_body = shift_realize_bounds(op -> body, op -> name, realize_mins, scope);
            // std::cout << "shifting " << op -> name << " by " << realize_mins << std::endl;

            //   if (has_variable_min) {
            //     internal_assert(realize_glb_indices.count(op->name) == 0);
            //     realize_glb_indices[op->name] = realize_mins;
            //   }

            //   //auto new_realize = Realize::make(op->name, op->types, op->memory_type, op->bounds, op->condition, new_body);

            //   new_body.accept(this);

            do_indent();
            // stream << "realize ";
            stream << print_name(op -> name) << " = ";

            for (size_t i = 0; i < op -> bounds.size(); i++) {
                stream << "[";
            }

            stream << "0";

            python_type = true;
            for (size_t i = 0; i < op -> bounds.size(); i++) {
                stream << " for _ in range(";
                print(op -> bounds[op -> bounds.size() - i - 1].extent);
                stream << ")]";
            }
            python_type = false;
            stream << "\n";

            print(new_body);
        }

        void CodeGen_Pono::visit(const Prefetch * op) {
            do_indent();
            const bool has_cond = !is_one(op -> condition);
            if (has_cond) {
                stream << "if (" << op -> condition << ") {\n";
                indent += 2;

                do_indent();
            }
            stream << "prefetch " << print_name(op -> name) << "(";
            for (size_t i = 0; i < op -> bounds.size(); i++) {
                stream << "[";
                print(op -> bounds[i].min);
                stream << ", ";
                print(op -> bounds[i].extent);
                stream << "]";
                if (i < op -> bounds.size() - 1) {
                    stream << ", ";
                }
            }
            stream << ")\n";
            if (has_cond) {
                indent -= 2;
                do_indent();
                stream << "}\n";
            }
            print(op -> body);
        }

        void CodeGen_Pono::visit(const Block * op) {
            print(op -> first);
            print(op -> rest);
        }

        void CodeGen_Pono::visit(const Fork * op) {
            vector < Stmt > stmts;
            stmts.push_back(op -> first);
            Stmt rest = op -> rest;
            while (const Fork * f = rest.as < Fork > ()) {
                stmts.push_back(f -> first);
                rest = f -> rest;
            }
            stmts.push_back(rest);

            do_indent();
            stream << "fork ";
            for (Stmt s: stmts) {
                stream << "{\n";
                indent += 2;

                print(s);

                stream << "} ";
                indent -= 2;
                do_indent();
            }
            stream << "\n";
        }

        void CodeGen_Pono::visit(const IfThenElse * op) {
            do_indent();
            while (1) {
                stream << "if (" << op -> condition << ") {\n";
                indent += 2;

                print(op -> then_case);
                indent -= 2;

                if (!op -> else_case.defined()) {
                    break;
                }

                if (const IfThenElse * nested_if = op -> else_case.as < IfThenElse > ()) {
                    do_indent();
                    stream << "} else ";
                    op = nested_if;
                } else {
                    do_indent();
                    stream << "} else {\n";
                    indent += 2;

                    print(op -> else_case);
                    indent -= 2;
                    break;
                }
            }

            do_indent();
            stream << "}\n";

        }

        void CodeGen_Pono::visit(const Evaluate * op) {
            do_indent();
            print(op -> value);
            stream << "\n";
        }

        void CodeGen_Pono::visit(const Shuffle * op) {
            if (op -> is_concat()) {
                stream << "concat_vectors(";
                print_list(op -> vectors);
                stream << ")";
            } else if (op -> is_interleave()) {
                stream << "interleave_vectors(";
                print_list(op -> vectors);
                stream << ")";
            } else if (op -> is_extract_element()) {
                stream << "extract_element(";
                print_list(op -> vectors);
                stream << ", " << op -> indices[0];
                stream << ")";
            } else if (op -> is_slice()) {
                stream << "slice_vectors(";
                print_list(op -> vectors);
                stream << ", " << op -> slice_begin() << ", " << op -> slice_stride() << ", " << op -> indices.size();
                stream << ")";
            } else {
                stream << "shuffle(";
                print_list(op -> vectors);
                stream << ", ";
                for (size_t i = 0; i < op -> indices.size(); i++) {
                    print(op -> indices[i]);
                    if (i < op -> indices.size() - 1) {
                        stream << ", ";
                    }
                }
                stream << ")";
            }
        }

        void CodeGen_Pono::visit(const ProducerConsumer * op) {
            // string target_prefix = "_hls_target";
            // if (starts_with(op->name, target_prefix) && op->is_producer) {
                // std::cout << "produceconsume " << op->name << std::endl;
                // if (op -> is_producer) {
                    Stmt hw_body = substitute_in_all_letstmts(op -> body);
                    print(hw_body);
                // } else {
                //     stream << "consume " << op->name << "{" << std::endl;
                //     print(op -> body);
                // }
            // }

        }

    }
} // namespace Internal