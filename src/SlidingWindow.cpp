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

std::ostream& operator<<(std::ostream& os, const std::vector<Expr>& vec) {
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


// Perform sliding window optimization for a function over a
// particular serial for loop
class SlidingWindowOnFunctionAndLoop : public IRMutator {
    Function func;
    string loop_var;
    Expr loop_min;
    Scope<Expr> scope;

    map<string, Expr> replacements;

    using IRMutator::visit;

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
            return IRMutator::visit(op);
        } else {
            Stmt stmt = op;
            //std::cout << "visiting pc sliding window for " << op->name << std::endl;
            //std::cout << stmt << std::endl;

            // We're interested in the case where exactly one of the
            // dimensions of the buffer has a min/extent that depends
            // on the loop_var.
            string dim = "";
            int dim_idx = 0;
            Expr min_required, max_required;

            debug(3) << "Considering sliding " << func.name()
                     << " along loop variable " << loop_var << "\n"
                     << "Region provided:\n";

            // initialize vectors
            //output_stencil_box = std::vector<Expr>(func.dimensions());
            //input_chunk_box = std::vector<Expr>(func.dimensions());

            string prefix = func.name() + ".s" + std::to_string(func.updates().size()) + ".";
            const std::vector<string> func_args = func.args();
            for (int i = 0; i < func.dimensions(); i++) {
              //std::cout << "looking at dim " << i << " for " << func.name() << std::endl;
              
                // Look up the region required of this function's last stage
                string var = prefix + func_args[i];
                internal_assert(scope.contains(var + ".min") && scope.contains(var + ".max"));
                Expr min_req = scope.get(var + ".min");
                Expr max_req = scope.get(var + ".max");
                min_req = expand_expr(min_req, scope);
                max_req = expand_expr(max_req, scope);
                //std::cout << "for dim=" << i << "  var=" << var
                //          << "  max=" << max_req << "  min=" << min_req << std::endl;

                debug(3) << func_args[i] << ":" << min_req << ", " << max_req  << "\n";
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
                debug(3) << "Could not perform sliding window optimization of "
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
                debug(3) << "Could not performance sliding window optimization of "
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
                debug(3) << "Not sliding " << func.name()
                         << " over dimension " << dim << "=" << std::to_string(dim_idx)
                         << " along loop variable " << loop_var
                         << " because I couldn't prove it moved monotonically along that dimension\n"
                         << "Min is " << min_required << "\n"
                         << "Max is " << max_required << "\n";
                return stmt;
            }

            // Ok, we've isolated a function, a dimension to slide
            // along, and loop variable to slide over.
            debug(3) << "Sliding " << func.name()
                     << " over dimension " << dim
                     << " along loop variable " << loop_var << "\n";

            Expr loop_var_expr = Variable::make(Int(32), loop_var);

            Expr prev_max_plus_one = substitute(loop_var, loop_var_expr - 1, max_required) + 1;
            Expr prev_min_minus_one = substitute(loop_var, loop_var_expr - 1, min_required) - 1;

            // compute sizes
            Box b_p = box_provided(op->body, func.name());
            Box b_r = box_required(op->body, func.name());
            //std::cout << "func " << func.name() << " provides=" << b_p << " requires=" << b_r << std::endl;
            Expr output_stencil_size = simplify(expand_expr(max_required - min_required + 1, scope));
            //std::cout << "output stencil: " << output_stencil_size << " for dim " << dim_idx << std::endl;
            output_stencil_box.push_back(output_stencil_size);
            output_min_pos.push_back(min_required);
            
            //Expr input_chunk_size = simplify(expand_expr(max_required + 1 - prev_max_plus_one, scope));
            Expr input_chunk_size = simplify(max_required + 1 - prev_max_plus_one);
            input_chunk_size = is_zero(input_chunk_size) ? 1 : input_chunk_size;
            //std::cout << "input stencil: " << input_chunk_size << " for dim_idx=" << dim_idx << std::endl;
            input_chunk_box.emplace_back(input_chunk_size);
            //input_chunk_box[dim_idx] = input_chunk_size;
            //std::cout << "input chunk so far for " << func.name() << " is " << input_chunk_box << std::endl;

            //std::cout << "Sliding " << func.name()
            //          << " over dimension " << dim << "=" << std::to_string(dim_idx)
            //          << " along loop variable " << loop_var << "\n"
            //          << " where min=" << min_required << "  max=" << max_required
            //          << " max_prev_plus_one=" << prev_max_plus_one << "\n"
            //          << "\n";


            // If there's no overlap between adjacent iterations, we shouldn't slide.
            if (can_prove(min_required >= prev_max_plus_one) ||
                can_prove(max_required <= prev_min_minus_one)) {
                debug(3) << "Not sliding " << func.name()
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
              //new_min = prev_max_plus_one;
                new_max = max_required;
            } else {
                new_min = min_required;
                new_max = select(loop_var_expr <= loop_min, max_required, likely_if_innermost(prev_min_minus_one));
                //new_max = prev_min_minus_one;
            }

            Expr early_stages_min_required = new_min;
            Expr early_stages_max_required = new_max;

            debug(3) << "Sliding " << func.name() << ", " << dim << "\n"
                     << "Pushing min up from " << min_required << " to " << new_min << "\n"
                     << "Shrinking max from " << max_required << " to " << new_max << "\n";

            //std::cout << "Sliding " << func.name() << ", " << dim << "\n"
            //          << "  Pushing min up from " << min_required << " to " << new_min << "\n"
            //          << "  Shrinking max from " << max_required << " to " << new_max << "\n";

            
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
            debug(3) << "Not entering loop over " << op->name
                     << " because the bounds depend on the var we're sliding over: "
                     << min << ", " << extent << "\n";
            //std::cout << "Not entering loop over " << op->name
            //         << " because the bounds depend on the var we're sliding over: "
            //         << min << ", " << extent << "\n";
            return op;
        } else {
            return IRMutator::visit(op);
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
    std::vector<Expr> output_stencil_box;
    std::vector<Expr> output_min_pos;
    std::vector<Expr> input_chunk_box;
    SlidingWindowOnFunctionAndLoop(Function f, string v, Expr v_min) :
      func(f), loop_var(v), loop_min(v_min) {
      //input_chunk_box = std::vector<Expr>(2);
    }

    // define the copy constructor without Scope (whose copy constructor is private)
    SlidingWindowOnFunctionAndLoop(const SlidingWindowOnFunctionAndLoop &obj) {
      output_stencil_box = obj.output_stencil_box;
      output_min_pos = obj.output_min_pos;
      input_chunk_box = obj.input_chunk_box;
    }

};

}

// Perform sliding window optimization for a particular function
class SlidingWindowOnFunction : public IRMutator {
    Function func;

    using IRMutator::visit;

    Stmt visit(const For *op) override {
        debug(3) << " Doing sliding window analysis over loop: " << op->name << "\n";

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

// Perform sliding window optimization for a particular function
class SlidingWindowVisitorOnFunction : public IRVisitor {
    Function func;

    using IRVisitor::visit;

    void visit(const For *op) override {
        debug(3) << " Doing sliding window analysis over loop: " << op->name << "\n";
        //std::cout << " Doing sliding window analysis over loop: " << op->name << "\n";

        Stmt new_body = op->body;
        new_body.accept(this);

        if (op->for_type == ForType::Serial ||
            op->for_type == ForType::Unrolled) {
          //std::cout << "searching for sliding window for " << func.name() << " using loop " << op->name << std::endl;
            SlidingWindowOnFunctionAndLoop sliding_window_mutator = SlidingWindowOnFunctionAndLoop(func, op->name, op->min);
            sliding_window_mutator.mutate(op->body);

            auto added_stencil = sliding_window_mutator.output_stencil_box;
            ss.output_stencil_box.insert(ss.output_stencil_box.end(), added_stencil.begin(), added_stencil.end());
            auto added_chunk = sliding_window_mutator.input_chunk_box;
            ss.input_chunk_box.insert(ss.input_chunk_box.end(), added_chunk.begin(), added_chunk.end());
            auto added_min_pos = sliding_window_mutator.output_min_pos;
            ss.output_min_pos.insert(ss.output_min_pos.end(), added_min_pos.begin(), added_min_pos.end());

            if (added_stencil.size() > 0) {
              //std::cout << "added sliding stencil called " << op->name << " with " << added_stencil.size()
              //          << " more loop(s) resulting in num_total_loops=" << ss.output_stencil_box.size() << "\n";
            }
            
            buffer_sliding_stencils[op->name] = ss;
        }
    }

public:
    // contiually adds to set of loops; needed for nested for loops
    SlidingStencils ss;
    std::map<std::string, SlidingStencils> buffer_sliding_stencils;
    SlidingWindowVisitorOnFunction(Function f) : func(f) {}
};

std::map<std::string, SlidingStencils> extract_sliding_stencils(Stmt s, Function f) {
  auto sliding_window_mutator = SlidingWindowVisitorOnFunction(f);
  //std::cout << "extracting sliding stencils for: \n" << s << std::endl;
  s.accept(&sliding_window_mutator);
  return sliding_window_mutator.buffer_sliding_stencils;
}


// Perform sliding window optimization for all functions
class SlidingWindow : public IRMutator {
    const map<string, Function> &env;

    using IRMutator::visit;

    Stmt visit(const Realize *op) override {
        // Find the args for this function
        map<string, Function>::const_iterator iter = env.find(op->name);

        // If it's not in the environment it's some anonymous
        // realization that we should skip (e.g. an inlined reduction)
        if (iter == env.end()) {
            return IRMutator::visit(op);
        }

        // If the Function in question has the same compute_at level
        // as its store_at level, skip it.
        const FuncSchedule &sched = iter->second.schedule();
        if (sched.compute_level() == sched.store_level()) {
            return IRMutator::visit(op);
        }

        Stmt new_body = op->body;

        debug(3) << "Doing sliding window analysis on realization of " << op->name << "\n";
        //std::cout << "Doing sliding window analysis on realization of " << op->name << "\n";

        new_body = SlidingWindowOnFunction(iter->second).mutate(new_body);

        new_body = mutate(new_body);

        if (new_body.same_as(op->body)) {
            return op;
        } else {
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
