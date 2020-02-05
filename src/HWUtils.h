#ifndef HALIDE_HW_UTILS_H
#define HALIDE_HW_UTILS_H

#include <string>
#include <vector>

#include "Bounds.h"
#include "HWStencil.h"
#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "Scope.h"
#include "Simplify.h"

using namespace std;

namespace Halide {
  namespace Internal {

    std::vector<std::string> get_tokens(const std::string &line, const std::string &delimiter);
    int id_const_value(const Expr e);
    std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec);
    bool is_parallelized(const For *op);
    std::string first_for_name(Stmt s);
    bool provide_at_level(Stmt s, std::string var);
    bool call_at_level(Stmt s, std::string var);
    Expr expand_expr(Expr e, const Scope<Expr> &scope);
    Stmt expand_expr(Stmt e, const Scope<Expr> &scope);
    void find_output_scope(Stmt s, Function func,
        LoopLevel compute_level,
        Scope<Expr> &scope);
    Box find_output_bounds(Stmt s, Function func,
        LoopLevel compute_level);
    std::vector<std::string> get_loop_levels_between(Stmt s, Function func,
        LoopLevel outer_level_exclusive,
        LoopLevel inner_level_inclusive,
        bool start_inside = false);

/*
 * Used to calculate input and output block size.
 * Also records the (writer and) reader loopnest.
 */
class CountBufferUsers : public IRVisitor {
  using IRVisitor::visit;
  string var;
  Scope<Expr> scope;
  For *current_for;
  Stmt full_stmt;

  // keep track of lets to later replace variables with constants
  void visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      IRVisitor::visit(op);
  }

  void visit(const ProducerConsumer *op) override {
    if (!op->is_producer) {
      if (provide_at_level(op->body, var)) {
        auto box_write = box_provided(op->body, var);
        auto interval = box_write;
        input_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          input_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        }

      }

      // look at the readers (reader ports, read address gen)
      if (call_at_level(op->body, var)) {
        auto box_read = box_required(op->body, var);
        auto interval = box_read;

        output_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        }
      }
    }
    IRVisitor::visit(op);
  }

  void visit(const For *op) override {
    // add this for loop to read address loop
    For *previous_for = current_for;
    Stmt for_stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, op->body);
    if (current_for == nullptr) {
      full_stmt = for_stmt;

      auto box_read = box_required(op->body, var);
      auto interval = box_read;
      for (size_t dim=0; dim<interval.size(); ++dim) {
        auto assertstmt = AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope)));
      }
    } else if (!is_parallelized(op)) {
      current_for->body = for_stmt;
    }
    current_for = const_cast<For *>(for_stmt.as<For>());

    IRVisitor::visit(op);

    // look at the writers (writer ports)
    if (provide_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_write = box_provided(op->body, var);
      auto interval = box_write;
      input_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        input_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
      }

    }

    // look at the readers (reader ports, read address gen)
    if (call_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_read = box_required(op->body, var);
      auto interval = box_read;

      vector<Stmt> stmts;
      output_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope))));
      }

      const vector<Stmt> &const_stmts = stmts;
      current_for->body = Block::make(const_stmts);
      reader_loopnest = simplify(expand_expr(full_stmt, scope));
    }


    // remove for loop to read address loop
    current_for = previous_for;

  }

public:
  vector<Expr> output_block_box;
  vector<Expr> input_block_box;
  vector<std::string> for_loops;
  Stmt reader_loopnest;

  CountBufferUsers(string v) :
    var(v), current_for(nullptr){}
};

class FindVarStride : public IRVisitor {
  string varname;
  string loopname;
  int cur_stride;
  bool in_var;
  bool in_div;

  using IRVisitor::visit;

  void visit(const Variable *op) override {
    if (op->name == loopname && in_var) {
      stride_for_var = cur_stride;
      is_div = in_div;
      IRVisitor::visit(op);
    }
  }

  void visit(const Call *op) override {
    if (op->name == varname) {
      in_var = true;
      IRVisitor::visit(op);
      in_var = false;
    } else {
      IRVisitor::visit(op);
    }
  }
  void visit(const Div *op) override {
    if (is_const(op->b)) {
      if (in_var) {
        cur_stride *= id_const_value(op->b);
        in_div = true;
        op->a.accept(this);
        in_div = false;
        cur_stride /= id_const_value(op->b);
      } else {
        op->a.accept(this);
      }

    } else {
      IRVisitor::visit(op);
    }
  }

  void visit(const Mul *op) override {
    if (is_const(op->a)) {
      if (in_var) {
        cur_stride *= id_const_value(op->a);
        op->b.accept(this);
        cur_stride /= id_const_value(op->a);
      } else {
        op->b.accept(this);
      }

    } else if (is_const(op->b)) {
      if (in_var) {
        cur_stride *= id_const_value(op->b);
        op->a.accept(this);
        cur_stride /= id_const_value(op->b);
      } else {
        op->a.accept(this);
      }

    } else {
      op->a.accept(this);
      op->b.accept(this);
    }
  }

public:
  int stride_for_var;
  bool is_div;

  FindVarStride(string varname, string forname) :
    varname(varname), loopname(forname),
    cur_stride(1), in_var(false), in_div(false),
    stride_for_var(0), is_div(false) { }
};


class ReplaceForBounds : public IRMutator {
  using IRMutator::visit;
  Scope<Expr> scope;

  Stmt visit(const LetStmt *op) override {
    if (!scope.contains(op->name)) {
      scope.push(op->name, simplify(expand_expr(op->value, scope)));
    } else {
    }
    return IRMutator::visit(op);
  }

  Stmt visit(const For *op) override {
    Expr min = mutate(op->min);
    Expr extent = mutate(op->extent);
    Stmt body = mutate(op->body);

    if (scope.contains(op->name + ".min")) {
      auto new_min = scope.get(op->name + ".min");
      auto new_max = scope.get(op->name + ".max");
      if (!new_min.same_as(min)) {
        min = new_min;
        min = 0;
        extent = 1;
        auto for_stmt = For::make(op->name, std::move(min), std::move(extent),
                                  op->for_type, op->device_api, std::move(body));

        auto return_stmt = LetStmt::make(op->name + ".loop_extent", 1, for_stmt);
        return_stmt = LetStmt::make(op->name + ".loop_max", 0, return_stmt);
        return_stmt = LetStmt::make(op->name + ".loop_min", 0, return_stmt);
        return return_stmt;
      } else {
      }

    } else {
    }


    if (min.same_as(op->min) &&
        extent.same_as(op->extent) &&
        body.same_as(op->body)) {
        return op;
    }
    return For::make(op->name, std::move(min), std::move(extent),
                     op->for_type, op->device_api, std::move(body));

  }

public:
  ReplaceForBounds() {}
};

class FindInputStencil : public IRVisitor {
  using IRVisitor::visit;
  string var;
  Scope<Expr> scope;
  string compute_level;

  // keep track of lets to later replace variables with constants
  void visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      IRVisitor::visit(op);
  }

  void visit(const For *op) override {

    if (op->name == compute_level) {
      auto box_write = box_provided(op->body, var);
      auto interval = box_write;
      input_chunk_box = vector<Expr>(interval.size());

      if (func.name() == var) {
        for (size_t i = 0; i < box_write.size(); i++) {
          string stage_name = func.name() + ".s0." + func.args()[i];
          stencil_bounds.push(stage_name + ".min", box_write[i].min);
          stencil_bounds.push(stage_name + ".max", box_write[i].max);
        }
      }

      output_min_pos_box = vector<Expr>(interval.size());

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        input_chunk_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        output_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, stencil_bounds));
      }

    }
    IRVisitor::visit(op);
  }
public:
  vector<Expr> input_chunk_box;
  vector<Expr> output_min_pos_box;
  bool found_stencil;
  Function func;
  Scope<Expr> &stencil_bounds;
  FindInputStencil(string v, Function func, string cl, Scope<Expr> &stencil_bounds) :
    var(v), compute_level(cl), found_stencil(false), func(func), stencil_bounds(stencil_bounds) {}
};


class FindOutputStencil : public IRVisitor {
  using IRVisitor::visit;
  string var;
  Scope<Expr> scope;
  string compute_level;

  // keep track of lets to later replace variables with constants
  void visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      IRVisitor::visit(op);
  }

  void visit(const For *op) override {

    auto fvs = FindVarStride(var, op->name);
    (op->body).accept(&fvs);
    int stride_for_var = fvs.stride_for_var;

    auto var_tokens = get_tokens(op->name, ".");
    auto varname = var_tokens.size() > 2 ? var_tokens.at(2) : op->name;
    stride_map[varname].stride = std::max(stride_map[varname].stride, stride_for_var);
    stride_map[varname].is_inverse = fvs.is_div;

    if (op->name == compute_level) {

      ReplaceForBounds rfb;
      Stmt new_body = rfb.mutate(op->body);

      auto box_read = box_required(new_body, var);


      auto interval = box_read;
      output_stencil_box = vector<Expr>(interval.size());
      output_min_pos_box = vector<Expr>(interval.size());

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        output_stencil_box[dim] = lower_expr.defined() ? lower_expr : port_expr;
        output_min_pos_box[dim] = interval[dim].min;
      }

    }
    IRVisitor::visit(op);
  }
public:
  vector<Expr> output_stencil_box;
  vector<Expr> output_min_pos_box;
  map<string, Stride> stride_map;
  bool found_stencil;

  FindOutputStencil(string v, string cl) :
    var(v), compute_level(cl), found_stencil(false) {
  }

};

Stmt substituteInConstants(Stmt& s);
Expr expand_expr_no_var(Expr e, const Scope<Expr> &scope);


class IdentifyAddressingVar : public IRVisitor {
  int asserts_found;

  using IRVisitor::visit;

  void visit(const Variable *op) {
    if (op->name == varname) {
      dim_ref = asserts_found;
      stride = 1;
    }
  }

  void visit(const Mul *op) {
    std::cout << "found a multiply: " << Expr(op) << "\n";
    if (const Variable* op_a = op->a.as<Variable>()) {
      if (op_a->name == varname) {
        stride = id_const_value(expand_expr(op->b, scope));
        dim_ref = asserts_found;
      }

    } else if (const Variable* op_b = op->b.as<Variable>()) {
      if (op_b->name == varname) {
        stride = id_const_value(expand_expr(op->a, scope));
        dim_ref = asserts_found;
      }
    }
  }

  void visit(const Div *op) {
    std::cout << "woah a divide: " << Expr(op) << "\n";
  }


  void visit(const AssertStmt *op) {
    IRVisitor::visit(op);
    asserts_found += 1;
  }

public:
  const Scope<Expr> &scope;
  const string varname;


  // if var not found, dim_ref should be 0
  int dim_ref;
  // if var not found, stride should be 0
  int stride;
  IdentifyAddressingVar(string varname, const Scope<Expr> &thescope) :
    asserts_found(0), scope(thescope), varname(varname), dim_ref(0), stride(0) {}

};

class IdentifyAddressing : public IRVisitor {
  //int stream_dim_idx;
  const Scope<Expr> &scope;

  using IRVisitor::visit;

  void visit(const For *op) {
    varnames.push_back(op->name);
    //internal_assert(is_zero(op->min)); FIXME

    // push range
    int range = id_const_value(expand_expr(op->extent, scope));
    //range = range < 0 && op->name=="conv.s1.r$z.r$z" ? 4 : range;
    internal_assert(range > 0) << "the range is " << range << " for " << op->extent << " for " << op->name << "\n";

    auto tokens = get_tokens(op->name, ".");
    auto varname = tokens.size() > 2 ? tokens.at(2) : op->name;
    std::cout << op->name << " is probably referring to storage " << varname << std::endl;

    //uint pos = std::find(storage_names.begin(), storage_names.end(), op->name) - storage_names.begin();

    // determine dim_ref and stride
    //if (pos < storage_names.size()) {
    if (dim_map.count(varname)>0) {
      std::cout << "this is a streaming loop: " << varname << " for " << op->name << "\n";

      cout << "stride_map: ";
      for (const auto& string_int_pair : stride_map) {
        std::cout << string_int_pair.first << "," << string_int_pair.second.stride << "  ";
      }
      std::cout << std::endl;

      std::cout << " and mapcount=" << stride_map.size() << " " << stride_map.count(varname) << std::endl;
      int stride = stride_map.count(varname)>0 ? stride_map.at(varname).stride : 1;
      bool is_inv = stride_map.count(varname)>0 ? stride_map.at(varname).is_inverse : false;
      //int stride = 1;
      std::cout << "stride=" << stride << " is_inv=" << is_inv << std::endl;

      if (is_inv) {
        // push outer loop first
        dim_refs.insert(dim_refs.begin(), dim_map.at(varname));
        strides_in_dim.insert(strides_in_dim.begin(), 1);
        ranges.insert(ranges.begin(), range / stride);
        // push inner loop that has stride=0
        dim_refs.insert(dim_refs.begin(), dim_map.at(varname));
        ranges.insert(ranges.begin(), stride);
        strides_in_dim.insert(strides_in_dim.begin(), 0);

      } else {
        dim_refs.insert(dim_refs.begin(), dim_map.at(varname));
        strides_in_dim.insert(strides_in_dim.begin(), stride);
        ranges.insert(ranges.begin(), range);
      }

      std::cout << op->name << " has stride=" << strides_in_dim.at(0)
                << " dim_ref=" << dim_refs.at(0)
                << " range=" << ranges.at(0) << "\n";

    } else {
      IdentifyAddressingVar iav(op->name, scope);
      std::cout << "finding stride and range for " << op->name
                << "\n in: " << op->body << std::endl;
      (op->body).accept(&iav);
      dim_refs.insert(dim_refs.begin(), iav.dim_ref);
      strides_in_dim.insert(strides_in_dim.begin(), iav.stride);
      ranges.insert(ranges.begin(), range);
      std::cout << op->name << " has stride=" << iav.stride << " dim_ref=" << iav.dim_ref << "\n";
    }

    IRVisitor::visit(op);
  }


public:
  const vector<string> &storage_names;
  const map<string,Stride> &stride_map;
  vector<string> varnames;

  map<string, int> dim_map;

  vector<int> ranges;
  vector<int> dim_refs;
  vector<int> strides_in_dim;
  IdentifyAddressing(const Function& func, const Scope<Expr> &scope, const map<string,Stride> &stride_map) :
    //stream_dim_idx(0), scope(scope), storage_names(func.args()), stride_map(stride_map) {
    scope(scope), storage_names(func.args()), stride_map(stride_map) {
    const auto &sch = func.definition().schedule();
    const auto &splits = sch.splits();

    // populate dim map with names
    for (size_t i=0; i < storage_names.size(); ++i) {
      dim_map[storage_names.at(i)] = i;
      std::cout << "storage name=" << storage_names.at(i) << " has dim_idx=" << i << "\n";
    }

    // add any splits
    for (auto split : splits) {
      std::cout << "remapping " << split.old_var << " with " << split.outer << " and " << split.inner << "\n";
      if (dim_map.count(split.old_var)) {
        dim_map[split.outer] = dim_map.at(split.old_var);
        dim_map[split.inner] = dim_map.at(split.old_var);
      }
    }

    std::cout << "going to be looking for range and strides where storage=" << storage_names << std::endl;
  }
};


    template<typename D, typename R>
      set<D> domain(const std::map<D, R>& m) {
        set<D> d;
        for (auto e : m) {
          d.insert(e.first);
        }
        return d;
      }

  class RealizeFinder : public IRGraphVisitor {
    public:

      using IRGraphVisitor::visit;

      const Realize* r;
      string target;

      RealizeFinder(const std::string& target_) : r(nullptr), target(target_) {}

      void visit(const Realize* rl) override {
        cout << "Searching realize: " << rl->name << " for " << target << endl;
        if (rl->name == target) {
          r = rl;
        } else {
          rl->body->accept(this);
        }
      }
  };

std::string exprString(const Expr e);

    class VarSpec {
      public:
        std::string name;
        Expr min;
        Expr extent;

        bool is_const() const {
          return name == "";
        }

        int const_value() const {
          internal_assert(is_const());
          return id_const_value(min);
        }
    };

    static inline
    bool operator==(const VarSpec& a, const VarSpec& b) {
      if (a.is_const() != b.is_const()) {
        return false;
      }

      if (a.is_const()) {
        return a.const_value() == b.const_value();
      } else {
        return a.name == b.name;
      }
    }

    typedef std::vector<VarSpec> StmtSchedule;

    static inline
    std::ostream& operator<<(std::ostream& out, const VarSpec& e) {
      if (e.name != "") {
        out << e.name << " : [" << e.min << " " << simplify(e.min + e.extent - 1) << "]";
      } else {
        internal_assert(is_const(e.min));
        internal_assert(is_one(e.extent));
        out << e.min;
      }
      return out;
    }

    static inline
    std::ostream& operator<<(std::ostream& out, const StmtSchedule& s) {
      for (auto e : s ) {
        out << e << ", ";
      }
      return out;
    }

  }
}
#endif

