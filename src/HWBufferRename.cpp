#include "HWBufferRename.h"
#include "HWBufferSimplifications.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Simplify.h"
#include "Substitute.h"
#include "UnrollLoops.h"

using std::map;
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
  const map<string, Function> &env;
  bool in_xcel;
  
    using IRMutator::visit;

    Stmt visit(const Realize *op) override {
      string funcname;
      if (ends_with(op->name, ".0")) { // complexfuncs get separated with .0 and .1
        funcname = op->name.substr(0, op->name.find(".0"));
      } else if (ends_with(op->name, ".1")) {
        funcname = op->name.substr(0, op->name.find(".1"));
      } else {
        funcname = op->name;
      }
      //std::cout << "looking for realize " << op->name << " using name " << funcname << std::endl;
      
      auto func = env.at(funcname);

      // ROMs should be flattened, so don't append ".stencil"
      auto realization_type = identify_realization(unroll_loops(Stmt(op)), op->name);
      if (realization_type != 5) {
        std::cout << op->name << " is a realization type " << realization_type << std::endl;
      }
      if (op->name == "f0.0") {
        //std::cout << Stmt(simplify(unroll_loops(op)));
      }

      // ROMs tagged as Stack should not be flattened
      bool tagged_rom = func.schedule().memory_type() == MemoryType::ROM;
      bool tagged_glb = func.schedule().memory_type() == MemoryType::GLB;

      string op_name;
      if (tagged_glb) {
        op_name = op->name + ".glb";
      } else {
        op_name = op->name;
      }

      if (!tagged_rom &&
          (realization_type != ROM_REALIZATION &&
           realization_type != CONSTS_REALIZATION &&
           (in_xcel || func.schedule().is_accelerator_input()))) {
        // roms should be allocates, not stencil realizes
        string new_name = op_name + ".stencil";
        //std::cout << op->name << " getting new name " << new_name << " with type=" << realization_type << std::endl;
        Stmt new_body = RenameRealize(op->name, new_name).mutate(op->body);
        new_body = mutate(new_body);
        return Realize::make(new_name, op->types, op->memory_type,
                             op->bounds, op->condition, new_body);
      } else if (realization_type == CONSTS_REALIZATION &&
                 (in_xcel || func.schedule().is_accelerator_input())) {
        string new_name = op_name + ".const.stencil";
        //std::cout << op->name << " getting new name " << new_name << " with type=" << realization_type << std::endl;
        Stmt new_body = RenameRealize(op->name, new_name).mutate(op->body);
        new_body = mutate(new_body);
        return Realize::make(new_name, op->types, op->memory_type,
                             op->bounds, op->condition, new_body);
        
      } else {

        if (!func.schedule().is_accelerated()) {
          return IRMutator::visit(op);
        }
      
        string new_name = op_name + ".stencil";
        Stmt new_body = RenameRealize(op->name, new_name).mutate(op->body);

        in_xcel = true;
        new_body = mutate(new_body);
        in_xcel = false;
        
        return Realize::make(new_name, op->types, op->memory_type,
                             op->bounds, op->condition, new_body);
      }
    }

public:
  RenameStencilRealizes(const map<string, Function> &env) : env(env), in_xcel(false){ }
};

Stmt rename_hwbuffers(Stmt s, const map<string, Function> &env) {
    auto renamed = RenameStencilRealizes(env).mutate(s);
    return renamed;
}

}  // namespace Internal
}  // namespace Halide
