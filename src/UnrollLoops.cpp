#include "UnrollLoops.h"
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

class UnrollLoops : public IRMutator {
    using IRMutator::visit;

    Stmt visit(const For *for_loop) override {
        if (for_loop->for_type == ForType::Unrolled) {
            // Give it one last chance to simplify to an int
            Expr extent = simplify(for_loop->extent);
            const IntImm *e = extent.as<IntImm>();

            if (e == nullptr && permit_failed_unroll) {
                // Not constant. Just rewrite to serial.
                Stmt body = mutate(for_loop->body);
                return For::make(for_loop->name, for_loop->min, for_loop->extent,
                                 ForType::Serial, for_loop->device_api, std::move(body));
            }

            user_assert(e)
                << "Can only unroll for loops over a constant extent.\n"
                << "Loop over " << for_loop->name << " has extent " << extent << ".\n";
            Stmt body = mutate(for_loop->body);

            if (e->value == 1) {
                user_warning << "Warning: Unrolling a for loop of extent 1: " << for_loop->name << "\n";
            }

            vector<Stmt> iters;
            // Make n copies of the body, each wrapped in a let that defines the loop var for that body
            for (int i = 0; i < e->value; i++) {
                iters.push_back(substitute(for_loop->name, for_loop->min + i, body));
            }
            return Block::make(iters);

        } else {
            return IRMutator::visit(for_loop);
        }
    }
    bool permit_failed_unroll = false;
public:
    UnrollLoops() {
        // Experimental autoschedulers may want to unroll without
        // being totally confident the loop will indeed turn out
        // to be constant-sized. If this feature continues to be
        // important, we need to expose it in the scheduling
        // language somewhere, but how? For now we do something
        // ugly and expedient.

        // For the tracking issue to fix this, see
        // https://github.com/halide/Halide/issues/3479
        permit_failed_unroll = get_env_variable("HL_PERMIT_FAILED_UNROLL") == "1";
    }
};


bool call_matches_provide(const Call *call, const Provide* provide) {
  bool match = true;

  if (call->args.size() != provide->args.size()) {
    return false;
  }
  
  for (size_t i=0; i<provide->args.size(); ++i) {
    match &= equal(provide->args[i], call->args[i]);
  }
      
  match &= (call->name == provide->name);
  
  return match;
}

class ContainsAccumulate : public IRVisitor {
  string name;
  
  using IRVisitor::visit;
  void visit(const Provide *op) override {
    //std::cout << "storing provide: " << Stmt(op) << std::endl;
    if (name == "") {
      provides[op->name] = op;
    } else if (name == op->name) {
      provides[name] = op;
    }
    
    IRVisitor::visit(op);
  }

  void visit(const Call *op) override {
    //std::cout << "have this call: " << Expr(op) << std::endl;
    if (provides.count(op->name) > 0) {
      auto provide = provides.at(op->name);
      if (call_matches_provide(op, provide)) {
        found = true;
      }
    }
    
    IRVisitor::visit(op);
  }

public:
  bool found;
  map<string, const Provide*> provides;
  ContainsAccumulate(string name) : name(name), found(false) {}
  ContainsAccumulate() : name(""), found(false) {}
};

bool contains_accumulate(string name, Stmt s) {
  ContainsAccumulate ca(name);
  s.accept(&ca);
  return ca.found;
}

bool contains_accumulate(Stmt s) {
  ContainsAccumulate ca;
  s.accept(&ca);
  return ca.found;
}

class ContainsProvide : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const Provide *op) override {
    provide = op;
    found = true;
  }

public:
  bool found;
  const Provide* provide;
  ContainsProvide() : found(false) {}
};

class ReplaceCall : public IRMutator {
  using IRMutator::visit;

  const Provide* provide;

  Expr visit(const Call *op) override {
    //std::cout << "inline: have this call: " << Expr(op) << std::endl;
    if (!replaced && call_matches_provide(op, provide)) {
      //std::cout << " replacing it with " << provide->values[0] << std::endl;
      replaced = true;
      return provide->values[0];
    } else {
      return IRMutator::visit(op);
    }
  }
  
public:
  bool replaced;
  ReplaceCall(const Provide* op) : provide(op), replaced(false) {}
};

class InlineProvideCallPairs : public IRMutator {
  using IRMutator::visit;

  Stmt visit(const Block *op) override {
    ContainsProvide cp;
    op->first.accept(&cp);

    if (cp.found) {
      const Provide* provide = cp.provide;
      ReplaceCall rc(provide);
      Stmt replaced = rc.mutate(op->rest);

      if (rc.replaced) {
        replaced = mutate(replaced);
        return replaced;
      } else {
        Stmt first = mutate(op->first);
        Stmt rest = mutate(op->rest);
        if (first.same_as(op->first) && rest.same_as(op->rest)) {
          return op;
        }
        return Block::make(std::move(first), std::move(rest));
      }
      
    } else {
      return IRMutator::visit(op);
    }
  }

public:
    InlineProvideCallPairs() { }
};

Stmt inline_provide_call_pairs(Stmt s) {
    return InlineProvideCallPairs().mutate(s);
}



class UnrollLoopsAndMerge : public IRMutator {
    using IRMutator::visit;

    bool permit_failed_unroll = false;

    //Stmt visit(const ProducerConsumer *pc) override {
    //  Stmt body = mutate(pc->body);
    //  if (pc->is_producer && contains_accumulate(pc->name, body)) {
    //    //std::cout << "block for inlining:\n" << cur_block << std::endl;
    //    //cur_block = substitute_in_all_letstmts(cur_block);
    //
    //    //std::cout << "start " << body;
    //    auto cur_block = inline_provide_call_pairs(body);
    //    //std::cout << "now " << cur_block;
    //    //cur_block = substitute_in_all_letstmts(cur_block);
    //          
    //    return ProducerConsumer::make_produce(pc->name, cur_block);
    //
    //  } else {
    //    return IRMutator::visit(pc);
    //  }
    //}
  
    Stmt visit(const For *for_loop) override {
        if (for_loop->for_type == ForType::Unrolled) {
            // Give it one last chance to simplify to an int
            Expr extent = simplify(for_loop->extent);
            const IntImm *e = extent.as<IntImm>();

            if (e == nullptr && permit_failed_unroll) {
                // Not constant. Just rewrite to serial.
                Stmt body = mutate(for_loop->body);
                return For::make(for_loop->name, for_loop->min, for_loop->extent,
                                 ForType::Serial, for_loop->device_api, std::move(body));
            }

            user_assert(e)
                << "Can only unroll for loops over a constant extent.\n"
                << "Loop over " << for_loop->name << " has extent " << extent << ".\n";
            Stmt body = mutate(for_loop->body);

            if (e->value == 1) {
                user_warning << "Warning: Unrolling a for loop of extent 1: " << for_loop->name << "\n";
            }

            //std::cout << "looking at :\n" << for_loop->body << std::endl;
            vector<Stmt> iters;
            // Make n copies of the body, each wrapped in a let that defines the loop var for that body
            for (int i = 0; i < e->value; i++) {
                iters.push_back(substitute(for_loop->name, for_loop->min + i, body));
            }

            if (!contains_accumulate(for_loop->body)) {
              auto cur_block = Block::make(iters);
              std::cout << "block with no inlining:\n" << cur_block << std::endl;
              return cur_block;
              //return substitute_in_all_letstmts(Block::make(iters));
            } else {
              auto cur_block = Block::make(iters);
              std::cout << "block for inlining:\n" << cur_block << std::endl;
              //cur_block = substitute_in_all_letstmts(cur_block);

              //std::cout << "start" << cur_block;
              for (int i=0; i<e->value + 1; ++i) {
                cur_block = inline_provide_call_pairs(cur_block);
                //std::cout << "now " << cur_block;
                //cur_block = substitute_in_all_letstmts(cur_block);
              }
              
              return cur_block;
            }

        } else {
            return IRMutator::visit(for_loop);
        }
    }

public:
    UnrollLoopsAndMerge() {
        // Experimental autoschedulers may want to unroll without
        // being totally confident the loop will indeed turn out
        // to be constant-sized. If this feature continues to be
        // important, we need to expose it in the scheduling
        // language somewhere, but how? For now we do something
        // ugly and expedient.

        // For the tracking issue to fix this, see
        // https://github.com/halide/Halide/issues/3479
        permit_failed_unroll = get_env_variable("HL_PERMIT_FAILED_UNROLL") == "1";
    }
};


Stmt unroll_loops(Stmt s) {
    return UnrollLoops().mutate(s);
}

Stmt unroll_loops_and_merge(Stmt s) {
  Stmt inlined = substitute_in_all_letstmts(s);
  return UnrollLoopsAndMerge().mutate(inlined);
}

}  // namespace Internal
}  // namespace Halide
