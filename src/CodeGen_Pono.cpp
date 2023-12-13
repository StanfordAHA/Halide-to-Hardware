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


CodeGen_Pono_Wrapper::CodeGen_Pono_Wrapper(ostream &s, Target t) : codegen(s, t) {

}

void CodeGen_Pono_Wrapper::compile(const Module &input) {
    for (const auto &f : input.functions()) {
        compile(f);
    }
}

void CodeGen_Pono_Wrapper::compile(const LoweredFunc &f) {
   f.body.accept(this);

}

void CodeGen_Pono_Wrapper::compile(const Buffer<> &buffer) {
    
}

void CodeGen_Pono_Wrapper::visit(const ProducerConsumer *op) {
    if (op->is_producer) {
        std::cout << "visiting producer " << op->name << std::endl;
        // codegen
    } else {
        std::cout << "visiting consumer " << op->name << std::endl;
        codegen.print(op);
    }
}





CodeGen_Pono::CodeGen_Pono(ostream &s, Target t) :
    IRPrinter(s), id("$$ BAD ID $$"), target(t), indent(0), producer(false) {

}

CodeGen_Pono::~CodeGen_Pono() {

}

void CodeGen_Pono::print(Expr ir) {
    ir.accept(this);
}

void CodeGen_Pono::print(Stmt ir) {
    ir.accept(this);
}


void CodeGen_Pono::print_arg_list(const std::vector<Expr> &exprs) {
    for (size_t i = 0; i < exprs.size(); i++) {
        print(exprs[i]);
        if (i < exprs.size() - 1) {
            stream << "][";
        }
    }
}

void CodeGen_Pono::print_list(const std::vector<Expr> &exprs) {
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

void CodeGen_Pono::visit(const IntImm *op) {
    // if (op->type == Int(32)) {
        stream << op->value;
    // } else {
        // stream << "(" << op->type << ")" << op->value;
    // }
}

void CodeGen_Pono::visit(const UIntImm *op) {
    // stream << "(" << op->type << ")" << op->value;
    stream << op->value;
}

void CodeGen_Pono::visit(const FloatImm *op) {
  switch (op->type.bits()) {
    case 64:
        stream << op->value;
        break;
    case 32:
        stream << op->value << 'f';
        break;
    case 16:
        stream << op->value << 'h';
        break;
    default:
        internal_error << "Bad bit-width for float: " << op->type << "\n";
    }
}

void CodeGen_Pono::visit(const StringImm *op) {
    stream << '"';
    for (size_t i = 0; i < op->value.size(); i++) {
        unsigned char c = op->value[i];
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

void CodeGen_Pono::visit(const Cast *op) {
    // stream << op->type << '(';
    print(op->value);
    // stream << ')';
}

string CodeGen_Pono::print_name(const string &name) {
    ostringstream oss;

    for (size_t i = 0; i < name.size(); i++) {
        if (name[i] == '.') {
            oss << '_';
        } else if (name[i] == '$') {
            oss << "__";
        }
        else oss << name[i];
    }
    return oss.str();
}

void CodeGen_Pono::visit(const Variable *op) {
    // omit the type
    // stream << print_name(op->name) << "." << op->type;

    stream << print_name(op->name);
}


void CodeGen_Pono::visit(const Add *op) {
    stream << '(';
    print(op->a);
    stream << " + ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Sub *op) {
    stream << '(';
    print(op->a);
    stream << " - ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Mul *op) {
    stream << '(';
    print(op->a);
    stream << "*";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Div *op) {
    stream << '(';
    print(op->a);
    stream << "/";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Mod *op) {
    stream << '(';
    print(op->a);
    stream << " % ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Min *op) {
    stream << "min(";
    print(op->a);
    stream << ", ";
    print(op->b);
    stream << ")";
}

void CodeGen_Pono::visit(const Max *op) {
    stream << "max(";
    print(op->a);
    stream << ", ";
    print(op->b);
    stream << ")";
}

void CodeGen_Pono::visit(const EQ *op) {
    stream << '(';
    print(op->a);
    stream << " == ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const NE *op) {
    stream << '(';
    print(op->a);
    stream << " != ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const LT *op) {
    stream << '(';
    print(op->a);
    stream << " < ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const LE *op) {
    stream << '(';
    print(op->a);
    stream << " <= ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const GT *op) {
    stream << '(';
    print(op->a);
    stream << " > ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const GE *op) {
    stream << '(';
    print(op->a);
    stream << " >= ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const And *op) {
    stream << '(';
    print(op->a);
    stream << " && ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Or *op) {
    stream << '(';
    print(op->a);
    stream << " || ";
    print(op->b);
    stream << ')';
}

void CodeGen_Pono::visit(const Not *op) {
    stream << '!';
    print(op->a);
}

void CodeGen_Pono::visit(const Select *op) {
    stream << "(";
    print(op->true_value);
    stream << " if ";
    print(op->condition);
    stream << " else ";
    print(op->false_value);
    stream << ")";
}

void CodeGen_Pono::visit(const Load *op) {
    const bool has_pred = !is_one(op->predicate);
    const bool show_alignment = op->type.is_vector() && op->alignment.modulus > 1;
    if (has_pred) {
        stream << "(";
    }
    stream << print_name(op->name) << "[";
    print(op->index);
    if (show_alignment) {
        stream << " aligned(" << op->alignment.modulus << ", " << op->alignment.remainder << ")";
    }
    stream << "]";
    if (has_pred) {
        stream << " if ";
        print(op->predicate);
        stream << ")";
    }
}

void CodeGen_Pono::visit(const Ramp *op) {
    stream << "ramp(";
    print(op->base);
    stream << ", ";
    print(op->stride);
    stream << ", " << op->lanes << ")";
}

void CodeGen_Pono::visit(const Broadcast *op) {
    stream << "x" << op->lanes << "(";
    print(op->value);
    stream << ")";
}

void CodeGen_Pono::visit(const Call *op) {
    // TODO: Print indication of C vs C++?
    if (op->is_intrinsic(Call::reinterpret) ||
        op->is_intrinsic(Call::make_struct)) {
        // For calls that define a type that isn't just a function of
        // the types of the args, we also print the type.
        stream << print_name(op->name) << "(";
        print_list(op->args);
        stream << ")";
    } else {
        stream << print_name(op->name) << "[";
        print_arg_list(op->args);
        stream << "]";
    }
}

void CodeGen_Pono::visit(const Let *op) {
//     stream << "(let " << print_name(op->name) << " = ";
//     print(op->value);
//     stream << " in ";
    // stream << "Let\n";
    print(op->body);
//     stream << ")";
}

void CodeGen_Pono::visit(const LetStmt *op) {
//     do_indent();
//     stream << "let " << op->name << " = ";
//     print(op->value);
//     stream << '\n';
    // stream << "LetStmt\n";
    ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
    print(op->body);
}

void CodeGen_Pono::visit(const AssertStmt *op) {
//     do_indent();
//     stream << "assert(";
//     print(op->condition);
//     stream << ", ";
//     print(op->message);
//     stream << ")\n";
}



void CodeGen_Pono::visit(const For *op) {

    do_indent();
    stream << op->for_type << op->device_api << " " << print_name(op->name) << " in range(";
    print(op->min);
    stream << ", ";
    print(op->min);
    stream << "+";
    print(op->extent);
    stream << "):\n";

    indent += 2;
    print(op->body);
    indent -= 2;

    // do_indent();
    // stream << "}\n";
}

void CodeGen_Pono::visit(const Acquire *op) {
//     do_indent();
//     stream << "acquire (";
//     print(op->semaphore);
//     stream << ", ";
//     print(op->count);
//     stream << ") {\n";
//     indent += 2;
stream << "Acquire\n";
    print(op->body);
//     indent -= 2;
//     do_indent();
//     stream << "}\n";
}

void CodeGen_Pono::visit(const Store *op) {
    do_indent();
    const bool has_pred = !is_one(op->predicate);
    const bool show_alignment = op->value.type().is_vector() && (op->alignment.modulus > 1);
    if (has_pred) {
        stream << "predicate (" << op->predicate << ")\n"; indent += 2;
        do_indent();
    }
    stream << print_name(op->name) << "[";
    print(op->index);
    if (show_alignment) {
        stream << " aligned("
               << op->alignment.modulus
               << ", "
               << op->alignment.remainder << ")";
    }
    stream << "] = ";
    print(op->value);
    stream << '\n';
    if (has_pred) {
        indent -= 2;
    }
}

void CodeGen_Pono::visit(const Provide *op) {
    do_indent();
    stream << print_name(op->name) << "[";
    print_arg_list(op->args);
    stream << "] = ";
    if (op->values.size() > 1) {
        stream << "(";
    }
    print_list(op->values);
    if (op->values.size() > 1) {
        stream << ")";
    }

    stream << '\n';
}

void CodeGen_Pono::visit(const Allocate *op) {
    do_indent();
    stream << print_name(op->name) << "[" << 0;
    for (size_t i = 0; i < op->extents.size(); i++) {
        stream  << " * ";
        print(op->extents[i]);
    }
    stream << "]";
    // if (op->memory_type != MemoryType::Auto) {
    //     stream << " in " << op->memory_type;
    // }
    if (!is_one(op->condition)) {
        stream << " if ";
        print(op->condition);
    }
    if (op->new_expr.defined()) {
        stream << "\n";
        do_indent();
        stream << " custom_new { " << op->new_expr << " }";
    }
    if (!op->free_function.empty()) {
        stream << "\n";
        do_indent();
        stream << " custom_delete { " << op->free_function << "(" << print_name(op->name) << "); }";
    }
    stream << "\n";
    print(op->body);
}

void CodeGen_Pono::visit(const Free *op) {
//     do_indent();
//     stream << "free " << print_name(op->name);
//     stream << '\n';
}

void CodeGen_Pono::visit(const Realize *op) {

//  bool has_variable_min = false;
  vector<Expr> realize_mins;
  for (size_t i = 0; i < op->bounds.size(); i++) {
    realize_mins.emplace_back(op->bounds[i].min);
    // if (!is_const(simplify(op->bounds[i].min))) {
    //   has_variable_min = true;
    // }
  }
//   //auto new_body = op->body;
  auto new_body = shift_realize_bounds(op->body, op->name, realize_mins, scope);
  std::cout << "shifting " << op->name << " by " << realize_mins << std::endl;

//   if (has_variable_min) {
//     internal_assert(realize_glb_indices.count(op->name) == 0);
//     realize_glb_indices[op->name] = realize_mins;
//   }


//   //auto new_realize = Realize::make(op->name, op->types, op->memory_type, op->bounds, op->condition, new_body);

//   new_body.accept(this);


    do_indent();
    // stream << "realize ";
    stream << print_name(op->name) << " = ";

    for (size_t i = 0; i < op->bounds.size(); i++) {
        stream << "[";
    }

     stream << "0";

    for (size_t i = 0; i < op->bounds.size(); i++) {
        stream << " for _ in range(";
        print(op->bounds[op->bounds.size() - i - 1].extent);
        stream << ")]";
    }
    stream << "\n";

    print(new_body);
}

void CodeGen_Pono::visit(const Prefetch *op) {
    do_indent();
    const bool has_cond = !is_one(op->condition);
    if (has_cond) {
        stream << "if (" << op->condition << ") {\n"; indent += 2;

        do_indent();
    }
    stream << "prefetch " << print_name(op->name) << "(";
    for (size_t i = 0; i < op->bounds.size(); i++) {
        stream << "[";
        print(op->bounds[i].min);
        stream << ", ";
        print(op->bounds[i].extent);
        stream << "]";
        if (i < op->bounds.size() - 1) { stream << ", ";}
    }
    stream << ")\n";
    if (has_cond) {
        indent -= 2;
        do_indent();
        stream << "}\n";
    }
    print(op->body);
}

void CodeGen_Pono::visit(const Block *op) {
    print(op->first);
    print(op->rest);
}

void CodeGen_Pono::visit(const Fork *op) {
    vector<Stmt> stmts;
    stmts.push_back(op->first);
    Stmt rest = op->rest;
    while (const Fork *f = rest.as<Fork>()) {
        stmts.push_back(f->first);
        rest = f->rest;
    }
    stmts.push_back(rest);

    do_indent();
    stream << "fork ";
    for (Stmt s : stmts) {
        stream << "{\n"; indent += 2;
        
        print(s);
        
        stream << "} "; indent -= 2;
        do_indent();
    }
    stream << "\n";
}

void CodeGen_Pono::visit(const IfThenElse *op) {
    do_indent();
    while (1) {
        stream << "if (" << op->condition << ") {\n"; indent += 2;
        
        print(op->then_case);
        indent -= 2;

        if (!op->else_case.defined()) {
            break;
        }

        if (const IfThenElse *nested_if = op->else_case.as<IfThenElse>()) {
            do_indent();
            stream << "} else ";
            op = nested_if;
        } else {
            do_indent();
            stream << "} else {\n"; indent += 2;
            
            print(op->else_case);
            indent -= 2;
            break;
        }
    }

    do_indent();
    stream << "}\n";

}

void CodeGen_Pono::visit(const Evaluate *op) {
    do_indent();
    print(op->value);
    stream << "\n";
}

void CodeGen_Pono::visit(const Shuffle *op) {
    if (op->is_concat()) {
        stream << "concat_vectors(";
        print_list(op->vectors);
        stream << ")";
    } else if (op->is_interleave()) {
        stream << "interleave_vectors(";
        print_list(op->vectors);
        stream << ")";
    } else if (op->is_extract_element()) {
        stream << "extract_element(";
        print_list(op->vectors);
        stream << ", " << op->indices[0];
        stream << ")";
    } else if (op->is_slice()) {
        stream << "slice_vectors(";
        print_list(op->vectors);
        stream << ", " << op->slice_begin() << ", " << op->slice_stride() << ", " << op->indices.size();
        stream << ")";
    } else {
        stream << "shuffle(";
        print_list(op->vectors);
        stream << ", ";
        for (size_t i = 0; i < op->indices.size(); i++) {
            print(op->indices[i]);
            if (i < op->indices.size() - 1) {
                stream << ", ";
            }
        }
        stream << ")";
    }
}


void CodeGen_Pono::visit(const ProducerConsumer *op) {
    if (op->name != "output") {
        do_indent();
        if (op->is_producer) {
            Stmt hw_body = substitute_in_all_letstmts(op->body);
            // indent += 2;
            print(hw_body);
            std::cout << hw_body << std::endl;
            // indent -= 2;
        } else {
            // stream << "consume " << op->name << " {\n";
            // indent += 2;
            print(op->body);
            // indent -= 2;
        }
        do_indent();
        // stream << "stop produce/consume " << op->name << "\n\n";
    }

}

}}  // namespace Internal
