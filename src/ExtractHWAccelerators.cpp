#include "ExtractHWAccelerators.h"
#include "Func.h"
#include "IRMutator.h"

namespace Halide {
namespace Internal {

using std::map;
using std::vector;
using std::string;

struct SimpleHWXcel {
  string name;
  LoopLevel store_level;
  LoopLevel compute_level;
};

class InsertHWXcel : public IRMutator {
  const SimpleHWXcel &xcel;

  using IRMutator::visit;

  // look for compute and store loops for generating hardware buffers
  Stmt visit(const For *op) {
    if (xcel.store_level.match(op->name)) {
      std::cout << "loop check: create xcel\n";
      // should be left with store level match; we should produce hls_target
      debug(3) << "find the pipeline producing " << xcel.name << "\n";

      Stmt body = Stmt(op);

      //stmt = For::make(xcel.name + ".accelerator", 0, 1, ForType::Serial, DeviceAPI::Host, body);
      Stmt new_body_produce = ProducerConsumer::make_produce("_hls_target." + xcel.name, body);
      Stmt new_body_consume = ProducerConsumer::make_consume("_hls_target." + xcel.name, Evaluate::make(0));
      Stmt stmt = Block::make(new_body_produce, new_body_consume);
      return stmt;

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
    // skip this function if it is not accelerated
    if(!func.schedule().is_accelerated())
      continue;

    SimpleHWXcel xcel;
    xcel.name = func.name();
    xcel.store_level = func.schedule().accelerate_store_level();
    xcel.compute_level = func.schedule().accelerate_compute_level();

    LoopLevel store_locked = xcel.store_level.lock();
    string store_varname =
      store_locked.is_root() ? "root" :
      store_locked.is_inlined() ? "inlined" :
      store_locked.var().name();
    
    std::cout << "found an xcel for func " << xcel.name << " with loop " << store_varname << std::endl;
  
    xcels.push_back(xcel);
  }
  
  return xcels;
}

Stmt extract_hwaccelerators(Stmt s, const map<string, Function> &env) {
  auto xcels = find_marked_hwxcels(s, env);
  Stmt stmt = s;
  for (auto xcel : xcels) {
    stmt = insert_hwxcel(stmt, xcel);
  }
  return stmt;
}

} // Internal
} // Halide
