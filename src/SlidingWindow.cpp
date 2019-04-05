#include "SlidingWindow.h"
#include "Bounds.h"
#include "Debug.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "IRPrinter.h"
#include "Monotonic.h"
#include "Scope.h"
#include "Simplify.h"
#include "Substitute.h"

namespace Halide {
namespace Internal {

using std::map;
using std::string;

namespace {
  int id_const_value(const Expr e) {
    if (const IntImm* e_int = e.as<IntImm>()) {
      return e_int->value;

    } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
      return e_uint->value;

    } else {
      return -1;
    }
  }

  bool is_parallelized(const For *op) {
    return op->for_type == ForType::Parallel ||
      op->for_type == ForType::Unrolled ||
      op->for_type == ForType::Vectorized;
  }
  
class ContainsAtLevel : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const For *op) {
    // don't recurse if not parallelized
    if (is_parallelized(op)) {
      IRVisitor::visit(op);
    }
  }

  void visit(const Provide *op) {
    if (var == "" || op->name == var) {
      found_provide = true;
    }
    IRVisitor::visit(op);
  }

  void visit(const Call *op) {
    if (var == "" || op->name == var) {
      found_call = true;
    }
    IRVisitor::visit(op);
  }

public:
  bool found_provide;
  bool found_call;
  string var;
  ContainsAtLevel(string var) : found_provide(false), found_call(false), var(var) {}
};

bool provide_at_level(Stmt s, string var = "") {
  ContainsAtLevel cal(var);
  s.accept(&cal);
  return cal.found_provide;
}

bool call_at_level(Stmt s, string var = "") {
  ContainsAtLevel cal(var);
  s.accept(&cal);
  return cal.found_call;
}
  
class ExpandExpr : public IRMutator2 {
    using IRMutator2::visit;
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

    Stmt visit(const For *old_op) override {
      Stmt s = IRMutator2::visit(old_op);
      const For *op = s.as<For>();
      if (op) {
        std::cout << "new op is " << s << std::endl;
      } else {
        std::cout << "this isn't a for loop anymore?\n";
      }
      
      if (is_one(op->extent)) {
        std::cout << "for loop with name " << op->name << " has min " << op->min << std::endl;
        Stmt new_body = substitute(op->name, op->min, op->body);
        std::cout << "replaced with:\n" << new_body << std::endl;
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

  
  class CountBufferUsers : public IRVisitor {
    using IRVisitor::visit;
    bool count_readers;
    bool count_writers;
    int mult_factor;
    string var;
    Scope<Expr> scope;
    For *current_for;
    Stmt full_stmt;

    void visit(const LetStmt *op) override {
        ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
        IRVisitor::visit(op);
    }

    void visit(const For *op) override {
      int factor = 1;
      if (is_parallelized(op)) {
        auto expanded_extent = expand_expr(op->extent, scope);
        std::cout << "found a for loop " << expanded_extent << "\n";
        if (is_const(expanded_extent)) {
          factor = id_const_value(expanded_extent);
        }
      }

      For *previous_for = current_for;
      Stmt for_stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, op->body);
      bool first_for = false;
      if (current_for == nullptr) {
        first_for = true;
        full_stmt = for_stmt;
      } else {
        current_for->body = for_stmt;
      }
      current_for = const_cast<For *>(for_stmt.as<For>());
      
      mult_factor *= factor;
      IRVisitor::visit(op);
      mult_factor /= factor;
      
      if (provide_at_level(op->body, var) && !is_parallelized(op)) {
        auto box_write = box_provided(op->body, var);
        std::cout << "writers inside loop " << op->name << std::endl;
        std::cout << "Box writer found for " << var << " with box " << box_write << std::endl;
        std::cout << "HWBuffer Parameter: writer ports - "
                  << "box extent=[";
        auto interval = box_write;
        for (size_t dim=0; dim<interval.size(); ++dim) {
          std::cout << find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Lower) << "-";
          std::cout << find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Upper) << " ";
        }
        std::cout << "]\n";

      }
                
        /*
        for (auto box_entry : boxes_write) {
          if (box_entry.first == var) {
            std::cout << "Box writer found for " << box_entry.first << " with box " << box_entry.second << std::endl;
        
            std::cout << "box extent=[";
            auto interval = box_entry.second;
            for (size_t dim=0; dim<interval.size(); ++dim) {
              std::cout << find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Lower) << "-";
              std::cout << find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Upper) << " ";
            }
            std::cout << "]\n";
          }
        }
        */

      if (call_at_level(op->body, var) && !is_parallelized(op)) {
        auto box_read = box_required(op->body, var);
        std::cout << "readers inside loop " << op->name << std::endl;
        std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
        std::cout << "HWBuffer Parameter: reader ports - "
                  << "box extent=[";
        auto interval = box_read;

        std::vector<Stmt> stmts;
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr lower_expr = find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Lower);
          Expr upper_expr = find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Upper);
          stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope))));
          std::cout << lower_expr << "-" << upper_expr << " ";
        }
        std::cout << "]\n";

        const std::vector<Stmt> &const_stmts = stmts;
        current_for->body = Block::make(const_stmts);
        std::cout << "HWBuffer Parameter - " << "nested reader loop:\n" << simplify(expand_expr(full_stmt, scope)) << std::endl;

      }

      current_for = previous_for;
      if (!first_for) {
        current_for->body = AssertStmt::make(var, Expr(0));
      }
      
    }

    void visit(const Store *op) override {
      count_writers = true;
      
      if (count_writers && op->name == var) {
        num_writers += mult_factor; 
      }
      IRVisitor::visit(op);
    }

    void visit(const Load *op) override {
      count_readers = true;
      
      if (count_readers && op->name == var) {
        num_readers += mult_factor;
      }
      IRVisitor::visit(op);
    }

    void visit(const Provide *op) override {
      if (op->name == var) {
        num_writers += mult_factor; 
      }
      IRVisitor::visit(op);
    }

    void visit(const Call *op) override {
      if (op->name == var) {
        num_readers += mult_factor; 
      }
      IRVisitor::visit(op);
    }
    
  public:
    int num_readers;
    int num_writers;
    CountBufferUsers(string v) : count_readers(false), count_writers(false),
                                 mult_factor(1), var(v),
                                 num_readers(0), num_writers(0) {}
  };

  int count_buffer_readers(Stmt e, string v) {
    CountBufferUsers counter(v);
    e.accept(&counter);
    return counter.num_readers;
  }

  int count_buffer_writers(Stmt e, string v) {
    CountBufferUsers counter(v);
    e.accept(&counter);
    return counter.num_writers;
  }

  
// Does an expression depend on a particular variable?
class ExprDependsOnVar : public IRVisitor {
    using IRVisitor::visit;

    void visit(const Variable *op) override {
        if (op->name == var) result = true;
    }

    void visit(const Let *op) override {
        op->value.accept(this);
        // The name might be hidden within the body of the let, in
        // which case there's no point descending.
        if (op->name != var) {
            op->body.accept(this);
        }
    }
public:

    bool result;
    string var;

    ExprDependsOnVar(string v) : result(false), var(v) {
    }
};

bool expr_depends_on_var(Expr e, string v) {
    ExprDependsOnVar depends(v);
    e.accept(&depends);
    return depends.result;
}  
}

// Perform sliding window optimization for a function over a
// particular serial for loop
class SlidingWindowOnFunctionAndLoop : public IRMutator2 {
    Function func;
    string loop_var;
    Expr loop_min;
    Scope<Expr> scope;

    map<string, Expr> replacements;

    using IRMutator2::visit;

    // Check if the dimension at index 'dim_idx' is always pure (i.e. equal to 'dim')
    // in the definition (including in its specializations)
    bool is_dim_always_pure(const Definition &def, const string& dim, int dim_idx) {
        const Variable *var = def.args()[dim_idx].as<Variable>();
        if ((!var) || (var->name != dim)) {
            return false;
        }

        for (const auto &s : def.specializations()) {
            bool pure = is_dim_always_pure(s.definition, dim, dim_idx);
            if (!pure) {
                return false;
            }
        }
        return true;
    }

    Stmt visit(const ProducerConsumer *op) override {
        if (!op->is_producer || (op->name != func.name())) {
            return IRMutator2::visit(op);
        } else {
            Stmt stmt = op;

            // We're interested in the case where exactly one of the
            // dimensions of the buffer has a min/extent that depends
            // on the loop_var.
            string dim = "";
            int dim_idx = 0;
            Expr min_required, max_required;

            debug(3) << "Considering sliding " << func.name()
                     << " along loop variable " << loop_var << "\n"
                     << "Region provided:\n";
            std::cout << "Considering sliding " << func.name()
                     << " along loop variable " << loop_var << "\n"
                     << "Region provided:\n";

            string prefix = func.name() + ".s" + std::to_string(func.updates().size()) + ".";
            const std::vector<string> func_args = func.args();
            for (int i = 0; i < func.dimensions(); i++) {
                // Look up the region required of this function's last stage
                string var = prefix + func_args[i];
                internal_assert(scope.contains(var + ".min") && scope.contains(var + ".max"));
                Expr min_req = scope.get(var + ".min");
                Expr max_req = scope.get(var + ".max");
                min_req = expand_expr(min_req, scope);
                max_req = expand_expr(max_req, scope);

                std::cout << func_args[i] << "=" << var
                          << ":" << min_req << ", " << max_req  << "\n";
                if (expr_depends_on_var(min_req, loop_var) ||
                    expr_depends_on_var(max_req, loop_var)) {
                    if (!dim.empty()) {
                        dim = "";
                        min_required = Expr();
                        max_required = Expr();
                        break;
                    } else {
                        dim = func_args[i];
                        dim_idx = i;
                        min_required = min_req;
                        max_required = max_req;
                    }
                } else if (!min_required.defined() &&
                           i == func.dimensions() - 1 &&
                           is_pure(min_req) &&
                           is_pure(max_req)) {
                    // The footprint doesn't depend on the loop var. Just compute everything on the first loop iteration.
                    dim = func_args[i];
                    dim_idx = i;
                    min_required = min_req;
                    max_required = max_req;
                }
            }

            if (!min_required.defined()) {
                std::cout << "Could not perform sliding window optimization of "
                         << func.name() << " over " << loop_var << " because multiple "
                         << "dimensions of the function dependended on the loop var\n";
                return stmt;
            }

            // If the function is not pure in the given dimension, give up. We also
            // need to make sure that it is pure in all the specializations
            bool pure = true;
            for (const Definition &def : func.updates()) {
                pure = is_dim_always_pure(def, dim, dim_idx);
                if (!pure) {
                    break;
                }
            }
            if (!pure) {
                std::cout << "Could not performance sliding window optimization of "
                         << func.name() << " over " << loop_var << " because the function "
                         << "scatters along the related axis.\n";
                return stmt;
            }

            bool can_slide_up = false;
            bool can_slide_down = false;

            Monotonic monotonic_min = is_monotonic(min_required, loop_var);
            Monotonic monotonic_max = is_monotonic(max_required, loop_var);

            if (monotonic_min == Monotonic::Increasing ||
                monotonic_min == Monotonic::Constant) {
                can_slide_up = true;
            }

            if (monotonic_max == Monotonic::Decreasing ||
                monotonic_max == Monotonic::Constant) {
                can_slide_down = true;
            }

            if (!can_slide_up && !can_slide_down) {
                std::cout << "Not sliding " << func.name()
                         << " over dimension " << dim
                         << " along loop variable " << loop_var
                         << " because I couldn't prove it moved monotonically along that dimension\n"
                         << "Min is " << min_required << "\n"
                         << "Max is " << max_required << "\n";
                return stmt;
            }

            // Ok, we've isolated a function, a dimension to slide
            // along, and loop variable to slide over.
            std::cout << "Sliding " << func.name()
                     << " over dimension " << dim
                     << " along loop variable " << loop_var << "\n";

            Expr loop_var_expr = Variable::make(Int(32), loop_var);

            Expr prev_max_plus_one = substitute(loop_var, loop_var_expr - 1, max_required) + 1;
            Expr prev_min_minus_one = substitute(loop_var, loop_var_expr - 1, min_required) - 1;

            Expr prev_min_plus_one = substitute(loop_var, loop_var_expr - 1, min_required) + 1;
            
            Expr stencil_extent = simplify(max_required - min_required + 1);
            Expr chunk_extent_from_max = simplify(prev_max_plus_one - max_required + 1);
            Expr chunk_extent_from_min = simplify(prev_min_plus_one - min_required + 1);
            std::cout << "HWBuffer Parameters: stencil extent in dim " << dim << ": " << stencil_extent
                      << " chunk (from max): " << chunk_extent_from_max
                      << " chunk (from min): " << chunk_extent_from_min
                      << std::endl;

            std::cout << "body is " << op->body << "\n";

            // If there's no overlap between adjacent iterations, we shouldn't slide.
            if (can_prove(min_required >= prev_max_plus_one) ||
                can_prove(max_required <= prev_min_minus_one)) {
                std::cout << "Not sliding " << func.name()
                         << " over dimension " << dim
                         << " along loop variable " << loop_var
                         << " there's no overlap in the region computed across iterations\n"
                         << "Min is " << min_required << "\n"
                         << "Max is " << max_required << "\n";
                return stmt;
            }

            Expr new_min, new_max;
            if (can_slide_up) {
                new_min = select(loop_var_expr <= loop_min, min_required, likely_if_innermost(prev_max_plus_one));
                new_max = max_required;
            } else {
                new_min = min_required;
                new_max = select(loop_var_expr <= loop_min, max_required, likely_if_innermost(prev_min_minus_one));
            }

            Expr early_stages_min_required = new_min;
            Expr early_stages_max_required = new_max;

            debug(3) << "Sliding " << func.name() << ", " << dim << "\n"
                     << "Pushing min up from " << min_required << " to " << new_min << "\n"
                     << "Shrinking max from " << max_required << " to " << new_max << "\n";

            std::cout << "Sliding " << func.name() << ", " << dim << "\n"
                     << "Pushing min up from " << min_required << " to " << new_min << "\n"
                     << "Shrinking max from " << max_required << " to " << new_max << "\n";

            // Now redefine the appropriate regions required
            if (can_slide_up) {
                replacements[prefix + dim + ".min"] = new_min;
            } else {
                replacements[prefix + dim + ".max"] = new_max;
            }

            for (size_t i = 0; i < func.updates().size(); i++) {
                string n = func.name() + ".s" + std::to_string(i) + "." + dim;
                replacements[n + ".min"] = Variable::make(Int(32), prefix + dim + ".min");
                replacements[n + ".max"] = Variable::make(Int(32), prefix + dim + ".max");
            }

            // Ok, we have a new min/max required and we're going to
            // rewrite all the lets that define bounds required. Now
            // we need to additionally expand the bounds required of
            // the last stage to cover values produced by stages
            // before the last one. Because, e.g., an intermediate
            // stage may be unrolled, expanding its bounds provided.
            if (!func.updates().empty()) {
                Box b = box_provided(op->body, func.name());
                if (can_slide_up) {
                    string n = prefix + dim + ".min";
                    Expr var = Variable::make(Int(32), n);
                    stmt = LetStmt::make(n, min(var, b[dim_idx].min), stmt);
                } else {
                    string n = prefix + dim + ".max";
                    Expr var = Variable::make(Int(32), n);
                    stmt = LetStmt::make(n, max(var, b[dim_idx].max), stmt);
                }
            }
            return stmt;
        }
    }

    Stmt visit(const For *op) override {
        // It's not safe to enter an inner loop whose bounds depend on
        // the var we're sliding over.
        Expr min = expand_expr(op->min, scope);
        Expr extent = expand_expr(op->extent, scope);
        if (is_one(extent)) {
            // Just treat it like a let
            Stmt s = LetStmt::make(op->name, min, op->body);
            s = mutate(s);
            // Unpack it back into the for
            const LetStmt *l = s.as<LetStmt>();
            internal_assert(l);
            return For::make(op->name, op->min, op->extent, op->for_type, op->device_api, l->body);
        } else if (is_monotonic(min, loop_var) != Monotonic::Constant ||
                   is_monotonic(extent, loop_var) != Monotonic::Constant) {
            std::cout << "Not entering loop over " << op->name
                     << " because the bounds depend on the var we're sliding over: "
                     << min << ", " << extent << "\n";
            return op;
        } else {
            return IRMutator2::visit(op);
        }
    }

    Stmt visit(const LetStmt *op) override {
        ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
        Stmt new_body = mutate(op->body);

        Expr value = op->value;

        map<string, Expr>::iterator iter = replacements.find(op->name);
        if (iter != replacements.end()) {
            value = iter->second;
            replacements.erase(iter);
        }

        if (new_body.same_as(op->body) && value.same_as(op->value)) {
            return op;
        } else {
            return LetStmt::make(op->name, value, new_body);
        }
    }

public:
    SlidingWindowOnFunctionAndLoop(Function f, string v, Expr v_min) : func(f), loop_var(v), loop_min(v_min) {}
};

// Perform sliding window optimization for a particular function
class SlidingWindowOnFunction : public IRMutator2 {
    Function func;

    using IRMutator2::visit;

    Stmt visit(const For *op) override {
        std::cout << " Doing sliding window analysis over loop: " << op->name << "\n";

        Stmt new_body = op->body;

        new_body = mutate(new_body);

        if (op->for_type == ForType::Serial ||
            op->for_type == ForType::Unrolled) {
            new_body = SlidingWindowOnFunctionAndLoop(func, op->name, op->min).mutate(new_body);
        }

        if (new_body.same_as(op->body)) {
            return op;
        } else {
            return For::make(op->name, op->min, op->extent, op->for_type, op->device_api, new_body);
        }
    }

public:
    SlidingWindowOnFunction(Function f) : func(f) {}
};

// Perform sliding window optimization for all functions
class SlidingWindow : public IRMutator2 {
    const map<string, Function> &env;
  
    using IRMutator2::visit;

    Stmt visit(const Realize *op) override {
        // Find the args for this function
        map<string, Function>::const_iterator iter = env.find(op->name);

        // If it's not in the environment it's some anonymous
        // realization that we should skip (e.g. an inlined reduction)
        if (iter == env.end()) {
            return IRMutator2::visit(op);
        }

        // If the Function in question has the same compute_at level
        // as its store_at level, skip it.
        const FuncSchedule &sched = iter->second.schedule();
        if (sched.compute_level() == sched.store_level()) {
            return IRMutator2::visit(op);
        }

        Stmt new_body = op->body;

        std::cout << "Doing sliding window analysis on realization of " << op->name << "\n";

        new_body = SlidingWindowOnFunction(iter->second).mutate(new_body);

        new_body = mutate(new_body);

        if (new_body.same_as(op->body)) {
          std::cout << "no sliding done\n";
            return op;
            
        } else {
          CountBufferUsers counter(op->name);
          new_body.accept(&counter);
          int num_readers = counter.num_readers;
          int num_writers = counter.num_writers;
          //int num_readers = count_buffer_readers(new_body, op->name);
          //int num_writers = count_buffer_writers(new_body, op->name);
          auto boxes_write = boxes_provided(new_body);
          for (auto box_entry : boxes_write) {
            std::cout << "Box writer found for " << box_entry.first << " with box " << box_entry.second << std::endl;
          }
          auto boxes_read = boxes_required(new_body);
          for (auto box_entry : boxes_read) {
            std::cout << "Box reader found for " << box_entry.first << " with box " << box_entry.second << std::endl;
          }

          std::cout << "HWBuffer Parameter: Total box is of size [";
          for (size_t i=0; i<boxes_read[op->name].size(); ++i) {
            Expr extent = simplify(boxes_read[op->name][i].max - boxes_read[op->name][i].min + 1);
            std::cout << extent << " ";
          }
          std::cout << "]\n";
          
          std::cout << "HWBuffer Parameter: " << op->name << " has readers=" << num_readers
                    << " writers=" << num_writers << std::endl;
          std::cout << "new body with sliding for " << op->name << "\n"
                    << "old body: \n" << op->body
                    << "new body: \n" << new_body << '\n';
          
            return Realize::make(op->name, op->types, op->memory_type,
                                 op->bounds, op->condition, new_body);
        }
    }
  
public:
    SlidingWindow(const map<string, Function> &e) : env(e) {}

};

Stmt sliding_window(Stmt s, const map<string, Function> &env) {
    return SlidingWindow(env).mutate(s);
}

}  // namespace Internal
}  // namespace Halide
