#include "HWBufferSimplifications.h"
#include "IREquality.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"

using std::string;
using std::map;
using std::set;
using std::vector;
using std::deque;

namespace Halide {
namespace Internal {

class CContainsCall : public IRVisitor {
    using IRVisitor::visit;
    void visit(const Call *op) override {
      // Found a call. Ignore any intrsincs (like abs)
      if ((match_any || calls.count(op->name) > 0) &&
          (op->is_intrinsic() && ends_with(op->name, ".stencil"))) {
        found_calls.emplace_back(op->name);
        //std::cout << "added call " << op->name << std::endl;
      }
      //std::cout << "call name is " << op->name << " intrinsic=" << op->is_intrinsic() << " match_any=" << match_any << " count=" << calls.count(op->name) << std::endl;
      IRVisitor::visit(op);
    }

    void visit(const Load *op) {
      if (match_any || calls.count(op->name) > 0) {
        found_calls.emplace_back(op->name);
      }
      //std::cout << "load name is " << op->name << std::endl;
      IRVisitor::visit(op);
    }

public:
  vector<string> found_calls;
  set<string> calls;
  bool match_any;

  CContainsCall(set<string> calls, bool match) : calls(calls), match_any(match) {}
};

vector<string> contains_call(Expr e, set<string> calls) {
    CContainsCall cc(calls, false);
    e.accept(&cc);
    return cc.found_calls;
}

vector<string> contains_call(Expr e) {
    CContainsCall cc({}, true);
    e.accept(&cc);
    return cc.found_calls;
}

bool contains_call(vector<Expr> args, vector<bool>& calls_found) {
  bool one_found = false;
  for (auto& e : args) {
    CContainsCall cc({}, true);
    e.accept(&cc);
    bool found = cc.found_calls.size() > 0;
    //std::cout << "running contains call: " << e << " found=" << found << " type=" << int(e.node_type()) << std::endl;
    calls_found.push_back(found);
    one_found = one_found || found;
  }
  //std::cout << "Found call=" << one_found << std::endl;
  return one_found;
}

// Collect a list of all calls that match a name.
class CallUsage : public IRVisitor {
  using IRVisitor::visit;

  string realize_name;
  
  void visit(const Call *op) {
    if (op->name == realize_name) {
      call_indices.emplace_back(op->args);
    }
    IRVisitor::visit(op);
  }
  
public:
  vector<vector<Expr>> call_indices;
  CallUsage(string realizename) : realize_name(realizename) {}
};

vector<vector<Expr>> matching_call_indices(Expr e, string realizename) {
  CallUsage cu(realizename);
  e.accept(&cu);
  return cu.call_indices;
}

// Check if two expr vectors are exactly equal for each element.
bool compare_expr_vectors(vector<Expr> load_indices, vector<Expr> store_indices) {
  if (load_indices.size() != store_indices.size()) {
    return false;
  }
  bool indices_match = true;
  for (size_t i=0; i<load_indices.size(); ++i) {
    //std::cout << "comparing " << load_indices.at(i) << " and " << store_indices.at(i) << "...";
    if (!equal(simplify(load_indices.at(i)), simplify(store_indices.at(i)))) {
      //std::cout << " not equal" << std::endl;
      return false;
    }
    //std::cout << " equal" << std::endl;
  }
  return indices_match;
}

class RealizeUsage : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const Call *op) {
    if (op->name == realize_name) {
      num_loads++;

      for (auto arg : op->args) {
        if (!is_const(simplify(arg))) {
          uses_variable_load_index = true;
        }
      }

      vector<bool> calls_found;
      bool is_dynamic = contains_call(op->args, calls_found);
      if (is_dynamic) {
        uses_dynamic_load_index = true;
      }
    }
    IRVisitor::visit(op);
  }

  void visit(const Provide *op) {
    if (op->name == realize_name) {
      num_stores++;
      auto store_index = op->args; // These are compared to the load indices

      vector<bool> calls_found;
      bool is_dynamic = contains_call(op->args, calls_found);
      if (is_dynamic) {
        uses_dynamic_store_index = true;
      }
      //std::cout << "is_dynamic=" << is_dynamic << " " << Expr(op->args[0]) << std::endl;

      for (auto arg : op->args) {
        if (!is_const(simplify(arg))) {
          uses_variable_store_index = true;
        }
      }
      for (auto value : op->values) {
        if (!is_const(simplify(value))) {
          uses_variable_store_value = true;
        }
      }

      for (auto value : op->values) {
        vector<vector<Expr>> call_indices = matching_call_indices(value, realize_name);
        if (call_indices.size() > 0) {
          load_and_store_use_same_realize = true;
          load_index_equals_store_index = true;
        }

        for (auto load_index : call_indices) {
          bool is_match = compare_expr_vectors(load_index, store_index);
          if (!is_match) {
            load_index_equals_store_index = false;
          }
        }
      }
    }
    IRVisitor::visit(op);
  }


 public:
  bool uses_variable_load_index; // variable: not a constant
  bool uses_variable_store_index;
  bool uses_variable_store_value;
  bool uses_dynamic_load_index;  // dynamic: data dependent (call to another func)
  bool uses_dynamic_store_index;
  bool load_and_store_use_same_realize;
  bool load_index_equals_store_index;
  uint num_loads;
  uint num_stores;
  string realize_name;

  RealizeUsage(string realizename) : uses_variable_load_index(false),
                                     uses_variable_store_index(false),
                                     uses_variable_store_value(false),
                                     uses_dynamic_load_index(false),
                                     uses_dynamic_store_index(false),
                                     load_and_store_use_same_realize(false),
                                     load_index_equals_store_index(false),
                                     num_loads(0),
                                     num_stores(0),
                                     realize_name(realizename) {}
};

//enum HWMemoryType {
//  NOT_FOUND,               // no loads or stores
//  INOUT_REALIZATION,       // only read or only written
//  CONSTS_REALIZATION,      // replace with const registers
//  ROM_REALIZATION,         // const values with variable index load
//  //UNROLLED_ROM_REALIZATION,// unrolled, so no variable store found
//  RMW_REALIZATION,         // variable reads and writes, but writes are to same address
//  RMW_DYNAMIC_REALIZATION, // data-dependent reads and writes, where writes are to same address
//  ITERATIVE_REALIZATION,   // variable reads and writes, writes coming from the same memory but different address
//  SRAM_REALIZATION,        // variable reads and variable writes
//  SRAM_DYNAMIC_REALIZATION,// variable reads and writes, including data-dependent index calculation
//  UNKNOWN_REALIZATION
//};

HWMemoryType identify_realization(Stmt s, string realizename) {
  RealizeUsage ru(realizename);
  s.accept(&ru);
  std::cout << realizename
            << " has num_stores=" << ru.num_stores
            << " num_loads=" << ru.num_loads
            << " var_load=" << ru.uses_variable_load_index
            << " var_store=" << ru.uses_variable_store_index
            << " var_sval=" << ru.uses_variable_store_value
            << " var_sval=" << ru.uses_variable_store_value
            << " iter=" << ru.load_and_store_use_same_realize
            << " rmw=" << ru.load_index_equals_store_index
            << std::endl;
  //std::cout << simplify(s);

  if (ru.num_stores == 0 && ru.num_loads == 0) {
    return NOT_FOUND;
    
  } else if (ru.num_stores == 0 || ru.num_loads == 0) {
    return INOUT_REALIZATION;

  } else if (!ru.uses_variable_load_index &&
             !ru.uses_variable_store_value) {
    return CONSTS_REALIZATION;

  //} else if (ru.uses_variable_load_index &&
  //           !ru.uses_variable_store_index && // kernel unrolled so no var store index in init
  //           !ru.uses_variable_store_value) {
  //  return UNROLLED_ROM_REALIZATION;
  } else if (ru.uses_variable_load_index &&
             //!ru.uses_variable_store_index &&
             !ru.uses_variable_store_value) {
    return ROM_REALIZATION;

  } else if (ru.uses_variable_load_index &&
             ru.uses_variable_store_index &&
             ru.load_index_equals_store_index) {
    if (ru.uses_dynamic_load_index || ru.uses_dynamic_store_index) {
      return RMW_DYNAMIC_REALIZATION;
    } else {
      return RMW_REALIZATION;
    }

  } else if (ru.uses_variable_load_index &&
             ru.uses_variable_store_index &&
             ru.load_and_store_use_same_realize) {
    return ITERATIVE_REALIZATION;
    
  } else if (ru.uses_variable_load_index &&
             ru.uses_variable_store_index &&
             !ru.load_index_equals_store_index) {
    if (ru.uses_dynamic_load_index || ru.uses_dynamic_store_index) {
      return SRAM_DYNAMIC_REALIZATION;
    } else {
      return SRAM_REALIZATION;
    }

  } else {
    return UNKNOWN_REALIZATION;
  }

}

bool uses_realization(Stmt s, string realizename) {
  auto mem_type = identify_realization(s, realizename);
  return mem_type != NOT_FOUND;
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

    map<string, deque<const Provide*> > realize_provides;
    set<string> roms;

    Stmt visit(const Realize* realize) override {

      // check the stores and loads from the underlying memory
      auto mem_type = identify_realization(realize->body, realize->name);

      if (ends_with(realize->name, ".stencil") && !ends_with(realize->name, ".const.stencil")) {

        if (mem_type == RMW_REALIZATION) {
          std::cout << "Realize " << realize->name << " is a RMW memory" << std::endl;
        } else if (mem_type == RMW_DYNAMIC_REALIZATION) {
          std::cout << "Realize " << realize->name << " is a dynamic RMW memory" << std::endl;
        } else if (mem_type == ITERATIVE_REALIZATION) {
          std::cout << "Realize " << realize->name << " is an iterative memory" << std::endl;
        } else if (mem_type == SRAM_DYNAMIC_REALIZATION) {
          std::cout << "Realize " << realize->name << " is a memory using dynamic loads and/or stores" << std::endl;
        }
        
        // .stencils are not roms
        return IRMutator::visit(realize);
        
      } else if (mem_type == ROM_REALIZATION) {
        std::cout << "Realize " << realize->name << " is a rom" << std::endl;
        roms.insert(realize->name);
        return IRMutator::visit(realize);

      } else if (mem_type == CONSTS_REALIZATION) {
        //   add ROM name to remove Producer of this name, and save provides
        std::cout << "Realize " << realize->name << " is a set of consts" << std::endl;
        realize_provides[realize->name] = deque<const Provide*>();
        auto mutated = IRMutator::visit(realize);
        //std::cout << "mutated:\n" << mutated << std::endl;
        if (uses_realization(mutated, realize->name)) {
          //std::cout << "uses realization\n";
          return Stmt(realize);
        } else {
          return mutate(realize->body);
        }
        
      } else {
        std::cout << "Realize " << realize->name << " is a " << mem_type << std::endl;
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
            //Expr call_equiv = Call::make(UInt(32), provide->name, provide->args, Call::Halide);
            Expr replacement = provide->values[0];
            //std::cout << "trying to emulate " << Stmt(provide) << "  by replacing " << call_equiv << " with " << replacement << std::endl;
            //body = graph_substitute(call_equiv, replacement, body);
          }
          return body;
        }
        
    } else {
      if (roms.count(pc->name) > 0 && pc->is_producer) {
        ///std::cout << "ROM " << pc->name << std::endl << "is created by: " << std::endl << pc->body;
      }
      return IRMutator::visit(pc);
    }
  }

  // in Provide: save for call replacement later
  Stmt visit(const Provide* provide) override {
    if (realize_provides.count(provide->name)) {
      realize_provides.at(provide->name).emplace_front(provide);
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
