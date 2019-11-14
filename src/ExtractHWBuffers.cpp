#include "ExtractHWBuffers.h"
#include "SlidingWindow.h"

#include "Bounds.h"
#include "Debug.h"
#include "Func.h"
#include "HWUtils.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "IRPrinter.h"
#include "Monotonic.h"
#include "Scope.h"
#include "Simplify.h"
#include "Substitute.h"

#include "coreir.h"

using namespace CoreIR;

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

namespace {

class ReplaceOutputAccessPatternRanges : public IRMutator {
  using IRMutator::visit;
  int count;
  int max_count;
  const HWBuffer& kernel;

  Stmt visit(const For *old_op) override {
    Expr new_extent;
    if (count < max_count) {
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

        // use sliding window to get stencil sizes
        auto sliding_stencil_map = extract_sliding_stencils(new_body, iter->second);
        new_body = mutate(new_body);

        std::string for_namer = first_for_name(new_body);

        // Simplification possible when compute and store is the same level
        if (sched.compute_level() == sched.store_level()) {

          auto boxes_write = boxes_provided(new_body);
          auto boxes_read = boxes_required(new_body);

          // extent is the same for total buffer box, input chunk, and output stencil for double buffer
          internal_assert(boxes_read[op->name].size() == boxes_write[op->name].size());
          vector<Expr> box(boxes_read[op->name].size());
          for (size_t i=0; i<boxes_read[op->name].size(); ++i) {
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            box[i] = extent;
          }

          //FindOutputStencil fos(op->name, func, xcel_store_level);
          FindOutputStencil fos(op->name, xcel_store_level);
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
          
        } else {
          // look for a sliding window that can be used in a line buffer

          // use sliding window to get stencil sizes
          // Parameters 1 and 2
          FindOutputStencil fos(op->name, xcel_compute_level);
          new_body.accept(&fos);
          auto output_stencil_box = fos.output_stencil_box;

          CountBufferUsers counter(op->name);
          new_body.accept(&counter);
          
          // Parameters 3, 4, 5
          auto output_block_box = counter.output_block_box;
          auto input_block_box = counter.input_block_box;
          auto reader_loopnest = counter.reader_loopnest;

          auto boxes_read = boxes_required(new_body);

          size_t i_max = boxes_read[op->name].size();
          // Parameter 6
          auto total_buffer_box = vector<Expr>(i_max);
          for (size_t i=0; i<i_max; ++i) {
            Expr extent = simplify(expand_expr(op->bounds.at(i).extent, scope));
            total_buffer_box[i] = extent;
          }

          // create the hwbuffer
          std::string for_name = first_for_name(new_body);
          hwbuffer.dims = std::vector<InOutDimSize>(output_block_box.size());

          hwbuffer.ldims = vector<LogicalDimSize>(output_block_box.size());
          hwbuffer.stride_map = fos.stride_map;

          // check that all of the extracted parameters are of the same vector length
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
          
        }

        if (buffers.count(hwbuffer.name) == 0) {
          buffers[hwbuffer.name] = hwbuffer;
        }

        return IRMutator::visit(op);


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
        hwbuffers.at(consumer.name).input_streams.insert(hwbuffer.name);
      } else {
      }

      const Box &consumer_bounds = stage.bounds.find(make_pair(consumer.name,
                                                               consumer.stage))->second;
      const HWBuffer &consumer_buffer = hwbuffers.at(consumer.name);
      string consumer_name;
      if (consumer.name != hwbuffer.name) {
        if (consumer_buffer.is_inlined) {
          consumer_name = consumer_buffer.consumer_buffers.begin()->first;
        } else {
          consumer_name = consumer_buffer.name;
        }
      } else {
        consumer_name = hwbuffer.name;
      }

      hwbuffer.consumer_buffers[consumer_name] = std::make_shared<HWBuffer>(hwbuffers.at(consumer.name));

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

      //FindOutputStencil fos(hwbuffer.name, cur_func, func_compute_level);
      FindOutputStencil fos(hwbuffer.name, func_compute_level);
      consumer_buffer.my_stmt.accept(&fos);
      hwbuffer.stride_map = fos.stride_map;

      FindInputStencil fis(consumer.name, cur_func, func_compute_level, stencil_bounds);
      hwbuffer.my_stmt.accept(&fis);

      if (hwbuffers.count(consumer.name) == 0) {
        continue;
      }
      
      for (size_t idx=0; idx<hwbuffer.dims.size(); ++idx) {
        hwbuffer.dims.at(idx).input_chunk = 1;
        if (hwbuffer.dims.size() > idx) {
          hwbuffer.dims.at(idx).input_chunk = hwbuffer.dims.at(idx).input_block;
        }
        
        if (fis.found_stencil && idx < fis.output_min_pos_box.size()) { // this works
          hwbuffer.dims.at(idx).output_min_pos = fis.output_min_pos_box.at(idx);
          
          if (is_zero(hwbuffer.dims.at(idx).output_min_pos)) { // alternatively, not output
            hwbuffer.dims.at(idx).output_min_pos = consumer_buffer.dims.at(idx).output_min_pos;
          }
          
        }

        if (fos.found_stencil && idx < fos.output_stencil_box.size()) {
          hwbuffer.dims.at(idx).output_stencil = fos.output_stencil_box.at(idx);
          hwbuffer.dims.at(idx).output_block = fos.output_stencil_box.at(idx);
          if (!consumer_buffer.is_output) {
            hwbuffer.dims.at(idx).output_min_pos += fos.output_min_pos_box.at(idx);
          }
        } else {
        }

        hwbuffer.dims.at(idx).output_min_pos = simplify(expand_expr(consumer_bounds[idx].min, stencil_bounds));
        hwbuffer.dims.at(idx).output_max_pos = simplify(expand_expr(consumer_bounds[idx].max, stencil_bounds));
        auto loop_range = simplify(expand_expr(consumer_bounds[idx].max - consumer_bounds[idx].min + 1, stencil_bounds));
      }

      if (!hwbuffer.is_inlined && hwbuffers.count(consumer.name)) {
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

  cout << "------ All input streams to " << xcel->name << endl;
  for (auto in : xcel->input_streams) {
    cout << "\t" << in << endl;
  }
}

typedef uint64_t vd;
typedef uint64_t ed;

template<typename V, typename E>
class DGraph {

  uint64_t nv;
  uint64_t ne;

  public:

    std::map<vd, V> vertLabels;
    std::map<ed, E> edgeLabels;

    DGraph() : nv(0), ne(0) {}

    vd addVert(const V& label) {
      auto v = nv;
      nv++;
      vertLabels[v] = label;

      return v;
    }
};

bool isAcceleratorOutput(const Function& f) {
  return f.schedule().is_accelerator_output();
}

bool isAcceleratorInput(const Function& f) {
  return f.schedule().is_accelerator_input();
}

bool isAcceleratorInternal(const Function& f) {
  return f.schedule().is_hw_kernel() && !isAcceleratorInput(f) && !isAcceleratorOutput(f);
}

template<typename T>
class LoopOp {
  public:
    std::vector<const For*> surroundingLoops;
    T op;

    std::string prefixString() const {
      std::string str  = "";
      for (auto lp : surroundingLoops) {
        str += lp->name + " : [" + exprString(lp->min) + ", " + exprString(lp->extent) + "], ";
      }
      return str;
    }
};

class MemInfo {
  public:
    std::vector<LoopOp<const Provide*> > provides;
    std::vector<LoopOp<const Call*> > calls;
};

class MemoryMap : public IRGraphVisitor {
  public:

    using IRGraphVisitor::visit;

    const map<string, Function>& env;
    std::vector<const For*> activeLoops;
    std::map<string, MemInfo> memInfo;

    MemoryMap(const map<string, Function>& env_) : env(env_) {}

  protected:

    void visit(const For* lp) override {
      activeLoops.push_back(lp);

      auto tch = boxes_touched(lp->body);
      cout << "--- Boxes touched inside: " << lp->name << endl;
      for (auto bx : tch) {
        cout << "\t" << bx.first << endl;
        cout << "\t" << bx.second << endl;
      }
      IRGraphVisitor::visit(lp);

      activeLoops.pop_back();
    }

    void visit(const Provide* p) override {
      IRGraphVisitor::visit(p);
      if (contains_key(p->name, env)) {
        Function f = map_find(p->name, env);
        if (isAcceleratorOutput(f)) {
          cout << "Found output provide: " << p->name << endl;
          memInfo[p->name].provides.push_back({activeLoops, p});
        } else if (isAcceleratorInternal(f)) {
          cout << "Found internal provide: " << p->name << endl;
          memInfo[p->name].provides.push_back({activeLoops, p});
        } else if (isAcceleratorInput(f)) {
          cout << "Found input provide: " << p->name << "... ignoring" << endl;
        }
      }
    }

    void visit(const Call* c) override {
      IRGraphVisitor::visit(c);
      
      if (contains_key(c->name, env)) {
        Function f = map_find(c->name, env);
        if (isAcceleratorOutput(f)) {
          cout << "Found output call: " << c->name << endl;
          memInfo[c->name].calls.push_back({activeLoops, c});
        } else if (isAcceleratorInternal(f)) {
          cout << "Found internal call: " << c->name << endl;
          memInfo[c->name].calls.push_back({activeLoops, c});
        } else if (isAcceleratorInput(f)) {
          cout << "Found input call: " << c->name << endl;
          memInfo[c->name].calls.push_back({activeLoops, c});
        }
      }

    }
};

vector<HWXcel> extract_hw_accelerators(Stmt s, const map<string, Function> &env,
                                const vector<BoundsInference_Stage> &inlined_stages) {

  vector<HWXcel> xcels;
 
  s = substituteInConstants(s);

  cout << "#### All functions in env..." << endl;
  DGraph<Function, int> dg;
  for (const auto &p : env) {
    Function func = p.second;
    cout << "\tName: " << func.name() << endl;
    cout << "\t\tIs accel input : " << func.schedule().is_accelerator_input() << endl;
    cout << "\t\tIs accel output: " << func.schedule().is_accelerator_output() << endl;
    cout << "\t\tIs accelerated : " << func.schedule().is_accelerated() << endl;
    cout << "\t\tIs hwkernel    : " << func.schedule().is_hw_kernel() << endl;
    LoopLevel store_level = func.schedule().store_level().lock();
    LoopLevel compute_level = func.schedule().compute_level().lock();
    cout << "\t\tStore level  : " << store_level << endl;
    cout << "\t\tCompute level: " << store_level << endl;
    if (func.schedule().is_accelerated()) {
      cout << "\t\tAccel compute: " << func.schedule().accelerate_compute_level().lock() << endl;
      cout << "\t\tAccel store: " << func.schedule().accelerate_store_level().lock() << endl;
    }
    if (func.schedule().is_hw_kernel()) {
      dg.addVert(func);
    }
  }

  MemoryMap memMap(env);
  s.accept(&memMap);
  cout << "Memory mapping..." << endl;
  for (auto mm : memMap.memInfo) {
    cout << "\t" << mm.first << endl;
    cout << "\t--- Provides..." << endl;
    for (auto p : mm.second.provides) {
      cout << "\t\t" << p.prefixString() << ": " << p.op->name << endl;
    }
    cout << "\t--- Calls..." << endl;
    for (auto p : mm.second.calls) {
      cout << "\t\t" << p.prefixString() << ": " << p.op->name << endl;
    }
  }
  internal_assert(false) << "Stopping so dillon can view\n";

  // for each accelerated function, build a hardware xcel: a dag of HW kernels 
  for (const auto &p : env) {
    
    Function func = p.second;
    // skip this function if it is not accelerated
    if(!func.schedule().is_accelerated()) {
      continue;
    }

    LoopLevel store_locked = func.schedule().store_level().lock();
    string store_varname =
      store_locked.is_root() ? "root" :
      store_locked.is_inlined() ? "inlined" :
      store_locked.var().name();

    debug(3) << "Found accelerate function " << func.name() << "\n";
    debug(3) << store_locked.func() << " " << store_varname << "\n";
    HWXcel xcel;
    extract_hw_xcel_top_parameters(s, func, env, inlined_stages, &xcel);
    xcels.push_back(xcel);
  }
  return xcels;
}


}  // namespace Internal
}  // namespace Halide
