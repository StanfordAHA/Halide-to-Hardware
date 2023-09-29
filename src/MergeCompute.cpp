#include "MergeCompute.h"

#include "Func.h"
#include "IRMutator.h"

namespace Halide {
namespace Internal {

using std::map;
using std::string;
using std::vector;
using std::set;

map<string, vector<vector<Expr>>> kernel_mapping_args(const map<string, Function> &env) {
  map<string, vector<vector<Expr>>> args_map;
  
  for (const auto& function_pair : env) {
    const auto& function = function_pair.second;
    auto& sch = const_cast<FuncSchedule&>(function.schedule());

    // check each for compute sharing
    if (sch.is_compute_shared() && !sch.is_compute_parent()) {
      for (auto& level_pair : sch.shared_compute_level()) {
        int stage_idx = level_pair.first;
        LoopLevel shared_locked = level_pair.second.lock();
        string loopname = 
          shared_locked.is_root() ? "root" :
          shared_locked.is_inlined() ? "inlined" :
          shared_locked.var().name();

        internal_assert(sch.shared_parent_stage().count(stage_idx) > 0);
        string kernel_name = function.name() + "_stencil" +
          (stage_idx == 0 ? "" : "_" + std::to_string(stage_idx));
        string shared_kernel = sch.shared_parent_stage()[stage_idx];

        // create unique call for kernel mapping:
        //  compute_share_mapping(kernel_name_to_map, existing_kernel_name, loop_level)
        vector<Expr> args;
        args.emplace_back(StringImm::make(kernel_name));
        args.emplace_back(StringImm::make(shared_kernel));
        args.emplace_back(StringImm::make(loopname));
        args_map[function.name()].emplace_back(args);
        
        //std::cout << "func shares compute for kernel=" << function.name() << " i=" << stage_idx
        //          << ": " << kernel_name << " -> "
        //          << shared_kernel
        //          << " loop=" << loopname << std::endl << "   "
        //          << Call::make(Handle(), "compute_share_mapping", args, Call::Intrinsic)
        //          << std::endl;
      }
    }
  }
  return args_map;
}

class InsertKernelMappings : public IRMutator {
  using IRMutator::visit;

  map<string, vector<vector<Expr>>> kernel_mappings;

  Stmt visit(const Realize *op) override {
    if (kernel_mappings.count(op->name)) {
      std::cout << "found a new kernel mapping for " << op->name
                << " with " << kernel_mappings[op->name].size() << " mappings"
                << std::endl;
      auto op_mutated = IRMutator::visit(op);

      // Create block of kernel mappings and rest of stmts
      vector<Stmt> stmts;
      for (auto args : kernel_mappings[op->name]) {
        auto kernel_mapping = Call::make(Handle(), "compute_share_mapping", args, Call::Intrinsic);
        stmts.emplace_back(Evaluate::make(kernel_mapping));
      }
      stmts.emplace_back(op_mutated);
      
      auto block = Block::make(stmts);
      //std::cout << "new block is:\n" << block;
      return block;
    } else {
      return IRMutator::visit(op);
    }
  }
  
public:
  InsertKernelMappings(map<string, vector<vector<Expr>>>& kernel_mappings) :
    kernel_mappings(kernel_mappings) {};

};

Stmt insert_kernel_mappings(Stmt s, map<string, vector<vector<Expr>>> &kernel_mappings) {
  return InsertKernelMappings(kernel_mappings).mutate(s);
}


vector<LoopLevel> coarse_loop_args(const map<string, Function> &env) {
  vector<LoopLevel> args;
  
  for (const auto& function_pair : env) {
    const auto& function = function_pair.second;
    auto& sch = const_cast<FuncSchedule&>(function.schedule());

    // check each for compute sharing
    if (sch.shared_compute_level().size() > 0) {
      for (auto& level_pair : sch.shared_compute_level()) {
        int stage_idx = level_pair.first;
        LoopLevel shared_locked = level_pair.second.lock();
        string varname = 
          shared_locked.is_root() ? "root" :
          shared_locked.is_inlined() ? "inlined" :
          shared_locked.var().name();
        //string loopname = shared_locked.to_string();
        //int stage = shared_locked.is_root() || shared_locked.is_inlined() ? 0 : shared_locked.stage_index();
        string loopname = shared_locked.func() + ".s" + std::to_string(stage_idx) + "." + varname;

        args.emplace_back(shared_locked);
        
        std::cout << "func has coarse-grain loop for kernel=" << function.name() << " i=" << stage_idx
                  << ": coarse loop=" << loopname << std::endl << "   "
                  << Call::make(Handle(), "coarse_grain_loop_tag", {StringImm::make(loopname)}, Call::Intrinsic)
                  << std::endl;
      }
    }
  }
  return args;
}

class InsertCoarseLoops : public IRMutator {
  using IRMutator::visit;

  vector<LoopLevel> coarse_loops;

  Stmt visit(const For *op) override {
    bool found = false;
    for (auto& loop : coarse_loops) {
      if (loop.match(op->name)) {
        found = true;
        //std::cout << "Loop " << loop.to_string() << " matches " << op->name << std::endl;
        break;
      }
    }
    
      //if (coarse_loops.count(op->name)) {
    if (found) {
      //std::cout << "found coarse-grain loop called " << op->name << std::endl;
      
      Expr min = mutate(op->min);
      Expr extent = mutate(op->extent);
      Stmt body = mutate(op->body);

      // Create block of kernel mappings and rest of stmts
      vector<Stmt> stmts;
      Expr arg = StringImm::make(op->name);
      auto coarse_loop_call = Call::make(Handle(), "coarse_grain_loop_tag", {arg} , Call::Intrinsic);
      stmts.emplace_back(Evaluate::make(coarse_loop_call));
      stmts.emplace_back(body);
      auto new_body = Block::make(stmts);

      auto new_for = For::make(op->name, std::move(min), std::move(extent),
                     op->for_type, op->device_api, std::move(new_body));

      //std::cout << "new block is:\n" << block;
      return new_for;
    } else {
      return IRMutator::visit(op);
    }
  }
  
public:
  InsertCoarseLoops(vector<LoopLevel>& coarse_loops) :
    coarse_loops(coarse_loops) {};

};

Stmt insert_coarse_loops(Stmt s, vector<LoopLevel> &coarse_loops) {
  return InsertCoarseLoops(coarse_loops).mutate(s);
}



Stmt merge_compute(Stmt s, const map<string, Function> &env) {
  //std::cout << "merging" << std::endl;// << s << std::endl;
  
  // For each update schedule, if compute is shared, save compute kernel mapping.
  auto kernel_mappings = kernel_mapping_args(env);
  Stmt s_mapped = insert_kernel_mappings(s, kernel_mappings);
  //std::cout << "mapped" << std::endl << s_mapped;
  
  // For each update schedule, if compute is shared, save loops that are tagged as coarse grained.
  auto coarse_loops = coarse_loop_args(env);
  Stmt s_coarse = insert_coarse_loops(s_mapped, coarse_loops);
  //std::cout << "coarse loop calls inserted" << std::endl << s_coarse;

  return s_coarse;
}

} // Internal
} // Halide
