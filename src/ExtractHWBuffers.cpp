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

namespace Halide {
namespace Internal {

using namespace std;

using std::map;
using std::vector;
using std::string;

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

std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer) {
  vector<Expr> total_buffer_box, input_chunk_box, input_block_box;
  vector<Expr> output_stencil_box, output_block_box, output_min_pos;
  for (const auto dim : buffer.ldims) {
    total_buffer_box.emplace_back(dim.logical_size);
  }
  
  for (const auto dim : buffer.dims) {
    input_chunk_box.emplace_back(dim.input_chunk);
    input_block_box.emplace_back(dim.input_block);
    output_stencil_box.emplace_back(dim.output_stencil);
    output_block_box.emplace_back(dim.output_block);
    output_min_pos.emplace_back(dim.output_min_pos);
  }

  //auto num_inputs = 0;//buffer.func.updates().size();
  //auto num_outputs = 0;//buffer.consumer_buffers.size();
  
  os << "HWBuffer: " << buffer.name << std::endl
     << "Logical Buffer: " << total_buffer_box << std::endl
     << "Input Chunk: " << input_chunk_box << std::endl
     << "Input Block: " << input_block_box << std::endl
     << "Output Stencil: " << output_stencil_box << std::endl
     << "Output Block: " << output_block_box << std::endl
     << "Output Access Pattern:\n " << buffer.output_access_pattern << std::endl
     << "Output Min Pos:\n " << output_min_pos << std::endl
     << "streaming loops:\n " << buffer.streaming_loops << std::endl
     << "compute level: " << buffer.compute_level << std::endl
     << "store level: " << buffer.store_level << std::endl
     << "is_inline=" << buffer.is_inlined << std::endl
     << "is_output=" << buffer.is_output << std::endl;
  //<< "num_inputs=" << num_inputs << std::endl
  //<< "num_output=" << num_outputs << std::endl;

  
  return os;
};

std::vector<MergedDimSize> create_hwbuffer_sizes(std::vector<int> logical_size,
                                                 std::vector<int> output_stencil, std::vector<int> output_block,
                                                 std::vector<int> input_chunk, std::vector<int> input_block) {
   internal_assert(logical_size.size() == output_stencil.size());
   internal_assert(logical_size.size() == output_block.size());
   internal_assert(logical_size.size() == input_chunk.size());
   internal_assert(logical_size.size() == input_block.size());

   std::vector<MergedDimSize> dims(logical_size.size());

   for (size_t i=0; i < logical_size.size(); ++i) {
     dims[i] = MergedDimSize({"loop" + std::to_string(i), Expr(logical_size.at(i)), Expr(0),
           Expr(input_chunk.at(i)), Expr(input_block.at(i)),
           Expr(output_stencil.at(i)), Expr(output_block.at(i)), Expr(0)});
   }
   
   return dims;
}

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

namespace {

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
    //if (false) {
      // look at the writers (writer ports)
      if (provide_at_level(op->body, var)) {
        auto box_write = box_provided(op->body, var);
        //std::cout << "writers inside pc " << op->name << std::endl;
        //std::cout << "Box writer found for " << var << " with box " << box_write << std::endl;

        //std::cout << "HWBuffer Parameter: writer ports - "
                  //<< "box extent=[";
        auto interval = box_write;
        input_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          //std::cout << lower_expr << "-" << upper_expr << " ";
          input_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        }
        //std::cout << "]\n";

      }

      // look at the readers (reader ports, read address gen)
      if (call_at_level(op->body, var)) {
        auto box_read = box_required(op->body, var);
        //std::cout << "readers inside pc " << op->name << std::endl;
        //std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
        //std::cout << Stmt(op->body) << std::endl;
        //std::cout << "HWBuffer Parameter: reader ports - "
                  //<< "box extent=[";
        auto interval = box_read;

        output_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
          //std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        }
        //std::cout << "]\n";
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
      //std::cout << "added first for loop: " << op->name << std::endl;

      auto box_read = box_required(op->body, var);
      auto interval = box_read;
      for (size_t dim=0; dim<interval.size(); ++dim) {
        auto assertstmt = AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope)));
        //std::cout << "min pos in dim " << dim << ":" << assertstmt;
      }
    } else if (!is_parallelized(op)) {
      current_for->body = for_stmt;
      //std::cout << "added for loop: " << op->name << std::endl;
    }
    current_for = const_cast<For *>(for_stmt.as<For>());

    IRVisitor::visit(op);

    // look at the writers (writer ports)
    if (provide_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_write = box_provided(op->body, var);
      //std::cout << "writers inside loop " << op->name << std::endl;
      //std::cout << "Box writer found for " << var << " with box " << box_write << std::endl;

      //std::cout << "HWBuffer Parameter: writer ports - "
                //<< "box extent=[";
      auto interval = box_write;
      input_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        //std::cout << lower_expr << "-" << upper_expr << " ";
        input_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
      }
      //std::cout << "]\n";

    }

    // look at the readers (reader ports, read address gen)
    if (call_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_read = box_required(op->body, var);
      //std::cout << "readers inside loop " << op->name << std::endl;
      //std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
      //std::cout << Stmt(op->body) << std::endl;
      //std::cout << "HWBuffer Parameter: reader ports - "
                //<< "box extent=[";
      auto interval = box_read;

      vector<Stmt> stmts;
      output_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope))));
        //std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
      }
      //std::cout << "]\n";

      const vector<Stmt> &const_stmts = stmts;
      current_for->body = Block::make(const_stmts);
      reader_loopnest = simplify(expand_expr(full_stmt, scope));
      //std::cout << "HWBuffer Parameter - " << var << " nested reader loop:\n" << reader_loopnest << std::endl;

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
    //std::cout << "saw this for loop " << op->name << " while compute=" << compute_level << std::endl;

    auto fvs = FindVarStride(var, op->name);
    (op->body).accept(&fvs);
    int stride_for_var = fvs.stride_for_var;

    auto var_tokens = get_tokens(op->name, ".");
    auto varname = var_tokens.size() > 2 ? var_tokens.at(2) : op->name;
    stride_map[varname].stride = std::max(stride_map[varname].stride, stride_for_var);
    stride_map[varname].is_inverse = fvs.is_div;
    
    //std::cout << op->name << " has stride=" << stride_for_var << " in call for " << var
    //          << " stride_map[" << varname << "]=" << stride_map[varname].stride << std::endl;
    //std::cout << op->body << std::endl;


    if (op->name == compute_level) {
      //std::cout << var << " found compute level with for loop " << op->name << std::endl;

      ReplaceForBounds rfb;
      Stmt new_body = rfb.mutate(op->body);

      //std::cout << Stmt(new_body);

      //if (scope.contains(op->name + ".min")) {
      //  std::cout << "loop min should be " << scope.get(op->name + ".min") << std::endl;
      //} else {
      //  std::cout << "couldn't find a loop min\n";
      //}
      
      //std::cout << op->body << std::endl;
      auto box_read = box_required(new_body, var);

      //Box box = box_provided(new_body, var);
      // std::cout << "let's save this output box num_dims=" << func.dimensions() << " box=" << box.size() << "\n";
      // // save the bounds values in scope
      // Scope<Expr> stencil_bounds;
      // if (func.name() == var) {
      //   for (int i = 0; i < func.dimensions(); i++) {
      //     string stage_name = func.name() + ".s0." + func.args()[i];
      //     stencil_bounds.push(stage_name + ".min", box[i].min);
      //     stencil_bounds.push(stage_name + ".max", box[i].max);
      //   }
      // }
      // std::cout << "box saved\n";

      auto interval = box_read;
      output_stencil_box = vector<Expr>(interval.size());
      output_min_pos_box = vector<Expr>(interval.size());

      //std::cout << "HWBuffer Parameter: " << var << " output stencil size - "
                //<< "box extent=[";

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        output_stencil_box[dim] = lower_expr.defined() ? lower_expr : port_expr;
        //std::cout << port_expr << "?" << lower_expr.defined() << ":" << lower_expr << "-" << upper_expr  << " ";
        //output_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, scope));
        output_min_pos_box[dim] = interval[dim].min;
        //std::cout << "(" << output_min_pos_box[dim] << "," << interval[dim].min << ")  ";
        //std::cout << " using " << interval[dim].max - interval[dim].min + 1 << std::endl;
      }
      //std::cout << "]\n";

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
    //std::cout << "looking to find " << v << " output stencil where compute_level=" << compute_level << std::endl;
  }
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
    //std::cout << "saw this for loop " << op->name << " while compute=" << compute_level << std::endl;

    if (op->name == compute_level) {
      //std::cout << op->body << std::endl;
      auto box_write = box_provided(op->body, var);
      auto interval = box_write;
      input_chunk_box = vector<Expr>(interval.size());

      //std::cout << "let's save this input box num_dims=" << func.dimensions() << " box=" << box_write.size() << "\n";
      // save the bounds values in scope
      //Scope<Expr> stencil_bounds;
      //std::cout << "doing findinputstencil for " << var << " " << stencil_bounds << std::endl;
      //std::cout << op->body << std::endl;
      if (func.name() == var) {
        for (size_t i = 0; i < box_write.size(); i++) {
          string stage_name = func.name() + ".s0." + func.args()[i];
          stencil_bounds.push(stage_name + ".min", box_write[i].min);
          stencil_bounds.push(stage_name + ".max", box_write[i].max);
          //std::cout << "this is stage: " << stage_name << " " << stencil_bounds << std::endl;
        }
      }

      output_min_pos_box = vector<Expr>(interval.size());
      
      //std::cout << "HWBuffer Parameter: " << var << " input chunk size - "
                //<< "box extent=[";

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        input_chunk_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        //std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        output_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, stencil_bounds));
        //std::cout << "(" << output_min_pos_box[dim] << "," << interval[dim].min << ")  ";
      }
      //std::cout << "]\n";

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

class ReplaceOutputAccessPatternRanges : public IRMutator {
  using IRMutator::visit;
  int count;
  int max_count;
  const HWBuffer& kernel;

  Stmt visit(const For *old_op) override {
    Expr new_extent;
    if (count < max_count) {
      //new_extent = kernel.dims.at(count).logical_size;
      new_extent = kernel.ldims.at(count).logical_size;
      count += 1;
    } else {
      new_extent = old_op->extent;
    }

    Stmt s = IRMutator::visit(old_op);
    const For *op = s.as<For>();
    Stmt for_stmt = For::make(op->name, op->min, new_extent, op->for_type, op->device_api, op->body);
    
    return for_stmt;
  }
  

public:
  ReplaceOutputAccessPatternRanges(const HWBuffer& hwkernel) :
    count(0), max_count(hwkernel.dims.size()), kernel(hwkernel) { }
};


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
                                       bool start_inside = false) {
  FindInnerLoops fil(func, outer_level_exclusive, inner_level_inclusive, start_inside);
  s.accept(&fil);
  //std::cout << "got some loops: " << fil.inner_loops << std::endl;
  return fil.inner_loops;
}

}

class HWBuffers : public IRMutator {
    const map<string, Function> &env;
    Scope<Expr> scope;

    using IRMutator::visit;
  
    Stmt visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      return IRMutator::visit(op);
    }

    Stmt visit(const Realize *op) override {
        // Find the args for this function
        map<string, Function>::const_iterator iter = env.find(op->name);

        // If it's not in the environment it's some anonymous
        // realization that we should skip (e.g. an inlined reduction)
        if (iter == env.end()) {
            return IRMutator::visit(op);
        }

        // find the function
        const FuncSchedule &sched = iter->second.schedule();
        if (!sched.is_hw_kernel()) {
          //std::cout << "skipping non-hwkernel realize " << op->name << std::endl;
          return IRMutator::visit(op);
        }
        Function func = iter->second;

        // create the hwbuffer
        HWBuffer hwbuffer;
        hwbuffer.name = op->name;
        hwbuffer.func = func;
        hwbuffer.my_stmt = op->body;
        Stmt new_body = op->body;

        // If the Function in question has the same compute_at level
        // as its store_at level, we know this is a double buffer.
        string xcel_compute_level = loop_names.at(loop_names.size()-1);
        string xcel_store_level = loop_names.at(0);

        // define the store and compute levels
        LoopLevel store_locked = func.schedule().store_level().lock();
        string store_varname =
          store_locked.is_root() ? "root" :
          store_locked.is_inlined() ? "inlined" :
          store_locked.var().name();
        string store_level = sched.store_level().func() + "." + store_varname;
        LoopLevel compute_locked = func.schedule().compute_level().lock();
        string compute_varname =
          compute_locked.is_root() ? "root" :
          compute_locked.is_inlined() ? "inlined" :
          compute_locked.var().name();
        string compute_level = sched.compute_level().func() + "." + compute_varname;

        // create the streaming loops
        if (compute_level == store_level) {
          hwbuffer.streaming_loops = {compute_level};
        } else if (xcel->store_level.match(store_level)) {
          hwbuffer.streaming_loops = get_loop_levels_between(Stmt(op), xcel->func, store_locked, compute_locked, true);
        } else {
          user_error << "xcel store loop is different from the hwbuffer store loop. no good.\n";
        }
        hwbuffer.compute_level = hwbuffer.streaming_loops.back();
        hwbuffer.store_level = xcel->store_level.to_string();

        // Simplification possible when compute and store is the same level
        if (sched.compute_level() == sched.store_level()) {

          // use sliding window to get stencil sizes
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          new_body = mutate(new_body);
          
          std::string for_namer = first_for_name(new_body);
          
          auto boxes_write = boxes_provided(new_body);
          auto boxes_read = boxes_required(new_body);

          // extent is the same for total buffer box, input chunk, and output stencil for double buffer
          internal_assert(boxes_read[op->name].size() == boxes_write[op->name].size());
          vector<Expr> box(boxes_read[op->name].size());
          for (size_t i=0; i<boxes_read[op->name].size(); ++i) {
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            box[i] = extent;
          }

          FindOutputStencil fos(op->name, func, xcel_store_level);
          new_body.accept(&fos);

          CountBufferUsers counter(op->name);
          new_body.accept(&counter);

          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          auto input_block_box = counter.input_block_box;
          if (input_block_box.size() == 0) {
            input_block_box = output_block_box;
          }
          auto reader_loopnest = counter.reader_loopnest;

          internal_assert(boxes_read.at(op->name).size() == output_block_box.size());
          
          std::string for_name = first_for_name(new_body);
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());

          LoopLevel store_l = sched.store_level();
          
          hwbuffer.stride_map = fos.stride_map;

          hwbuffer.ldims = vector<LogicalDimSize>(output_block_box.size());

          for (size_t i = 0; i < output_block_box.size(); ++i) {
            hwbuffer.ldims[i].logical_min = Expr(0);
            hwbuffer.ldims[i].logical_size = box.at(i);
            
            hwbuffer.dims[i].input_chunk = input_block_box.at(i);
            hwbuffer.dims[i].input_block = input_block_box.at(i);

            hwbuffer.dims[i].output_min_pos = boxes_read.at(op->name)[i].min;
            hwbuffer.dims[i].loop_name = i < loop_names.size() ? loop_names.at(i) : unique_name("loopvar");
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          //std::cout << "created hwbuffer\n";

          if (buffers.count(hwbuffer.name) == 0) {
            //std::cout << "Here is the hwbuffer (store=compute):"
                      //<< " [" << buffers.count(hwbuffer.name) << "]\n"
                      //<< hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }
          
          return IRMutator::visit(op);
          
        } else {
          // look for a sliding window that can be used in a line buffer

          //std::cout << "Doing sliding window analysis on realization of " << op->name << "\n";

          // use sliding window to get stencil sizes
          // Parameters 1 and 2
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          new_body = mutate(new_body);
          
          std::string for_namer = first_for_name(new_body);

          FindOutputStencil fos(op->name, func, xcel_compute_level);
          new_body.accept(&fos);
          auto output_stencil_box = fos.output_stencil_box;

          CountBufferUsers counter(op->name);
          new_body.accept(&counter);
          
          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          auto input_block_box = counter.input_block_box;
          auto reader_loopnest = counter.reader_loopnest;

          auto boxes_read = boxes_required(new_body);

          //std::cout << "HWBuffer Parameter: " << op->name << " has Total box of size [";
          size_t i_max = boxes_read[op->name].size();
          // Parameter 6
          auto total_buffer_box = vector<Expr>(i_max);
          for (size_t i=0; i<i_max; ++i) {
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            total_buffer_box[i] = extent;

            std::cout << total_buffer_box[i];
            if (i < i_max - 1) { std::cout << extent << " "; }
          }
          std::cout << "]\n";

          // create the hwbuffer
          std::string for_name = first_for_name(new_body);
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());

          hwbuffer.ldims = vector<LogicalDimSize>(output_block_box.size());
          hwbuffer.stride_map = fos.stride_map;
          std::cout << hwbuffer.name << " stride_x=" << hwbuffer.stride_map["x"].stride << std::endl;

          // check that all of the extracted parameters are of the same vector length
          //FIXMEyikes
          std::cout << "HWBuffer has " << hwbuffer.dims.size() << " dims, while " << total_buffer_box.size() << " num box_dims\n";
          std::cout << " loops are: " << loop_names << std::endl;

          internal_assert(hwbuffer.dims.size() == output_stencil_box.size());
          
          internal_assert(hwbuffer.dims.size() == total_buffer_box.size());
          internal_assert(hwbuffer.dims.size() == output_block_box.size());
          internal_assert(hwbuffer.dims.size() == input_block_box.size());

          for (size_t i = 0; i < hwbuffer.dims.size(); ++i) {
            hwbuffer.ldims[i].logical_size = total_buffer_box.at(i);
            hwbuffer.ldims[i].logical_min = Expr(0);

            hwbuffer.dims[i].input_chunk = 1;
            hwbuffer.dims[i].input_block = input_block_box.at(i);
            hwbuffer.dims[i].loop_name = i < loop_names.size() ? loop_names.at(i) : unique_name("loopname");
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          if (buffers.count(hwbuffer.name) == 0) {          
            buffers[hwbuffer.name] = hwbuffer;
          }


          return Realize::make(op->name, op->types, op->memory_type,
                               op->bounds, op->condition, new_body);
        }
    }
  
public:
  HWBuffers(const map<string, Function> &e, const vector<string> &ln, HWXcel *xcel) :
    env(e), loop_names(ln), xcel(xcel) {}

  const std::vector<std::string> &loop_names;
  HWXcel* xcel;
  std::map<std::string, HWBuffer> buffers;
};

map<string, HWBuffer> extract_hw_buffers(Stmt s, const map<string, Function> &env,
                                         HWXcel *xcel) {
  HWBuffers ehb(env, xcel->streaming_loop_levels, xcel);
  ehb.mutate(s);

  for (auto hwbuffer : ehb.buffers) {
    std::cout << hwbuffer.first << " is ehb w/ inline=" << hwbuffer.second.is_inlined << std::endl;
    std::cout << hwbuffer.second << std::endl;
  }
    
    return ehb.buffers;
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


// Second pass through hwbuffers, setting some more parameters, including the consumer outputs.
void set_opt_params(HWXcel *xcel, 
                    const map<string, Function> &env,
                    const vector<BoundsInference_Stage> &inlined_stages,
                    const vector<string> &streaming_loop_levels,
                    Scope<Expr>& output_scope,
                    Box output_bounds) {

  auto &hwbuffers = xcel->hwbuffers;
  size_t i = inlined_stages.size() - 1 + 1;
  string xcel_compute_level = streaming_loop_levels.at(streaming_loop_levels.size()-1);

  bool in_output = true;
  // scan through each stage before the output stage

  Scope<Expr>& stencil_bounds = output_scope;
  std::cout << "stencil bounds when in the set_opt method: " << stencil_bounds << std::endl;
    
  // go through the stages from the output back to the input
  while (i >= 1) {
    i--;
    
    const BoundsInference_Stage &stage = inlined_stages[i];
    cout << "Inlined stage = " << stage.name << endl;
    if (in_output && stage.name != xcel->name) {
      continue;
    }

    // run through hwbuffers looking for a particular name
    auto iterator = std::find_if(hwbuffers.begin(), hwbuffers.end(), [stage](const std::pair<std::string, HWBuffer>& buffer_pair){
        if (stage.name == buffer_pair.first) {
          return true;
        } else {
          return false;
        }
      });
    internal_assert(iterator != hwbuffers.end()) << "Looking for " << stage.name << "\n";
    auto &hwbuffer = iterator->second;
    internal_assert(hwbuffer.name == iterator->first);
    
    std::cout << hwbuffer.name << " before func\n";

    Function cur_func = hwbuffer.func;
    if (stage.stage > 0) {
      StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
      auto rvars = update_schedule.rvars();
      for (size_t i=0; i<rvars.size(); ++i) {
        hwbuffer.dims[i].output_min_pos = rvars[i].min;
        std::cout << hwbuffer.name << " " << i << " update has min_pos=" << rvars[i].min << std::endl;
      }
    }

    if (!cur_func.schedule().is_hw_kernel()) {
      std::cout << "skipping " << hwbuffer.name << std::endl;
      continue;
    }
    
    // HWBuffer Parameter: bool is_output;
    if (xcel->name == hwbuffer.name) {
      hwbuffer.is_output = true;
      for (auto &dim : hwbuffer.dims) {
        dim.output_block = dim.input_block;
        dim.output_stencil = dim.output_block;
      }
    }
    
    if (in_output) {
      if (inlined_stages[i].name == xcel->name) {
        in_output = false;
      }
      continue; // output kernel doesn't need to keep going (no consumers)
    }

    // HWBuffer Parameter: bool is_inlined;
    if (cur_func.schedule().is_accelerator_input() ||
        //(cur_func.schedule().compute_level() != xcel->store_level &&
        (cur_func.schedule().compute_level().match(xcel->compute_level) &&
         cur_func.schedule().store_level().match(xcel->store_level))) {
      hwbuffer.is_inlined = false;
    } else {
      hwbuffer.is_inlined = true;
    }
    
    // HWBuffer Parameter: map<string, HWBuffer&> consumer_buffers
    for (size_t j = 0; j < stage.consumers.size(); j++) {
      internal_assert(stage.consumers[j] < (int)inlined_stages.size());
      const BoundsInference_Stage &consumer = inlined_stages[stage.consumers[j]];

      if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name)) {
        //hwbuffers.at(consumer.name).input_streams.push_back(hwbuffer.name);
        hwbuffers.at(consumer.name).input_streams.insert(hwbuffer.name);
      } else {
      }

      const Box &consumer_bounds = stage.bounds.find(make_pair(consumer.name,
                                                               consumer.stage))->second;
      const HWBuffer &consumer_buffer = hwbuffers.at(consumer.name);
      string consumer_name;
      if (consumer.name != hwbuffer.name) {
        if (consumer_buffer.is_inlined) {
          //internal_assert(consumer_buffer.consumer_buffers.size() == 1) << "The inlined kernel " << consumer.name << " has more than one consumer.\n";
          consumer_name = consumer_buffer.consumer_buffers.begin()->first;
        } else {
          consumer_name = consumer_buffer.name;
        }
      } else {
        consumer_name = hwbuffer.name;
      }

      hwbuffer.consumer_buffers[consumer_name] = std::make_shared<HWBuffer>(hwbuffers.at(consumer.name));

      //FindOutputStencil fos(hwbuffer.name, cur_func, consumer_buffer.compute_level);
      //FindOutputStencil fos(hwbuffer.name, cur_func, xcel->compute_level.lock().func());

      std::string func_compute_level = xcel->streaming_loop_levels.at(xcel->streaming_loop_levels.size()-1);
      const FuncSchedule &sched = cur_func.schedule();
      auto compute_looplevel = sched.compute_level();
      
      for (auto loopname : streaming_loop_levels) {
        if (compute_looplevel.lock().defined() &&
            !compute_looplevel.lock().is_inlined() &&
            !compute_looplevel.lock().is_root() &&
            compute_looplevel.lock().match(loopname)) {
          func_compute_level = loopname;
        }
      }

      FindOutputStencil fos(hwbuffer.name, cur_func, func_compute_level);
      consumer_buffer.my_stmt.accept(&fos);
      hwbuffer.stride_map = fos.stride_map;

      FindInputStencil fis(consumer.name, cur_func, func_compute_level, stencil_bounds);
      hwbuffer.my_stmt.accept(&fis);

      if (hwbuffers.count(consumer.name) == 0) {
        continue;
      }
      
      for (size_t idx=0; idx<hwbuffer.dims.size(); ++idx) {
        //hwbuffer.dims.at(idx).input_chunk = hwbuffer.name == "hw_input" ? 3 : consumer_sliding_stencils->input_chunk_box.at(idx);
        hwbuffer.dims.at(idx).input_chunk = 1;
        if (hwbuffer.dims.size() > idx) {
        //if (fis.found_stencil) {
          //hwbuffer.dims.at(idx).input_chunk = fis.input_chunk_box.at(idx);
          hwbuffer.dims.at(idx).input_chunk = hwbuffer.dims.at(idx).input_block;
        }
        
        if (fis.found_stencil && idx < fis.output_min_pos_box.size()) { // this works
          hwbuffer.dims.at(idx).output_min_pos = fis.output_min_pos_box.at(idx);
          
          if (is_zero(hwbuffer.dims.at(idx).output_min_pos)) { // alternatively, not output
            hwbuffer.dims.at(idx).output_min_pos = consumer_buffer.dims.at(idx).output_min_pos;

            //if (fos.found_stencil && idx < fos.output_stencil_box.size()) {
            //  std::cout << "old min_pos: " << hwbuffer.dims.at(idx).output_min_pos << std::endl;
            //  hwbuffer.dims.at(idx).output_min_pos += fos.output_min_pos_box.at(idx);
            //  std::cout << "new min_pos: " << hwbuffer.dims.at(idx).output_min_pos << std::endl;
            //}
          }
          
          //std::cout << "replaced min pos for " << hwbuffer.name << " " << idx << " with "
                    //<< hwbuffer.dims.at(idx).output_min_pos << "\n";
          //std::cout << consumer_bounds << std::endl;
          if (!consumer_buffer.is_output) {
            //std::cout << simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds)) << std::endl;
          }
          //if (consumer_bounds) {
          //  std::cout << " consumer_bounds=" << consumer_bounds[i].min << " = " 
          //            << simplify(expand_expr(consumer_bounds[i].min, stencil_bounds)) << std::endl;
          //}
        }

        //if (hwbuffer.name == "hw_input") {
        //  hwbuffer.dims.at(idx).output_min_pos = b[i].min;
        //}
        
        //hwbuffer.dims.at(idx).output_stencil = consumer_sliding_stencils->output_stencil_box.at(idx);
        //if (true) {
        if (fos.found_stencil && idx < fos.output_stencil_box.size()) {
          hwbuffer.dims.at(idx).output_stencil = fos.output_stencil_box.at(idx);
          hwbuffer.dims.at(idx).output_block = fos.output_stencil_box.at(idx);
          //std::cout << "replaced output stencil for " << hwbuffer.name << " based on consumer " << consumer.name << std::endl;
          //std::cout << "output min position is: " << fos.output_min_pos_box.at(idx) << std::endl;

          //hwbuffer.dims.at(idx).output_stencil = hwbuffer.dims.at(idx).output_block;
          //if (!hwbuffer.dims.at(idx).output_min_pos.same_as(fos.output_min_pos_box.at(idx))) {
          //  std::cout << "old min_pos: " << hwbuffer.dims.at(idx).output_min_pos << std::endl;
          if (!consumer_buffer.is_output) {
            hwbuffer.dims.at(idx).output_min_pos += fos.output_min_pos_box.at(idx);
          }
          //  std::cout << "new min_pos: " << hwbuffer.dims.at(idx).output_min_pos << std::endl;
          //}
        } else {
          //std::cout << "couldn't find that output stencil thing\n";
        }

        //hwbuffer.dims.at(idx).output_min_pos = simplify(hwbuffer.dims.at(idx).output_min_pos);
        //if (!consumer_buffer.is_output) {
        hwbuffer.dims.at(idx).output_min_pos = simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds));
        hwbuffer.dims.at(idx).output_max_pos = simplify(expand_expr(consumer_bounds[idx].max, stencil_bounds));
        //std::cout << "replaced output min pos with new algo: " << hwbuffer.name << idx << "=" << hwbuffer.dims.at(idx).output_min_pos << std::endl;
        auto loop_range = simplify(expand_expr(consumer_bounds[idx].max - consumer_bounds[idx].min + 1, stencil_bounds));
        //std::cout << "  range would/should be: " << loop_range << std::endl;

        //std::cout << "replaced input=" << hwbuffer.dims.at(idx).input_chunk
                  //<< " and output=" << hwbuffer.dims.at(idx).output_stencil << std::endl;
      }

      //std::cout << "right before " << consumer.name << " inputs\n";
    // std::vector<std::string> input_streams;  // used when inserting read_stream calls      
      if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name)) {
        //hwbuffers.at(consumer.name).input_streams.push_back(hwbuffer.name);
        hwbuffers.at(consumer.name).input_streams.insert(hwbuffer.name);
        ReplaceOutputAccessPatternRanges roapr(consumer_buffer);
        hwbuffer.output_access_pattern = roapr.mutate(hwbuffer.output_access_pattern);
      }

    }

    // save the bounds values in scope
    for (int i = 0; i < cur_func.dimensions(); i++) {
      string arg = cur_func.name() + ".s" + std::to_string(stage.stage) + "." + cur_func.args()[i];
      stencil_bounds.push(arg + ".min", hwbuffer.dims[i].output_min_pos);
      stencil_bounds.push(arg + ".max", hwbuffer.dims[i].output_max_pos);
    }
    if(stage.stage > 0) {
      StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
      // TODO check the sliding dimensions are all pure, referring to
      for (ReductionVariable i : update_schedule.rvars()) {
        string arg = cur_func.name() + ".s" + std::to_string(stage.stage) + "." + i.var;
        internal_assert(is_const(i.min));
        internal_assert(is_const(i.extent));
        Expr min = i.min;
        Expr max = simplify(i.extent + i.min - 1);
        stencil_bounds.push(arg + ".min", min);
        stencil_bounds.push(arg + ".max", max);
      }
    }

  }
  
}


void extract_hw_xcel_top_parameters(Stmt s, Function func,
                                    const map<string, Function> &env,
                                    const vector<BoundsInference_Stage> &inlined,
                                    HWXcel *xcel) {
  xcel->name = func.name();
  xcel->func = func;
  xcel->store_level = func.schedule().accelerate_store_level();
  xcel->compute_level = func.schedule().accelerate_compute_level();
  xcel->streaming_loop_levels = get_loop_levels_between(s, func, xcel->store_level, xcel->compute_level);
  xcel->input_streams = func.schedule().accelerate_inputs();

  Scope<Expr> output_scope;
  find_output_scope(s, func, xcel->compute_level, output_scope);

  auto output_box = find_output_bounds(s, func, xcel->compute_level);
  
  // use realizes to define each hwbuffer
  xcel->hwbuffers = extract_hw_buffers(s, env, xcel);

  // set output parameters for hwbuffers based on consumers
  set_opt_params(xcel, env, inlined, xcel->streaming_loop_levels, output_scope, output_box);

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
    // skip this function if it is not accelerated
    if(!func.schedule().is_accelerated()) {
      continue;
    }
    
    //std::cout << "Found accelerate function " << func.name() << "\n";
    LoopLevel store_locked = func.schedule().store_level().lock();
    string store_varname =
      store_locked.is_root() ? "root" :
      store_locked.is_inlined() ? "inlined" :
      store_locked.var().name();

    debug(3) << "Found accelerate function " << func.name() << "\n";
    //std::cout << "Found accelerate function " << func.name() << "\n";
    debug(3) << store_locked.func() << " " << store_varname << "\n";
    HWXcel xcel;
    extract_hw_xcel_top_parameters(s, func, env, inlined_stages, &xcel);
    xcels.push_back(xcel);
  }
  return xcels;
}


}  // namespace Internal
}  // namespace Halide
