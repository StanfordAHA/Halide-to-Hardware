#include "HWUtils.h"

#include "Func.h"
#include "IROperator.h"
#include "Schedule.h"
#include "Substitute.h"


using namespace std;

namespace Halide {
  namespace Internal {

std::vector<std::string> get_tokens(const std::string &line, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    std::string token;
    // copied from https://stackoverflow.com/a/7621814
    while ((pos = line.find_first_of(delimiter, prev)) != std::string::npos) {
        if (pos > prev) {
            tokens.emplace_back(line.substr(prev, pos - prev));
        }
        prev = pos + 1;
    }
    if (prev < line.length()) tokens.emplace_back(line.substr(prev, std::string::npos));
    // remove empty ones
    std::vector<std::string> result;
    result.reserve(tokens.size());
    for (auto const &t : tokens)
        if (!t.empty()) result.emplace_back(t);
    return result;
}


int id_const_value(const Expr e) {
  if (const IntImm* e_int = e.as<IntImm>()) {
    return e_int->value;

  } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    return e_uint->value;

  } else {
    return -1;
  }
}

std::ostream& operator<<(std::ostream& os, const std::vector<string>& vec) {
  os << "[";
  for (size_t i=0; i<vec.size(); ++i) {
    os << vec.at(i);
    if (i < vec.size() - 1) {
      os << ",";
    }
  }
  os << "]";
  return os;
};

// return true if the for loop is not serialized
bool is_parallelized(const For *op) {
  return op->for_type == ForType::Parallel ||
    op->for_type == ForType::Unrolled ||
    op->for_type == ForType::Vectorized;
}

class FirstForName : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const For *op) override {
    var = op->name;
  }
public:
  string var;
  FirstForName() {}
};

std::string first_for_name(Stmt s) {
  FirstForName ffn;
  s.accept(&ffn);
  return ffn.var;
}

class ExamineLoopLevel : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const For *op) override {
    // don't recurse if not parallelized
    if (is_parallelized(op) || is_one(op->extent)) {
      IRVisitor::visit(op);
    }
  }

  void visit(const Provide *op) override {
    if (var == "" || op->name == var) {
      found_provide = true;
    }
    IRVisitor::visit(op);
  }

  void visit(const Call *op) override {
    if (var == "" || op->name == var) {
      found_call = true;
    }
    IRVisitor::visit(op);
  }

public:
  bool found_provide;
  bool found_call;
  string var;
  ExamineLoopLevel(string var) : found_provide(false), found_call(false), var(var) {}
};

// return if provide creates the var at this level (not recursing into serial loops)
bool provide_at_level(Stmt s, string var = "") {
  ExamineLoopLevel ell(var);
  s.accept(&ell);
  return ell.found_provide;
}

// return if call uses the var at this level (not recursing into serial loops)
bool call_at_level(Stmt s, string var = "") {
  ExamineLoopLevel ell(var);
  s.accept(&ell);
  return ell.found_call;
}
  
class ExpandExpr : public IRMutator {
    using IRMutator::visit;
    const Scope<Expr> &scope;
  
    Expr visit(const Variable *var) override {
        if (scope.contains(var->name)) {
            Expr expr = scope.get(var->name);
            debug(3) << "Fully expanded " << var->name << " -> " << expr << "\n";
            return expr;
        } else {
            return var;
        }
    }

    // remove for loops of length 1
    Stmt visit(const For *old_op) override {
      Stmt s = IRMutator::visit(old_op);
      const For *op = s.as<For>();
      
      if (is_one(op->extent)) {
        Stmt new_body = substitute(op->name, op->min, op->body);
        return new_body;
        
      } else {
        return op;
      }
    }

public:
    ExpandExpr(const Scope<Expr> &s) : scope(s) {}

};

// Perform all the substitutions in a scope
Expr expand_expr(Expr e, const Scope<Expr> &scope) {
    ExpandExpr ee(scope);
    Expr result = ee.mutate(e);
    debug(3) << "Expanded " << e << " into " << result << "\n";
    return result;
}

Stmt expand_expr(Stmt e, const Scope<Expr> &scope) {
    ExpandExpr ee(scope);
    Stmt result = ee.mutate(e);
    debug(3) << "Expanded " << e << " into " << result << "\n";
    return result;
}

class FindOutputBounds : public IRVisitor {
  LoopLevel compute_level;
  Function func;

  using IRVisitor::visit;

  void visit(const For *op) override {
    //std::cout << "visiting " << op->name << std::endl;
    if (compute_level.match(op->name)) {
      Box box = box_provided(op->body, func.name());
      output_bounds = box;
    }
    IRVisitor::visit(op);
  }

public:
  Box output_bounds;
  
  FindOutputBounds(LoopLevel compute, Function func)
    : compute_level(compute), func(func) {
    std::cout << "func trying to find output bounds: " << func.name()
              << " " << compute.var().name() << std::endl;
  }
};

Box find_output_bounds(Stmt s, Function func,
                       LoopLevel compute_level) {
  FindOutputBounds fob(compute_level, func);
  s.accept(&fob);
  return fob.output_bounds;
}

void find_output_scope(Stmt s, Function func,
                       LoopLevel compute_level,
                       Scope<Expr> &scope) {
  FindOutputBounds fob(compute_level, func);
  s.accept(&fob);
  auto box = fob.output_bounds;
  
  for (int i = 0; i < func.dimensions(); i++) {
    string stage_name = func.name() + ".s0." + func.args()[i];
    scope.push(stage_name + ".min", box[i].min);
    std::cout << stage_name << ".min being set to " << box[i].min << std::endl;
    scope.push(stage_name + ".max", box[i].max);
  }
}

class FindInnerLoops : public IRVisitor {
  Function func;

  LoopLevel outer_loop_exclusive;
  LoopLevel inner_loop_inclusive;
  bool in_inner_loops;

  using IRVisitor::visit;

  void visit(const ProducerConsumer *op) override {
    // match store level at PC node in case the looplevel is outermost
    if (outer_loop_exclusive.lock().func() == op->name &&
        outer_loop_exclusive.lock().var().name() == Var::outermost().name()) {
      in_inner_loops = true;
    }
    IRVisitor::visit(op);
  }

  void visit(const For *op) override {
    // scan loops are loops between the outer store level (exclusive) and
    // the inner compute level (inclusive) of the accelerated function
    if (in_inner_loops && starts_with(op->name, func.name() + ".")) {
      debug(3) << "added loop " << op->name << " to inner loops.\n";
      inner_loops.emplace_back(op->name);
      //loop_mins[op->name] = op->min;
      //loop_maxes[op->name] = simplify(op->min + op->extent - 1);
      //std::cout << "added loop to scan loop named " << op->name << " with extent=" << op->extent << std::endl;
    }

    // reevaluate in_scan_loops in terms of loop levels
    if (outer_loop_exclusive.match(op->name)) {
      in_inner_loops = true;
    }
    if (inner_loop_inclusive.match(op->name)) {
      in_inner_loops = false;
    }

    // Recurse
    IRVisitor::visit(op);
  }

public:  
  FindInnerLoops(Function f, LoopLevel outer_level, LoopLevel inner_level, bool inside)
    : func(f), outer_loop_exclusive(outer_level), inner_loop_inclusive(inner_level),
      in_inner_loops(inside) { }

  vector<string> inner_loops;
  
};

// Produce a vector of the loops within a for-loop-nest.
//   Note: this can be used to find the streaming loops between the store and compute level.
vector<string> get_loop_levels_between(Stmt s, Function func,
                                       LoopLevel outer_level_exclusive,
                                       LoopLevel inner_level_inclusive,
                                       bool start_inside) {
  FindInnerLoops fil(func, outer_level_exclusive, inner_level_inclusive, start_inside);
  s.accept(&fil);
  //std::cout << "got some loops: " << fil.inner_loops << std::endl;
  return fil.inner_loops;
}



// Because storage folding runs before simplification, it's useful to
// at least substitute in constants before running it, and also simplify the RHS of Let Stmts.
class SubstituteInConstants : public IRMutator {
    using IRMutator::visit;

    Scope<Expr> scope;
    Stmt visit(const LetStmt *op) override {
        Expr value = simplify(mutate(op->value));

        Stmt body;
        if (is_const(value)) {
            ScopedBinding<Expr> bind(scope, op->name, value);
            body = mutate(op->body);
        } else {
            body = mutate(op->body);
        }

        if (body.same_as(op->body) && value.same_as(op->value)) {
            return op;
        } else {
            return LetStmt::make(op->name, value, body);
        }
    }

    Expr visit(const Variable *op) override {
        if (scope.contains(op->name)) {
            return scope.get(op->name);
        } else {
            return op;
        }
    }
};


Stmt substituteInConstants(Stmt& s) {
  return SubstituteInConstants().mutate(s);
}

class ExpandExprNoVar : public IRMutator {
    using IRMutator::visit;
    const Scope<Expr> &scope;

    Expr visit(const Variable *var) {
        if (scope.contains(var->name)) {
          debug(4) << "Fully expanded " << var->name << " -> " << scope.get(var->name) << "\n";
          //std::cout << "Fully expanded " << var->name << " -> " << scope.get(var->name) << "\n";
          if (var->name == "hw_output.s0.x.xi") { return Expr(0); }
          if (var->name == "hw_output.s0.y.yi") { return Expr(0); }
          return Expr(0);
          //return scope.get(var->name);


        } else {
          std::cout << "Scope does not contain  " << var->name << "\n";
          return var;
        }
    }

public:
    ExpandExprNoVar(const Scope<Expr> &s) : scope(s) {}

};

// Perform all the substitutions in a scope
Expr expand_expr_no_var(Expr e, const Scope<Expr> &scope) {
    ExpandExprNoVar ee(scope);
    Expr result = ee.mutate(e);
    debug(4) << "Expanded " << e << " into " << result << "\n";
    return result;
}



  }
}
