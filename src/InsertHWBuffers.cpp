#include "InsertHWBuffers.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Scope.h"
#include "Debug.h"
#include "Substitute.h"
#include "IRPrinter.h"
#include "Simplify.h"
#include "Bounds.h"

#include <iostream>
#include <algorithm>
using std::ostream;

namespace Halide {
namespace Internal {

using std::string;
using std::map;
using std::set;
using std::pair;
using std::vector;
using std::cout;

namespace {

class ExpandExpr : public IRMutator {
    using IRMutator::visit;
    const Scope<Expr> &scope;

    Expr visit(const Variable *var) {
        if (scope.contains(var->name)) {
          debug(4) << "Fully expanded " << var->name << " -> " << scope.get(var->name) << "\n";
          //std::cout << "Fully expanded " << var->name << " -> " << scope.get(var->name) << "\n";
          return scope.get(var->name);


        } else {
          std::cout << "Scope does not contain  " << var->name << "\n";
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
    debug(4) << "Expanded " << e << " into " << result << "\n";
    return result;
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


}


int to_int(Expr expr) {
  //FIXMEyikes internal_assert(is_const(simplify(expr)));
  if (is_const(simplify(expr))) {
    return (int)*as_const_int(expr);
  } else {
    return -1;
  }
}

class ReplaceReferencesWithBufferStencil : public IRMutator {
    const HWBuffer &kernel;
    const HWXcel &xcel;  // TODO not needed
    Scope<Expr> scope;

    using IRMutator::visit;

    Stmt visit(const For *op) {
      std::cout << "starting this for replace\n";
        if (!starts_with(op->name, kernel.name)) {
          std::cout << "trivial for\n";
            // try to simplify trivial reduction loops
            // TODO add assertions to check loop type
            Expr loop_extent = simplify(expand_expr(op->extent, scope));
            if (is_one(loop_extent)) {
                scope.push(op->name, simplify(expand_expr(op->min, scope)));
                Stmt body = mutate(op->body);
                scope.pop(op->name);
                return LetStmt::make(op->name, op->min, body);
            } else {
                Stmt body = mutate(op->body);
                return For::make(op->name, op->min, op->extent, op->for_type, op->device_api, body);
            }
        } else {
          std::cout << "starting this replace: " << op->name << "\n";
            // replace the loop var over the dimensions of the original function
            // realization with the loop var over the stencil dimension.
            // e.g. funcA.s0.x -> funcA.stencil.s0.x
            //      funcA.s1.x -> funcA.stencil.s1.x
            string old_var_name = op->name;
            string stage_dim_name = op->name.substr(kernel.name.size()+1, old_var_name.size() - kernel.name.size());
            string new_var_name = kernel.name + ".stencil." + stage_dim_name;
            Expr new_var = Variable::make(Int(32), new_var_name);

            // find the stencil dimension given dim_name
            int dim_idx = -1;
            for(size_t i = 0; i < kernel.func.args().size(); i++)
                if(ends_with(stage_dim_name, "." + kernel.func.args()[i])) {
                    dim_idx = i;
                    break;
                }
            if (dim_idx == -1) {
              std::cout << "nvm this is a reduction\n";
                // it is a loop over reduction domain, and we keep it
                // TODO add an assertion
              return IRMutator::visit(op);
            }
            Expr new_min = 0;
            // FIXME(is this correct?): Expr new_extent = kernel.dims[dim_idx].step
            // FIXMEyikes
            std::cout << "doing dim=" << dim_idx << " for kernel size " << kernel.dims.size() << std::endl;
            if (dim_idx >= (int) kernel.dims.size()) { dim_idx = kernel.dims.size()-1; }
            internal_assert(dim_idx < (int) kernel.dims.size());
            Expr new_extent = kernel.dims.at(dim_idx).input_chunk;

            // create a let statement for the old_loop_var
            Expr old_min = op->min;
            Expr old_var_value = new_var + old_min;
            std::cout << "replacing " << old_var_name << " with the value=" << old_var_value << "\n";
            
            // traversal down into the body
            scope.push(old_var_name, simplify(expand_expr(old_var_value, scope)));
            Stmt new_body = mutate(op->body);
            scope.pop(old_var_name);

            std::cout << "finishing this for\n";
            new_body = LetStmt::make(old_var_name, old_var_value, new_body);
            return For::make(new_var_name, new_min, new_extent, op->for_type, op->device_api, new_body);
        }
        std::cout << "finished replacing for ref\n";
    }

    Stmt visit(const Provide *op) {
        std::cout << "looking at this provide: " << op->name << " while kernel is " << kernel.name << "\n";
        if(op->name != kernel.name) {
          return IRMutator::visit(op);
        } else {
            // Replace the provide node of func with provide node of func.stencil
            string stencil_name = kernel.name + ".stencil";

            vector<Expr> new_args(op->args.size());
            internal_assert(new_args.size() == kernel.dims.size());

            // Replace the arguments. e.g.
            //   func.s0.x -> func.stencil.x
            for (size_t i = 0; i < op->args.size(); i++) {
              std::cout << "op->arg " << op->args[i] << " - " << kernel.dims.at(i).output_min_pos << std::endl;

              
              //FIXME  new_args[i] = simplify(expand_expr(mutate(op->args[i]) - kernel.dims[i].min_pos, scope));
              //CORRECT new_args[i] = simplify(expand_expr_no_var(mutate(op->args[i]) - kernel.dims.at(i).output_min_pos, scope));
              new_args[i] = simplify(expand_expr(mutate(op->args[i]) - kernel.dims.at(i).output_min_pos, scope));
              //new_args[i] = simplify(expand_expr_no_var(mutate(op->args[i]), scope));
              std::cout << "old_arg" << i << " is " << op->args[i] << " while shift is " << kernel.dims.at(i).output_min_pos << "\n";
              std::cout << "new_arg" << i << " is " << new_args[i] << "\n";
            }

            vector<Expr> new_values(op->values.size());
            for (size_t i = 0; i < op->values.size(); i++) {
                new_values[i] = mutate(op->values[i]);
            }
            Stmt new_op = Provide::make(stencil_name, new_values, new_args);
            std::cout << "old provide replaced " << Stmt(op) << " with " << new_op
                      << " using min_x=" << kernel.dims.at(0).output_min_pos << std::endl;

            return Provide::make(stencil_name, new_values, new_args);
        }
    }

    Expr visit(const Call *op) {
      std::cout << op->name << " is a call in the replacerefs\n";
      std::cout << "kernelname=" << kernel.name << " input_streams=" << kernel.input_streams << "\n";
        if(op->name == kernel.name || // call to this kernel itself (in update definition)
           std::find(kernel.input_streams.begin(), kernel.input_streams.end(),
                     op->name) != kernel.input_streams.end() // call to an input stencil
           ) {
          std::cout << "inside call\n";
            // check assumptions
            internal_assert(op->call_type == Call::Halide);
            
            const auto it = xcel.hwbuffers.find(op->name);
            internal_assert(it != xcel.hwbuffers.end());
            const HWBuffer &stencil_kernel = it->second;
            internal_assert(op->args.size() == stencil_kernel.func.args().size());
          
            // Replace the call node of func with call node of func.stencil
            string stencil_name = stencil_kernel.name + ".stencil";
            vector<Expr> new_args(op->args.size());


            // Mutate the arguments.
            // The value of the new argment is the old_value - stencil.min_pos.
            // The new value shouldn't refer to old loop vars any more
            for (size_t i = 0; i < op->args.size(); i++) {
                Expr old_arg = mutate(op->args[i]);
                Expr offset;
                if (stencil_kernel.name == kernel.name) {
                //if (false) {
                    // The call is in an update definition of the kernel itself
                    // FIXME: offset = stencil_kernel.dims[i].min_pos;
                    offset = stencil_kernel.dims[i].output_min_pos;
                    std::cout << "offset from own kernel for " << kernel.name << std::endl;
                    std::cout << "offset=" << offset << std::endl;
                } else { // this works 
                  
                    // This is call to input stencil
                    // we use the min_pos stored in in_kernel.consumer_buffers
                    const auto it = stencil_kernel.consumer_buffers.find(kernel.name);
                    internal_assert(it != kernel.consumer_buffers.end());
                    // FIXMEyikes consumer buffers doesn't seem to work
                    std::cout << "tricky offset here for " << kernel.name << std::endl;
                    internal_assert(it->second->dims.size() > i);
                    offset = it->second->dims.at(i).output_min_pos;
                    std::cout << "offset=" << offset << std::endl;
                    //offset = Expr(0);

                  //offset = Expr(0);
                  //offset = it->second->dims.at(i).output_min_pos;
                  
                }

                offset = stencil_kernel.dims[i].output_min_pos;
                //offset = Expr(0);
                Expr new_arg = old_arg - offset;
                //new_arg = simplify(expand_expr_no_var(new_arg, scope));
                new_arg = simplify(expand_expr(new_arg, scope));
                std::cout << "new_arg" << i << " = " << new_arg << std::endl;
                // TODO check if the new_arg only depends on the loop vars
                // inside the producer
                new_args[i] = new_arg;
                std::cout << "replacing call to " << op->name << ": "
                  << "offset=" << offset << " since stencil_name=" << stencil_kernel.name << " kernel_name=" <<  kernel.name << "\n";
            }
            Expr expr = Call::make(op->type, stencil_name, new_args, Call::Intrinsic);
            debug(4) << "replacing call "  << Expr(op) << " with\n"
                     << "\t" << expr << "\n";
            std::cout << "done with call\n";
            std::cout << "replacing call " << Expr(op) << " with\n" << "\t" << expr << "\n";

            return expr;
        } else {
          return IRMutator::visit(op);
        }
    }

    Stmt visit(const Realize *op) {
      
      std::cout << "found this realize: " << op->name << "\n";
      std::cout << op << std::endl;

        // this must be a realize node of a inlined function
      for (const auto &hwbuffer_p : xcel.hwbuffers) {
        std::cout << hwbuffer_p.first << ", ";
      }
      std::cout << std::endl;
      
      internal_assert(xcel.hwbuffers.count(op->name));
      internal_assert(xcel.hwbuffers.find(op->name)->second.is_inlined);
        // expand and simplify bound expressions
        // TODO this may not be needed if the letstmt in the scope is well preserved

        Region new_bounds(op->bounds.size());
        bool bounds_changed = false;

        // Mutate the bounds
        for (size_t i = 0; i < op->bounds.size(); i++) {
            Expr old_min    = op->bounds[i].min;
            Expr old_extent = op->bounds[i].extent;
            Expr new_min    = simplify(expand_expr(mutate(op->bounds[i].min), scope));
            Expr new_extent = simplify(expand_expr(mutate(op->bounds[i].extent), scope));
            if (!new_min.same_as(old_min))       bounds_changed = true;
            if (!new_extent.same_as(old_extent)) bounds_changed = true;
            new_bounds[i] = Range(new_min, new_extent);
        }

        std::cout << "just about done of realize " << op->name << std::endl;
        Stmt body = mutate(op->body);
        Expr condition = mutate(op->condition);
        if (!bounds_changed &&
            body.same_as(op->body) &&
            condition.same_as(op->condition)) {
            return op;
        } else {
          return Realize::make(op->name, op->types, MemoryType::Auto, new_bounds,
                                 condition, body);
        }
    }

    Expr visit(const Let *op) {
        Expr new_value = simplify(expand_expr(mutate(op->value), scope));
        scope.push(op->name, new_value);
        Expr new_body = mutate(op->body);
        if (new_value.same_as(op->value) &&
            new_body.same_as(op->body)) {
            return op;
        } else {
            return Let::make(op->name, new_value, new_body);
        }
        scope.pop(op->name);
    }

    Stmt visit(const LetStmt *op) {
        Expr new_value = simplify(expand_expr(op->value, scope));
        scope.push(op->name, new_value);
        Stmt new_body = mutate(op->body);
        if (new_value.same_as(op->value) &&
            new_body.same_as(op->body)) {
            return op;
        } else {
            return LetStmt::make(op->name, new_value, new_body);
        }
        scope.pop(op->name);
    }

public:
  ReplaceReferencesWithBufferStencil(const HWBuffer &k, const HWXcel &accel,
                                 const Scope<Expr> *s = NULL)
    : kernel(k), xcel(accel) {
    //: kernel(HWBuffer()), xcel(HWXcel()) {
        scope.set_containing_scope(s);
        std::cout << "created pass to replace refs for " << k.name << "\n";
    }
};

// FIXME many uses of dims
Stmt create_hwbuffer_dispatch_call(const HWBuffer& kernel, int min_fifo_depth = 0) {
  std::cout << "creating dispatch from " << kernel.name << " of "
            << kernel.dims.size() << " dims" << std::endl;
    // dispatch the stream into seperate streams for each of its consumers
    // syntax:
    //   dispatch_stream(stream_name, num_of_dimensions,
    //                   stencil_size_dim_0, stencil_step_dim_0, store_extent_dim_0,
    //                   [stencil_size_dim_1, stencil_step_dim_1, store_extent_dim_1, ...]
    //                   num_of_consumers,
    //                   consumer_0_name, fifo_0_depth,
    //                   consumer_0_offset_dim_0, consumer_0_extent_dim_0,
    //                   [consumer_0_offset_dim_1, consumer_0_extent_dim_1, ...]
    //                   [consumer_1_name, ...])
    Expr stream_var = Variable::make(Handle(), kernel.name + ".stencil.stream");
    vector<Expr> dispatch_args({stream_var, (int)kernel.dims.size()});
    for (size_t i = 0; i < kernel.dims.size(); i++) {
        dispatch_args.push_back(kernel.dims[i].output_stencil);
        dispatch_args.push_back(kernel.dims[i].input_chunk);
        dispatch_args.push_back(kernel.ldims[i].logical_size);
        //dispatch_args.push_back(kernel.dims[i].logical_size);
        //Expr store_extent = simplify(kernel.dims[i].store_bound.max - kernel.dims[i].store_bound.min + 1);
        //internal_assert(is_const(store_extent));
        //dispatch_args.push_back((int)*as_const_int(store_extent));
    }
    dispatch_args.push_back((int)kernel.consumer_buffers.size());
    std::cout << "   going to " << kernel.consumer_buffers.size() << " consumers: ";
    for (const auto& p : kernel.consumer_buffers) {
      std::cout << p.first << "(" << p.second->dims.size() << " dims)" << ", \n";
        dispatch_args.push_back(p.first);
        //internal_assert(kernel.consumer_fifo_depths.count(p.first));
        //dispatch_args.push_back(std::max(min_fifo_depth, kernel.consumer_fifo_depths.find(p.first)->second));
        dispatch_args.push_back(0); // assume a 0 fifo_depth
        //internal_assert(p.second->dims.size() == kernel.dims.size()); FIXME, should this be the case?
        for (size_t i = 0; i < kernel.dims.size(); i++) {
          //dispatch_args.push_back(simplify(p.second->dims.at(i).logical_min - kernel.dims.at(i).output_min_pos)); FIXME
          //dispatch_args.push_back(p.second->dims.at(i).logical_min);
          //dispatch_args.push_back(p.second->dims.at(i).logical_size);
          dispatch_args.push_back(p.second->ldims.at(i).logical_min);
          dispatch_args.push_back(p.second->ldims.at(i).logical_size);
          
          //FIXME: ... Expr store_offset = simplify(p.second.dims[i].store_bound.min - kernel.dims[i].store_bound.min);
          //Expr store_extent = simplify(p.second[i].store_bound.max - p.second[i].store_bound.min + 1);
          //internal_assert(is_const(store_offset));
          //internal_assert(is_const(store_extent));
          //dispatch_args.push_back((int)*as_const_int(store_offset));
          //dispatch_args.push_back((int)*as_const_int(store_extent));
        }
    }
    return Evaluate::make(Call::make(Handle(), "dispatch_stream", dispatch_args, Call::Intrinsic));
}


// Add realize and read_stream calls arround IR s
Stmt add_hwinput_stencil(Stmt s, const HWBuffer &kernel, const HWBuffer &input) {
    string stencil_name = input.name + ".stencil";
    string stream_name = stencil_name + ".stream";
    Expr stream_var = Variable::make(Handle(), stream_name);
    Expr stencil_var = Variable::make(Handle(), stencil_name);

    // syntax for read_stream()
    // read_stream(src_stream, dest_stencil, [consumer_name])
    vector<Expr> args({stream_var, stencil_var});
    if (input.name != kernel.name) {
        // for non-output kernel, we add an additional argument
        args.push_back(kernel.name);
    }
    Stmt read_call = Evaluate::make(Call::make(Handle(), "read_stream", args, Call::Intrinsic));
    //Stmt pc = ProducerConsumer::make(stencil_name, read_call, Stmt(), s);
    Stmt producer = ProducerConsumer::make_produce(stencil_name, read_call);
    Stmt consumer = ProducerConsumer::make_consume(stencil_name, s);
    Stmt pc = Block::make(producer, consumer);

    // create a realizeation of the stencil image
    Region bounds;
    for (const auto dim: input.dims) {
        bounds.push_back(Range(0, dim.output_stencil));
    }
    s = Realize::make(stencil_name, input.func.output_types(), MemoryType::Auto, bounds, const_true(), pc);
    return s;
}

bool need_hwbuffer(const HWBuffer &kernel) {
    // check if we need a hwbuffer
    bool ret = false;
    for (size_t i = 0; i < kernel.dims.size(); i++) {
      std::cout << "kernel " << kernel.name
                << " has size=" << kernel.dims.at(i).output_stencil
                << " step=" << kernel.dims.at(i).input_chunk << std::endl;
      if (to_int(kernel.dims.at(i).output_stencil) != to_int(kernel.dims.at(i).input_chunk)) {
            ret = true;
            break;
        }
    }

    // check if there are loops that repeat
    for (auto stride_pair : kernel.stride_map) {
      std::cout << stride_pair.first << " has stride=";
      if (stride_pair.second.is_inverse) {
        std::cout << "1/";
      }
      std::cout << stride_pair.second.stride << "\n";
      
      if (stride_pair.second.is_inverse) {
        return true;
      }
    }
    
    return ret;
}

// IR for line buffers
// A line buffer is instantiated to buffer the stencil_update.stream and
// to generate the stencil.stream
// The former is smaller, which only consist of the new pixels
// sided in each shift of the stencil window.
Stmt add_hwbuffer(Stmt s, const HWBuffer &kernel, const HWXcel &xcel, const Scope<Expr> &scope) {
  std::cout << "considering a hwbuffer for " << kernel.name << std::endl;
    Stmt ret;
    if (need_hwbuffer(kernel)) {
      std::cout << "creating a hwbuffer for " << kernel.name << std::endl;
        // Before mutation:
        //       stmt...
        //
        // After mutation:
        //       realize func.stencil.stream {
        //         hwbuffer(...)
        //         dispatch(...)
        //         stmt...
        //       }
        string stream_name = kernel.name + ".stencil.stream";
        string update_stream_name = kernel.name + ".stencil_update.stream";
        Expr stream_var = Variable::make(Handle(), stream_name);
        Expr update_stream_var = Variable::make(Handle(), update_stream_name);

        vector<Expr> hwbuffer_args({update_stream_var, stream_var});

        std::cout << "hwbuffer num_dims=" << kernel.dims.size() << "\n";
        hwbuffer_args.push_back(Expr((int) kernel.dims.size()));
        
        // extract the buffer size, and put it into args
        for (size_t i = 0; i < kernel.dims.size(); i++) {
          //Expr store_extent = simplify(kernel.dims[i].store_bound.max -
          //                             kernel.dims[i].store_bound.min + 1);
          //Expr store_extent = kernel.dims[i].logical_size;
          Expr store_extent = kernel.ldims[i].logical_size;
          hwbuffer_args.push_back(store_extent);
          
            std::cout << "store extent in this hwbuffer: " << store_extent << std::endl;
        }
        for (size_t i = 0; i < kernel.dims.size(); i++) {
          hwbuffer_args.push_back(kernel.dims.at(i).input_chunk);
        }
        for (size_t i = 0; i < kernel.dims.size(); i++) {
          hwbuffer_args.push_back(kernel.dims.at(i).input_block);
        }
        for (size_t i = 0; i < kernel.dims.size(); i++) {
          hwbuffer_args.push_back(kernel.dims.at(i).output_stencil);
        }
        for (size_t i = 0; i < kernel.dims.size(); i++) {
          std::cout << "output_blk" << i << " = " << kernel.dims.at(i).output_block << "\n";
          hwbuffer_args.push_back(kernel.dims.at(i).output_block);
        }

        std::cout << "doing some addressing for kernel=" << kernel.name << "\n";
        for (const auto& string_int_pair : kernel.stride_map) {
          std::cout << string_int_pair.first << "," << string_int_pair.second.stride << std::endl;
        }

        IdentifyAddressing id_addr(kernel.func, scope, kernel.stride_map);
        kernel.output_access_pattern.accept(&id_addr);

        hwbuffer_args.push_back(Expr((int) id_addr.ranges.size()));
        internal_assert(id_addr.ranges.size() == id_addr.dim_refs.size());
        internal_assert(id_addr.ranges.size() == id_addr.strides_in_dim.size());        
        
        for (size_t i = 0; i < id_addr.ranges.size(); i++) {
          std::cout << "range" << i << " = " << id_addr.ranges.at(i) << "\n";
          hwbuffer_args.push_back(id_addr.ranges.at(i));
        }
        for (size_t i = 0; i < id_addr.ranges.size(); i++) {
          hwbuffer_args.push_back(id_addr.dim_refs.at(i));
        }
        for (size_t i = 0; i < id_addr.ranges.size(); i++) {
          hwbuffer_args.push_back(id_addr.strides_in_dim.at(i));
        }

        
        Stmt hwbuffer_call = Evaluate::make(Call::make(Handle(), "hwbuffer", hwbuffer_args, Call::Intrinsic));
        Stmt dispatch_call = create_hwbuffer_dispatch_call(kernel);
        Stmt buffer_calls = Block::make(hwbuffer_call, dispatch_call);

        // create a realization of the stencil of the window-size
        Region window_bounds;
        for (const auto dim: kernel.dims) {
            window_bounds.push_back(Range(0, dim.output_stencil));
        }
        ret = Realize::make(stream_name, kernel.func.output_types(), MemoryType::Auto, window_bounds, const_true(), Block::make(hwbuffer_call, Block::make(dispatch_call, s)));
    } else {
        user_warning << "No hwbuffer inserted after function " << kernel.name
                     << ".\n";

        // Before mutation:
        //       stmt...
        //
        // After mutation:
        //       dispatch(...)
        //       stmt...

        // Ad-hoc fix: place a fifo buffer at the inputs if the inputs is not hwbuffered.
        // This works around the issue that read stencil call from a interface stream
        // in the compute kernel cannot fully unrolled
        int force_buffer_depth = 0;
        if (kernel.input_streams.empty() && kernel.consumer_buffers.size() == 1)
            force_buffer_depth = 1;
        Stmt dispatch_call = create_hwbuffer_dispatch_call(kernel, force_buffer_depth);
        ret = Block::make(dispatch_call, s);
    }
    return ret;
}

Stmt transform_hwkernel(Stmt s, const HWXcel &xcel, Scope<Expr> &scope) {
    Stmt ret;

    std::cout << "transforming:\n";
    std::cout << s << "\n";
    
    if (const Block *op = s.as<Block>()) {
      //Stmt pc_block_stmt = find_pcblock(s);
      //const Block *pc_block = pc_block_stmt.as<Block>();
      Stmt block_first = op->first;

      //std::cout << "first block is: \n" << block_first << "\n";
      //Stmt first_pc = find_produce(block_first);

      Stmt body = block_first;
      vector<pair<string, Expr>> lets;
      while (const LetStmt *let = body.as<LetStmt>()) {
        body = let->body;
        scope.push(let->name, simplify(expand_expr(let->value, scope)));
        lets.push_back(make_pair(let->name, let->value));
      }


      //const ProducerConsumer *produce_node = pc_block->first.as<ProducerConsumer>();
      //const ProducerConsumer *produce_node = first_pc.as<ProducerConsumer>();
        const ProducerConsumer *produce_node = body.as<ProducerConsumer>();
        const ProducerConsumer *consume_node = op->rest.as<ProducerConsumer>();
        if (!(consume_node && produce_node)) {
          //std::cout << "didn't find pc: produce=" << produce_node
          //          << " first_let=" << op->first.as<Let>()
          //          << " first_letstmt=" << op->first.as<LetStmt>()
          //          << " consume=" << consume_node << std::endl;
          //Stmt first = op->first;
          //std::cout << "first: " << first << std::endl;
          //std::cout << "rest_body: " << consume_node->body << std::endl;
          std::cout << "didn't find pc";
          return s;
        }

        internal_assert(produce_node && produce_node->is_producer);
        internal_assert(consume_node->name == produce_node->name);
        internal_assert(consume_node && !consume_node->is_producer);

        Stmt produce_stmt = body;//first_pc;//pc_block->first;
        Stmt consume_stmt = op->rest;//pc_block->rest;

        for (auto &kernel : xcel.hwbuffers) {
          std::cout << kernel.first << "=" << kernel.second.name << ",";
        }
        std::cout << std::endl;

        internal_assert(xcel.hwbuffers.count(produce_node->name));
        //const HWBuffer &kernel = xcel.hwbuffers.find(produce_node->name)->second;
        const HWBuffer &kernel = xcel.hwbuffers.at(produce_node->name);

        std::cout << "going through block " << kernel.name << std::endl;
        std::cout << kernel.name << " has inline=" << kernel.is_inlined
                  << " num_dims=" << kernel.dims.size() << std::endl;
        //std::cout << " compute=" << kernel.func.schedule().compute_level()      << std::endl;
        //std::cout          << " store=" << kernel.func.schedule().store_level() << std::endl;
        std::cout << " compute=" << xcel.compute_level      << std::endl;
        std::cout          << " store=" << xcel.store_level << std::endl;

        internal_assert(!kernel.is_output);

        if (kernel.is_inlined) {
            // if it is a function inlined into the output function,
            // skip transforming this funciton

            // TODO it is buggy as the inlined function should really be
            // nested in the scan loops of the output function
            internal_error;
        }

        // Before mutation:
        //    produce func {...}
        //    consume func
        //
        // After mutation:
        //     realize func.stencil_update.stream {
        //       produce func.stencil_update.stream {
        //         for scan_loops {
        //           realize input1.stencil {
        //             produce input1.stencil {
        //               read_stream(input1.stencil.stream, input1.stencil)
        //             }
        //             ...
        //             realize func.stencil {
        //               produce func.stencil {...}
        //               write_stream(func.stencil_update.stream, func.stencil)
        //       } } } }
        //       realize func.stencil.stream {
        //         hwbuffer(...)
        //         dispatch(...)
        //         consume func.stencil.stream
        //       }
        //     }
        string stencil_name = kernel.name + ".stencil";
        string stream_name = need_hwbuffer(kernel) ?
            kernel.name + ".stencil_update.stream" : kernel.name + ".stencil.stream";
        Expr stencil_var = Variable::make(Handle(), stencil_name);
        Expr stream_var = Variable::make(Handle(), stream_name);

        // replacing the references to the original realization with refences to stencils
        std::cout << "replacing some refs: " << produce_node->body << std::endl;
        Stmt produce = ReplaceReferencesWithBufferStencil(kernel, xcel, &scope).mutate(produce_node->body);
        std::cout << "continuing\n";
        //Stmt update = ReplaceReferencesWitBufferhStencil(kernel, xcel, &scope).mutate(op->update);

        // syntax for write_stream()
        // write_stream(des_stream, src_stencil)
        vector<Expr> write_args({stream_var, stencil_var});
        Stmt write_call = Evaluate::make(Call::make(Handle(), "write_stream", write_args, Call::Intrinsic));

        // realize and PC node for func.stencil
        Stmt stencil_produce = ProducerConsumer::make_produce(stencil_name, produce);
        Stmt stencil_consume = ProducerConsumer::make_produce(stencil_name, write_call);
        //Stmt stencil_pc = ProducerConsumer::make(stencil_name, produce, update, write_call);
        Stmt stencil_pc = Block::make(stencil_produce, stencil_consume);

        // create a realization of the stencil of the step-size
        Region step_bounds;
        for (const auto &dim: kernel.dims) {
            step_bounds.push_back(Range(0, dim.input_chunk));
        }
        Stmt stencil_realize = Realize::make(stencil_name, kernel.func.output_types(), MemoryType::Auto, step_bounds, const_true(), stencil_pc);

        std::cout << "write streams created\n";
        // add read_stream for each input stencil (producers fed to func)
        for (const string& s : kernel.input_streams) {
            const auto it = xcel.hwbuffers.find(s);
            internal_assert(it != xcel.hwbuffers.end());
            stencil_realize = add_hwinput_stencil(stencil_realize, kernel, it->second);
        }
        std::cout << "read streams created\n";

        // insert scan loops
        Stmt scan_loops = stencil_realize;
        int scan_dim = 0;
        std::cout << "writing kernel named " << stencil_name << " with bounds \n";
        for(size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].loop_name == "undef" )
                continue;

            string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                + ".__scan_dim_" + std::to_string(scan_dim++);

            //Expr store_extent = simplify(kernel.dims[i].store_bound.max -
            //                             kernel.dims[i].store_bound.min + 1);
            std::cout << "store=" << kernel.ldims[i].logical_size
                      << " in_chunk=" << kernel.dims[i].input_chunk
                      << std::endl;
            int store_extent_int = to_int(kernel.ldims[i].logical_size);
            //std::cout << store_extent << " ";

            debug(3) << "kernel " << kernel.name << " store_extent = " << store_extent_int << '\n';
            // check the condition for the new loop for sliding the update stencil
            //const IntImm *store_extent_int = store_extent.as<IntImm>();
            //internal_assert(store_extent_int);
            if (false) {
              //if (store_extent_int % to_int(kernel.dims[i].input_chunk) != 0) { //FIXMEyikes
                // we cannot handle this scenario yet
                internal_error
                    << "Line buffer extent (" << store_extent_int
                    << ") is not divisible by the stencil step " << to_int(kernel.dims[i].input_chunk) << '\n';
            }
            int loop_extent = store_extent_int / to_int(kernel.dims[i].input_chunk);

            // add letstmt to connect old loop var to new loop var_name
            // TODO this is not correct in general
            scan_loops = LetStmt::make(kernel.dims[i].loop_name, Variable::make(Int(32), loop_var_name), scan_loops);
            scan_loops = For::make(loop_var_name, 0, loop_extent, ForType::Serial, DeviceAPI::Host, scan_loops);
        }
        //std::cout << "\n";

        // Recurse
        //std::cout << consume_node->name << " before recursion\n";
        
        Stmt stream_consume = transform_hwkernel(consume_node->body, xcel, scope);
        //Stmt stream_consume = transform_hwkernel(consume_stmt, xcel, scope);

        //std::cout << consume_node->name << " after recursion\n";
        
        // Add line buffer and dispatcher
        Stmt stream_realize = add_hwbuffer(stream_consume, kernel, xcel, scope);

        // create the PC node for update stream
        //Stmt stream_pc = ProducerConsumer::make(stream_name, scan_loops, Stmt(), stream_realize);
        Stmt stream_producer = ProducerConsumer::make_produce(stream_name, scan_loops);
        Stmt stream_consumer = ProducerConsumer::make_consume(stream_name, stream_realize);
        Stmt stream_pc = Block::make(stream_producer, stream_consumer);

        // create a realizeation of the stencil stream
        ret = Realize::make(stream_name, kernel.func.output_types(), MemoryType::Auto, step_bounds, const_true(), stream_pc);

        // Rewrap the let statements
        for (size_t i = lets.size(); i > 0; i--) {
          ret = LetStmt::make(lets[i-1].first, lets[i-1].second, ret);
        }

    } else {
        std::cout << "this is output\n";        
        // this is the output kernel of the xcel
        const HWBuffer &kernel = xcel.hwbuffers.find(xcel.name)->second;
        std::cout << "this is output: " << xcel.name << "=" << kernel.name << std::endl;        
        internal_assert(kernel.is_output);

        string stencil_name = kernel.name + ".stencil";
        string stream_name = kernel.name + ".stencil.stream";
        Expr stream_var = Variable::make(Handle(), stream_name);
        Expr stencil_var = Variable::make(Handle(), stencil_name);

        // replacing the references to the original realization with refences to stencils
        std::cout << "replacing some output refs: " << s << std::endl;
        Stmt produce = ReplaceReferencesWithBufferStencil(kernel, xcel, &scope).mutate(s);

        // syntax for write_stream()
        // write_stream(des_stream, src_stencil)
        vector<Expr> write_args({stream_var, stencil_var});
        // for xcel output kernel, we want to record the scan loop vars,
        // so that code gen knows when to assert TLAST signal
        int scan_dim = 0;
        for (size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].loop_name != "undef") {
                string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                    + ".__scan_dim_" + std::to_string(scan_dim++);
                //Expr store_extent = simplify(kernel.dims[i].store_bound.max -
                //                             kernel.dims[i].store_bound.min + 1);
                //const IntImm *store_extent_int = store_extent.as<IntImm>();
                //internal_assert(store_extent_int);
                
                int store_extent_int = to_int(kernel.ldims[i].logical_size);
                int loop_extent = store_extent_int / to_int(kernel.dims[i].input_chunk);

                Expr loop_var = Variable::make(Int(32), loop_var_name);
                Expr loop_max = make_const(Int(32), loop_extent - 1);
                write_args.push_back(loop_var);
                write_args.push_back(loop_max);
            }
        }
        Stmt write_call = Evaluate::make(Call::make(Handle(), "write_stream", write_args, Call::Intrinsic));

        //Stmt stencil_pc = ProducerConsumer::make(stencil_name, produce, Stmt(), write_call);
        Stmt stencil_produce = ProducerConsumer::make_produce(stencil_name, produce);
        Stmt stencil_consume = ProducerConsumer::make_consume(stencil_name, write_call);
        Stmt stencil_pc = Block::make(stencil_produce, stencil_consume);

        // create a realization of the stencil image
        Region bounds;
        for (const auto dim: kernel.dims) {
            bounds.push_back(Range(0, dim.input_chunk));
        }
        Stmt stencil_realize = Realize::make(stencil_name, kernel.func.output_types(), MemoryType::Auto, bounds, const_true(), stencil_pc);

        // add read_stream for each input stencil (producers fed to func)
        for (const string& s : kernel.input_streams) {
            const auto it = xcel.hwbuffers.find(s);
            internal_assert(it != xcel.hwbuffers.end());
            stencil_realize = add_hwinput_stencil(stencil_realize, kernel, it->second);
        }

        // insert scan loops
        Stmt scan_loops = stencil_realize;
        scan_dim = 0;
        for(size_t i = 0; i < kernel.dims.size(); i++) {
            if (kernel.dims[i].loop_name == "undef" )
                continue;

            string loop_var_name = kernel.name + "." + kernel.func.args()[i]
                + ".__scan_dim_" + std::to_string(scan_dim++);

            //Expr store_extent = simplify(kernel.dims[i].store_bound.max -
            //                             kernel.dims[i].store_bound.min + 1);
            int store_extent_int = to_int(kernel.ldims[i].logical_size);
            debug(3) << "kernel " << kernel.name << " store_extent = " << store_extent_int << '\n';

            // check the condition for the new loop for sliding the update stencil
            //const IntImm *store_extent_int = store_extent.as<IntImm>();
            //internal_assert(store_extent_int);
            if (store_extent_int % to_int(kernel.dims[i].input_chunk) != 0) {
                // we cannot handle this scenario yet
                internal_error
                    << "Line buffer extent (" << store_extent_int
                    << ") is not divisible by the stencil step " << kernel.dims[i].input_chunk << '\n';
            }
            int loop_extent = store_extent_int / to_int(kernel.dims[i].input_chunk);

            // add letstmt to connect old loop var to new loop var_name
            // TODO this is not correct in general
            scan_loops = LetStmt::make(kernel.dims[i].loop_name, Variable::make(Int(32), loop_var_name), scan_loops);
            scan_loops = For::make(loop_var_name, 0, loop_extent, ForType::Serial, DeviceAPI::Host, scan_loops);
        }

        Stmt ret_produce = ProducerConsumer::make_produce(stream_name, scan_loops);
        Stmt ret_consume = ProducerConsumer::make_consume(stream_name, Evaluate::make(0));
        Stmt ret_pc = Block::make(ret_produce, ret_consume);
        ret = ret_pc;
        std::cout << "finished with output stencil:\n" << ret_pc;
    }
    return ret;
}

/*
class TransformTapStencils : public IRMutator {
    const map<string, HWTap> &taps;

    using IRMutator::visit;

    // Replace calls to ImageParam with calls to Stencil
    Expr visit(const Call *op) {
        if (taps.count(op->name) == 0) {
          return IRMutator::visit(op);
        } else if (op->call_type == Call::Image || op->call_type == Call::Halide) {
            debug(3) << "replacing " << op->name << '\n';
            const HWTap &tap = taps.find(op->name)->second;

            // Replace the call node of func with call node of func.tap.stencil
            string stencil_name = op->name + ".tap.stencil";
            vector<Expr> new_args(op->args.size());

            // Mutate the arguments.
            // The value of the new argment is the old_value - min_pos
            // b/c stencil indices always start from zero
            internal_assert(tap.dims.size() == op->args.size());
            for (size_t i = 0; i < op->args.size(); i++) {
                 new_args[i] = op->args[i]- tap.dims[i].min_pos;
            }
            return Call::make(op->type, stencil_name, new_args, Call::Intrinsic);
        } else {
            internal_error << "unexpected call_type\n";
            return IRMutator::visit(op);
        }
    }

public:
    TransformTapStencils(const map<string, HWTap> &t) : taps(t) {}
};
*/

// Perform streaming optimization for all functions
class InsertHWBuffers : public IRMutator {
    const HWXcel &xcel;
    Scope<Expr> scope;

    using IRMutator::visit;

    Stmt visit(const For *op) {
        Stmt stmt;
        std::cout << "visiting for loop named " << op->name
                  << " where store_lvl=" << xcel.store_level
                  << " where compute_lvl=" << xcel.compute_level
                  << std::endl;

        std::cout << "streaming loops:\n";
        for (auto &ll : xcel.streaming_loop_levels) {
          std::cout << ll << ",";
        }
        std::cout << std::endl;



        bool is_loop_var = std::find(xcel.streaming_loop_levels.begin(), xcel.streaming_loop_levels.end(), op->name) != xcel.streaming_loop_levels.end();
        
        // store level doesn't match name AND loop var is not found in xcel
        if (!xcel.store_level.match(op->name) &&
            !is_loop_var) {
          std::cout << "just continue\n";
            stmt = IRMutator::visit(op);

        // compute level matches name
        } else if (xcel.compute_level.match(op->name)) {
          std::cout << "xcel compute\n";
            internal_assert(is_loop_var); // compute level was supposed to be inclusive

            // walk inside of any let statements
            Stmt body = op->body;

            //std::cout << "visited for loop: " << body;

            // place all let statements in here
            vector<pair<string, Expr>> lets;          
            while (const LetStmt *let = body.as<LetStmt>()) {
                body = let->body;
                scope.push(let->name, simplify(expand_expr(let->value, scope)));
                lets.push_back(make_pair(let->name, let->value));
            }

            std::cout << "about to transform\n";
            Stmt new_body = transform_hwkernel(body, xcel, scope);

            // insert hardware buffers for input streams
            for (const string &input_name : xcel.input_streams) {
                const HWBuffer &input_kernel = xcel.hwbuffers.find(input_name)->second;
                std::cout << "let's add input stream " << input_name << std::endl;
                new_body = add_hwbuffer(new_body, input_kernel, xcel, scope);
            }

            // Rewrap the let statements
            for (size_t i = lets.size(); i > 0; i--) {
                new_body = LetStmt::make(lets[i-1].first, lets[i-1].second, new_body);
            }

            // remove the loop statement if it is one of the scan loops
            stmt = new_body;

        // loop var is found in xcel
        } else if (is_loop_var){
          std::cout << "loopy\n";
            // remove the loop statement if it is one of the scan loops
            stmt = mutate(op->body);

        } else {
          std::cout << "create xcel\n";
            // should be left with store level match; we should produce hls_target
            internal_assert(xcel.store_level.match(op->name));
            debug(3) << "find the pipeline producing " << xcel.name << "\n";
            std::cout << "find the pipeline producing " << xcel.name << "\n";

            // walk inside of any let statements
            Stmt body = op->body;

            vector<pair<string, Expr>> lets;
            while (const LetStmt *let = body.as<LetStmt>()) {
                body = let->body;
                scope.push(let->name, simplify(expand_expr(let->value, scope)));
                lets.push_back(make_pair(let->name, let->value));
            }

            Stmt new_body = mutate(body);

            //stmt = For::make(xcel.name + ".accelerator", 0, 1, ForType::Serial, DeviceAPI::Host, body);
            Stmt new_body_produce = ProducerConsumer::make_produce("_hls_target." + xcel.name, new_body);
            Stmt new_body_consume = ProducerConsumer::make_consume("_hls_target." + xcel.name, Evaluate::make(0));
            new_body = Block::make(new_body_produce, new_body_consume);
            
            // add declarations of inputs and output (external) streams outside the hardware pipeline IR
            vector<string> external_streams;  // store input, self, and output streams
            external_streams.push_back(xcel.name);
            external_streams.insert(external_streams.end(), xcel.input_streams.begin(), xcel.input_streams.end());
            for (const string &name : external_streams) {
                const HWBuffer &kernel = xcel.hwbuffers.find(name)->second;
                string stream_name = need_hwbuffer(kernel) ?
                  kernel.name + ".stencil_update.stream" :
                  kernel.name + ".stencil.stream";

                string direction = kernel.is_output ? "stream_to_buffer" : "buffer_to_stream";
                Expr stream_var = Variable::make(Handle(), stream_name);

                // derive the coordinate of the sub-image block
                internal_assert(kernel.func.output_types().size() == 1);
                vector<Expr> image_args;
                for (size_t i = 0; i < kernel.dims.size(); i++) {
                    image_args.push_back(kernel.ldims[i].logical_min);
                }
                Expr address_of_subimage_origin = Call::make(Handle(), Call::buffer_get_host, {Call::make(kernel.func, image_args, 0)}, Call::Extern);
                Expr buffer_var = Variable::make(type_of<struct buffer_t *>(), kernel.name + ".buffer");

                // add intrinsic functions to convert memory buffers to streams
                // syntax:
                //   stream_subimage(direction, buffer_var, stream_var, address_of_subimage_origin,
                //                   dim_0_stride, dim_0_extent, ...)
                vector<Expr> stream_call_args({direction, buffer_var, stream_var, address_of_subimage_origin});
                for (size_t i = 0; i < kernel.dims.size(); i++) {
                    stream_call_args.push_back(Variable::make(Int(32), kernel.name + ".stride." + std::to_string(i)));
                    stream_call_args.push_back(kernel.ldims[i].logical_size);
                    //stream_call_args.push_back(simplify(kernel.dims[i].store_bound.max - kernel.dims[i].store_bound.min + 1));
                }
                Stmt stream_subimg = Evaluate::make(Call::make(Handle(), "stream_subimage", stream_call_args, Call::Intrinsic));

                Region bounds;
                for (auto &dim : kernel.dims) {
                    bounds.push_back(Range(0, dim.input_chunk));
                }
                new_body = Realize::make(stream_name, kernel.func.output_types(), MemoryType::Auto, bounds, const_true(), Block::make(stream_subimg, new_body));
            }
        

            /*
            // Handle tap values
            new_body = TransformTapStencils(xcel.taps).mutate(new_body);

            // Declare and initialize tap stencils with buffers
            // TODO move this call out side the tile loops over the kernel launch
            for (const auto &p : xcel.taps) {
                const HWTap &tap = p.second;
                const string stencil_name = tap.name + ".tap.stencil";

                Expr buffer_var = Variable::make(type_of<struct buffer_t *>(), tap.name + ".buffer");
                Expr stencil_var = Variable::make(Handle(), stencil_name);
                vector<Expr> args({buffer_var, stencil_var});
                Stmt convert_call = Evaluate::make(Call::make(Handle(), "buffer_to_stencil", args, Call::Intrinsic));

                // create a realizeation of the stencil
                Region bounds;
                for (const auto &dim : tap.dims) {
                    bounds.push_back(Range(0, dim.size));
                }
                vector<Type> types = tap.is_func ? tap.func.output_types() :
                    vector<Type>({tap.param.type()});
                new_body = Realize::make(stencil_name, types, MemoryType::Auto, bounds, const_true(), Block::make(convert_call, new_body));
            }
            */

            // Rewrap the let statements
            for (size_t i = lets.size(); i > 0; i--) {
                new_body = LetStmt::make(lets[i-1].first, lets[i-1].second, new_body);
            }
            stmt = For::make(op->name, op->min, op->extent, op->for_type, op->device_api, new_body);
        }
        return stmt;
    }

    // Remove the realize node for intermediate functions in the hardware pipeline,
    // as we will create hwbuffers in the pipeline to hold these values
    Stmt visit(const Realize *op) {
      std::cout << "this a realize\n";
      if (xcel.hwbuffers.count(op->name)) {
        const HWBuffer& buffer = xcel.hwbuffers.find(op->name)->second;
        
        if (!buffer.is_inlined && // is a hwbuffered function
            !buffer.is_output && // not the output
            //xcel.name != op->name && // not the output
            xcel.input_streams.count(op->name) == 0 // not a input
          ) {
          return mutate(op->body);
          // check constraints
          /*
            for (size_t i = 0; i < kernel.dims.size(); i++) {
            Expr store_extent = simplify(kernel.dims[i].store_bound.max -
            kernel.dims[i].store_bound.min + 1);
            if (!is_zero(simplify(op->bounds[i].extent - store_extent))) {
            user_error << "hwbuffer (" << store_extent << ") for " << op->name
            << " is not equal to realize extent (" << op->bounds[i].extent << ").\n";
            }
            }
          */
        }
      } //else
      return IRMutator::visit(op);
    }

  Stmt visit(const LetStmt *op) {
    Stmt stmt;
    Expr new_value = simplify(expand_expr(op->value, scope));
    scope.push(op->name, new_value);
    Stmt new_body = mutate(op->body);
    if (new_value.same_as(op->value) &&
        new_body.same_as(op->body)) {
      stmt = op;
    } else {
      stmt = LetStmt::make(op->name, new_value, new_body);
    }
    scope.pop(op->name);
    return stmt;
  }

public:
    InsertHWBuffers(const HWXcel &accel)
        : xcel(accel) {}
};

Stmt insert_hwbuffers(Stmt s, const HWXcel &xcel) {
  debug(3) << s << "\n";

  //for (auto &hwbuffer_pair : xcel.hwbuffers) {
    //std::cout << hwbuffer_pair.first << " is inline=" << hwbuffer_pair.second.is_inlined
              //<< " with num_dims=" << hwbuffer_pair.second.dims.size() << std::endl;
  //}
  
  s = InsertHWBuffers(xcel).mutate(s);
  debug(3) << s << "\n";
  //std::cout << "Inserted hwbuffers: \n" << s << "\n";
  return s;
}

}
}

