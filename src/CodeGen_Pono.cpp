#include <iostream>
#include <limits>

#include "CodeGen_Pono.h"
// #include "CodeGen_Internal.h"
// #include "Deinterleave.h"
// #include "IROperator.h"
// #include "Lerp.h"
// #include "Param.h"
// #include "Simplify.h"
// #include "Substitute.h"
// #include "Var.h"

namespace Halide {
namespace Internal {

using std::endl;
using std::map;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;


CodeGen_Pono::CodeGen_Pono(ostream &s, Target t) :
    IRPrinter(s), id("$$ BAD ID $$"), target(t) {

    stream << "Uhh" << "\n";

}

CodeGen_Pono::~CodeGen_Pono() {

    stream << "Uhh 2" << "\n";
}

void CodeGen_Pono::compile(const Module &input) {
    stream << "Uhh 3" << "\n";
}

void CodeGen_Pono::compile(const LoweredFunc &f) {
    stream << "Uhh 4" << "\n";
}

void CodeGen_Pono::compile(const Buffer<> &buffer) {
    stream << "Uhh 5" << "\n";
    
}

void CodeGen_Pono::visit(const Add *op) {
    stream << "Add!" << "\n";
}

void CodeGen_Pono::visit(const Mul *op) {
    stream << "mul!" << "\n";
}



}  // namespace Internal
}  // namespace Halide
