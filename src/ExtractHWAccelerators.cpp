#include "ExtractHWAccelerators.h"
#include "Func.h"
#include "IRMutator.h"

namespace Halide {
namespace Internal {

using std::map;
using std::vector;
using std::string;

enum AccelerateType { SingleCall, EnclosingFunc, CombinedCallFunc, Undefined };
std::ostream& operator<< (std::ostream& os, AccelerateType type) {
  switch (type) {
  case AccelerateType::SingleCall :      return os << "SingleCall";
  case AccelerateType::EnclosingFunc:    return os << "EnclosingFunc";
  case AccelerateType::CombinedCallFunc: return os << "CombinedCallFunc";
  case AccelerateType::Undefined:        return os << "Undefined";
    // omit default case to trigger compiler warning for missing cases
  };
  return os << static_cast<std::uint16_t>(type);
}

struct SimpleHWXcel {
  string name;
  LoopLevel store_level;
  LoopLevel compute_level;
  AccelerateType type;
};

class InsertHWXcel : public IRMutator {
  const SimpleHWXcel &xcel;

  using IRMutator::visit;

  Stmt create_xcel_call(Stmt stmt) {
    Stmt body = stmt;
    
    bool create_call = (xcel.type == AccelerateType::SingleCall) || (xcel.type == AccelerateType::CombinedCallFunc);
    bool create_xcel = (xcel.type == AccelerateType::EnclosingFunc) || (xcel.type == AccelerateType::CombinedCallFunc);
      
    if (create_call) {
      string name = "_hls_target." + xcel.name;
      Stmt new_body_produce = ProducerConsumer::make_produce(name, body);
      Stmt new_body_consume = ProducerConsumer::make_consume(name, Evaluate::make(0));
      body = Block::make(new_body_produce, new_body_consume);
      std::cout << "Found an xcel call: " << name << std::endl;
    }

    if (create_xcel) {
      string name = "_hls_accelerator." + xcel.name;
      Stmt new_body_produce = ProducerConsumer::make_produce(name, body);
      Stmt new_body_consume = ProducerConsumer::make_consume(name, Evaluate::make(0));
      body = Block::make(new_body_produce, new_body_consume);
      std::cout << "Found an accelerator: " << name << std::endl;
    }

    return body;
  }

  // look for compute and store loops for generating hardware buffers
  Stmt visit(const For *op) {
    if (xcel.store_level.match(op->name)) {
      //std::cout << "loop check: create xcel\n";
      // should be left with store level match; we should produce hls_target
      debug(3) << "find the pipeline producing " << xcel.name << "\n";

      //Stmt body = Stmt(op);
      //stmt = For::make(xcel.name + ".accelerator", 0, 1, ForType::Serial, DeviceAPI::Host, body);
      
      Stmt body = op->body;
      
      body = create_xcel_call(body);

      Stmt stmt = body;
      //std::cout << "body is:\n" << new_body_produce;

      return For::make(op->name, op->min, op->extent, op->for_type, op->device_api, stmt);

    } else {
      return IRMutator::visit(op);
    }
  }

  Stmt visit(const ProducerConsumer *op) {
    //if (xcel.store_level.match(op->name)) {
    if (xcel.store_level.var().name() == Var::outermost().name() &&
        op->name == xcel.name && op->is_producer) {
      //std::cout << "loop check: create xcel\n";
      // should be left with store level match; we should produce hls_target
      debug(3) << "find the pipeline producing " << xcel.name << "\n";

      //Stmt body = Stmt(op);
      //stmt = For::make(xcel.name + ".accelerator", 0, 1, ForType::Serial, DeviceAPI::Host, body);
      
      Stmt body = op->body;
      
      body = create_xcel_call(body);

      Stmt stmt = body;
      //std::cout << "body is:\n" << new_body_produce;

      return ProducerConsumer::make(op->name, op->is_producer, stmt);

    } else {
      return IRMutator::visit(op);
    }
  }

public:
  InsertHWXcel(const SimpleHWXcel &accel)
    : xcel(accel) {}
};

Stmt insert_hwxcel(Stmt s, const SimpleHWXcel &xcel) {

  //std::cout << "stream start: " << std::endl << s << std::endl;
  s = InsertHWXcel(xcel).mutate(s);
  debug(3) << s << "\n";
  //std::cout << "Inserted hwbuffers: \n" << s << "\n";
  return s;
}

vector<SimpleHWXcel> find_marked_hwxcels(Stmt s, const map<string, Function> &env) {
  vector<SimpleHWXcel> xcels;

  // for each accelerated function, build a hardware xcel: a dag of HW kernels
  for (const auto &p : env) {

    Function func = p.second;
    auto& sched = func.schedule();
    // skip this function if it is not accelerated
    if (!sched.is_accelerate_call_output() &&
        !sched.is_accelerator_output()) {
      continue;
    }

    SimpleHWXcel xcel;
    xcel.name = func.name();
    xcel.store_level = sched.accelerate_store_level();
    xcel.compute_level = sched.accelerate_compute_level();
    xcel.type =
      sched.is_accelerate_call_output() && sched.is_accelerator_output() ? AccelerateType::CombinedCallFunc :
      sched.is_accelerate_call_output() ? AccelerateType::SingleCall :
      sched.is_accelerator_output() ? AccelerateType::EnclosingFunc :
      AccelerateType::Undefined;

    LoopLevel store_locked = xcel.store_level.lock();
    string store_varname =
      store_locked.is_root() ? "root" :
      store_locked.is_inlined() ? "inlined" :
      store_locked.var().name();
    
    std::cout << "found an xcel for func " << xcel.name << " with loop "
              << store_varname << " of type " << xcel.type << std::endl;
  
    xcels.push_back(xcel);
  }
  
  return xcels;
}

Stmt extract_hwaccelerators(Stmt s, const map<string, Function> &env) {
  auto xcels = find_marked_hwxcels(s, env);
  Stmt stmt = s;
  for (auto xcel : xcels) {
    std::cout << "processing " << xcel.name << std::endl;
    stmt = insert_hwxcel(stmt, xcel);
  }
  return stmt;
}

} // Internal
} // Halide
