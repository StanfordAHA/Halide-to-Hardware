#include "HWBufferSimplications.h"
#include "IREquality.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"

using std::map;
using std::string;
using std::vector;

namespace Halide {
namespace Internal {

class RealizeUsage : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const Call *op) {
    if (op->name == realize_name) {
      num_loads++;

      for (auto arg : op->args) {
        if (!is_const(arg)) {
          uses_variable_load_index = true;
        }
      }
    }
    IRVisitor::visit(op);
  }

  void visit(const Provide *op) {
    if (op->name == realize_name) {
      num_stores++;

      for (auto arg : op->args) {
        if (!is_const(arg)) {
          uses_variable_store_index = true;
        }
      }
      for (auto value : op->values) {
        if (!is_const(value)) {
          uses_variable_store_value = true;
        }
      }

    }
    IRVisitor::visit(op);
  }


 public:
  bool uses_variable_load_index;
  bool uses_variable_store_index;
  bool uses_variable_store_value;
  bool load_index_equals_store_index;
  uint num_loads;
  uint num_stores;
  string realize_name;

  RealizeUsage(string realizename) : uses_variable_load_index(false),
                                      uses_variable_store_index(false),
                                      uses_variable_store_value(false),
                                      load_index_equals_store_index(false),
                                      num_loads(0),
                                      num_stores(0),
                                      realize_name(realizename) {}
};

enum MemoryType {
  NO_REALIZATION,
  INOUT_REALIZATION,
  ROM_REALIZATION,
  REGS_REALIZATION,
  SRAM_REALIZATION,
  RMW_REALIZATION,
  UNKNOWN_REALIZATION
};

MemoryType identify_realization(Stmt s, string realizename) {
  RealizeUsage ru(realizename);
  s.accept(&ru);

  if (ru.num_stores == 0 || ru.num_loads == 0) {
    return INOUT_REALIZATION;

  } else if (!ru.uses_variable_load_index &&
             !ru.uses_variable_store_value) {
    //&& !ru.uses_variable_store_value) {
    return NO_REALIZATION;

  } else if (ru.uses_variable_load_index &&
             //!ru.uses_variable_store_index &&
             !ru.uses_variable_store_value) {
    return ROM_REALIZATION;

  } else if (ru.uses_variable_load_index &&
             ru.uses_variable_store_index &&
             ru.load_index_equals_store_index) {
    return RMW_REALIZATION;

  } else if (ru.uses_variable_load_index &&
             ru.uses_variable_store_index &&
             !ru.load_index_equals_store_index) {
    return SRAM_REALIZATION;

  } else {
    return UNKNOWN_REALIZATION;
  }

}

bool provide_matches_call(const Provide* provide, const Call *call) {
  bool match = true;

  if (call->args.size() != provide->args.size()) {
    return false;
  }
  
  for (size_t i=0; i<provide->args.size(); ++i) {
    match &= equal(simplify(provide->args[i]), simplify(call->args[i]));
  }
      
  match &= (call->name == provide->name);
  
  return match;
}

class InlineMemoryConstants : public IRMutator {
    using IRMutator::visit;

    map<string, vector<const Provide*> > realize_provides;

    Stmt visit(const Realize* realize) override {

      // check the stores and loads from the underlying memory
      auto mem_type = identify_realization(realize->body, realize->name);


      if (mem_type == ROM_REALIZATION) {
        //   add ROM name to remove Producer of this name, and save provides
        std::cout << "Realize " << realize->name << " is a rom" << std::endl;
        realize_provides[realize->name] = vector<const Provide*>();
        return mutate(realize->body);
        
      } else {
        //std::cout << "Realize " << realize->name << " is a " << mem_type << std::endl;
        return IRMutator::visit(realize);
      }      
    }


  Stmt visit(const ProducerConsumer* pc) override {
    if (realize_provides.count(pc->name)) {
        //std::cout << "looking at this pc: " << std::endl << Stmt(pc);
        if (pc->is_producer) {
          // in Produce: save provides, return empty stmt;
          // FIXME: only remove if all replacements work properly
          mutate(pc->body);
          return AssertStmt::make(Expr(true), Expr(1));

        } else {
          return mutate(pc->body);

          
          // in Consume: replace calls with provide->value
          Stmt body = pc->body;
          for (auto provide : realize_provides.at(pc->name)) {
            //Expr call_equiv = Call::make(provide->values[0].type, provide->name, provide->args, Call::Intrinsic);
            Expr call_equiv = Call::make(UInt(32), provide->name, provide->args, Call::Intrinsic);
            Expr replacement = provide->values[0];
            //std::cout << "trying to emulate " << Stmt(provide) << "  by replacing " << call_equiv << " with " << replacement << std::endl;
            //body = graph_substitute(call_equiv, replacement, body);
          }
          return body;
        }
        
      } else {
        return IRMutator::visit(pc);
      }
  }

  // in Provide: save for call replacement later
  Stmt visit(const Provide* provide) override {
    if (realize_provides.count(provide->name)) {
      realize_provides.at(provide->name).emplace_back(provide);
    }
    return IRMutator::visit(provide);
  }

  Expr visit(const Call* call) override {
    if (realize_provides.count(call->name)) {
      for (auto provide : realize_provides.at(call->name)) {
        if (provide_matches_call(provide, call)) {
          return provide->values[0];
        }
      }
        
      return IRMutator::visit(call);
    } else {
      return IRMutator::visit(call);
    }
  }

  
public:
    InlineMemoryConstants() { }
};



Stmt inline_memory_constants(Stmt s) {
    return InlineMemoryConstants().mutate(s);
}

}  // namespace Internal
}  // namespace Halide
