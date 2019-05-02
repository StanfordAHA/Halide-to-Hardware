#include "ExtractHWBuffers.h"
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
using std::vector;
using std::string;

namespace {

// replace an Expr with an int
int id_const_value(const Expr e) {
  if (const IntImm* e_int = e.as<IntImm>()) {
    return e_int->value;

  } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    return e_uint->value;

  } else {
    return -1;
  }
}

// return true if the for loop is not serialized
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

// return if provide creates the var at this level (not recursing into serial loops)
bool provide_at_level(Stmt s, string var = "") {
  ContainsAtLevel cal(var);
  s.accept(&cal);
  return cal.found_provide;
}

// return if call uses the var at this level (not recursing into serial loops)
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

    // remove for loops of length 1
    Stmt visit(const For *old_op) override {
      Stmt s = IRMutator2::visit(old_op);
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

  void visit(const For *op) override {
    if (is_parallelized(op)) {
      auto expanded_extent = expand_expr(op->extent, scope);
    }

    // add this for loop to read address loop
    For *previous_for = current_for;
    Stmt for_stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, op->body);
    bool first_for = false;
    if (current_for == nullptr) {
      first_for = true;
      full_stmt = for_stmt;
      std::cout << "added first for loop: " << op->name << std::endl;
    } else {
      current_for->body = for_stmt;
      std::cout << "added for loop: " << op->name << std::endl;
    }
    current_for = const_cast<For *>(for_stmt.as<For>());

    IRVisitor::visit(op);

    // look at the writers (writer ports)
    if (provide_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_write = box_provided(op->body, var);
      std::cout << "writers inside loop " << op->name << std::endl;
      std::cout << "Box writer found for " << var << " with box " << box_write << std::endl;

      std::cout << "HWBuffer Parameter: writer ports - "
                << "box extent=[";
      auto interval = box_write;
      input_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr lower_expr = find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Lower);
        Expr upper_expr = find_constant_bound(simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope)), Direction::Upper);
        std::cout << lower_expr << "-" << upper_expr << " ";
        input_block_box[dim] = lower_expr;
      }
      std::cout << "]\n";

    }

    // look at the readers (reader ports, read address gen)
    if (call_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_read = box_required(op->body, var);
      std::cout << "readers inside loop " << op->name << std::endl;
      std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
      std::cout << "HWBuffer Parameter: reader ports - "
                << "box extent=[";
      auto interval = box_read;

      vector<Stmt> stmts;
      output_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        output_block_box[dim] = lower_expr;
        stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope))));
        std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
      }
      std::cout << "]\n";

      const vector<Stmt> &const_stmts = stmts;
      current_for->body = Block::make(const_stmts);
      reader_loopnest = simplify(expand_expr(full_stmt, scope));
      std::cout << "HWBuffer Parameter - " << "nested reader loop:\n" << reader_loopnest << std::endl;

    }

    // remove for loop to read address loop
    current_for = previous_for;
    if (!first_for) {
      current_for->body = AssertStmt::make(var, Expr(0));
    }
    
  }
  
public:
  vector<Expr> output_block_box;
  vector<Expr> input_block_box;
  Stmt reader_loopnest;
  CountBufferUsers(string v) : var(v), current_for(nullptr) {}
};

}


// Perform sliding window optimization for all functions
class HWBuffers : public IRMutator2 {
    const map<string, Function> &env;
  
    using IRMutator2::visit;

    Stmt visit(const Realize *op) override {
      std::cout << "checking hwbuffers in realize " << op->name << std::endl;
        // Find the args for this function
        map<string, Function>::const_iterator iter = env.find(op->name);

        // If it's not in the environment it's some anonymous
        // realization that we should skip (e.g. an inlined reduction)
        if (iter == env.end()) {
            return IRMutator2::visit(op);
        }

        // If the Function in question has the same compute_at level
        // as its store_at level, we know this is a double buffer.
        const FuncSchedule &sched = iter->second.schedule();
        if (sched.compute_level() == sched.store_level()) {

          Stmt new_body = op->body;
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          //new_body = SlidingWindowOnFunction(iter->second).mutate(new_body);
          new_body = mutate(new_body);
          
          CountBufferUsers counter(op->name);
          new_body.accept(&counter);

          auto boxes_write = boxes_provided(new_body);
          for (auto box_entry : boxes_write) {
            std::cout << "Box writer found for " << box_entry.first << " with box " << box_entry.second << std::endl;
          }
          auto boxes_read = boxes_required(new_body);
          for (auto box_entry : boxes_read) {
            std::cout << "Box reader found for " << box_entry.first << " with box " << box_entry.second << std::endl;
          }

          // extent is the same for total buffer box, input chunk, and output stencil for double buffer
          vector<Expr> box(boxes_read[op->name].size());
          for (size_t i=0; i<boxes_read[op->name].size(); ++i) {
            Expr extent = simplify(boxes_read[op->name][i].max - boxes_read[op->name][i].min + 1);
            box[i] = extent;
          }

          std::cout << "HWBuffer Parameter: Total buffer box is of size [";          
          for (size_t i=0; i<box.size(); ++i) {
            std::cout << box[i];
            if (i < box.size() - 1) {
              std::cout << " ";
            }
          }
          std::cout << "]\n";

          std::cout << "HWBuffer Parameter: Input Chunk box is of size [";
          for (size_t i=0; i<box.size(); ++i) {
            std::cout << box[i];
            if (i < box.size() - 1) {
              std::cout << " ";
            }
          }
          std::cout << "]\n";
          
          std::cout << "HWBuffer Parameter: Output Stencil box is of size [";
          for (size_t i=0; i<box.size(); ++i) {
            std::cout << box[i];
            if (i < box.size() - 1) {
              std::cout << " ";
            }
          }
          std::cout << "]\n";
          
          //std::cout << "HWBuffer Parameter: " << op->name << " has readers=" << num_readers
          //          << " writers=" << num_writers << std::endl;
          std::cout << "new body with sliding for " << op->name << "\n"
                    << "old body: \n" << op->body
                    << "new body: \n" << new_body << '\n';

          return IRMutator2::visit(op);
          
        } else {
          // look for a sliding window that can be used in a line buffer
          Stmt new_body = op->body;

          std::cout << "Doing sliding window analysis on realization of " << op->name << "\n";
          // Parameters 1 and 2
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          //new_body = SlidingWindowOnFunction(iter->second).mutate(new_body);
          new_body = mutate(new_body);

          CountBufferUsers counter(op->name);
          new_body.accept(&counter);
          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          auto input_block_box = counter.output_block_box;
          auto reader_loopnest = counter.reader_loopnest;

          //auto boxes_write = boxes_provided(new_body);
          //for (auto box_entry : boxes_write) {
          //  std::cout << "Box writer found for " << box_entry.first << " with box " << box_entry.second << std::endl;
          //}
          auto boxes_read = boxes_required(new_body);
          //for (auto box_entry : boxes_read) {
          //  std::cout << "Box reader found for " << box_entry.first << " with box " << box_entry.second << std::endl;
          //}

          std::cout << "HWBuffer Parameter: " << op->name << " has Total box of size [";
          size_t i_max = boxes_read[op->name].size();
          // Parameter 6
          auto total_buffer_box = vector<Expr>(i_max);
          for (size_t i=0; i<i_max; ++i) {
            Expr extent = simplify(boxes_read[op->name][i].max - boxes_read[op->name][i].min + 1);
            total_buffer_box.at(i) = extent;

            if (i < i_max - 1) {
              std::cout << extent << " ";
            }
          }
          std::cout << "]\n";

          HWBuffer hwbuffer;
          hwbuffer.name = op->name;
          hwbuffer.total_buffer_box = total_buffer_box;
          hwbuffer.input_chunk_box = sliding_stencil_map[hwbuffer.name].input_chunk_box;
          hwbuffer.input_block_box = input_block_box;
          hwbuffer.output_stencil_box = sliding_stencil_map[hwbuffer.name].output_stencil_box;
          hwbuffer.output_block_box = output_block_box;
          hwbuffer.output_access_pattern = reader_loopnest;
          
          //std::cout << "new body with sliding for " << op->name << "\n"
          //          << "old body: \n" << op->body
          //          << "new body: \n" << new_body << '\n';
          
          return Realize::make(op->name, op->types, op->memory_type,
                               op->bounds, op->condition, new_body);
        }
    }
  
public:
    HWBuffers(const map<string, Function> &e) : env(e) {}

};

Stmt extract_hw_buffers(Stmt s, const map<string, Function> &env) {
    return HWBuffers(env).mutate(s);
}

}  // namespace Internal
}  // namespace Halide
