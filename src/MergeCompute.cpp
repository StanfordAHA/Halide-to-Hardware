#include "MergeCompute.h"

#include "Closure.h"
#include "Func.h"
#include "IRMutator.h"
#include "HWBufferUtils.h"
#include "Substitute.h"

namespace Halide {
namespace Internal {

using std::map;
using std::vector;
using std::string;

struct SharedKernel {
  string compute_parent;
  string looplevel;
  LoopLevel compute_looplevel;
  Stmt shared_compute_func;
  Type shared_compute_type;
  size_t num_func_inputs;

  map<string, vector<string>> func_inputs;
  map<string, vector<Expr>> input_exprs; // vector of different inputs to use
  vector<Expr> provide_args; // vector of provide variables
};

std::ostream& operator<<(std::ostream& os, const SharedKernel& sk) {
  os << "compute kernel:  parent=" << sk.compute_parent
     << " loop=" << sk.looplevel << std::endl;
  for (auto func_mp : sk.func_inputs) {
    os << " func " << func_mp.first << ": ";
    for (auto input : func_mp.second) {
      os << input << ", ";
    }
    os << std::endl;
  }
  return os;
} 

class Function_Closure : public Closure {
public:
  vector<string> input_vars;

  Function_Closure(Stmt s, std::set<string> ignoring)  {
    for (auto s : ignoring) {
      ignore.push(s);
    }

    s.accept(this);
  }

  void visit(const Call *op) override;

protected:
  using Closure::visit;
};

void Function_Closure::visit(const Call *op) {
  input_vars.emplace_back(op->name);
  //std::cout << "found call " << op->name << std::endl;
  if (op->call_type == Call::Intrinsic) {
    // consider call to stencil and stencil_update
    debug(3) << "visit call " << op->name << ": ";
    if (!ignore.contains(op->name)) {
      debug(3) << "adding to closure.\n";
      //std::cout << "adding to closure through call: " << op->name << "\n";
      input_vars.emplace_back(op->name);
    } else {
      debug(3) << "not adding to closure.\n";
    }
  }
  IRVisitor::visit(op);
}

vector<SharedKernel> find_shared_compute(const map<string, Function> &env) {
  vector<SharedKernel> shared_kernels;
  
  for (const auto &p : env) {
    Function func = p.second;
    auto sch = func.schedule();

    std::vector<Expr> vars;
    for (auto arg : func.args()) {
      auto var = Variable::make(Int(16), arg);
      vars.emplace_back(var);
    }

    //std::cout << func.name() << " is in env; " 
    //          << "  values for pure def: ";
    //auto vals = func.values();
    //for (auto val : vals) {
    //  std::cout << val << ", ";
    //}
    //std::cout << std::endl;
    //std::cout << " shared=" << sch.is_compute_shared()
    //          << " parent=" << sch.is_compute_parent()
    //          << std::endl;
    //
    //auto provide = Provide::make(func.name(), func.values(), vars);
    //auto closure = Function_Closure(Stmt(provide), {});
    //std::cout << "  vars: ";
    //for (auto var_type : closure.vars) {
    //  std::cout << var_type.first << ", ";
    //}
    //std::cout << "  buffers: ";
    //for (auto var_buffer : closure.buffers) {
    //  std::cout << var_buffer.first << ", ";
    //}
    //std::cout << "  inputs: ";
    //for (auto input : closure.input_vars) {
    //  std::cout << input << ", ";
    //}
    //std::cout << std::endl;

    if (sch.is_compute_parent()) {
      SharedKernel shared_kernel;
      shared_kernel.compute_parent = func.name();
      LoopLevel shared_locked = sch.shared_compute_level().lock();
      shared_kernel.looplevel =
        shared_locked.is_root() ? "root" :
        shared_locked.is_inlined() ? "inlined" :
        shared_locked.var().name();
      shared_kernel.compute_looplevel = shared_locked;
      
      map<string, vector<string>> compute_inputs; // shared_func -> input names
      for (const auto &input : sch.shared_func_names()) {
        Function shared_func = env.at(input);
        auto shared_provide = Provide::make(shared_func.name(), shared_func.values(), vars);
        auto shared_closure = Function_Closure(Stmt(shared_provide), {});
        compute_inputs[input] = shared_closure.input_vars;
      }

      shared_kernel.func_inputs = compute_inputs;
      shared_kernels.emplace_back(shared_kernel);

      std::cout << shared_kernel;
    }
  }

  return shared_kernels;
}

class PrintAcceleratedPortion : public IRVisitor {
  
  using IRVisitor::visit;

  void visit(const ProducerConsumer *op) {
    //string target_prefix = "hw_output";
    string target_prefix = "_hls_target.";
    if (starts_with(op->name, target_prefix) && op->is_producer) {
      Stmt hw_body = substitute_in_all_letstmts(op->body);
      std::cout << hw_body;
    } else {
      IRVisitor::visit(op);
    }
  }
  
public:
  PrintAcceleratedPortion() {};
};

void print_xcel(Stmt s) {
  PrintAcceleratedPortion pap;
  s.accept(&pap);
}

class GatherInputs : public IRVisitor {
  SharedKernel& kernel;
  string func_name;
  string call_target;
  
  using IRVisitor::visit;

  void visit(const Call *op) {
    if (op->name == call_target) {
      auto new_call = Call::make(op->type, op->name, kernel.provide_args, op->call_type);
      //kernel.input_exprs[func_name].emplace_back(Expr(op));
      kernel.input_exprs[func_name].emplace_back(Expr(new_call));
      std::cout << " input: " << Expr(op) << std::endl;
    } else {
      IRVisitor::visit(op);
    }
  }

public:
  GatherInputs(SharedKernel& kernel, string func_name, string call_target) :
    kernel(kernel), func_name(func_name), call_target(call_target) { };
};
  
class GatherInputOutputs : public IRVisitor {
  SharedKernel& kernel;
  bool only_parent;
  
  using IRVisitor::visit;

  void visit(const Provide *op) {
    if ((only_parent && op->name == kernel.compute_parent) || 
        (!only_parent && kernel.func_inputs.count(op->name) > 0)) {
      internal_assert(op->values.size() == 1);
      Expr value = op->values.at(0);

      std::cout << op->name << " provide  args=" << op->args << std::endl;
      if (op->name == kernel.compute_parent) {
        kernel.provide_args = op->args;
      }

      internal_assert(kernel.func_inputs.at(op->name).size() == kernel.num_func_inputs);
      for (auto input : kernel.func_inputs.at(op->name)) {
        GatherInputs gi(kernel, op->name, input);
        value.accept(&gi);
      }

    } else {
      IRVisitor::visit(op);
    }
  }

public:
  GatherInputOutputs(SharedKernel& kernel, bool only_parent)
    : kernel(kernel), only_parent(only_parent) {}
};

void gather_inouts(Stmt s, SharedKernel& kernel) {
  kernel.num_func_inputs = kernel.func_inputs[kernel.compute_parent].size();
  
  GatherInputOutputs gio1(kernel, true);
  s.accept(&gio1);
  GatherInputOutputs gio2(kernel, false);
  s.accept(&gio2);

}

class GetRootFunction : public IRVisitor {
  string root_function;
  SharedKernel& kernel;
  
  using IRVisitor::visit;

  void visit(const Provide *op) {
    if (root_function == op->name) {
      internal_assert(op->values.size() == 1);
      Expr value = op->values[0];
      kernel.shared_compute_type = value.type();
      Expr index = Variable::make(Int(32), "index");
      std::cout << "root function: " << Stmt(op);
      std::cout << " value: " << value << std::endl;
      
      //auto load = Load::make(value.type(), "share_input", index, Buffer<>(), Parameter(), const_true(), ModulusRemainder());
      //auto gen_load = substitute(kernel.input_exprs[op->name][0], Expr(load), Expr(value));
      //std::cout << " sub: " << gen_load << std::endl;
      auto gen_call = Expr(value);
      internal_assert(kernel.input_exprs.count(op->name) > 0) << op->name << " is not an input for ";
      size_t i=0;
      for (auto input : kernel.input_exprs.at(op->name)) {
        auto call = Call::make(value.type(), "share_input" + std::to_string(i), {index}, Call::Intrinsic);
        gen_call = substitute(input, Expr(call), gen_call);
        std::cout << " sub: " << gen_call << std::endl;
        i = i+1;
      }
      
      //auto gen_store = Store::make("share_output", gen_load, index, Parameter(), const_true(), ModulusRemainder());
      //std::cout << " store: " << gen_store << std::endl;
      auto gen_provide = Provide::make("share_output", {gen_call}, {index});
      std::cout << " store: " << gen_provide << std::endl;

      general_function = Stmt(gen_provide);
 
    } else {
      IRVisitor::visit(op);
    }
  }

public:
  Stmt general_function;
  GetRootFunction(string root_function, SharedKernel& kernel)
    : root_function(root_function), kernel(kernel) {}
};

Stmt generalize_root_function(Stmt s, SharedKernel& kernel) {
 GetRootFunction grf(kernel.compute_parent, kernel);
  s.accept(&grf);
  return grf.general_function;
}

class InsertMergedFunction : public IRMutator {
  SharedKernel& kernel;
  bool in_root_producer;
  bool in_compute_loop;

  const Realize* realize;
  
  using IRMutator::visit;


  vector<Stmt> create_muxes() {
    Expr index = Variable::make(Int(32), "index");
    vector<Stmt> muxes;
        
    auto& in_map = kernel.input_exprs;
    // go through each input to the compute unit, and create mux
    for (size_t in_idx = 0; in_idx < kernel.num_func_inputs; ++in_idx) {
      // mux is needed to build for each input
      Expr select;
          
      // go through iterative funcs in reverse order
      int iter = in_map.size()-1;
      for (auto func_pair = in_map.rbegin(); func_pair != in_map.rend(); ++func_pair, --iter) {
        auto funcname = func_pair->first;
            
        // innermost becomes the "else" with the last case
        if (func_pair == in_map.rbegin()) {
          select = func_pair->second[in_idx];
        } else {
          auto condition = EQ::make(index, iter);
          select = Select::make(condition, func_pair->second[in_idx], select);
        }
      }
      Stmt mux = Provide::make("share_input" + std::to_string(in_idx), {select}, {index});
      std::cout << "mux: " << mux << std::endl;

      muxes.emplace_back(mux);
    }
    return muxes;
  }

  Stmt create_demux() {
    Expr index = Variable::make(Int(32), "index");
    Type type = kernel.shared_compute_type;
    Stmt demux;

    auto& in_map = kernel.input_exprs;
    int iter = in_map.size()-1;
    
    // go through iterative funcs in reverse order, and create demux    
    for (auto func_pair = in_map.rbegin(); func_pair != in_map.rend(); ++func_pair, --iter) {
      auto funcname = func_pair->first;
            
      // innermost becomes the "else" with the last case
      if (func_pair == in_map.rbegin()) {
        auto call = Call::make(type, "share_output", {index}, Call::Intrinsic);
        demux = Provide::make(funcname, {call}, kernel.provide_args);
      } else {
        auto condition = EQ::make(index, iter);
        auto call = Call::make(type, "share_output", {index}, Call::Intrinsic);
        auto provide = Provide::make(funcname, {call}, kernel.provide_args);
        demux = IfThenElse::make(condition, provide, demux);
      }
    }
    std::cout << "demux: " << demux << std::endl;

    return demux;
  }

  Stmt visit(const Realize* op) {
    if (op->name == kernel.compute_parent) {
      realize = op;
    }
    return IRMutator::visit(op);
  }
  
  Stmt visit(const ProducerConsumer* op) {
    // look for PC of the root function
    if (op->name == kernel.compute_parent && op->is_producer) {
      in_root_producer = true;
      std::cout << "found pc" << std::endl;
      if (in_root_producer && in_compute_loop) {
        std::cout << "let's mutate" << std::endl;
      }
    }

    return IRMutator::visit(op);
  }

  Stmt visit(const For* op) {
    //std::cout << "this is loop " << op->name << " vs " << kernel.looplevel << std::endl;
    // look for shared compute loop
   if (kernel.compute_looplevel.match(op->name)) {
      in_compute_loop = true;
      std::cout << "found loop" << std::endl;
      if (in_root_producer && in_compute_loop) {
        std::cout << "let's mutate" << std::endl;
        std::cout << Stmt(op);
  
        // create select, compute, demux
        vector<Stmt> muxes = create_muxes();
        Stmt compute = kernel.shared_compute_func;
        Stmt demux = create_demux();

        // put together statements
        vector<Stmt> stmts = muxes;
        stmts.emplace_back(compute);
        stmts.emplace_back(demux);
        auto block = Block::make(stmts);
        auto iterative_compute  = For::make("index", 0, int(kernel.func_inputs.size()), ForType::Serial, DeviceAPI(), block);
        std::cout << iterative_compute;

        auto new_body = Stmt(iterative_compute);
        for (auto func_input : kernel.func_inputs) {
          if (func_input.first != kernel.compute_parent) {
            new_body = Realize::make(func_input.first, realize->types, realize->memory_type, realize->bounds, realize->condition, new_body);
          }
        }
        for (size_t i=0; i<kernel.num_func_inputs; ++i) {
          new_body = Realize::make("share_input"+std::to_string(i), realize->types, MemoryType::Auto, {Range(0,int(kernel.func_inputs.size()))}, const_true(), new_body);
        }
        new_body = Realize::make("share_output", realize->types, MemoryType::Auto, {Range(0,int(kernel.func_inputs.size()))}, const_true(), new_body);
        std::cout << "merged compute:\n" << new_body;
        return For::make(op->name, op->min, op->extent, op->for_type, op->device_api, new_body);
      }
    }
  
    return IRMutator::visit(op);
  }
    
public:
  InsertMergedFunction(SharedKernel& kernel) :
    kernel(kernel), in_root_producer(false), in_compute_loop(false) { };
};

// create merged function (realize inputs/outputs, loop, set inputs, compute, set output)
Stmt merge_function(Stmt s, SharedKernel& kernel) {
  // example:
  //   before: realize out4, for, for
  //           realize out1, prod out1, provide out1
  //           realize out2, prod out2, provide out2
  //           realize out3, prod out3, provide out3
  //   after:  realize out4, for, for (compute share looplevel)
  //           realize out1,out2,out3
  //           for i=0:3 { set gen_in, execute gen_out, store gen_out }
  return InsertMergedFunction(kernel).mutate(s);
}


Stmt merge_kernels(Stmt s, vector<SharedKernel> kernels) {
  Stmt stmt = s;
  
  for (auto kernel : kernels) {
    // get list of inputs and outputs
    gather_inouts(stmt, kernel);
    // go through root function and generalize inputs and output
    auto gen_func = generalize_root_function(stmt, kernel);
    kernel.shared_compute_func = gen_func;
    // create merged function (realize inputs/outputs, loop, set inputs, compute, set output)
    stmt = merge_function(stmt, kernel);
  }
  
  return stmt;
}

Stmt merge_compute(Stmt s, const map<string, Function> &env) {
  print_xcel(s);
  
  auto shared_kernels = find_shared_compute(env);
  auto stmt = merge_kernels(s, shared_kernels);

  //std::cout << "stmt at shared compute:\n" << s;
  return stmt;
}

} // Internal
} // Halide
