#include <iostream>
#include <limits>

#include "CodeGen_Internal.h"
#include "CodeGen_CoreHLS_Kernel.h"
#include "CodeGen_CoreIR_Testbench.h"
#include "Substitute.h"
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
using std::pair;
using std::map;


  void CodeGen_CoreHLS_Kernel::visit(const ProducerConsumer *op) {
    string target_prefix = "_hls_target.";
    if (starts_with(op->name, target_prefix)) {
      if (op->is_producer) {
        std::cout << "found a pc for generating coreir" << std::endl;
        Stmt hw_body = op->body;
        std::cout << hw_body << std::endl;

        debug(1) << "compute the closure for " << op->name << '\n';

        // get substring after target_prefix
        string output_name = op->name.substr(target_prefix.length()); 
        CoreIR_Closure c(hw_body, output_name);
        vector<CoreIR_Argument> args = c.arguments(stencils);

        // generate CoreIR target code using the child code generator
        string ip_name = unique_name("coreir_target");


        // emits the target function call
        
      } else { // is consumer
        IRGraphVisitor::visit(op);
      }
    } else {
      IRGraphVisitor::visit(op);
    }
  }


void CodeGen_CoreHLS_Kernel::visit(const Call *op) {
  IRGraphVisitor::visit(op);
}

void CodeGen_CoreHLS_Kernel::visit(const Realize *op) {
    if (ends_with(op->name, ".stream")) {
        // create a AXI stream type
        internal_assert(op->types.size() == 1);
        Stencil_Type stream_type({Stencil_Type::StencilContainerType::AxiStream,
                    op->types[0], op->bounds, 1});
        stencils.push(op->name, stream_type);

        // traverse down
        op->body.accept(this);

        stencils.pop(op->name);
    } else {
        IRGraphVisitor::visit(op);
    }
}

void CodeGen_CoreHLS_Kernel::visit(const Block *op) {
    IRGraphVisitor::visit(op);
    return;
}
}
}
