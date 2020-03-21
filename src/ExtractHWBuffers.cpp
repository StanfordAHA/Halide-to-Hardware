#include "ExtractHWBuffers.h"
#include "HWBuffer.h"
#include "HWBufferUtils.h"
#include "SlidingWindow.h"

#include "Bounds.h"
#include "Debug.h"
#include "Func.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "IRPrinter.h"
#include "Monotonic.h"
#include "RemoveTrivialForLoops.h"
#include "Scope.h"
#include "Simplify.h"
#include "Substitute.h"
#include "UnrollLoops.h"

namespace Halide {
namespace Internal {

using std::map;
using std::vector;
using std::string;

namespace {

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

  void visit(const ProducerConsumer *op) {
    if (!op->is_producer) {
    //if (false) {
      // look at the writers (writer ports)
      if (provide_at_level(op->body, var)) {
        auto box_write = box_provided(op->body, var);
        //std::cout << "writers inside pc " << op->name << std::endl;
        //std::cout << "Box writer found for " << var << " with box " << box_write << std::endl;

        //std::cout << "HWBuffer Parameter: writer ports - "
        //          << "box extent=[";
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
        //          << "box extent=[";
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
    if (contains_call(op->body, var)) {
      // add this for loop to read address loop
      For *previous_for = current_for;
      Stmt for_stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, op->body);
      if (current_for == nullptr) {
        full_stmt = for_stmt;
        //std::cout << "added first for loop: " << op->name << " for " << var << std::endl;

        auto box_read = box_required(op->body, var);
        auto interval = box_read;
        for (size_t dim=0; dim<interval.size(); ++dim) {
          auto assertstmt = AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope)));
          //std::cout << "min pos in dim " << dim << ":" << assertstmt;
        }
      } else if (!is_parallelized(op)) {
        current_for->body = for_stmt;
        //std::cout << "added nonparallel for loop: " << op->name << std::endl;
      }
      current_for = const_cast<For *>(for_stmt.as<For>());
      
      IRVisitor::visit(op);

      // look at the readers (reader ports, read address gen)
      if (call_at_level(op->body, var) && !is_parallelized(op)) {
        auto box_read = box_required(op->body, var);
        //std::cout << "readers inside loop " << op->name << std::endl;
        //std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
        //std::cout << Stmt(op->body) << std::endl;
        //std::cout << "HWBuffer Parameter: reader ports - "
        //          << "box extent=[";
        auto interval = box_read;

        vector<Stmt> stmts;
        output_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
          //stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope))));
          stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(interval[dim].min)));
          //std::cout << port_expr << ":" << simplify(expand_expr(interval[dim].min, scope)) << "-"
          //          << expand_expr(interval[dim].min, scope) << " ";
        }
        //std::cout << "]\n";

        const vector<Stmt> &const_stmts = stmts;
        current_for->body = Block::make(const_stmts);
        reader_loopnest = simplify(expand_expr(full_stmt, scope));
        //std::cout << "HWBuffer Parameter - " << var << " nested reader loop:\n" << reader_loopnest << std::endl;

      }

      // remove for loop to read address loop
      current_for = previous_for;
      return;
    }
    

    //std::cout << "added for loop: " << op->name << std::endl;
    IRVisitor::visit(op);

    // look at the writers (writer ports)
    if (provide_at_level(op->body, var) && !is_parallelized(op)) {
      auto box_write = box_provided(op->body, var);
      //std::cout << "writers inside loop " << op->name << std::endl;
      //std::cout << "Box writer found for " << var << " with box " << box_write << std::endl;

      //std::cout << "HWBuffer Parameter: writer ports - "
      //          << "box extent=[";
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

  void visit(const Variable *op) {
    if (op->name == loopname && in_var) {
      //std::cout << "found loop=" << op->name << " and setting stride=" << cur_stride << std::endl;
      stride_for_var = cur_stride;
      is_div = in_div;
      IRVisitor::visit(op);
    }
  }
  
  void visit(const Call *op) {
    if (op->name == varname) {
      in_var = true;
      //std::cout << "call for " << op->name << " includes: " << op->args << std::endl;
      IRVisitor::visit(op);
      in_var = false;
    } else {
      IRVisitor::visit(op);
    }
  }
  void visit(const Div *op) {
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

  void visit(const Mul *op) {
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

  Stmt visit(const For *op) {
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
  string consumer;
  Scope<Expr> scope;
  string store_level;
  string compute_level;
  
  // keep track of lets to later replace variables with constants
  void visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      IRVisitor::visit(op);
  }

  void visit(const ProducerConsumer *op) override {
    if (store_level == compute_level && op->is_producer && op->name == consumer && false) {
      found_stencil = true;

      auto box_read = box_required(op->body, var);
      auto interval = box_read;
      output_min_pos_box = vector<Expr>(interval.size());
      output_block_box = vector<Expr>(interval.size());
      //std::cout << "working on " << op->name << " "
      //           << " with size: " << interval.size() << std::endl
      //           << op->body << std::endl;

      for (size_t dim=0; dim<interval.size(); ++dim) {
        output_min_pos_box[dim] = interval[dim].min;
        auto output_block = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        output_block_box[dim] = output_block;
        //std::cout << "(" << output_min_pos_box[dim] << "," << interval[dim].min << ")  ";
      }
    }
    
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
    stride_map[varname].stride = std::max(stride_map[varname].stride, 1);
    stride_map[varname].is_inverse = fvs.is_div;
    
    //std::cout << op->name << " has stride=" << stride_for_var << " in call for " << var
    //          << " stride_map[" << varname << "]=" << stride_map[varname].stride << std::endl;
    //std::cout << op->body << std::endl;


    if (op->name == compute_level) {
      //std::cout << var << " found compute level with for loop " << op->name << std::endl;
      //std::cout << "before for bound replacement\n" << Stmt(op->body);

      ReplaceForBounds rfb;
      Stmt new_body = rfb.mutate(op->body);

      //std::cout << "after for bound replacement\n" << Stmt(new_body);

      //if (scope.contains(op->name + ".min")) {
      //std::cout << "loop min should be " << scope.get(op->name + ".min") << std::endl;
      //} else {
      //std::cout << "couldn't find a loop min\n";
      //}
      
      //std::cout << op->body << std::endl;
      //std::cout << new_body << std::endl;
      auto box_read = box_required(new_body, var);

      //Box box = box_provided(new_body, var);
      //std::cout << "let's save this output box num_dims=" << func.dimensions() << " box=" << box.size() << "\n";
      // // save the bounds values in scope
      // Scope<Expr> stencil_bounds;
      // if (func.name() == var) {
      //   for (int i = 0; i < func.dimensions(); i++) {
      //     string stage_name = func.name() + ".s0." + func.args()[i];
      //     stencil_bounds.push(stage_name + ".min", box[i].min);
      //     stencil_bounds.push(stage_name + ".max", box[i].max);
      //   }
      // }
      //std::cout << "box saved\n";

      auto interval = box_read;
      output_stencil_box = vector<Expr>(interval.size());
      output_min_pos_box = vector<Expr>(interval.size());
      //std::cout << "HWBuffer Parameter: " << var << " output stencil size - " << "box extent=[";

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
    if (call_at_level(op->body, var) && provide_at_level(op->body, consumer) && search_for_min) {
      found_output_min = true;
      search_for_min = false;
      //std::cout << "we found the output min pos for " << var << " to " << consumer << std::endl
      //          << op->body << std::endl;
      auto box_read = box_required(op->body, var);
      auto interval = box_read;
      output_min_pos_box = vector<Expr>(interval.size());
      output_block_box = vector<Expr>(interval.size());
      //std::cout << var << " output min pos box is " << interval << std::endl;

      for (size_t dim=0; dim<interval.size(); ++dim) {
        output_min_pos_box[dim] = interval[dim].min;
        auto output_block = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        output_block_box[dim] = output_block;
        //std::cout << "(" << output_min_pos_box[dim] << "," << interval[dim].min << ")  ";
        //std::cout << "(" << output_block << ")  ";
      }
      //std::cout << "]\n";
      auto unrolled = unroll_loops(remove_trivial_for_loops(Stmt(op)));
      auto letstmtexpr = LetStmt::make("hello", 0, unrolled);
      const LetStmt* letstmt = letstmtexpr.as<LetStmt>();
      
      IRVisitor::visit(letstmt);
      search_for_min = true;
    } else {
      IRVisitor::visit(op);
    }
  }

  // register call as a port
  void visit(const Call *op) override {
    if (op->name == var && found_output_min) {
      //std::cout << "retrieving values for " << op->name
      //          << " with found=" << found_output_min << std::endl;

      // get mem indices
      vector<Expr> call_indices(op->args.size());
      vector<Expr> mem_indices(op->args.size());
      //std::cout << "lets do the args " << output_min_pos_box << std::endl;
      internal_assert(output_min_pos_box.size() == op->args.size());
      for (size_t i=0; i<op->args.size(); i++) {
        call_indices.at(i) = op->args[i];
        //std::cout << "done with call" << std::endl;
        //auto sub = op->args[i] - output_min_pos_box.at(i);
        //std::cout << "done with sub " << sub << std::endl;
        //auto result = id_const_value(simplify(sub));
        //std::cout << "int is " << result << std::endl;
        //mem_indices.at(i) = result;
        auto param = simplify(expand_expr(op->args[i] - output_min_pos_box.at(i), scope));
        //std::cout << "param=" << param << std::endl;
        mem_indices.at(i) = id_const_value(param);
      }
      //std::cout << "got the args" << std::endl;

      // calculate new port number
      int port_number = oports.size();

      // create port
      Port oport;
      oport.stream_index = port_number;
      oport.mem_indices = mem_indices;
        
      oports[call_indices] = oport;
      //std::cout << "added port " << port_number << " with mem_indices " << mem_indices
      //          << " for " << op->name << call_indices << std::endl;
    } else {
      //std::cout << "not retrieving values for " << op->name
      //          << " with found=" << found_output_min << std::endl;
    }
  }
  
  
public:
  vector<Expr> output_block_box;
  vector<Expr> output_stencil_box;
  vector<Expr> output_min_pos_box;
  std::map<std::vector<Expr>, Port, ExprVecCompare> oports;
  map<string, Stride> stride_map;
  bool found_stencil;
  bool found_output_min;
  bool search_for_min;
  FindOutputStencil(string v, string c, string sl, string cl) :
    var(v), consumer(c), store_level(sl), compute_level(cl), found_stencil(false),
    found_output_min(false), search_for_min(true) {
    if (v.find("db_") != string::npos) {
      //std::cout << "looking to find " << v
      //          << " output stencil where compute_level=" << compute_level
      //          << " in:" << std::endl;
    }
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
      auto box_write = box_provided(op->body, var);
      auto interval = box_write;
      input_chunk_box = vector<Expr>(interval.size());

      //std::cout << interval << std::endl;
      //std::cout << "we found the input stencil for " << var << op->body << std::endl;
      
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

      input_min_pos_box = vector<Expr>(interval.size());
      
      //std::cout << "HWBuffer Parameter: " << var << " input chunk size - "
      //          << "box extent=[";

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        input_chunk_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        //std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        //input_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, stencil_bounds));
        //input_min_pos_box[dim] = interval[dim].min;
        //std::cout << "(" << input_min_pos_box[dim] << "," << interval[dim].min << ")  ";
      }
      //std::cout << "]\n";

    }

    if (provide_at_level(op->body, var)) {// && call_at_level(op->body, producer)) {
      auto box_write = box_provided(op->body, var);
      auto interval = box_write;
      //std::cout << "we found the input min pos for " << var << ": " << interval << std::endl;
      input_min_pos_box = vector<Expr>(interval.size());
    
      for (size_t dim=0; dim<interval.size(); ++dim) {
        input_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, stencil_bounds));
        //std::cout << "(" << input_min_pos_box[dim] << "," << interval[dim].min << ")  ";
      }
      //std::cout << "]\n";
    
    }
    IRVisitor::visit(op);
  }
public:
  vector<Expr> input_chunk_box;
  vector<Expr> input_min_pos_box;
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

}


/*
 * Extract the HWBuffers by looking for each of the realize statements.
 */
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
        //std::cout << hwbuffer.name << " found realize for new hwbuffer" << std::endl;
        
        hwbuffer.func = func;
        hwbuffer.my_stmt = op->body;
        Stmt new_body = op->body;
        //std::cout << "Encountered realize " << op->name << " with body:\n" << op->body << std::endl;

        // If the Function in question has the same compute_at level
        // as its store_at level, we know this is a double buffer.
        //std::cout << "creating buffer for realize " << op->name << std::endl;
        string xcel_compute_level = loop_names.at(loop_names.size()-1);
        string xcel_store_level = loop_names.at(0);
        //std::cout << "xcel compute level is " << xcel_compute_level << std::endl;

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
        //std::cout << "looking at loops, func " << op->name << " has store=" << store_locked.to_string()
        //          << "  compute=" << compute_locked.to_string() << std::endl;
        //hwbuffer.store_level = store_locked.to_string();
        //hwbuffer.compute_level = compute_locked.to_string();

        // create the streaming loops
        if (compute_level == store_level) {
          hwbuffer.streaming_loops = {compute_level};
        } else if (xcel->store_level.match(store_level)) {
          //std::cout << hwbuffer.name << " store level matches xcel\n";
          hwbuffer.streaming_loops = get_loop_levels_between(Stmt(op), xcel->func, store_locked, compute_locked, true);
        } else {
          user_error << "xcel store loop is different from the hwbuffer store loop. no good.\n";
        }
        //std::cout << Stmt(op) << std::endl;
        //std::cout << "streaming loops: " << hwbuffer.streaming_loops << std::endl;
        //std::cout << "store: " << store_locked << "  compute: " << compute_locked << std::endl;
        //std::cout << "xcel streaming loops: " << loop_names << std::endl;
        hwbuffer.compute_level = hwbuffer.streaming_loops.back();
        //hwbuffer.store_level = hwbuffer.streaming_loops.front();
        hwbuffer.store_level = xcel->store_level.to_string();

        // Simplification possible when compute and store is the same level
        hwbuffer.ldims = vector<LogicalDimSize>(op->bounds.size());
        for (size_t i=0; i<hwbuffer.ldims.size(); ++i) {
          Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
          hwbuffer.ldims[i].logical_size = extent;
          hwbuffer.ldims[i].logical_min = Expr(0);
        }

        // HWBuffer Parameter: bool is_output;
        // HWBuffer Parameter: bool is_inlined;
        if (xcel->name == hwbuffer.name) {
          hwbuffer.is_output = true;
          hwbuffer.is_inlined = false;
          
        } else {
          hwbuffer.is_output = false;
          
          if (hwbuffer.func.schedule().is_accelerator_input() ||
              hwbuffer.func.schedule().store_level().match(xcel->store_level)) {
            hwbuffer.is_inlined = false;
          } else {
            hwbuffer.is_inlined = true;
          }
        }

        if (sched.compute_level() == sched.store_level()) {
          //std::cout << op->name << " has compute=store level\n";

          // use sliding window to get stencil sizes
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          new_body = mutate(new_body);
          
          std::string for_namer = first_for_name(new_body);
          //std::cout << op->name << " sliding output=" << sliding_stencil_map.at(for_namer).output_stencil_box
          //          << " input=" << sliding_stencil_map.at(for_namer).input_chunk_box << std::endl;
          
          auto boxes_write = boxes_provided(new_body);
          auto boxes_read = boxes_required(new_body);

          // extent is the same for total buffer box, input chunk, and output stencil for double buffer
          internal_assert(boxes_read[op->name].size() == boxes_write[op->name].size());
          vector<Expr> box(boxes_read[op->name].size());
          for (size_t i=0; i<boxes_read[op->name].size(); ++i) {
            //Expr extent = simplify(boxes_read[op->name][i].max - boxes_read[op->name][i].min + 1);
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            box[i] = extent;
          }

          //std::cout << "HWBuffer Parameter: Total buffer box is of size [";          
          for (size_t i=0; i<box.size(); ++i) {
            //std::cout << box[i];
            //if (i < box.size() - 1) {std::cout << " "; }
          }
          //std::cout << "]\n";

          //std::cout << "HWBuffer Parameter: Input Chunk box is of size [";
          for (size_t i=0; i<box.size(); ++i) {
            //std::cout << box[i];
            //if (i < box.size() - 1) {std::cout << " "; }
          }
          //std::cout << "]\n";
          
          //std::cout << "HWBuffer Parameter: Output Stencil box is of size [";
          for (size_t i=0; i<box.size(); ++i) {
            //std::cout << box[i];
            //if (i < box.size() - 1) {std::cout << " "; }
          }
          //std::cout << "]\n";

          //FindOutputStencil fos(op->name, func, func_store_level);
          //FindOutputStencil fos(op->name, op->name, hwbuffer.store_level, hwbuffer.compute_level);
          //new_body.accept(&fos);

          //std::cout << "counting for " << hwbuffer.name << ":\n";// << new_body << std::endl;
          CountBufferUsers counter(op->name);
          new_body.accept(&counter);

          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          //std::cout << "output block: " << output_block_box << std::endl;
          auto input_block_box = counter.input_block_box;
          //std::cout << "input block: " << input_block_box << std::endl;
          if (input_block_box.size() == 0) {
            input_block_box = output_block_box;
          }
          auto reader_loopnest = counter.reader_loopnest;

          internal_assert(boxes_read.at(op->name).size() == output_block_box.size());
          //internal_assert(boxes_read.at(op->name).size() == input_block_box.size());
          
          //std::cout << "transferring parameters\n";
          std::string for_name = first_for_name(new_body);
          //HWBuffer hwbuffer(output_block_box.size(), sliding_stencil_map.at(for_name));
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());

          LoopLevel store_l = sched.store_level();
          //hwbuffer.store_level = store_l.lock().func();
          
          //comment hwbuffer.stride_map = fos.stride_map;
          //std::cout << hwbuffer.name << " stride_x=" << hwbuffer.stride_map["x"].stride << std::endl;

          //hwbuffer.ldims = vector<LogicalDimSize>(output_block_box.size());

          //hwbuffer.dims = vector<MergedDimSize>(output_block_box.size());
          for (size_t i = 0; i < output_block_box.size(); ++i) {
            hwbuffer.dims[i].input_chunk = box.at(i); // needed for db
            hwbuffer.dims[i].input_block = input_block_box.at(i);  // needed for db

            hwbuffer.dims[i].output_stencil = box.at(i); // used for hw_input
            //hwbuffer.dims[i].output_block   = output_block_box.at(i); // used for hw_input

            //hwbuffer.dims[i].output_min_pos = boxes_read.at(op->name)[i].min;
            //std::cout << "hwbuffer " << hwbuffer.name << " in dim " << i <<
            //  " has min_pos=" << hwbuffer.dims[i].output_min_pos << std::endl;
            hwbuffer.dims[i].loop_name = i < loop_names.size() ? loop_names.at(i) : unique_name("loopvar");
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          //std::cout << "created hwbuffer " << hwbuffer.name << "\n";

          if (buffers.count(hwbuffer.name) == 0) {
            //std::cout << "Here is the hwbuffer (store=compute):" << hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }
          
          return IRMutator::visit(op);
          
        } else {
          // look for a sliding window that can be used in a line buffer
          //std::cout << "looking at hwbuffer with diff compute and store levels: " << hwbuffer.name << std::endl;


          auto boxes_read = boxes_required(new_body);
          //std::cout << "HWBuffer Parameter: " << op->name << " has Total box of size [";
          size_t i_max = boxes_read[op->name].size();
          // Parameter 6
          auto total_buffer_box = vector<Expr>(i_max);
          for (size_t i=0; i<i_max; ++i) {
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            total_buffer_box[i] = extent;

            //std::cout << total_buffer_box[i];
            //if (i < i_max - 1) {std::cout << extent << " "; }
          }
          //std::cout << "]\n";

          //std::cout << "Doing sliding window analysis on realization of " << op->name << "\n";
          // use sliding window to get stencil sizes
          // Parameters 1 and 2
          std::string for_namer = first_for_name(new_body);
          
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          new_body = mutate(new_body);
          //std::cout << op->name << " sliding output=" << sliding_stencil_map.at(for_namer).output_stencil_box << " input=" << sliding_stencil_map.at(for_namer).input_chunk_box << std::endl;

          auto slide_box = sliding_stencil_map.at(for_namer).input_chunk_box;
          //auto input_chunk = vector<Expr>(slide_box.size());
          auto input_chunk = total_buffer_box; // init to logical size, and replace
          for (size_t i=0; i<slide_box.size(); ++i) { // replace chunk with slide from the back
            input_chunk.at(input_chunk.size()-1 - i) = slide_box.at(i);
          }
          //std::cout << hwbuffer.name << " has the real size of " << input_chunk << std::endl;
          
          //FindOutputStencil fos(op->name, func, xcel_compute_level);
          //FindOutputStencil fos(op->name, op->name, hwbuffer.store_level, hwbuffer.compute_level);
          //new_body.accept(&fos);
          //auto output_stencil_box = fos.output_stencil_box;
          //std::cout << "output stencil is " << output_stencil_box << " using loops: " << hwbuffer.streaming_loops << std::endl;

          //std::cout << "counting lbed for " << hwbuffer.name << ":\n" << new_body << std::endl;
          CountBufferUsers counter(op->name);
          ReplaceForBounds rfb;
          auto replaced_body = rfb.mutate(new_body);
          //std::cout << "looking for those access patterns for buffer named: " << op->name << new_body;
          //new_body.accept(&counter);
          replaced_body.accept(&counter);
          
          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          auto input_block_box = counter.input_block_box;
          auto reader_loopnest = counter.reader_loopnest;
          auto output_stencil_box = counter.input_block_box;
          //std::cout << hwbuffer.name << " has reader loopnest:\n" << reader_loopnest;

          // create the hwbuffer
          //std::string for_name = first_for_name(new_body);
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());

          //std::cout << hwbuffer.name << " stride_x=" << hwbuffer.stride_map["x"].stride << std::endl;

          // check that all of the extracted parameters are of the same vector length
          //std::cout << "HWBuffer has " << hwbuffer.dims.size() << " dims, while " << total_buffer_box.size() << " num box_dims\n";
          //std::cout << " loops are: " << loop_names << std::endl;
          //std::cout << " hwbuffer loops are: " << hwbuffer.streaming_loops << std::endl;
          
          internal_assert(hwbuffer.dims.size() == total_buffer_box.size());
          internal_assert(hwbuffer.dims.size() == output_block_box.size());
          internal_assert(hwbuffer.dims.size() == input_block_box.size());
          internal_assert(hwbuffer.dims.size() == input_chunk.size());
          //internal_assert(hwbuffer.dims.size() == loop_names.size()) << "HWBuffer has " << hwbuffer.dims.size() << " dims, while only " << loop_names.size() << " loops\n";

          for (size_t i = 0; i < hwbuffer.dims.size(); ++i) {
            hwbuffer.dims[i].input_chunk = input_chunk.at(i);
            hwbuffer.dims[i].input_block = input_block_box.at(i);
            //std::cout << "finished input " << output_block_box << " and " << loop_names << "\n";

            hwbuffer.dims[i].loop_name = i < loop_names.size() ? loop_names.at(i) : unique_name("loopname");
            //std::cout << "hwbuffer " << hwbuffer.name << " finished dim " << i << " has min_pos=" << hwbuffer.dims[i].output_min_pos << std::endl;
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          if (buffers.count(hwbuffer.name) == 0) {          
            //std::cout << "Here is the hwbuffer:" << hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }

          //std::cout << "sliding stencil map is \n";
          for (auto pair : sliding_stencil_map) {
            //std::cout << pair.first << ", ";
          }
          //std::cout << std::endl;

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

  //for (auto& hwbuffer : ehb.buffers) {
    //std::cout << hwbuffer.first << " is ehb w/ inline=" << hwbuffer.second.is_inlined << std::endl;
    //std::cout << hwbuffer.second << std::endl;
  //}
    
  return ehb.buffers;
}

class FindOutputBounds : public IRVisitor {
  LoopLevel compute_level;
  Function func;

  using IRVisitor::visit;

  void visit(const For *op) {
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
    //std::cout << "func trying to find output bounds: " << func.name() << " " << compute.var().name() << std::endl;
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
    //std::cout << stage_name << ".min being set to " << box[i].min << std::endl;
    scope.push(stage_name + ".max", box[i].max);
  }
}


// Second pass through hwbuffers, setting some more parameters, including the consumer outputs.
void set_output_params(HWXcel *xcel, 
                       const map<string, Function> &env,
                       const vector<BoundsInference_Stage> &inlined_stages,
                       const vector<string> &streaming_loop_levels,
                       Scope<Expr>& output_scope) {
  //Box output_bounds) {

  auto &hwbuffers = xcel->hwbuffers;
  size_t i = inlined_stages.size() - 1 + 1;
  string xcel_compute_level = streaming_loop_levels.at(streaming_loop_levels.size()-1);

  bool in_output = true;

  Scope<Expr>& stencil_bounds = output_scope;
  //std::cout << "stencil bounds when in the set_output_params method: " << stencil_bounds << std::endl;

  // Scan through each stage before the output stage.
  // Go through the stages from the output back to the input.
  while (i >= 1) {
    i--;
    
    const BoundsInference_Stage &stage = inlined_stages[i];
    if (in_output && stage.name != xcel->name) {
      continue;
    }

    internal_assert(hwbuffers.count(stage.name));
    auto& hwbuffer = hwbuffers.at(stage.name);
    internal_assert(hwbuffer.name == stage.name);
    
    //std::cout << hwbuffer.name << " before func\n";

    Function cur_func = hwbuffer.func;
    if (stage.stage > 0) {
      StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
      auto rvars = update_schedule.rvars();
      for (size_t i=0; i<rvars.size(); ++i) {
        //hwbuffer.dims[i].output_min_pos = rvars[i].min;
        //std::cout << hwbuffer.name << " " << i << " update has min_pos=" << rvars[i].min << std::endl;
      }
    }

    //std::cout << "HWBuffer has " << hwbuffer.dims.size() << " dims, while \n";
    //std::cout << " loops are: " << hwbuffer.streaming_loops << std::endl;
    //std::cout << " args are: " << hwbuffer.func.args() << std::endl;
    
    if (!cur_func.schedule().is_hw_kernel()) {
      //std::cout << "skipping " << hwbuffer.name << std::endl;
      continue;
    }
    
    // HWBuffer Parameter: bool is_output;
    //if (xcel->name == hwbuffer.name) {
    if (hwbuffer.is_output) {
      for (auto &dim : hwbuffer.dims) {
        dim.output_block = dim.input_block;
        dim.output_stencil = dim.output_block;
      }
      std::cout << hwbuffer.my_stmt << std::endl;
      
      in_output = false;

      internal_assert(stage.consumers.size() == 1);
      const BoundsInference_Stage &consumer = inlined_stages[stage.consumers[0]];
      auto &ostream = hwbuffer.ostreams[consumer.name];
      ostream.name = consumer.name;
      ostream.output_access_pattern = hwbuffer.output_access_pattern;

      FindOutputStencil fos(hwbuffer.name, ostream.name, hwbuffer.store_level, "hw_output.s0.y.yo");
      hwbuffer.my_stmt.accept(&fos);

      ostream.stride_map = fos.stride_map;
      ostream.oports = fos.oports;

      ostream.odims.resize(hwbuffer.dims.size());
      internal_assert(hwbuffer.dims.size() == ostream.odims.size());
      for (size_t dim=0; dim<hwbuffer.dims.size(); dim++) {
        ostream.odims[dim].output_block = hwbuffer.dims.at(dim).output_block;
        ostream.odims[dim].output_stencil = hwbuffer.dims.at(dim).output_stencil;
      }
      
      continue; // output kernel doesn't need to keep going (no consumers)
    }
    //std::cout << "finished is_output " << stage.consumers.size() << " consumers\n";
    
    //std::cout << hwbuffer.name << " is inline=" << hwbuffer.is_inlined
    //          << " store=" << cur_func.schedule().store_level()
    //          << " compute=" << cur_func.schedule().compute_level()
    //          << " is_xcel=" << cur_func.schedule().is_accelerated()
    //          << " is_hw_kernel=" << cur_func.schedule().is_hw_kernel()
    //          << std::endl;
    
    // HWBuffer Parameter: map<string, HWBuffer&> consumer_buffers
    for (size_t j = 0; j < stage.consumers.size(); j++) {
      internal_assert(stage.consumers[j] < (int)inlined_stages.size());

      // pick the consumer stages in order
      const BoundsInference_Stage &consumer = inlined_stages[stage.consumers[j]];
      //std::cout << "right before " << consumer.name << " consumer of " << hwbuffer.name << "\n";

      // create an output based on the consumer name
      internal_assert(hwbuffers.count(consumer.name) > 0);
      auto &ostream = hwbuffer.ostreams[consumer.name];
      ostream.name = consumer.name;
      ostream.output_access_pattern = hwbuffer.output_access_pattern;

      // create an input stream for the consumer as well
      //if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name) &&
      //if (hwbuffers.count(consumer.name) &&  // does this work?
      if ((!hwbuffer.is_inlined || hwbuffers.at(consumer.name).is_output || xcel->input_streams.count(hwbuffer.name)>0) &&
          hwbuffers.at(consumer.name).istreams.count(hwbuffer.name) == 0 &&
          !hwbuffer.is_inlined) {
        
        InputStream istream;
        istream.name = hwbuffer.name;
        istream.hwref = &hwbuffer;
        hwbuffers.at(consumer.name).istreams[hwbuffer.name] = istream;

      } else {
        //std::cout << "couldn't find consumer " << consumer.name << std::endl;
      }

      const Box &consumer_bounds = stage.bounds.find(make_pair(consumer.name,
                                                               consumer.stage))->second;
      //std::cout << " size of consumer box is " << consumer_bounds << std::endl;
      const HWBuffer &consumer_buffer = hwbuffers.at(consumer.name);

      ostream.hwref = &hwbuffers.at(consumer.name);
      
      std::string func_compute_level = hwbuffer.compute_level;
      const FuncSchedule &sched = cur_func.schedule();
      auto compute_looplevel = sched.compute_level();
      
      //std::cout << hwbuffer.name << " compute loop is using " << func_compute_level << " based on func " << cur_func.name() << " compute=" << cur_func.schedule().compute_level() << " consumer_compute=" << consumer_buffer.compute_level << std::endl;

      //FindOutputStencil fos(hwbuffer.name, consumer.name, hwbuffer.store_level, func_compute_level);
      FindOutputStencil fos(hwbuffer.name, consumer.name, hwbuffer.store_level, consumer_buffer.compute_level);
      //std::cout << hwbuffer.name << " going to consumer named " << consumer_buffer.name << std::endl;
      //std::cout << consumer_buffer.my_stmt << std::endl;
      consumer_buffer.my_stmt.accept(&fos);
      //hwbuffer.my_stmt.accept(&fos);
      //std::cout << "output min pos is " << fos.output_min_pos_box << std::endl;
      //std::cout << "looking for output stencil and min for " << hwbuffer.name << " to consumer " << consumer.name
      //          << ": \n" << std::endl << consumer_buffer.my_stmt << std::endl;

      if (fos.found_output_min) {
        //std::cout << hwbuffer.name << " has " << fos.oports << std::endl;
        //std::cout << hwbuffer.name << " to " << consumer.name << " output_min_pos0=" << fos.output_min_pos_box[0] << std::endl;
      }

      ostream.stride_map = fos.stride_map;
      ostream.oports = fos.oports;

      // calculate how large each stride is with a flat dimension
      vector<int> flat_dim_strides(hwbuffer.ldims.size());
      internal_assert(flat_dim_strides.size() == hwbuffer.ldims.size());
      for (size_t i=0; i<hwbuffer.ldims.size(); ++i) {
        if (i == 0) {
          flat_dim_strides.at(i) = 1;
        } else {
          flat_dim_strides.at(i) = flat_dim_strides.at(i-1) *
            id_const_value(hwbuffer.ldims.at(i-1).logical_size);
        }
      }
      //std::cout << "flat " << flat_dim_strides << std::endl;

      //std::cout << "hwbuffer " << hwbuffer.name << " has ports: " << std::endl;
      for (auto& oport_pair : ostream.oports) {
        auto& port = oport_pair.second;
        int start_addr = 0;
        for (size_t idx = 0; idx < port.mem_indices.size(); ++idx) {
          int stride = flat_dim_strides.at(idx);
          start_addr += id_const_value(port.mem_indices.at(idx)) * stride;
        }
        port.starting_addr = start_addr;
        //std::cout << port << std::endl;
      }
      
      //std::cout << hwbuffer.name << " stride_x=" << ostream.stride_map["x"].stride << std::endl;

      //for (const auto& string_int_pair : hwbuffer.stride_map) {
      //std::cout << string_int_pair.first << "," << string_int_pair.second.stride << std::endl;
      //}
      
      FindInputStencil fis(hwbuffer.name, cur_func, func_compute_level, stencil_bounds);
      hwbuffer.my_stmt.accept(&fis);
      //std::cout << hwbuffer.name << " has input min pos " << fis.input_min_pos_box << std::endl;
      internal_assert(hwbuffer.dims.size() == fis.input_min_pos_box.size());
      //if (hwbuffer.dims.size() == fis.input_min_pos_box.size()) {
      {
        for (size_t idx=0; idx<hwbuffer.dims.size(); ++idx) {
          hwbuffer.dims.at(idx).input_min_pos = fis.input_min_pos_box.at(idx);
        }
      }
      //std::cout << hwbuffer.name << " set the input min pos as: " << fis.input_min_pos_box << std::endl
      //          << " while the output min pos is: " << fos.output_min_pos_box << std::endl;

      //std::cout << hwbuffer.my_stmt << std::endl;
      //std::cout << "the input chunk of " << hwbuffer.name
      //          << " at " << hwbuffer.compute_level
      //          << " is " << fis.input_chunk_box << std::endl;


      ostream.odims = vector<OutputDimSize>(hwbuffer.ldims.size());
      auto &odims = ostream.odims;
      if (stage.stage > 0) {
        StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
        auto rvars = update_schedule.rvars();
        for (size_t i=0; i<rvars.size(); ++i) {
          odims.at(i).output_min_pos = rvars[i].min;
          //std::cout << hwbuffer.name << " " << i << " update has min_pos=" << rvars[i].min << std::endl;
        }
      }

      for (size_t idx=0; idx<hwbuffer.dims.size(); ++idx) {
        //if (fos.found_stencil && idx < fos.output_stencil_box.size()) {
        if (fos.found_stencil) {
          internal_assert(hwbuffer.dims.size() == fos.output_stencil_box.size());
          hwbuffer.dims.at(idx).output_stencil = fos.output_stencil_box.at(idx);
          odims.at(idx).output_stencil = fos.output_stencil_box.at(idx);

          //std::cout << "replaced output stencil for " << hwbuffer.name << " based on consumer " << consumer.name << std::endl;
          //std::cout << "output min position is: " << fos.output_min_pos_box.at(idx) << std::endl;

          //std::cout << odims.at(idx).output_min_pos << " + " << fos.output_min_pos_box.at(idx) << std::endl;
        } else {
          //std::cout << hwbuffer.name << " couldn't find that output stencil thing\n";
          //std::cout << "doing some defaults for " << hwbuffer.name << std::endl;
          //std::cout << "before- output: " << odims.at(idx).output_block << "," << odims.at(idx).output_stencil << std::endl;
          odims.at(idx).output_stencil = hwbuffer.dims.at(idx).output_stencil;
          //std::cout << "after- output: " << odims.at(idx).output_block << "," << odims.at(idx).output_stencil << std::endl;
        }

        //std::cout << "replaced output min pos with new algo: " << hwbuffer.name << idx << "=" << odims.at(idx).output_min_pos << std::endl;
        
        /*
        if (fos.found_output_min) {
          //std::cout << "found the output min for " << hwbuffer.name << " to " << consumer.name << " idx" << idx << ": " << odims.at(idx).output_min_pos;
          //if (!is_zero(simplify(fos.output_min_pos_box.at(idx)))) {
          {
            odims.at(idx).output_min_pos = simplify(fos.output_min_pos_box.at(idx));
          }
          //std::cout << " => " << odims.at(idx).output_min_pos << std::endl;

        } else {
          //std::cout << "couldn't find the output min for " << hwbuffer.name << " to " << consumer.name << std::endl;
          odims.at(idx).output_min_pos = 0;
        }
        */

        odims.at(idx).output_min_pos = simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds));
        odims.at(idx).output_max_pos = simplify(expand_expr(consumer_bounds[idx].max, stencil_bounds));


        //auto loop_range = simplify(expand_expr(consumer_bounds[idx].max - consumer_bounds[idx].min + 1, stencil_bounds));
        //std::cout << "  range would/should be: " << loop_range << std::endl;

        //std::cout << "replaced input=" << hwbuffer.dims.at(idx).input_chunk
        //          << " and output=" << hwbuffer.dims.at(idx).output_stencil << std::endl;
      }

      //std::cout << "right before " << consumer.name << " replacements\n" << hwbuffer.output_access_pattern;
      // std::vector<std::string> input_streams;  // used when inserting read_stream calls
      //const auto& input_streams = hwbuffers.at(consumer.name).input_streams;
      //if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name)) {
      if (hwbuffers.count(consumer.name) &&  // does this work?
          (!hwbuffer.is_inlined || xcel->input_streams.count(hwbuffer.name)>0)) {
        ostream.output_access_pattern = hwbuffer.output_access_pattern;      
        //hwbuffers.at(consumer.name).input_streams.emplace_back(hwbuffer.name);
        ReplaceOutputAccessPatternRanges roapr(consumer_buffer);
        hwbuffer.output_access_pattern = roapr.mutate(hwbuffer.output_access_pattern);
        //ostream.output_access_pattern = roapr.mutate(hwbuffer.output_access_pattern);
      }
      //std::cout << "right after " << consumer.name << " replacements\n" << hwbuffer.output_access_pattern;


      for (size_t i=0; i < fos.output_block_box.size(); ++i) {
        ostream.odims.at(i).output_block   = fos.output_block_box.at(i);
        ostream.odims.at(i).output_min_pos = fos.output_min_pos_box.at(i);
        if (fos.found_stencil) {
          //std::cout << hwbuffer.name << " output stencil set\n";
          ostream.odims.at(i).output_stencil = fos.output_stencil_box.at(i);
        } else {
          if (!hwbuffer.is_inlined) {
            //std::cout << hwbuffer.name << " could not have output stencil set\n";
          }
        }
      }
      
    }

    // save the bounds values in scope
    internal_assert(cur_func.dimensions() == (int)hwbuffer.ostreams.begin()->second.odims.size());
    for (int i = 0; i < cur_func.dimensions(); i++) {
      string arg = cur_func.name() + ".s" + std::to_string(stage.stage) + "." + cur_func.args()[i];
      
      auto min_pos = hwbuffer.ostreams.begin()->second.odims.at(i).output_min_pos;
      auto max_pos = hwbuffer.ostreams.begin()->second.odims.at(i).output_max_pos;
      stencil_bounds.push(arg + ".min", min_pos);
      stencil_bounds.push(arg + ".max", max_pos);

      //std::cout << "pushed min pos " << arg + ".min" << " " << i << "=" << hwbuffer.dims[i].output_min_pos << "\n";
      //std::cout << "pushed max pos " << arg + ".max" << " " << i << "=" << hwbuffer.dims[i].output_max_pos << "\n";
    }
    if (stage.stage > 0) {
      //const Definition &r = cur_func.updates()[stage.stage - 1];
      StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
      // TODO check the sliding dimensions are all pure, referring to
      // BoundsInference::Stage::define_bounds()
      //if (r.domain.defined()) {
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
    //std::cout << "found a multiply: " << Expr(op) << "\n";
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
    //std::cout << "woah a divide: " << Expr(op) << "\n";
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
  Function func;
  int stream_dim_idx;
  const Scope<Expr> &scope;
  
  using IRVisitor::visit;
  
  void visit(const For *op);

public:
  const std::vector<std::string> &storage_names;
  const std::map<std::string,Stride> &stride_map;
  std::vector<std::string> varnames;

  std::map<std::string, int> dim_map;
  
  std::vector<int> ranges;
  std::vector<int> dim_refs;
  std::vector<int> strides_in_dim;
  IdentifyAddressing(const Function& func, const Scope<Expr> &scope, const std::map<std::string,Stride> &stride_map);
};



void IdentifyAddressing::visit(const For *op) {
  std::cout << "looking at " << op->name << std::endl;
  varnames.push_back(op->name);
  //internal_assert(is_zero(op->min)); FIXME

  // push range
  int range = id_const_value(expand_expr(op->extent, scope));
  //range = range < 0 && op->name=="conv.s1.r$z.r$z" ? 4 : range;
  internal_assert(range >= 0) << "the range is " << range << " for " << op->extent << " for " << op->name << "\n";

  auto tokens = get_tokens(op->name, ".");
  auto varname = tokens.size() > 2 ? tokens.at(2) : op->name;
  std::cout << op->name << " is probably referring to storage " << varname << std::endl;
    
  //uint pos = std::find(storage_names.begin(), storage_names.end(), op->name) - storage_names.begin();

  // determine dim_ref and stride
  //if (pos < storage_names.size()) {
  if (dim_map.count(varname)>0) {
    //std::cout << "this is a streaming loop: " << varname << " for " << op->name << "\n";

    //std::cout << "stride_map: ";
    //for (const auto& string_int_pair : stride_map) {
      //std::cout << string_int_pair.first << "," << string_int_pair.second.stride << "  ";
    //}
    //std::cout << std::endl;
      
    //std::cout << " and mapcount=" << stride_map.size() << " " << stride_map.count(varname) << std::endl;
    int stride = stride_map.count(varname)>0 ? stride_map.at(varname).stride : 1;
    bool is_inv = stride_map.count(varname)>0 ? stride_map.at(varname).is_inverse : false;
    //int stride = 1;
    //std::cout << "stride=" << stride << " is_inv=" << is_inv << std::endl;

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
      //std::cout << "not inverse variable " << varname << " for func " << func.name() << std::endl;
      dim_refs.insert(dim_refs.begin(), dim_map.at(varname));
      strides_in_dim.insert(strides_in_dim.begin(), stride);
      ranges.insert(ranges.begin(), range);
    }

    //std::cout << op->name << " has stride=" << strides_in_dim.at(0)
    //          << " dim_ref=" << dim_refs.at(0)
    //           << " range=" << ranges.at(0) << "\n";
      
  } else {
    IdentifyAddressingVar iav(op->name, scope);
    //std::cout << "finding stride and range for " << op->name << "\n in: " << op->body << std::endl;
    (op->body).accept(&iav);
    dim_refs.insert(dim_refs.begin(), iav.dim_ref);
    strides_in_dim.insert(strides_in_dim.begin(), iav.stride);
    ranges.insert(ranges.begin(), range);
    //std::cout << op->name << " has stride=" << iav.stride << " dim_ref=" << iav.dim_ref << "\n";
  }

  IRVisitor::visit(op);
}



IdentifyAddressing::IdentifyAddressing(const Function& func, const Scope<Expr> &scope, const map<string,Stride> &stride_map) :
  func(func), stream_dim_idx(0), scope(scope), storage_names(func.args()), stride_map(stride_map) {
  const auto &sch = func.definition().schedule();
  const auto &splits = sch.splits();

  //std::cout << "populating dim map for " << func.name() << std::endl;
  // populate dim map with names
  for (size_t i=0; i < storage_names.size(); ++i) {
    dim_map[storage_names.at(i)] = i;
    //std::cout << "storage name=" << storage_names.at(i) << " has dim_idx=" << i << "\n";
  }

  // add any splits
  for (auto split : splits) {
    //std::cout << "remapping " << split.old_var << " with " << split.outer << " and " << split.inner << "\n";
    if (dim_map.count(split.old_var)) {
      dim_map[split.outer] = dim_map.at(split.old_var);
      dim_map[split.inner] = dim_map.at(split.old_var);
    }
  }
    
  //std::cout << "going to be looking for range and strides where storage=" << storage_names << std::endl;
}

void linearize_address_space(HWBuffer &kernel) {
  std::cout << "linearizing for " << kernel.name << std::endl;
  if (kernel.is_output) {
    std::cout << kernel << std::endl;
    OutputStream& ostream = kernel.ostreams.at("output");
    IdentifyAddressing id_addr(kernel.func, Scope<Expr>(), ostream.stride_map);
    ostream.output_access_pattern.accept(&id_addr);
    std::cout << "output_access for " << kernel.name << " to " << ostream.name << std::endl
              << id_addr.ranges;

      auto num_access_levels = id_addr.ranges.size();
      assert(num_access_levels == id_addr.strides_in_dim.size());
      assert(num_access_levels == id_addr.dim_refs.size());
      
      //istream.linear_addr.resize(num_access_levels);
      ostream.linear_access.resize(num_access_levels);
      size_t j = 0;
      for (size_t i=0; i<num_access_levels; ++i) {
        if (id_addr.ranges.at(i) != 1) {
          //istream.linear_addr.at(j).range = id_addr.ranges.at(i);
          //istream.linear_addr.at(j).stride = id_addr.strides_in_dim.at(i);
          //istream.linear_addr.at(j).dim_ref = id_addr.dim_refs.at(i);
          
          ostream.linear_access.at(j).range = id_addr.ranges.at(i);
          ostream.linear_access.at(j).stride = id_addr.strides_in_dim.at(i);
          ostream.linear_access.at(j).dim_ref = id_addr.dim_refs.at(i);

          j += 1;
        }
      }
      //istream.linear_addr.resize(j);
      ostream.linear_access.resize(j);

    
  }
  //for (auto& istream_pair : kernel.producer_buffers) {
  //HWBuffer& istream = *istream_pair.second;
  for (auto& istream_pair : kernel.istreams) {
    HWBuffer& istream = *istream_pair.second.hwref;
    //std::cout << "dealing with istream " << istream.name << std::endl;
    
    OutputStream& ostream = istream.ostreams.at(kernel.name);

    IdentifyAddressing id_addr(istream.func, Scope<Expr>(), ostream.stride_map);
    //std::cout << "output_access for " << istream.name << " to " << kernel.name << std::endl
    //          << istream.output_access_pattern;
    istream.output_access_pattern.accept(&id_addr);

    if (istream.name != kernel.name) {
      //std::cout << istream.output_access_pattern;
      //std::cout << istream.name << " is a producer for " << kernel.name << std::endl
      //          << "  range: " << id_addr.ranges << std::endl
      //          << "  stride: " << id_addr.strides_in_dim << std::endl
      //          << "  dim_refs: " << id_addr.dim_refs << std::endl;

      auto num_access_levels = id_addr.ranges.size();
      assert(num_access_levels == id_addr.strides_in_dim.size());
      assert(num_access_levels == id_addr.dim_refs.size());
      
      //istream.linear_addr.resize(num_access_levels);
      ostream.linear_access.resize(num_access_levels);
      size_t j = 0;
      for (size_t i=0; i<num_access_levels; ++i) {
        if (id_addr.ranges.at(i) != 1) {
          //istream.linear_addr.at(j).range = id_addr.ranges.at(i);
          //istream.linear_addr.at(j).stride = id_addr.strides_in_dim.at(i);
          //istream.linear_addr.at(j).dim_ref = id_addr.dim_refs.at(i);
          
          ostream.linear_access.at(j).range = id_addr.ranges.at(i);
          ostream.linear_access.at(j).stride = id_addr.strides_in_dim.at(i);
          ostream.linear_access.at(j).dim_ref = id_addr.dim_refs.at(i);

          j += 1;
        }
      }
      //istream.linear_addr.resize(j);
      ostream.linear_access.resize(j);
      //std::cout << ostream.linear_access << std::endl << istream;
      //std::cout << &istream << std::endl;
    }
  }
}

void calculate_accumulation(HWBuffer &kernel) {
    bool has_accum = kernel.ostreams.count(kernel.name) > 0;
    int num_logical_pixels = 1;
    for (size_t i = 0; i < kernel.ldims.size(); i++) {
      Expr store_extent = kernel.ldims[i].logical_size;
      num_logical_pixels *= to_int(store_extent);
    }
    
    //for (auto& istream_pair : kernel.producer_buffers) {
    //auto& istream = *istream_pair.second;
    for (auto& istream_pair : kernel.istreams) {
      HWBuffer& istream = *istream_pair.second.hwref;
      auto& ostream = istream.ostreams.at(kernel.name);

      IdentifyAddressing id_addr(istream.func, Scope<Expr>(), ostream.stride_map);
      istream.output_access_pattern.accept(&id_addr);
      int num_iter=1;
      for (size_t i = 0; i < id_addr.ranges.size(); i++) {
        num_iter *= id_addr.ranges.at(i);
      }
      
      if (has_accum && num_logical_pixels != num_iter) {
        kernel.num_accum_iters = num_iter / num_logical_pixels;
      }

      //std::cout << kernel.name << " has a total of " << num_logical_pixels << " iterations and producer "
      //          << istream.name << " has " << num_iter << " iterations"
      //          << "  has_accum=" << has_accum << std::endl;
    }
}

void fixup_hwbuffer(HWBuffer& hwbuffer) {
  for (auto& ostream_pair : hwbuffer.ostreams) {
    auto& ostream = ostream_pair.second;

    for (auto& dim : ostream.odims) {
      if (!dim.output_stencil.defined()) {
        dim.output_stencil = Expr(99);
      }
      if (!dim.output_block.defined()) {
        dim.output_block = Expr(99);
      }

    }
    
    for (auto& dim : ostream.linear_access) {
      if (is_one(dim.dim_ref > 10 || dim.dim_ref < 0)) {
        dim.dim_ref = 0;
      }
    }
  }

  for (auto& dim : hwbuffer.dims) {
    if (!dim.output_stencil.defined()) {
      dim.output_stencil = Expr(99);
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
  for (auto xcel_input : func.schedule().accelerate_inputs()) {
    xcel->input_streams.emplace(xcel_input);
  }

  //std::cout << "creating an accelerator for " << func.name() << std::endl;// << s << std::endl;

  //std::cout << xcel->name << " has the streaming loops: ";
  //for (const auto& streaming_loop_name : xcel->streaming_loop_levels) {
    //std::cout << streaming_loop_name << " ";
  //}
  //std::cout << "\n";

  Scope<Expr> output_scope;
  //find_output_scope(s, func, xcel->compute_level, output_scope);
  auto output_box = find_output_bounds(s, func, xcel->compute_level);
  //std::cout << "output bounds: " << output_box << std::endl;

  // use realizes to define each hwbuffer
  //std::cout << "lets extract\n" << s << std::endl;
  xcel->hwbuffers = extract_hw_buffers(s, env, xcel);
  //std::cout << "got some buffers" << std::endl;

  // set output parameters for hwbuffers based on consumers
  set_output_params(xcel, env, inlined, xcel->streaming_loop_levels, output_scope);
  //std::cout << "set stream params" << std::endl;

  for (auto &hwbuffer_pair : xcel->hwbuffers) {
    linearize_address_space(hwbuffer_pair.second);
    calculate_accumulation(hwbuffer_pair.second);
  }
  //std::cout << "linearized address space" << std::endl;
  
  for (auto &hwbuffer_pair : xcel->hwbuffers) {

    auto& kernel = hwbuffer_pair.second;
    fixup_hwbuffer(kernel);
    //std::cout << hwbuffer_pair.first << " is extracted w/ inline=" << kernel.is_inlined << " and num_dims=" << kernel.dims.size() << std::endl;
    std::cout << "Final buffer:\n" << kernel << std::endl; // << kernel.my_stmt;

    //auto num_inputs = kernel.func.updates().size() + 1;
    //auto num_outputs = kernel.consumer_buffers.size();
    //auto num_outputs = kernel.ostreams.size();
    //std::cout << "num_in=" << num_inputs << "   num_out=" << num_outputs << std::endl;

    if (kernel.ostreams.count(kernel.name)) {
      //std::cout << "accumulate=" << " cycles" << std::endl;
    } else {
      //std::cout << "no accumulation" << std::endl;
    }
    //std::cout << std::endl;
    
    //kernel.streaming_loops = xcel->streaming_loop_levels;
    //kernel.compute_level = xcel->streaming_loop_levels.back();
  }

}

vector<HWXcel> extract_hw_accelerators(Stmt s, const map<string, Function> &env,
                                const vector<BoundsInference_Stage> &inlined_stages) {

  vector<HWXcel> xcels;
  
  s = substitute_in_constants(s);
  
  for (auto stage : inlined_stages) {
    //std::cout << "stage includes " << stage.name << std::endl;
  }
    
  // for each accelerated function, build a hardware xcel: a dag of HW kernels 
  for (const auto &p : env) {
    
    Function func = p.second;
    // skip this function if it is not accelerated
    if(!func.schedule().is_accelerated())
      continue;
    
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
    //std::cout << "initial loopnest is: \n" << s << std::endl;
    extract_hw_xcel_top_parameters(s, func, env, inlined_stages, &xcel);
    xcels.push_back(xcel);
  }
  return xcels;
}


}  // namespace Internal
}  // namespace Halide
