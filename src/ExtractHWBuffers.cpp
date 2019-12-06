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

std::ostream& operator<<(std::ostream& os, const std::vector<AccessDimSize>& vec) {
  vector<Expr> range, stride, dim_ref;
  for (const auto& dim : vec) {
    range.emplace_back(dim.range);
    stride.emplace_back(dim.stride);
    dim_ref.emplace_back(dim.dim_ref);
  }
  
  os << "Range: " << range << std::endl
     << "Stride: " << stride << std::endl
     << "Dim Ref: " << dim_ref << std::endl;
  
  return os;
};


std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer) {
  vector<Expr> total_buffer_box, input_chunk_box, input_block_box;
  vector<Expr> output_stencil_box, output_block_box, output_min_pos;
  for (const auto& dim : buffer.ldims) {
    total_buffer_box.emplace_back(dim.logical_size);
  }
  for (const auto& dim : buffer.dims) {
    input_chunk_box.emplace_back(dim.input_chunk);
    input_block_box.emplace_back(dim.input_block);
    output_stencil_box.emplace_back(dim.output_stencil);
    output_block_box.emplace_back(dim.output_block);
    //output_min_pos.emplace_back(dim.output_min_pos);
  }

  vector<string> input_istreams, output_ostreams;
  map<string, vector<Expr> > ostream_output_mins;
  map<string, vector<Expr> > ostream_output_stencils;
  for (const auto& istream_pair : buffer.istreams) {
    input_istreams.emplace_back(istream_pair.first);
  }
  for (const auto& ostream_pair : buffer.ostreams) {
    vector<Expr> consumer_output_min;
    vector<Expr> consumer_output_stencil;
    output_ostreams.emplace_back(ostream_pair.first);
    for (const auto& dim : ostream_pair.second.odims) {
      consumer_output_min.emplace_back(dim.output_min_pos);
      consumer_output_stencil.emplace_back(dim.output_stencil);
    }
    ostream_output_mins[ostream_pair.first] = consumer_output_min;
    ostream_output_stencils[ostream_pair.first] = consumer_output_stencil;
  }

  //auto num_inputs = 0;//buffer.func.updates().size();
  //auto num_outputs = 0;//buffer.consumer_buffers.size();
  
  os << "HWBuffer: " << buffer.name << std::endl
     << "Logical Buffer: " << total_buffer_box << std::endl
     << "Input Chunk: " << input_chunk_box << std::endl
     << "Input Block: " << input_block_box << std::endl
     << "Output Stencil: " << output_stencil_box << std::endl
     << "Output Block: " << output_block_box << std::endl
     << "Output Access Pattern:\n " << buffer.output_access_pattern << std::endl;
    //<< "Output Min Pos: " << output_min_pos << std::endl;

  os << buffer.linear_addr << std::endl;
  
  for (const auto& omp_pair : ostream_output_mins) {
    os << "Ostream " << omp_pair.first << " Min Pos: "
       << omp_pair.second << std::endl;
  }
  for (const auto& osten_pair : ostream_output_stencils) {
    os << "Ostream " << osten_pair.first << " Output Stencil: "
       << osten_pair.second << std::endl;
  }
  os << "streaming loops: " << buffer.streaming_loops << std::endl
     << "compute level: " << buffer.compute_level << std::endl
     << "store level: " << buffer.store_level << std::endl
     << "is_inline=" << buffer.is_inlined << std::endl
     << "is_output=" << buffer.is_output << std::endl
     << "input_streams=" << input_istreams << std::endl
     << "output_streams=" << output_ostreams << std::endl;
  //<< "num_inputs=" << num_inputs << std::endl
  //<< "num_output=" << num_outputs << std::endl;

  
  return os;
};

int to_int(Expr expr) {
  //FIXMEyikes internal_assert(is_const(simplify(expr)));
  if (is_const(simplify(expr))) {
    return (int)*as_const_int(expr);
  } else {
    return -1;
  }
}

HWBuffer::HWBuffer(string name, vector<MergedDimSize> mdims, vector<AccessDimSize> linear_addr,
                   vector<string> loops, int store_index, int compute_index, bool is_inlined, bool is_output,
                   string iname, string oname) :
  name(name), store_level(store_index < 0 ? "" : loops[store_index]),
  compute_level(compute_index < 0 ? "" : loops[compute_index]),
  is_inlined(is_inlined), is_output(is_output), linear_addr(linear_addr) {
  loops.erase(loops.begin());
  streaming_loops = loops;

  ldims = std::vector<LogicalDimSize>(mdims.size());
  for (size_t i=0; i<mdims.size(); ++i) {
    ldims[i].logical_size = mdims[i].logical_size;
    ldims[i].logical_min = mdims[i].logical_min;
  }

  InputStream istream;
  istream.idims = std::vector<InputDimSize>(mdims.size());
    
  OutputStream ostream;
  ostream.odims = std::vector<OutputDimSize>(mdims.size());
    
  dims = std::vector<InOutDimSize>(mdims.size());
  for (size_t i=0; i<mdims.size(); ++i) {
    istream.idims.at(i).loop_name       = mdims.at(i).loop_name;
    istream.idims.at(i).input_chunk     = mdims.at(i).input_chunk;
    istream.idims.at(i).input_block     = mdims.at(i).input_block;
      
    ostream.odims.at(i).loop_name      = mdims.at(i).loop_name;
    ostream.odims.at(i).output_stencil = mdims.at(i).output_stencil;
    ostream.odims.at(i).output_block   = mdims.at(i).output_block;
    ostream.odims.at(i).output_min_pos = mdims.at(i).output_min_pos;
    ostream.odims.at(i).output_max_pos = mdims.at(i).output_max_pos;
  }
  istreams[iname] = istream;
  ostreams[oname] = ostream;

  // old way
  dims = std::vector<InOutDimSize>(mdims.size());
  for (size_t i=0; i<mdims.size(); ++i) {
    dims[i].loop_name      = mdims[i].loop_name;
    dims[i].input_chunk    = mdims[i].input_chunk;
    dims[i].input_block    = mdims[i].input_block;
    dims[i].output_stencil = mdims[i].output_stencil;
    dims[i].output_block   = mdims[i].output_block;
    //dims[i].output_min_pos = mdims[i].output_min_pos;
    //dims[i].output_max_pos = mdims[i].output_max_pos;
  }
    
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

std::vector<AccessDimSize> create_linear_addr(std::vector<int> ranges,
                                              std::vector<int> strides,
                                              std::vector<int> dim_refs) {
   internal_assert(ranges.size() == strides.size());
   internal_assert(ranges.size() == dim_refs.size());

   std::vector<AccessDimSize> dims(ranges.size());

   for (size_t i=0; i < ranges.size(); ++i) {
     dims[i] = AccessDimSize({Expr(ranges.at(i)), Expr(strides.at(i)), Expr(dim_refs.at(i))});
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

class ContainsCall : public IRVisitor {
  string var;
  
  using IRVisitor::visit;
  
  void visit(const Call *op) {
    if (op->name == var) {
      found = true;
    } else {
      IRVisitor::visit(op);
    }
  }
public:
  bool found;
  ContainsCall(string var) : var(var), found(false) {}
};

bool contains_call(Stmt s, string var) {
  ContainsCall cc(var);
  s.accept(&cc);
  return cc.found;
}

class ExamineLoopLevel : public IRVisitor {
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

        std::cout << "HWBuffer Parameter: writer ports - "
                  << "box extent=[";
        auto interval = box_write;
        input_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          std::cout << lower_expr << "-" << upper_expr << " ";
          input_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        }
        std::cout << "]\n";

      }

      // look at the readers (reader ports, read address gen)
      if (call_at_level(op->body, var)) {
        auto box_read = box_required(op->body, var);
        //std::cout << "readers inside pc " << op->name << std::endl;
        //std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
        //std::cout << Stmt(op->body) << std::endl;
        std::cout << "HWBuffer Parameter: reader ports - "
                  << "box extent=[";
        auto interval = box_read;

        output_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
          std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        }
        std::cout << "]\n";
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
        std::cout << "added first for loop: " << op->name << " for " << var << std::endl;

        auto box_read = box_required(op->body, var);
        auto interval = box_read;
        for (size_t dim=0; dim<interval.size(); ++dim) {
          auto assertstmt = AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope)));
          //std::cout << "min pos in dim " << dim << ":" << assertstmt;
        }
      } else if (!is_parallelized(op)) {
        current_for->body = for_stmt;
        std::cout << "added nonparallel for loop: " << op->name << std::endl;
      }
      current_for = const_cast<For *>(for_stmt.as<For>());

      
      IRVisitor::visit(op);


      if (call_at_level(op->body, var)) {
        std::cout << op->name << " call found for reader loopnest\n";
      }
    
      // look at the readers (reader ports, read address gen)
      if (call_at_level(op->body, var) && !is_parallelized(op)) {
        auto box_read = box_required(op->body, var);
        //std::cout << "readers inside loop " << op->name << std::endl;
        std::cout << "Box reader found for " << var << " with box " << box_read << std::endl;
        //std::cout << Stmt(op->body) << std::endl;
        std::cout << "HWBuffer Parameter: reader ports - "
                  << "box extent=[";
        auto interval = box_read;

        vector<Stmt> stmts;
        output_block_box = vector<Expr>(interval.size());
        for (size_t dim=0; dim<interval.size(); ++dim) {
          Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
          Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
          Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
          output_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
          stmts.push_back(AssertStmt::make(var + "_dim" + std::to_string(dim), simplify(expand_expr(interval[dim].min, scope))));
          std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        }
        std::cout << "]\n";

        const vector<Stmt> &const_stmts = stmts;
        current_for->body = Block::make(const_stmts);
        reader_loopnest = simplify(expand_expr(full_stmt, scope));
        std::cout << "HWBuffer Parameter - " << var << " nested reader loop:\n" << reader_loopnest << std::endl;

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

      std::cout << "HWBuffer Parameter: writer ports - "
                << "box extent=[";
      auto interval = box_write;
      input_block_box = vector<Expr>(interval.size());
      for (size_t dim=0; dim<interval.size(); ++dim) {
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        std::cout << lower_expr << "-" << upper_expr << " ";
        input_block_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
      }
      std::cout << "]\n";

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
      // std::cout << "call for " << op->name << " includes: " << op->args << std::endl;
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


class ReplaceForBounds : public IRMutator2 {
  using IRMutator2::visit;
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
    return IRMutator2::visit(op);
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
      std::cout << var << " found compute level with for loop " << op->name << std::endl;
      //std::cout << "before for bound replacement\n" << Stmt(op->body);

      ReplaceForBounds rfb;
      Stmt new_body = rfb.mutate(op->body);

      //std::cout << "after for bound replacement\n" << Stmt(new_body);

      //if (scope.contains(op->name + ".min")) {
      //  std::cout << "loop min should be " << scope.get(op->name + ".min") << std::endl;
      //} else {
      //  std::cout << "couldn't find a loop min\n";
      //}
      
      //std::cout << op->body << std::endl;
      std::cout << new_body << std::endl;
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

      std::cout << "HWBuffer Parameter: " << var << " output stencil size - "
                << "box extent=[";

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        output_stencil_box[dim] = lower_expr.defined() ? lower_expr : port_expr;
        std::cout << port_expr << "?" << lower_expr.defined() << ":" << lower_expr << "-" << upper_expr  << " ";
        //output_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, scope));
        output_min_pos_box[dim] = interval[dim].min;
        std::cout << "(" << output_min_pos_box[dim] << "," << interval[dim].min << ")  ";
        std::cout << " using " << interval[dim].max - interval[dim].min + 1 << std::endl;
      }
      std::cout << "]\n";

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
    std::cout << "looking to find " << v << " output stencil where compute_level=" << compute_level << std::endl;
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
      
      std::cout << "HWBuffer Parameter: " << var << " input chunk size - "
                << "box extent=[";

      for (size_t dim=0; dim<interval.size(); ++dim) {
        found_stencil = true;
        Expr port_expr = simplify(expand_expr(interval[dim].max - interval[dim].min + 1, scope));
        Expr lower_expr = find_constant_bound(port_expr, Direction::Lower);
        Expr upper_expr = find_constant_bound(port_expr, Direction::Upper);
        input_chunk_box[dim] = is_undef(lower_expr) ? port_expr : lower_expr;
        std::cout << port_expr << ":" << lower_expr << "-" << upper_expr  << " ";
        output_min_pos_box[dim] = simplify(expand_expr(interval[dim].min, stencil_bounds));
        std::cout << "(" << output_min_pos_box[dim] << "," << interval[dim].min << ")  ";
      }
      std::cout << "]\n";

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

class ReplaceOutputAccessPatternRanges : public IRMutator2 {
  using IRMutator2::visit;
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

    Stmt s = IRMutator2::visit(old_op);
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

  void visit(const ProducerConsumer *op) {
    // match store level at PC node in case the looplevel is outermost
    std::cout << "looking at pc " << op->name << " where " << outer_loop_exclusive.lock().func()
              << outer_loop_exclusive.lock().var().name() << " and " << Var::outermost().name()
              << std::endl;
    if (outer_loop_exclusive.lock().func() == op->name && 
        outer_loop_exclusive.lock().var().name() == Var::outermost().name()) {
      in_inner_loops = true;
    }
    IRVisitor::visit(op);
  }

  void visit(const For *op) {
    // scan loops are loops between the outer store level (exclusive) and
    // the inner compute level (inclusive) of the accelerated function
    std::cout << "considering loop " << op->name << " to inner loops. in="
              << in_inner_loops << " sw=" << starts_with(op->name, func.name() + ".") << "\n";
    //if (in_inner_loops && starts_with(op->name, func.name() + ".")) {
    bool added_loop = false;
    if (in_inner_loops) {
      added_loop = true;
      debug(3) << "added loop " << op->name << " to inner loops.\n";
      inner_loops.emplace_back(op->name);
      std::cout << "added loop to scan loop named " << op->name << " with extent=" << op->extent << std::endl;
    }

    // reevaluate in_scan_loops in terms of loop levels
    if (outer_loop_exclusive.match(op->name)) {
      in_inner_loops = true;
    }
    if (in_inner_loops && inner_loop_inclusive.match(op->name)) {
      returned_inner_loops = inner_loops;
      in_inner_loops = false;
      std::cout << "updated inner loops: " << returned_inner_loops << std::endl;
    }

    // Recurse
    IRVisitor::visit(op);
    if (added_loop) {
      inner_loops.pop_back();
      std::cout << "updated loops: " << inner_loops << std::endl;
      in_inner_loops = true;
    }
  }

public:  
  FindInnerLoops(Function f, LoopLevel outer_level, LoopLevel inner_level, bool inside)
    : func(f), outer_loop_exclusive(outer_level), inner_loop_inclusive(inner_level),
      in_inner_loops(inside) { }

  vector<string> inner_loops;
  vector<string> returned_inner_loops;
  
};

// Produce a vector of the loops within a for-loop-nest.
//   Note: this can be used to find the streaming loops between the store and compute level.
vector<string> get_loop_levels_between(Stmt s, Function func,
                                       LoopLevel outer_level_exclusive,
                                       LoopLevel inner_level_inclusive,
                                       bool start_inside = false) {
  FindInnerLoops fil(func, outer_level_exclusive, inner_level_inclusive, start_inside);
  std::cout << "find some loops: " << s << std::endl;
  s.accept(&fil);
  std::cout << "got some loops: " << fil.returned_inner_loops << std::endl;
  return fil.returned_inner_loops;
}

}

class HWBuffers : public IRMutator2 {
    const map<string, Function> &env;
    Scope<Expr> scope;

    using IRMutator2::visit;
  
    Stmt visit(const LetStmt *op) override {
      ScopedBinding<Expr> bind(scope, op->name, simplify(expand_expr(op->value, scope)));
      return IRMutator2::visit(op);
    }

    Stmt visit(const Realize *op) override {
        // Find the args for this function
        map<string, Function>::const_iterator iter = env.find(op->name);

        // If it's not in the environment it's some anonymous
        // realization that we should skip (e.g. an inlined reduction)
        if (iter == env.end()) {
            return IRMutator2::visit(op);
        }

        // find the function
        const FuncSchedule &sched = iter->second.schedule();
        if (!sched.is_hw_kernel()) {
          std::cout << "skipping non-hwkernel realize " << op->name << std::endl;
          return IRMutator2::visit(op);
        }
        Function func = iter->second;

        // create the hwbuffer
        HWBuffer hwbuffer;
        hwbuffer.name = op->name;
        std::cout << hwbuffer.name << " found realize for new hwbuffer" << std::endl;
        hwbuffer.func = func;
        hwbuffer.my_stmt = op->body;
        Stmt new_body = op->body;
        //std::cout << "Encountered realize " << op->name << " with body:\n" << op->body << std::endl;

        // If the Function in question has the same compute_at level
        // as its store_at level, we know this is a double buffer.
        std::cout << "creating buffer for realize " << op->name << std::endl;
        string xcel_compute_level = loop_names.at(loop_names.size()-1);
        string xcel_store_level = loop_names.at(0);
        std::cout << "xcel compute level is " << xcel_compute_level << std::endl;

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
        std::cout << "looking at loops, func " << op->name << " has store=" << store_locked.to_string()
                  << "  compute=" << compute_locked.to_string() << std::endl;
        //hwbuffer.store_level = store_locked.to_string();
        //hwbuffer.compute_level = compute_locked.to_string();

        // create the streaming loops
        if (compute_level == store_level) {
          hwbuffer.streaming_loops = {compute_level};
        } else if (xcel->store_level.match(store_level)) {
          std::cout << hwbuffer.name << " store level matches xcel\n";
          hwbuffer.streaming_loops = get_loop_levels_between(Stmt(op), xcel->func, store_locked, compute_locked, true);
        } else {
          user_error << "xcel store loop is different from the hwbuffer store loop. no good.\n";
        }
        //std::cout << Stmt(op) << std::endl;
        std::cout << "streaming loops: " << hwbuffer.streaming_loops << std::endl;
        std::cout << "store: " << store_locked << "  compute: " << compute_locked << std::endl;
        std::cout << "xcel streaming loops: " << loop_names << std::endl;
        hwbuffer.compute_level = hwbuffer.streaming_loops.back();
        //hwbuffer.store_level = hwbuffer.streaming_loops.front();
        hwbuffer.store_level = xcel->store_level.to_string();

        // Simplification possible when compute and store is the same level
        if (sched.compute_level() == sched.store_level()) {
          std::cout << op->name << " has compute=store level\n";

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

          std::cout << "HWBuffer Parameter: Total buffer box is of size [";          
          for (size_t i=0; i<box.size(); ++i) {
            std::cout << box[i];
            if (i < box.size() - 1) { std::cout << " "; }
          }
          std::cout << "]\n";

          std::cout << "HWBuffer Parameter: Input Chunk box is of size [";
          for (size_t i=0; i<box.size(); ++i) {
            std::cout << box[i];
            if (i < box.size() - 1) { std::cout << " "; }
          }
          std::cout << "]\n";
          
          std::cout << "HWBuffer Parameter: Output Stencil box is of size [";
          for (size_t i=0; i<box.size(); ++i) {
            std::cout << box[i];
            if (i < box.size() - 1) { std::cout << " "; }
          }
          std::cout << "]\n";

          //FindOutputStencil fos(op->name, func, func_store_level);
          FindOutputStencil fos(op->name, func, hwbuffer.compute_level);
          new_body.accept(&fos);

          //std::cout << "counting:\n" << new_body << std::endl;
          CountBufferUsers counter(op->name);
          new_body.accept(&counter);

          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          std::cout << "output block: " << output_block_box << std::endl;
          auto input_block_box = counter.input_block_box;
          std::cout << "input block: " << input_block_box << std::endl;
          if (input_block_box.size() == 0) {
            input_block_box = output_block_box;
          }
          auto reader_loopnest = counter.reader_loopnest;

          internal_assert(boxes_read.at(op->name).size() == output_block_box.size());
          //internal_assert(boxes_read.at(op->name).size() == input_block_box.size());
          
          std::cout << "transferring parameters\n";
          std::string for_name = first_for_name(new_body);
          //HWBuffer hwbuffer(output_block_box.size(), sliding_stencil_map.at(for_name));
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());

          LoopLevel store_l = sched.store_level();
          //hwbuffer.store_level = store_l.lock().func();
          
          hwbuffer.stride_map = fos.stride_map;
          std::cout << hwbuffer.name << " stride_x=" << hwbuffer.stride_map["x"].stride << std::endl;

          hwbuffer.ldims = vector<LogicalDimSize>(output_block_box.size());

          //hwbuffer.dims = vector<MergedDimSize>(output_block_box.size());
          for (size_t i = 0; i < output_block_box.size(); ++i) {
            hwbuffer.ldims[i].logical_min = Expr(0);
            hwbuffer.ldims[i].logical_size = box.at(i);
            
            //Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            //hwbuffer.dims.at(i).logical_size = extent;
            //std::cout << hwbuffer.name << " has a logical size at " << i << " of " << extent << std::endl;
            //hwbuffer.dims[i].logical_size = simplify(boxes_read.at(op->name)[i].max - boxes_write.at(op->name)[i].min + 1);

            //hwbuffer.dims[i].input_chunk = box.at(i);
            hwbuffer.dims[i].input_chunk = input_block_box.at(i);
            hwbuffer.dims[i].input_block = input_block_box.at(i);

            hwbuffer.dims[i].output_stencil = output_block_box.at(i); // used for hw_input
            hwbuffer.dims[i].output_block   = output_block_box.at(i); // used for hw_input

            //hwbuffer.dims[i].output_min_pos = boxes_read.at(op->name)[i].min;
            //std::cout << "hwbuffer " << hwbuffer.name << " in dim " << i <<
            //  " has min_pos=" << hwbuffer.dims[i].output_min_pos << std::endl;
            hwbuffer.dims[i].loop_name = i < loop_names.size() ? loop_names.at(i) : unique_name("loopvar");
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          std::cout << "created hwbuffer " << hwbuffer.name << "\n";

          if (buffers.count(hwbuffer.name) == 0) {
            std::cout << "Here is the hwbuffer (store=compute):"
                      << " [" << buffers.count(hwbuffer.name) << "]\n"
                      << hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }
          
          return IRMutator2::visit(op);
          
        } else {
          // look for a sliding window that can be used in a line buffer
          std::cout << "looking at hwbuffer with diff compute and store levels: " << hwbuffer.name << std::endl;

          //std::cout << "Doing sliding window analysis on realization of " << op->name << "\n";

          // use sliding window to get stencil sizes
          // Parameters 1 and 2
          auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
          new_body = mutate(new_body);
          
          std::string for_namer = first_for_name(new_body);
          std::cout << op->name << " sliding output=" << sliding_stencil_map.at(for_namer).output_stencil_box
                    << " input=" << sliding_stencil_map.at(for_namer).input_chunk_box << std::endl;

          //FindOutputStencil fos(op->name, func, xcel_compute_level);
          FindOutputStencil fos(op->name, func, hwbuffer.compute_level);
          new_body.accept(&fos);
          auto output_stencil_box = fos.output_stencil_box;
          std::cout << "output stencil is " << output_stencil_box << " using loops: " << hwbuffer.streaming_loops << std::endl;

          //std::cout << "counting lbed:\n" << new_body << std::endl;
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
          std::cout << hwbuffer.name << " has reader loopnest:\n" << reader_loopnest;

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
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            total_buffer_box[i] = extent;

            std::cout << total_buffer_box[i];
            if (i < i_max - 1) { std::cout << extent << " "; }
          }
          std::cout << "]\n";

          // create the hwbuffer
          std::string for_name = first_for_name(new_body);
          //HWBuffer hwbuffer(total_buffer_box.size(), sliding_stencil_map.at(for_name));//(sliding_stencil_map.at(for_name).input_chunk_box.size());
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());
          //hwbuffer.compute_level = compute_locked.to_string();//func_compute_level;
          //hwbuffer.store_level = store_locked.to_string();//func_store_level;

          hwbuffer.ldims = vector<LogicalDimSize>(output_block_box.size());
          hwbuffer.stride_map = fos.stride_map;
          std::cout << hwbuffer.name << " stride_x=" << hwbuffer.stride_map["x"].stride << std::endl;

          // check that all of the extracted parameters are of the same vector length
          //FIXMEyikes
          std::cout << "HWBuffer has " << hwbuffer.dims.size() << " dims, while " << total_buffer_box.size() << " num box_dims\n";
          std::cout << " loops are: " << loop_names << std::endl;
          std::cout << " hwbuffer loops are: " << hwbuffer.streaming_loops << std::endl;

          //internal_assert(hwbuffer.dims.size() == loop_names.size()) << "HWBuffer has " << hwbuffer.dims.size() << " dims, while only " << loop_names.size() << " loops\n";
          //internal_assert(loop_names.size() == hwbuffer.input_stencil->output_stencil_box.size());
          //internal_assert(loop_names.size() == sliding_stencil_map.at(for_name).output_stencil_box.size());
          //internal_assert(hwbuffer.dims.size() == output_stencil_box.size()) << "output_stencil_box size is " << output_stencil_box.size() << "\n";
          
          internal_assert(hwbuffer.dims.size() == total_buffer_box.size());
          internal_assert(hwbuffer.dims.size() == output_block_box.size());
          internal_assert(hwbuffer.dims.size() == input_block_box.size());

          for (size_t i = 0; i < hwbuffer.dims.size(); ++i) {
            //hwbuffer.dims[i].logical_size = total_buffer_box.at(i);
            //hwbuffer.dims[i].logical_min = Expr(0);
            hwbuffer.ldims[i].logical_size = total_buffer_box.at(i);
            hwbuffer.ldims[i].logical_min = Expr(0);

            //hwbuffer.dims[i].input_chunk = sliding_stencil_map.at(for_name).input_chunk_box.at(i);
            hwbuffer.dims[i].input_chunk = 1;
            hwbuffer.dims[i].input_block = input_block_box.at(i);
            //hwbuffer.dims[i].output_stencil = sliding_stencil_map.at(for_name).output_stencil_box.at(i);
            //FIXMEhwbuffer.dims[i].output_stencil = 1;
            //hwbuffer.dims[i].output_block = i < output_block_box.size() ? output_block_box.at(i) : 0;
            //hwbuffer.dims[i].output_min_pos = i < sliding_stencil_map.at(for_name).output_min_pos.size() ? sliding_stencil_map.at(for_name).output_min_pos.at(i) : 0;
            //hwbuffer.dims[i].output_min_pos = 0;
            hwbuffer.dims[i].loop_name = i < loop_names.size() ? loop_names.at(i) : unique_name("loopname");
            //std::cout << "hwbuffer " << hwbuffer.name << " finished dim " << i << " has min_pos=" << hwbuffer.dims[i].output_min_pos << std::endl;
          }
          hwbuffer.output_access_pattern = reader_loopnest;
          
          if (buffers.count(hwbuffer.name) == 0) {          
            std::cout << "Here is the hwbuffer:"
                      << " [" << buffers.count(hwbuffer.name) << "]\n" 
                      << hwbuffer << std::endl;
            buffers[hwbuffer.name] = hwbuffer;
          }

          std::cout << "sliding stencil map is ";
          for (auto pair : sliding_stencil_map) {
            std::cout << pair.first << ", ";
          }
          std::cout << std::endl;

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

  for (auto& hwbuffer : ehb.buffers) {
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
void set_output_params(HWXcel *xcel, 
                       const map<string, Function> &env,
                       const vector<BoundsInference_Stage> &inlined_stages,
                       const vector<string> &streaming_loop_levels,
                       Scope<Expr>& output_scope,
                       Box output_bounds) {

  auto &hwbuffers = xcel->hwbuffers;
  size_t i = inlined_stages.size() - 1 + 1;
  string xcel_compute_level = streaming_loop_levels.at(streaming_loop_levels.size()-1);

  bool in_output = true;


  Scope<Expr>& stencil_bounds = output_scope;
  std::cout << "stencil bounds when in the set_output_params method: " << stencil_bounds << std::endl;

  // Scan through each stage before the output stage.
  // Go through the stages from the output back to the input.
  while (i >= 1) {
    i--;
    
    const BoundsInference_Stage &stage = inlined_stages[i];
    if (in_output && stage.name != xcel->name) {
      continue;
    }

    // run through hwbuffers looking for a particular name
    //auto iterator = std::find_if(hwbuffers.begin(), hwbuffers.end(), [stage](const std::pair<std::string, HWBuffer>& buffer_pair){
    //    if (stage.name == buffer_pair.first) {
    //      return true;
    //    } else {
    //      return false;
    //    }
    //  });
    //internal_assert(iterator != hwbuffers.end());
    //auto &hwbuffer = iterator->second;
    //internal_assert(hwbuffer.name == iterator->first);
    auto& hwbuffer = hwbuffers.at(stage.name);
    internal_assert(hwbuffer.name == stage.name);
    
    std::cout << hwbuffer.name << " before func\n";

    Function cur_func = hwbuffer.func;
    if (stage.stage > 0) {
      StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
      auto rvars = update_schedule.rvars();
      for (size_t i=0; i<rvars.size(); ++i) {
        //hwbuffer.dims[i].output_min_pos = rvars[i].min;
        std::cout << hwbuffer.name << " " << i << " update has min_pos=" << rvars[i].min << std::endl;
      }
    }

    std::cout << "HWBuffer has " << hwbuffer.dims.size() << " dims, while \n";
    std::cout << " loops are: " << hwbuffer.streaming_loops << std::endl;
    std::cout << " args are: " << hwbuffer.func.args() << std::endl;
    
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
    std::cout << "finished is_output " << stage.consumers.size() << " consumers\n";
    
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
    std::cout << hwbuffer.name << " is inline=" << hwbuffer.is_inlined
              << " store=" << cur_func.schedule().store_level()
              << " compute=" << cur_func.schedule().compute_level()
              << " is_xcel=" << cur_func.schedule().is_accelerated()
              << " is_hw_kernel=" << cur_func.schedule().is_hw_kernel()
              << std::endl;
    
    // HWBuffer Parameter: map<string, HWBuffer&> consumer_buffers
    for (size_t j = 0; j < stage.consumers.size(); j++) {
      internal_assert(stage.consumers[j] < (int)inlined_stages.size());
      const BoundsInference_Stage &consumer = inlined_stages[stage.consumers[j]];
      std::cout << "right before " << consumer.name << " consumer of " << hwbuffer.name << "\n";

      internal_assert(hwbuffers.count(consumer.name) > 0);
      auto &ostream = hwbuffer.ostreams[consumer.name];
      ostream.name = consumer.name;
      
      if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name) &&
          hwbuffers.at(consumer.name).producer_buffers.count(hwbuffer.name) == 0) {
      //if (hwbuffers.count(consumer.name)) {
        std::cout << "adding " << hwbuffer.name << " as an input of " << consumer.name << "\n";
        hwbuffers.at(consumer.name).input_streams.push_back(hwbuffer.name);
        //hwbuffers.at(consumer.name).producer_buffers[hwbuffer.name] = std::make_shared<HWBuffer>(hwbuffer);
        hwbuffers.at(consumer.name).producer_buffers[hwbuffer.name] = &hwbuffer;
      } else {
        std::cout << "couldn't find consumer " << consumer.name << std::endl;
      }

      const Box &consumer_bounds = stage.bounds.find(make_pair(consumer.name,
                                                               consumer.stage))->second;
      std::cout << " size of consumer box is " << consumer_bounds << std::endl;
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

      std::cout << "for kernel " << hwbuffer.name << ", adding consumer "
                << consumer_name << " based on kernel " << consumer.name << std::endl;
      //hwbuffer.consumer_buffers[consumer_name] = std::make_shared<HWBuffer>(hwbuffers.at(consumer.name));
      hwbuffer.consumer_buffers[consumer_name] = &hwbuffers.at(consumer.name);
      //ostream.hwref = std::make_shared<HWBuffer>(hwbuffers.at(consumer.name));
      ostream.hwref = &hwbuffers.at(consumer.name);
      
      std::string func_compute_level = hwbuffer.compute_level;
      const FuncSchedule &sched = cur_func.schedule();
      auto compute_looplevel = sched.compute_level();
      
      std::cout << hwbuffer.name << " compute loop is using " << func_compute_level << " based on func " << cur_func.name()
                << " compute=" << cur_func.schedule().compute_level() << std::endl;

      FindOutputStencil fos(hwbuffer.name, cur_func, func_compute_level);
      std::cout << "looking for " << hwbuffer.name << " in consumer: \n" << consumer_buffer.my_stmt << std::endl;
      consumer_buffer.my_stmt.accept(&fos);
      hwbuffer.stride_map = fos.stride_map;
      ostream.stride_map = fos.stride_map;
      std::cout << hwbuffer.name << " stride_x=" << hwbuffer.stride_map["x"].stride << std::endl;
      //for (const auto& string_int_pair : hwbuffer.stride_map) {
      //  std::cout << string_int_pair.first << "," << string_int_pair.second.stride << std::endl;
      //}

      //std::cout << consumer_buffer.my_stmt << std::endl;
      std::cout << "the output stencil of " << hwbuffer.name << " to " << consumer_buffer.name
                << " at " << hwbuffer.compute_level
                << " is " << fos.output_stencil_box << std::endl;
      
      FindInputStencil fis(consumer.name, cur_func, func_compute_level, stencil_bounds);
      hwbuffer.my_stmt.accept(&fis);

      //std::cout << hwbuffer.my_stmt << std::endl;
      std::cout << "the input chunk of " << hwbuffer.name
                << " at " << hwbuffer.compute_level
                << " is " << fis.input_chunk_box << std::endl;

      // FIXMEyikes: this doesn't seem to work
      //const auto consumer_sliding_stencils = hwbuffers.at(consumer.name).input_stencil;

      internal_assert(hwbuffers.count(consumer.name) != 0);
      
      std::cout << "here we have consumer " << hwbuffers.at(consumer.name).name
                << " for hwbuffer " << hwbuffer.name << "\n";

      ostream.odims = vector<OutputDimSize>(hwbuffer.ldims.size());
      auto &odims = ostream.odims;
      if (stage.stage > 0) {
        StageSchedule update_schedule = cur_func.update_schedule(stage.stage - 1);
        auto rvars = update_schedule.rvars();
        for (size_t i=0; i<rvars.size(); ++i) {
          odims.at(i).output_min_pos = rvars[i].min;
          std::cout << hwbuffer.name << " " << i << " update has min_pos=" << rvars[i].min << std::endl;
        }
      }

      for (size_t idx=0; idx<hwbuffer.dims.size(); ++idx) {
        hwbuffer.dims.at(idx).input_chunk = 1;
        if (hwbuffer.dims.size() > idx) {
        //if (fis.found_stencil) {
          //hwbuffer.dims.at(idx).input_chunk = fis.input_chunk_box.at(idx);
          hwbuffer.dims.at(idx).input_chunk = hwbuffer.dims.at(idx).input_block;
        }
        
        if (fis.found_stencil && idx < fis.output_min_pos_box.size()) { // this works
          //hwbuffer.dims.at(idx).output_min_pos = fis.output_min_pos_box.at(idx);
          odims.at(idx).output_min_pos = fis.output_min_pos_box.at(idx);
          
          //if (is_zero(hwbuffer.dims.at(idx).output_min_pos)) { // alternatively, not output
          //if (is_zero(odims.at(idx).output_min_pos)) { // alternatively, not output
          //if (!hwbuffer.is_output) {
          if (!consumer_buffer.is_output) {
            //hwbuffer.dims.at(idx).output_min_pos = consumer_buffer.dims.at(idx).output_min_pos;
            //odims.at(idx).output_min_pos = consumer_buffer.dims.at(idx).output_min_pos;
            odims.at(idx).output_min_pos = consumer_buffer.ostreams.begin()->second.odims.at(idx).output_min_pos;

            //if (fos.found_stencil && idx < fos.output_stencil_box.size()) {
            //  std::cout << "old min_pos: " << hwbuffer.dims.at(idx).output_min_pos << std::endl;
            //  hwbuffer.dims.at(idx).output_min_pos += fos.output_min_pos_box.at(idx);
            //  std::cout << "new min_pos: " << hwbuffer.dims.at(idx).output_min_pos << std::endl;
            //}
          }
          
          std::cout << "replaced min pos for " << hwbuffer.name << " " << idx << " with "
                    << odims.at(idx).output_min_pos << std::endl;
            //<< hwbuffer.dims.at(idx).output_min_pos << "\n";
          std::cout << consumer_bounds << std::endl;
          if (!consumer_buffer.is_output) {
            std::cout << simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds)) << std::endl;
          }
          //if (consumer_bounds) {
          //  std::cout << " consumer_bounds=" << consumer_bounds[i].min << " = " 
          //            << simplify(expand_expr(consumer_bounds[i].min, stencil_bounds)) << std::endl;
          //}
        }

        
        //hwbuffer.dims.at(idx).output_stencil = consumer_sliding_stencils->output_stencil_box.at(idx);
        //if (true) {
        if (fos.found_stencil && idx < fos.output_stencil_box.size()) {
          hwbuffer.dims.at(idx).output_stencil = fos.output_stencil_box.at(idx);
          hwbuffer.dims.at(idx).output_block = fos.output_stencil_box.at(idx);
          odims.at(idx).output_stencil = fos.output_stencil_box.at(idx);
          odims.at(idx).output_block = fos.output_stencil_box.at(idx);

          
          std::cout << "replaced output stencil for " << hwbuffer.name << " based on consumer " << consumer.name << std::endl;
          std::cout << "output min position is: " << fos.output_min_pos_box.at(idx) << std::endl;

          if (!consumer_buffer.is_output) {
            std::cout << odims.at(idx).output_min_pos << " + " << fos.output_min_pos_box.at(idx) << std::endl;
            //hwbuffer.dims.at(idx).output_min_pos += fos.output_min_pos_box.at(idx);
            odims.at(idx).output_min_pos += fos.output_min_pos_box.at(idx);
          }
        } else {
          std::cout << hwbuffer.name << " couldn't find that output stencil thing\n";
          std::cout << "doing some defaults for " << hwbuffer.name << std::endl;
          std::cout << "before- output: " << odims.at(idx).output_block << "," << odims.at(idx).output_stencil << std::endl;
          odims.at(idx).output_block = hwbuffer.dims.at(idx).output_block; // needed for hw_input
          odims.at(idx).output_stencil = hwbuffer.dims.at(idx).output_block;
          std::cout << "after- output: " << odims.at(idx).output_block << "," << odims.at(idx).output_stencil << std::endl;
        }

        //hwbuffer.dims.at(idx).output_min_pos = simplify(hwbuffer.dims.at(idx).output_min_pos);
        //if (!consumer_buffer.is_output) {
        //hwbuffer.dims.at(idx).output_min_pos = simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds));
        odims.at(idx).output_min_pos = simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds));
        //hwbuffer.dims.at(idx).output_max_pos = simplify(expand_expr(consumer_bounds[idx].max, stencil_bounds));
        odims.at(idx).output_max_pos = simplify(expand_expr(consumer_bounds[idx].max, stencil_bounds));
        //std::cout << "replaced output min pos with new algo: " << hwbuffer.name << idx << "=" << hwbuffer.dims.at(idx).output_min_pos << std::endl;

        auto loop_range = simplify(expand_expr(consumer_bounds[idx].max - consumer_bounds[idx].min + 1, stencil_bounds));
        std::cout << "  range would/should be: " << loop_range << std::endl;

        //std::cout << "replaced input=" << hwbuffer.dims.at(idx).input_chunk
        //          << " and output=" << hwbuffer.dims.at(idx).output_stencil << std::endl;
      }

      std::cout << "right before " << consumer.name << " replacements\n"
                << hwbuffer.output_access_pattern;
    // std::vector<std::string> input_streams;  // used when inserting read_stream calls      
      if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name)) {
        hwbuffers.at(consumer.name).input_streams.emplace_back(hwbuffer.name);
        ReplaceOutputAccessPatternRanges roapr(consumer_buffer);
        hwbuffer.output_access_pattern = roapr.mutate(hwbuffer.output_access_pattern);
        ostream.output_access_pattern = roapr.mutate(hwbuffer.output_access_pattern);
      }
      std::cout << "right after " << consumer.name << " replacements\n"
                << hwbuffer.output_access_pattern;

      
    }

    // save the bounds values in scope
    for (int i = 0; i < cur_func.dimensions(); i++) {
      string arg = cur_func.name() + ".s" + std::to_string(stage.stage) + "." + cur_func.args()[i];
      // calculate the max position of the stencil windows
//      Expr stencil_max;
//      if (cur_kernel.is_inlined) {
//        stencil_max = simplify(cur_kernel.dims[i].min_pos + cur_kernel.dims[i].size - 1);
//      } else {
//        // NOTE we use 'step' here since r we will have line buffer
//        stencil_max = simplify(cur_kernel.dims[i].min_pos + cur_kernel.dims[i].step - 1);
//      }
      auto min_pos = hwbuffer.ostreams.size() == 0 ? Expr(0) : hwbuffer.ostreams.begin()->second.odims.at(i).output_min_pos;
      auto max_pos = hwbuffer.ostreams.size() == 0 ? Expr(0) : hwbuffer.ostreams.begin()->second.odims.at(i).output_max_pos;
      stencil_bounds.push(arg + ".min", min_pos);
      stencil_bounds.push(arg + ".max", max_pos);
      //stencil_bounds.push(arg + ".min", hwbuffer.dims[i].output_min_pos);
      //stencil_bounds.push(arg + ".max", hwbuffer.dims[i].output_max_pos);
      //std::cout << "pushed min pos " << arg + ".min" << " " << i << "=" << hwbuffer.dims[i].output_min_pos << "\n";
      //std::cout << "pushed max pos " << arg + ".max" << " " << i << "=" << hwbuffer.dims[i].output_max_pos << "\n";
    }
    if(stage.stage > 0) {
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

void IdentifyAddressing::visit(const For *op) {
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

    std::cout << "stride_map: ";
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
      std::cout << "not inverse variable " << varname << " for func " << func.name() << std::endl;
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



IdentifyAddressing::IdentifyAddressing(const Function& func, const Scope<Expr> &scope, const map<string,Stride> &stride_map) :
    func(func), stream_dim_idx(0), scope(scope), storage_names(func.args()), stride_map(stride_map) {
    const auto &sch = func.definition().schedule();
    const auto &splits = sch.splits();

    std::cout << "populating dim map for " << func.name() << std::endl;
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
/*
void linearize_address_space(HWBuffer &kernel) {
  for (auto& istream_pair : kernel.producer_buffers) {
    auto& istream = *istream_pair.second;

    IdentifyAddressing id_addr(istream.func, Scope<Expr>(), istream.stride_map);
    istream.output_access_pattern.accept(&id_addr);

    if (istream.name != kernel.name) {
      std::cout << istream.output_access_pattern;
      std::cout << istream.name << " is a producer for " << kernel.name << std::endl
                << "  range: " << id_addr.ranges << std::endl
                << "  stride: " << id_addr.strides_in_dim << std::endl
                << "  dim_refs: " << id_addr.dim_refs << std::endl;

      auto num_access_levels = id_addr.ranges.size();
      assert(num_access_levels == id_addr.strides_in_dim.size());
      assert(num_access_levels == id_addr.dim_refs.size());
      
      std::vector<AccessDimSize> linear_addr(num_access_levels);
      size_t j = 0;
      for (size_t i=0; i<num_access_levels; ++i) {
        if (id_addr.ranges.at(i) != 1) {
          linear_addr.at(j).range = id_addr.ranges.at(i);
          linear_addr.at(j).stride = id_addr.strides_in_dim.at(i);
          linear_addr.at(j).dim_ref = id_addr.dim_refs.at(i);
          j += 1;
        }
      }
      linear_addr.resize(j);
      istream_pair.second->linear_addr = linear_addr;
      //kernel.linear_addr = linear_addr;
      std::cout << istream.name << " getting linear addr " << istream.linear_addr << std::endl;
      std::cout << istream << std::endl;
    }
  }
}
*/

void linearize_address_space(HWBuffer &kernel) {
  for (auto& istream_pair : kernel.producer_buffers) {
    HWBuffer& istream = *istream_pair.second;
    //std::shared_ptr<HWBuffer> istream = istream_pair.second;

    IdentifyAddressing id_addr(istream.func, Scope<Expr>(), istream.stride_map);
    istream.output_access_pattern.accept(&id_addr);

    if (istream.name != kernel.name) {
      std::cout << istream.output_access_pattern;
      std::cout << istream.name << " is a producer for " << kernel.name << std::endl
                << "  range: " << id_addr.ranges << std::endl
                << "  stride: " << id_addr.strides_in_dim << std::endl
                << "  dim_refs: " << id_addr.dim_refs << std::endl;

      auto num_access_levels = id_addr.ranges.size();
      assert(num_access_levels == id_addr.strides_in_dim.size());
      assert(num_access_levels == id_addr.dim_refs.size());
      
      //istream_pair.second->linear_addr = std::vector<AccessDimSize>(num_access_levels);
      istream.linear_addr.resize(num_access_levels);
      //auto linear_addr = std::vector<AccessDimSize>(num_access_levels);
      size_t j = 0;
      for (size_t i=0; i<num_access_levels; ++i) {
        if (id_addr.ranges.at(i) != 1) {
          istream.linear_addr.at(j).range = id_addr.ranges.at(i);
          istream.linear_addr.at(j).stride = id_addr.strides_in_dim.at(i);
          istream.linear_addr.at(j).dim_ref = id_addr.dim_refs.at(i);
          j += 1;
        }
      }
      istream.linear_addr.resize(j);
      //kernel.linear_addr = istream.linear_addr;
      std::cout << istream.linear_addr << std::endl << istream;
      std::cout << &istream << std::endl;
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
    
    for (auto& istream_pair : kernel.producer_buffers) {
      auto& istream = *istream_pair.second;

      IdentifyAddressing id_addr(istream.func, Scope<Expr>(), istream.stride_map);
      istream.output_access_pattern.accept(&id_addr);
      int num_iter=1;
      for (size_t i = 0; i < id_addr.ranges.size(); i++) {
        num_iter *= id_addr.ranges.at(i);
      }
      
      if (has_accum && num_logical_pixels != num_iter) {
        kernel.num_accum_iters = num_iter / num_logical_pixels;
      }

      std::cout << kernel.name << " has a total of " << num_logical_pixels << " iterations and producer "
                << istream.name << " has " << num_iter << " iterations"
                << "  has_accum=" << has_accum << std::endl;
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

  std::cout << "creating an accelerator for " << func.name() << std::endl
            << s << std::endl;

  std::cout << xcel->name << " has the streaming loops: ";
  for (const auto& streaming_loop_name : xcel->streaming_loop_levels) {
    std::cout << streaming_loop_name << " ";
  }
  std::cout << "\n";

  Scope<Expr> output_scope;
  find_output_scope(s, func, xcel->compute_level, output_scope);

  auto output_box = find_output_bounds(s, func, xcel->compute_level);
  
  std::cout << "output bounds: " << output_box << std::endl;

  // use realizes to define each hwbuffer
  xcel->hwbuffers = extract_hw_buffers(s, env, xcel);

  // set output parameters for hwbuffers based on consumers
  set_output_params(xcel, env, inlined, xcel->streaming_loop_levels, output_scope, output_box);

  for (auto &hwbuffer_pair : xcel->hwbuffers) {
    linearize_address_space(hwbuffer_pair.second);
    calculate_accumulation(hwbuffer_pair.second);
  }
  for (auto &hwbuffer_pair : xcel->hwbuffers) {
    std::cout << hwbuffer_pair.first << " is extracted w/ inline=" << hwbuffer_pair.second.is_inlined
              << " and num_dims=" << hwbuffer_pair.second.dims.size() << std::endl;
    std::cout << "Final buffer:\n" << hwbuffer_pair.second;

    auto& kernel = hwbuffer_pair.second;
    auto num_inputs = kernel.func.updates().size() + 1;
    auto num_outputs = kernel.consumer_buffers.size();
    std::cout << "num_in=" << num_inputs << "   num_out=" << num_outputs << std::endl;

    if (kernel.ostreams.count(kernel.name)) {
      std::cout << "accumulate=" << " cycles" << std::endl;
    } else {
      std::cout << "no accumulation" << std::endl;
    }
    std::cout << std::endl;

    
    //hwbuffer_pair.second.streaming_loops = xcel->streaming_loop_levels;
    //hwbuffer_pair.second.compute_level = xcel->streaming_loop_levels.back();
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
    // skip this function if it is not accelerated
    if(!func.schedule().is_accelerated())
      continue;
    
    std::cout << "Found accelerate function " << func.name() << "\n";
    LoopLevel store_locked = func.schedule().store_level().lock();
    string store_varname =
      store_locked.is_root() ? "root" :
      store_locked.is_inlined() ? "inlined" :
      store_locked.var().name();

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
