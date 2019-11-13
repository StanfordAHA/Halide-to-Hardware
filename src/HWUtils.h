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
      //std::cout << "found loop=" << op->name << " and setting stride=" << cur_stride << std::endl;
      stride_for_var = cur_stride;
      is_div = in_div;
      IRVisitor::visit(op);
    }
  }
  
  void visit(const Call *op) override {
    if (op->name == varname) {
      in_var = true;
      // std::cout << "call for " << op->name << " includes: " << op->args << std::endl;
      IRVisitor::visit(op);
      in_var = false;
    } else {
      IRVisitor::visit(op);
    }
  }
  void visit(const Div *op) override {
    //std::cout << "woah, dis a divide: " << Expr(op) << std::endl;
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
    //std::cout << "this is a multiply: " << Expr(op) << std::endl;
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
    //if (true) {
      //ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      scope.push(op->name, simplify(expand_expr(op->value, scope)));
      //std::cout << "binded scope: " << op->name << " set to " << scope.get(op->name) << std::endl;
    } else {
      //std::cout << "already have " << op->name << " set to " << scope.get(op->name) << std::endl;
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
        //std::cout << op->name << " replaced this min from " << min << " to " << new_min << std::endl;
        min = new_min;
        min = 0;
        //extent = simplify(extent - new_min);
        extent = 1;
        auto for_stmt = For::make(op->name, std::move(min), std::move(extent),
                                  op->for_type, op->device_api, std::move(body));

        //auto return_stmt = LetStmt::make(op->name + ".loop_extent", simplify(new_max-new_min+1), for_stmt);
        //return_stmt = LetStmt::make(op->name + ".loop_max", new_max, return_stmt);
        //return_stmt = LetStmt::make(op->name + ".loop_min", new_min, return_stmt);
        auto return_stmt = LetStmt::make(op->name + ".loop_extent", 1, for_stmt);
        return_stmt = LetStmt::make(op->name + ".loop_max", 0, return_stmt);
        return_stmt = LetStmt::make(op->name + ".loop_min", 0, return_stmt);
        return return_stmt;
      } else {
        //std::cout << op->name << " same replaced this min from " << min << " to " << new_min << std::endl;
      }
      
    } else {
      //std::cout << "couldn't find for loop " << op->name + ".min\n";
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
  Function func;
  FindOutputStencil(string v, Function func, string cl) :
    var(v), compute_level(cl), found_stencil(false), func(func) {
  }
};
Stmt substituteInConstants(Stmt& s);
  }
}
#endif

