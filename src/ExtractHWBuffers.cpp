#include "ExtractHWBuffers.h"
#include "SlidingWindow.h"

#include "Bounds.h"
#include "Debug.h"
#include "Func.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "IRPrinter.h"
#include "Monotonic.h"
#include "Scope.h"
#include "Simplify.h"
#include "Substitute.h"
//#include "Var.h"

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

class FirstForName : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const For *op) {
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

class ContainsAtLevel : public IRVisitor {
  using IRVisitor::visit;
  
  void visit(const For *op) {
    // don't recurse if not parallelized
    if (is_parallelized(op) || is_one(op->extent)) {
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

  void visit(const ProducerConsumer *op) {
    if (!op->is_producer) {
    //if (false) {
      // look at the writers (writer ports)
      if (provide_at_level(op->body, var)) {
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
      if (call_at_level(op->body, var)) {
        auto box_read = box_required(op->body, var);
        std::cout << "readers inside loop " << op->name << std::endl;
        std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
        std::cout << "HWBuffer Parameter: reader ports - "
                  << "box extent=[";
        auto interval = box_read;

        output_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          output_block_box[dim] = lower_expr;
          std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        }
        std::cout << "]\n";
      }
    }
    IRVisitor::visit(op);
  }

  void visit(const For *op) override {
    if (is_parallelized(op)) {
      auto expanded_extent = expand_expr(op->extent, scope);
    }

    // add this for loop to read address loop
    For *previous_for = current_for;
    Stmt for_stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, op->body);
    if (current_for == nullptr) {
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
    Scope<Expr> scope;

    using IRMutator2::visit;
  
    Stmt visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      return IRMutator2::visit(op);
    }

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

        if (!sched.is_hw_kernel()) {
          std::cout << "skipping non-hwkernel realize " << op->name << std::endl;
          return IRMutator2::visit(op);
        }

        std::cout << "creating buffer for realize " << op->name << std::endl;

          
        //if (sched.compute_level() == sched.store_level()) {
        if (sched.compute_level() == sched.store_level()) {

          Stmt new_body = op->body;
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          //new_body = SlidingWindowOnFunction(iter->second).mutate(new_body);
          new_body = mutate(new_body);
          
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
            //Expr extent = simplify(boxes_read[op->name][i].max - boxes_read[op->name][i].min + 1);
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
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

          CountBufferUsers counter(op->name);
          new_body.accept(&counter);
          std::cout << "counted: " << new_body << std::endl;
          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          std::cout << "output block: " << output_block_box << std::endl;
          auto input_block_box = counter.input_block_box;
          auto reader_loopnest = counter.reader_loopnest;

          std::cout << "transferring parameters\n";
          
          HWBuffer hwbuffer(output_block_box.size());
          hwbuffer.name = op->name;

          internal_assert(boxes_read.at(op->name).size() >= output_block_box.size());
          //hwbuffer.dims = vector<BufferDimSize>(output_block_box.size());
          for (size_t i = 0; i < output_block_box.size(); ++i) {
            hwbuffer.dims.at(i).logical_min = Expr(0); // FIXMEyikes
            hwbuffer.dims[i].logical_size = box.at(i);
            hwbuffer.dims[i].input_chunk = box.at(i);
            hwbuffer.dims[i].input_block = input_block_box.at(i);
            hwbuffer.dims[i].output_stencil = box.at(i);
            hwbuffer.dims[i].output_block = output_block_box.at(i);
            hwbuffer.dims[i].output_min_pos = boxes_read.at(op->name)[i].min;
            std::cout << "hwbuffer " << hwbuffer.name << " in dim " << i <<
              " has min_pos=" << hwbuffer.dims[i].output_min_pos << std::endl;
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          std::cout << "created hwbuffer\n";

          if (buffers.count(hwbuffer.name) == 0) {
            std::cout << "Here is the hwbuffer (store=compute):"
                      << " [" << buffers.count(hwbuffer.name) << "]\n"
                      << hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }
          
          //std::cout << "HWBuffer Parameter: " << op->name << " has readers=" << num_readers
          //          << " writers=" << num_writers << std::endl;
          //std::cout << "new body with sliding for " << op->name << "\n"
          //          << "old body: \n" << op->body
          //          << "new body: \n" << new_body << '\n';

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
          auto input_block_box = counter.input_block_box;
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
            //Expr extent = simplify(boxes_read[op->name][i].max - boxes_read[op->name][i].min + 1);
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            total_buffer_box[i] = extent;

            if (i < i_max - 1) {
              std::cout << extent << " ";
            }
          }
          std::cout << "]\n";

          std::string for_name = first_for_name(new_body);
          
          HWBuffer hwbuffer;//(sliding_stencil_map.at(for_name).input_chunk_box.size());
          hwbuffer.dims = vector<BufferDimSize>(output_block_box.size());
          hwbuffer.name = op->name;
          for (size_t i = 0; i < output_block_box.size(); ++i) {
            std::cout << "hwbuffer " << hwbuffer.name << " in begin " << i << " here\n";
            hwbuffer.dims[i].logical_size = total_buffer_box.at(i);
            hwbuffer.dims[i].logical_min = Expr(0);
            hwbuffer.dims[i].input_chunk = sliding_stencil_map.at(for_name).input_chunk_box.at(i);
            hwbuffer.dims[i].input_block = input_block_box.at(i);
            std::cout << "hwbuffer " << hwbuffer.name << " in dim " << i << " here\n";
            hwbuffer.dims[i].output_stencil = sliding_stencil_map.at(for_name).output_stencil_box.at(i);
            hwbuffer.dims[i].output_block = output_block_box.at(i);
            hwbuffer.dims[i].output_min_pos = sliding_stencil_map.at(for_name).output_min_pos.at(i);
            std::cout << "hwbuffer " << hwbuffer.name << " in dim " << i <<
              " has min_pos=" << hwbuffer.dims[i].output_min_pos << std::endl;

          }
          hwbuffer.output_access_pattern = reader_loopnest;

          if (buffers.count(hwbuffer.name) == 0) {          
            std::cout << "Here is the hwbuffer:"
                      << " [" << buffers.count(hwbuffer.name) << "]\n" 
                      << hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }

          std::cout << "map is ";
          for (auto pair : sliding_stencil_map) {
            std::cout << pair.first << ", ";
          }
          std::cout << std::endl;

          
          //std::cout << "new body with sliding for " << op->name << "\n"
          //          << "old body: \n" << op->body
          //          << "new body: \n" << new_body << '\n';
          
          return Realize::make(op->name, op->types, op->memory_type,
                               op->bounds, op->condition, new_body);
        }
    }
  
public:
    HWBuffers(const map<string, Function> &e) : env(e) {}
    std::map<std::string, HWBuffer> buffers;
    
};

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

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
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


std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer) {
  vector<Expr> total_buffer_box, input_chunk_box, input_block_box;
  vector<Expr> output_stencil_box, output_block_box;
  for (const auto dim : buffer.dims) {
    total_buffer_box.emplace_back(dim.logical_size);
    input_chunk_box.emplace_back(dim.input_chunk);
    input_block_box.emplace_back(dim.input_block);
    output_stencil_box.emplace_back(dim.output_stencil);
    output_block_box.emplace_back(dim.output_block);
  }
  
  os << "HWBuffer: " << buffer.name << std::endl
     << "Logical Buffer: " << total_buffer_box << std::endl
     << "Input Chunk: " << input_chunk_box << std::endl
     << "Input Block: " << input_block_box << std::endl
     << "Output Stencil: " << output_stencil_box << std::endl
     << "Output Block: " << output_block_box << std::endl
     << "Output Access Pattern:\n " << buffer.output_access_pattern << std::endl
     << "is_inline=" << buffer.is_inlined << std::endl
     << "is_output=" << buffer.is_output << std::endl;

  
  return os;
};


//Stmt extract_hw_buffers(Stmt s, const map<string, Function> &env) {
//    return HWBuffers(env).mutate(s);
//}

map<string, HWBuffer> extract_hw_buffers(Stmt s, const map<string, Function> &env) {
    HWBuffers ehb(env);
    ehb.mutate(s);


    
  for (auto hwbuffer : ehb.buffers) {
    std::cout << hwbuffer.first << " is ehb w/ inline=" << hwbuffer.second.is_inlined << std::endl;
    std::cout << hwbuffer.second << std::endl;
  }

    
    return ehb.buffers;
}


// Because storage folding runs before simplification, it's useful to
// at least substitute in constants before running it, and also simplify the RHS of Let Stmts.
class SubstituteInConstants : public IRMutator2 {
    using IRMutator2::visit;

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

class FindInnerLoops : public IRVisitor {
  Function func;

  LoopLevel outer_loop_exclusive;
  LoopLevel inner_loop_inclusive;
  bool in_inner_loops;

  using IRVisitor::visit;

  void visit(const ProducerConsumer *op) {
    // match store level at PC node in case the looplevel is outermost
    if (outer_loop_exclusive.lock().func() == op->name &&
        outer_loop_exclusive.lock().var().name() == Var::outermost().name()) {
      in_inner_loops = true;
    }
    IRVisitor::visit(op);
  }

  void visit(const For *op) {
    // scan loops are loops between the outer store level (exclusive) and
    // the inner compute level (inclusive) of the accelerated function
    if (in_inner_loops && starts_with(op->name, func.name() + ".")) {
      debug(3) << "added loop " << op->name << " to inner loops.\n";
      inner_loops.insert(op->name);
      //loop_mins[op->name] = op->min;
      //loop_maxes[op->name] = simplify(op->min + op->extent - 1);
      std::cout << "added loop to scan loop named " << op->name << " with extent=" << op->extent << std::endl;
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
  FindInnerLoops(Function f, LoopLevel outer_level, LoopLevel inner_level)
    : func(f), outer_loop_exclusive(outer_level), inner_loop_inclusive(inner_level),
      in_inner_loops(false) { }

  set<string> inner_loops;
  
};

set<string> get_loop_levels_between(Stmt s, Function func,
                                    LoopLevel outer_level_exclusive,
                                    LoopLevel inner_level_inclusive) {
  FindInnerLoops fil(func, outer_level_exclusive, inner_level_inclusive);
  s.accept(&fil);
  return fil.inner_loops;
}

void set_opt_params(HWXcel *xcel, 
                    const map<string, Function> &env,
                    const vector<BoundsInference_Stage> &inlined_stages) {

  auto &hwbuffers = xcel->hwbuffers;
  size_t i = inlined_stages.size() - 1 + 1;

  bool in_output = true;
  // scan through each stage before the output stage

  while (i >= 1) {
    i--;
    const BoundsInference_Stage &stage = inlined_stages[i];
  
  //for (auto &hwbuffer_pair : hwbuffers) {
    //auto &hwbuffer = hwbuffer_pair.second;

    // Function func;
    //const BoundsInference_Stage &stage = std::find(inlined_stages.begin(), inlined_stages.end(), hwbuffer.name);


    // inlined stages
    for (const auto &hwbuffer_pair : hwbuffers) {
      std::cout << hwbuffer_pair.second << ",";
    }
    std::cout << std::endl;
    std::cout << "there are " << hwbuffers.size() << " hwbuffers\n";
    std::cout << "looking for " << stage.name << std::endl;

    if (in_output && inlined_stages[i].name != xcel->name) {
      continue;
    }

    
    auto iterator = std::find_if(hwbuffers.begin(), hwbuffers.end(), [stage](const std::pair<std::string, HWBuffer>& buffer_pair){
        if (stage.name == buffer_pair.first)
          return true;
        return false;
      });
    internal_assert(iterator != hwbuffers.end());
    auto &hwbuffer = iterator->second;
    internal_assert(hwbuffer.name == iterator->first);
    
    std::cout << hwbuffer.name << " before func\n";
    //Function cur_func = env.at(hwbuffer.name);

    //Function cur_func_to_copy = env.at(hwbuffer.name);
    //FunctionPtr cur_func_ptr;
    //std::map<FunctionPtr, FunctionPtr> copied_map;
    //cur_func_to_copy.deep_copy(hwbuffer.name, cur_func_ptr, copied_map);
    //Function cur_func = Function(cur_func_ptr);

    FunctionPtr func_ptr =  env.at(hwbuffer.name).get_contents();
    Function cur_func = Function(func_ptr);
    
    hwbuffer.func = cur_func;
    if (!cur_func.schedule().is_hw_kernel()) {
      std::cout << "skipping " << hwbuffer.name << std::endl;
      continue;
    }
    
     // bool is_output;
    if (xcel->name == hwbuffer.name) {
      hwbuffer.is_output = true;
    }

    if (in_output) {
      if (inlined_stages[i].name == xcel->name) {
        in_output = false;
      }
      continue;
    }

    
      
    // bool is_inlined;
    if (xcel->input_streams.count(stage.name) ||
        (cur_func.schedule().compute_level().match(xcel->compute_level) &&
         xcel->store_level == cur_func.schedule().store_level())) {
      hwbuffer.is_inlined = false;
    } else {
      hwbuffer.is_inlined = true;
    }
    std::cout << hwbuffer.name << " is inline=" << hwbuffer.is_inlined
              << " store=" << cur_func.schedule().store_level()
              << " compute=" << cur_func.schedule().compute_level()
              << " is_xcel=" << cur_func.schedule().is_accelerated()
              << " is_hw_kernel=" << cur_func.schedule().is_hw_kernel()
              << std::endl;

    
    
    for (size_t j = 0; j < stage.consumers.size(); j++) {
      const BoundsInference_Stage &consumer = inlined_stages[stage.consumers[j]];
      std::cout << consumer.name << ",";
    }
    std::cout << std::endl;

    
    std::cout << "finished is_output " << stage.consumers.size() << " consumers\n";
    // std::map<std::string, HWBuffer&> consumer_buffers;   // used for transforming call nodes and inserting dispatch calls
    for (size_t j = 0; j < stage.consumers.size(); j++) {
      internal_assert(stage.consumers[j] < (int)inlined_stages.size());
      const BoundsInference_Stage &consumer = inlined_stages[stage.consumers[j]];
      std::cout << "right before " << consumer.name << " consumer of " << hwbuffer.name << "\n";

      string consumer_name;
      if (consumer.name != hwbuffer.name) {
        const HWBuffer &consumer_buffer = hwbuffers.at(consumer.name);
        if (consumer_buffer.is_inlined) {
          internal_assert(consumer_buffer.consumer_buffers.size() == 1)
            << "The inlined kernel " << consumer.name << " has more than one consumer.\n";
          consumer_name = consumer_buffer.consumer_buffers.begin()->first;
        } else {
          consumer_name = consumer_buffer.name;
        }
      } else {
        consumer_name = hwbuffer.name;
      }

      std::cout << "for kernel " << hwbuffer.name << ", adding consumer "
                << consumer_name << " based on kernel " << consumer.name << std::endl;
      hwbuffer.consumer_buffers[consumer_name] = &hwbuffers.at(consumer.name);

      std::cout << "right before " << consumer.name << " inputs\n";
    // std::vector<std::string> input_streams;  // used when inserting read_stream calls      
      if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name)) {
        hwbuffers.at(consumer.name).input_streams.push_back(hwbuffer.name);
      }
    }

  }
}

void extract_hw_xcel_top_parameters(Stmt s, Function func,
                                      const map<string, Function> &env,
                                      const vector<BoundsInference_Stage> &inlined,
                                      HWXcel *xcel) {
  xcel->name = func.name();
  xcel->store_level = func.schedule().accelerate_store_level();
  xcel->compute_level = func.schedule().accelerate_compute_level();
  xcel->streaming_loop_levels = get_loop_levels_between(s, func, xcel->store_level, xcel->compute_level);
  xcel->input_streams = func.schedule().accelerate_inputs();
  xcel->hwbuffers = extract_hw_buffers(s, env);

  set_opt_params(xcel, env, inlined);

  for (auto &hwbuffer_pair : xcel->hwbuffers) {
    std::cout << hwbuffer_pair.first << " is extracted w/ inline=" << hwbuffer_pair.second.is_inlined
              << " and num_dims=" << hwbuffer_pair.second.dims.size() << std::endl;
  }

}

vector<HWXcel> extract_hw_accelerators(Stmt s, const map<string, Function> &env,
                                const vector<BoundsInference_Stage> &inlined_stages) {

  vector<HWXcel> xcels;
  
  s = SubstituteInConstants().mutate(s);
  
  for (auto stage : inlined_stages) {
    //std::cout << "stage includes " << stage.name << std::endl;
  }
    
  // for each accelerated function, build a hardware xcel: a dag of HW kernels 
  for (const auto &p : env) {
    Function func = p.second;
    //std::cout << "Found function " << func.name() << "\n";

    // skip this function if it is not accelerated
    if(!func.schedule().is_accelerated())
      continue;
    
    std::cout << "Found accelerate function " << func.name() << "\n";
    LoopLevel store_locked = func.schedule().store_level().lock();
    string store_varname =
      store_locked.is_root() ? "root" :
      store_locked.is_inlined() ? "inlined" :
      store_locked.var().name();
    if (!store_locked.defined() || !starts_with(func.name(), "hw_output")) {
      //continue;
    }
    debug(3) << "Found accelerate function " << func.name() << "\n";
    std::cout << "Found accelerate function " << func.name() << "\n";
    debug(3) << store_locked.func() << " " << store_varname << "\n";
    HWXcel xcel;
    extract_hw_xcel_top_parameters(s, func, env, inlined_stages, &xcel);
    xcels.push_back(xcel);
  }
  return xcels;
}


}  // namespace Internal
}  // namespace Halide
