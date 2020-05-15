#include "HWBufferRename.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"

using std::string;
using std::vector;

namespace Halide {
namespace Internal {

class RenameRealize : public IRMutator {
  const string &orig_name;
  const string &new_name;

  using IRMutator::visit;

  Stmt visit(const ProducerConsumer *op) override {
    if (op->name == orig_name) {
      Stmt new_body = mutate(op->body);
      return ProducerConsumer::make(new_name, op->is_producer, new_body);
    } else {
      return IRMutator::visit(op);
    }
  }
  
  Expr visit(const Call *op) override {
    if (op->name == orig_name ) {
      vector<Expr> new_args;
      for (auto arg : op->args) {
        Expr new_arg = mutate(arg);
        new_args.emplace_back(new_arg);
      }
      //return Call::make(op->type, new_name, new_args, op->call_type);
      return Call::make(op->type, new_name, new_args, Call::Intrinsic);
    } else {
      return IRMutator::visit(op);
    }
  }

  Stmt visit(const Provide *op) override {
    if (op->name == orig_name ) {
      vector<Expr> new_values;
      for (auto value : op->values) {
        Expr new_value = mutate(value);
        new_values.emplace_back(new_value);
      }
      vector<Expr> new_args;
      for (auto arg : op->args) {
        Expr new_arg = mutate(arg);
        new_args.emplace_back(new_arg);
      }
      
      return Provide::make(new_name, new_values, new_args);
    } else {
      return IRMutator::visit(op);
    }
  }

public:
  RenameRealize(const string &o, const string &n)
    : orig_name(o), new_name(n) {}
};


class RenameStencilRealizes : public IRMutator {
    using IRMutator::visit;

    Stmt visit(const Realize *op) override {
      string realize_name = op->name + ".stencil";
      Stmt new_body = RenameRealize(op->name, realize_name).mutate(op->body);
      new_body = mutate(new_body);
      return Realize::make(realize_name, op->types, op->memory_type,
                           op->bounds, op->condition, new_body);
    }
public:
    RenameStencilRealizes() { }
};

Stmt rename_hwbuffers(Stmt s) {
    auto renamed = RenameStencilRealizes().mutate(s);
    return renamed;
}

}  // namespace Internal
}  // namespace Halide
