#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>

#include "CodeGen_Internal.h"
#include "CodeGen_CoreIR_Target.h"
#include "Debug.h"
#include "Substitute.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Param.h"
#include "Var.h"
#include "Lerp.h"
#include "Simplify.h"
#include "Debug.h"
#include "Float16.h"
#include "IRPrinter.h"

#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"
#include "coreir/simulator/interpreter.h"

namespace Halide {
namespace Internal {

using std::ostream;
using std::endl;
using std::string;
using std::vector;
using std::ostringstream;
using std::ofstream;
using std::cout;

using CoreIR::map_find;
using CoreIR::elem;
using CoreIR::contains_key;

namespace {

  int func_id_const_value(const Expr e) {
    if (const IntImm* e_int = e.as<IntImm>()) {
      return e_int->value;

    } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
      return e_uint->value;

    } else {
      return -1;
    }
  }
  
template<typename K, typename V>
V map_get(const K& k, const std::map<K, V>& m) {
  internal_assert(contains_key(k, m));
  return map_find(k, m);
}

int getConstInt(const Expr e) {
  if (const IntImm* e_int = e.as<IntImm>()) {
    return e_int->value;

  } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    return e_uint->value;

  } else {
    //internal_error << "invalid constant expr\n";
    return -1;
  }
}

class StoreCollector : public IRGraphVisitor {
  public:

    std::vector<const Store*> stores;
    std::map<std::string, std::map<int, int> > constStores;

    void visit(const Store* st) {
      stores.push_back(st);
      int storeIndex = getConstInt(st->index);
      // TODO: Change to isConst
      int storeValue = getConstInt(st->value);
      if  ((storeValue >= 0) && storeIndex >= 0) {
        constStores[st->name][storeIndex] = storeValue;
      }
      // Populate the constStores code
      IRGraphVisitor::visit(st);
    }
};

bool isCall(const std::string& str, const HWInstr* instr) {
  return instr->tp == HWINSTR_TP_INSTR && instr->name == str;
}

bool isStreamWrite(HWInstr* const instr) {
  return isCall("write_stream", instr);
}

std::vector<HWInstr*> outputStreams(HWFunction& f) {
  vector<HWInstr*> ins;
  for (auto instr : f.body) {
    if (isCall("write_stream", instr)) {
      ins.push_back(instr->getOperand(0));
    }
  }

  return ins;
}

std::vector<HWInstr*> inputStreams(HWFunction& f) {
  vector<HWInstr*> ins;
  for (auto instr : f.body) {
    if (isCall("rd_stream", instr)) {
      ins.push_back(instr->getOperand(0));
    }
  }

  return ins;
}

std::vector<HWInstr*> allStreams(HWFunction& f) {
  auto st = outputStreams(f);
  for (auto is : inputStreams(f)) {
    st.push_back(is);
  }
  return st;
}

std::set<std::string> allStreamNames(HWFunction& f) {
  std::set<std::string> strms;
  for (auto st : allStreams(f)) {
    strms.insert(st->name);
  }
  return strms;
}

template<typename K, typename V>
std::ostream& operator<<(std::ostream& out, const std::map<K, V>& strs) {
  out << "{";
  for (auto str : strs) {
    out << "{" << str.first << ", " << str.second << "}, ";
  }
  out << "}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& strs) {
  out << "{";
  for (auto str : strs) {
    out << str << ", ";
  }
  out << "}";
  return out;
}
vector<int> getDimRanges(const vector<int>& ranges) {
  vector<int> rngs;
  for (int i = 0; i < (int) (ranges.size() / 2); i++) {
    rngs.push_back(ranges[2*i + 1] - ranges[2*i]);
  }
  return rngs;
}

std::vector<int> toInts(const std::vector<std::string>& strs) {
  vector<int> ints;
  for (auto s : strs) {
    ints.push_back(stoi(s));
  }
  return ints;
}

std::string exprString(const Expr e) {
  ostringstream ss;
  ss << e;
  string en = ss.str();
  return en;
}

class ContainForLoop : public IRVisitor {
  using IRVisitor::visit;
  void visit(const For *op) {
    found = true;
    varnames.push_back(op->name);
  }

public:
  bool found;
  vector<string> varnames;
  ContainForLoop() : found(false) {}
};

// identifies for loops in code statement
bool contain_for_loop(Stmt s) {
  ContainForLoop cfl;
  s.accept(&cfl);
  return cfl.found;
}

// Identifies for loop name in code statement.
//  Gives name of first for loop
string name_for_loop(Stmt s) {
  ContainForLoop cfl;
  s.accept(&cfl);
  return cfl.varnames[0];
}

// Identifies all for loop names in code statement.
vector<string> contained_for_loop_names(Stmt s) {
  ContainForLoop cfl;
  s.accept(&cfl);
  return cfl.varnames;
}


class UsesVariable : public IRVisitor {
  using IRVisitor::visit;
  void visit(const Variable *op) {
    if (op->name == varname) {
      used = true;
    }
    return;
  }

  void visit(const Call *op) {
    // only go first two variables, not loop bound checks
    if (op->name == "write_stream" && op->args.size() > 2) {
      op->args[0].accept(this);
      op->args[1].accept(this);
    } else {
      IRVisitor::visit(op);
    }
  }

public:
  bool used;
  string varname;
  UsesVariable(string varname) : used(false), varname(varname) {}
};

// identifies target variable string in code statement
bool variable_used(Stmt s, string varname) {
  UsesVariable uv(varname);
  s.accept(&uv);
  return uv.used;
}

class ROMInit : public IRVisitor {
  using IRVisitor::visit;

  bool is_const(const Expr e) {
    if (e.as<IntImm>() || e.as<UIntImm>()) {
      return true;
    } else {
      return false;
    }
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
  
  void visit(const Store *op) {
    if (op->name == allocname) {
      auto value_expr = op->value;
      auto index_expr = op->index;
      internal_assert(is_const(value_expr) && is_const(index_expr));

      int index = id_const_value(index_expr);
      int value = id_const_value(value_expr);
      init_values["init"][index] = value;
    }
  }

public:
  nlohmann::json init_values;
  string allocname;
  ROMInit(string allocname) : allocname(allocname) {}
};

// returns a map with all the initialization values for a rom
nlohmann::json rom_init(Stmt s, string allocname) {
  ROMInit rom_init(allocname);
  s.accept(&rom_init);
  return rom_init.init_values;
}

  
class AllocationUsage : public IRVisitor {
  using IRVisitor::visit;
  void visit(const Load *op) {
    if (op->name == alloc_name) {
      num_loads++;
      load_index_exprs.emplace_back(op->index);
      
      if (!is_const(op->index)) {
        uses_variable_load_index = true;
      }
    }
  }

  void visit(const Store *op) {
    if (op->name == alloc_name) {
      num_stores++;
      store_index_exprs.emplace_back(op->index);
      
      if (!is_const(op->index)) {
        uses_variable_store_index = true;
      }
      if (!is_const(op->value)) {
        uses_variable_store_value = true;
      }

    }
  }


 public:
  bool uses_variable_load_index;
  bool uses_variable_store_index;
  bool uses_variable_store_value;
  bool load_index_equals_store_index;
  uint num_loads;
  uint num_stores;
  vector<Expr> store_index_exprs;
  vector<Expr> load_index_exprs;
  string alloc_name;
  
  AllocationUsage(string allocname) : uses_variable_load_index(false),
                                      uses_variable_store_index(false),
                                      uses_variable_store_value(false),
                                      load_index_equals_store_index(false),
                                      num_loads(0),
                                      num_stores(0),
                                      alloc_name(allocname) {}
};

enum AllocationType {
  NO_ALLOCATION,
  INOUT_ALLOCATION,
  ROM_ALLOCATION,
  REGS_ALLOCATION,
  SRAM_ALLOCATION,
  RMW_ALLOCATION,
  UNKNOWN_ALLOCATION
};

AllocationType identify_allocation(Stmt s, string allocname) {
  AllocationUsage au(allocname);
  s.accept(&au);

  if (au.num_stores == 0 || au.num_loads == 0) {
    return INOUT_ALLOCATION;
    
  } else if (!au.uses_variable_load_index &&
             !au.uses_variable_store_value) {
    //&& !au.uses_variable_store_value) {
    return NO_ALLOCATION;

  } else if (au.uses_variable_load_index &&
             !au.uses_variable_store_index &&
             !au.uses_variable_store_value) {
    return ROM_ALLOCATION;

  } else if (au.uses_variable_load_index &&
             au.uses_variable_store_index &&
             au.load_index_equals_store_index) {
    return RMW_ALLOCATION;

  } else if (au.uses_variable_load_index &&
             au.uses_variable_store_index &&
             !au.load_index_equals_store_index) {
    return SRAM_ALLOCATION;
    
  } else {
    return UNKNOWN_ALLOCATION;
  }
      
}
  
bool variable_index_load(Stmt s, string allocname) {
  AllocationUsage au(allocname);
  s.accept(&au);
  return au.uses_variable_load_index;
}

bool can_use_rom(Stmt s, string allocname) {
  AllocationUsage au(allocname);
  s.accept(&au);
  return (!au.uses_variable_store_index &&
          !au.uses_variable_store_value);
}


}

void loadHalideLib(CoreIR::Context* context) {
  auto hns = context->newNamespace("halidehw");

  CoreIR::Params widthParams{{"width", context->Int()}};
  CoreIR::Params widthDimParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}};
  CoreIR::Params widthDimParams3{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}, {"nchannels", context->Int()}};
  CoreIR::Params stencilReadParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()},
    {"r", context->Int()}, {"c", context->Int()}};
  CoreIR::Params stencilReadParams3{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}, {"nchannels", context->Int()},
    {"r", context->Int()}, {"c", context->Int()}, {"b", context->Int()}};

  CoreIR::TypeGen* tg = hns->newTypeGen("rd_stream", widthDimParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      auto nr = args.at("nrows")->get<int>();
      auto nc = args.at("ncols")->get<int>();
      auto w = args.at("width")->get<int>();
      return c->Record({{"in", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}});
      });
  auto rdStreamGen = hns->newGeneratorDecl("rd_stream", tg, widthDimParams);
  rdStreamGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      def->connect("self.in", "self.out");
      });


  {
    CoreIR::TypeGen* ws = hns->newTypeGen("write_stream", widthDimParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"stream", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}, {"stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)}});
        });
    auto wrStream = hns->newGeneratorDecl("write_stream", ws, widthDimParams);

    wrStream->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        def->connect("self.stream", "self.stencil");
        });

  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("write_stream_3", widthDimParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"stream", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}, {"stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto wrStream = hns->newGeneratorDecl("write_stream_3", ws, widthDimParams3);

    wrStream->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        def->connect("self.stream", "self.stencil");
        });

  }
  
  {
    CoreIR::TypeGen* ws = hns->newTypeGen("init_stencil", widthDimParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}});
        });
    auto initStencil = hns->newGeneratorDecl("init_stencil", ws, widthDimParams);

    initStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto m = def->getModule();
        auto r = m->getGenArgs().at("nrows")->get<int>();
        auto cls = m->getGenArgs().at("ncols")->get<int>();

        cout << "r = " << r << endl;
        cout << "cls = " << cls << endl;
        auto w = m->getGenArgs().at("width")->get<int>();
        for (int i = 0; i < r; i++) {
        for (int j = 0; j < cls; j++) {

          auto cs = def->addInstance("init_const_" + std::to_string(i) + "_" + std::to_string(j), "coreir.const", {{"width", CoreIR::Const::make(c, w)}}, {{"value", CoreIR::Const::make(c, BitVector(w, 0))}});
          def->connect(def->sel("self")->sel("out")->sel(j)->sel(i), cs->sel("out"));
        }
        }
        });
  }


  {
    CoreIR::TypeGen* ws = hns->newTypeGen("init_stencil_3", widthDimParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto initStencil = hns->newGeneratorDecl("init_stencil_3", ws, widthDimParams3);

    initStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto m = def->getModule();
        auto r = m->getGenArgs().at("nrows")->get<int>();
        auto cls = m->getGenArgs().at("ncols")->get<int>();
        auto chans = m->getGenArgs().at("nchannels")->get<int>();
        cout << "row = " << r << endl;
        cout << "col = " << cls << endl;
        cout << "chn = " << chans << endl;
        auto w = m->getGenArgs().at("width")->get<int>();
        for (int i = 0; i < r; i++) {
        for (int j = 0; j < cls; j++) {
        for (int b = 0; b < chans; b++) {
          auto cs = def->addInstance("init_const_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(b),
              "coreir.const",
              {{"width", CoreIR::Const::make(c, w)}}, {{"value", CoreIR::Const::make(c, BitVector(w, 0))}});
          def->connect(def->sel("self")->sel("out")->sel(b)->sel(j)->sel(i), cs->sel("out"));
          }
        }
        }
        });
  }
  {
    CoreIR::TypeGen* ws = hns->newTypeGen("stencil_read", stencilReadParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto width = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(width)->Arr(nr)->Arr(nc)}, {"out", c->Bit()->Arr(width)}});
        });
    auto readStencil = hns->newGeneratorDecl("stencil_read", ws, stencilReadParams);
    readStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto row = args.at("r")->get<int>();
        auto col = args.at("c")->get<int>();

        def->connect(def->sel("self")->sel("in")->sel(col)->sel(row), def->sel("self")->sel("out"));
        //def->connect(def->sel("self")->sel("in")->sel(0)->sel(row), def->sel("self")->sel("out"));
        });
  }


  {
    CoreIR::TypeGen* ws = hns->newTypeGen("create_stencil", stencilReadParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"new_val", c->BitIn()->Arr(w)}, {"in_stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}});
        });
    auto createStencil = hns->newGeneratorDecl("create_stencil", ws, stencilReadParams);
    createStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto nRows = args.at("nrows")->get<int>();
        auto nCols = args.at("ncols")->get<int>();
        //auto width = args.at("width")->get<int>();
        auto newR = args.at("r")->get<int>();

        cout << "Got r from create stencil = " << newR << endl;
        auto newC = args.at("c")->get<int>();
        cout << "Got C " << endl;

        auto self = def->sel("self");
        for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
          if ((i == newR) && (j == newC)) {
            //def->connect(def->sel("self")->sel("new_val"), self->sel("out")->sel(i)->sel(j));
            def->connect(def->sel("self")->sel("new_val"), self->sel("out")->sel(j)->sel(i));
          } else {
            //def->connect(self->sel("in_stencil")->sel(i)->sel(j), self->sel("out")->sel(i)->sel(j));
            // TODO: Fix this mismatch?
            //def->connect(self->sel("in_stencil")->sel(i)->sel(j), self->sel("out")->sel(j)->sel(i));
            def->connect(self->sel("in_stencil")->sel(j)->sel(i), self->sel("out")->sel(j)->sel(i));
          }
        }
        }
        });
  }
  
  {
    CoreIR::TypeGen* ws = hns->newTypeGen("create_stencil_3", stencilReadParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"new_val", c->BitIn()->Arr(w)}, {"in_stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto createStencil = hns->newGeneratorDecl("create_stencil_3", ws, stencilReadParams3);
    createStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto nRows = args.at("nrows")->get<int>();
        auto nCols = args.at("ncols")->get<int>();
        auto nChans = args.at("nchannels")->get<int>();
        
        auto newR = args.at("r")->get<int>();
        auto newC = args.at("c")->get<int>();
        auto newB = args.at("b")->get<int>();

        auto self = def->sel("self");
        for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
        for (int b = 0; b < nChans; b++) {
          if ((i == newR) && (j == newC) && (b == newB)) {
            def->connect(def->sel("self")->sel("new_val"), self->sel("out")->sel(b)->sel(j)->sel(i));
          } else {
            def->connect(self->sel("in_stencil")->sel(b)->sel(j)->sel(i), self->sel("out")->sel(b)->sel(j)->sel(i));
          }
          }
        }
        }
        });
  }
}


int c_inst_bitwidth(int input_bitwidth) {
   //FIXME: properly create bitwidths 1, 8, 16, 32
  if (input_bitwidth == 1) {
    return 1;
  } else {
    return 16;
  }
}


class NestExtractor : public IRGraphVisitor {
  public:
    std::vector<const For*> loops;

    bool inFor;

    NestExtractor() : inFor(false) {}

    void visit(const For* l) {
      if (inFor) {
        return;
      }

      loops.push_back(l);
      inFor = true;

      l->min.accept(this);
      l->extent.accept(this);
      l->body.accept(this);

      inFor = false;
    }
};

std::ostream& operator<<(std::ostream& out, const HWInstr& instr) {
  if (instr.tp == HWINSTR_TP_VAR) {
    out << instr.name;
  } else {
    out << (instr.predicate == nullptr ? "T" : instr.predicate->compactString()) << ": " << ("%" + std::to_string(instr.uniqueNum)) << " = " << instr.name << "(";
    for (auto op : instr.operands) {
      out << op->compactString() << ", ";
    }
    out << ");";
  }
  return out;
}

// Now: Schedule and collect the output
// Q: What is the schedule going to look like?
// A: List of stages containing instructions + II
//    And: Resource mapping for each operation
class HWLoopSchedule {
  public:
    vector<HWInstr*> body;
    vector<vector<HWInstr*> > stages;
    int II;

    std::map<HWInstr*, std::string> unitMapping;

    // Annoying things
    // 1. Hierarchical types in coreir
    // 2. Not sure if I want one record type per stream argument, or multiple ports for arguments
    // 3. Not sure if I want to change provides on stencils in to value producers or keep them as stores
    // 4. I dont want a bunch of special cases in hw instructions, I want them to be stateful or stateless

    // Q: What to do about provide instructions?
    // A: What is the problem there?
    //    Normal values just need to be set to
    //    their value in one pipeline stage, but then need
    //    to be used in many. stencils that are the subject
    //    of provide ops may be defined in several places
    //    and then used in several places. Though I suppose
    //    I could create the first instance of it as an
    //    undefined value, then set it later. That way
    //    I would not have to deal with implicitly set values
    //    in each provide operation.
    //
    // Also: Need a mapping from stages in the pipeline
    // to value locations in registers?
    // And we need a mapping from stream names in the
    // code to arguments?
};

class InnermostLoopChecker : public IRGraphVisitor {
  public:
    bool foundSubLoop;

    InnermostLoopChecker() : foundSubLoop(false) {}

    void visit(const For* f) {
      foundSubLoop = true;
    }
};

bool isInnermostLoop(const For* f) {
  InnermostLoopChecker c;
  f->body->accept(&c);
  return !c.foundSubLoop;
}

class LetEraser : public IRMutator {

  public:

    Stmt visit(const LetStmt* let) {
      auto newBody = this->mutate(let->body);
      return newBody;
    }

    Stmt visit(const For* f) {
      if (isInnermostLoop(f)) {
        return f;
      } else {
        return IRMutator::visit(f);
      }
    }
};

class LetPusher : public IRMutator {
  public:

    vector<const LetStmt*> letStack;

    Stmt visit(const LetStmt* let) {
      letStack.push_back(let);
      auto res = IRMutator::visit(let);
      letStack.pop_back();

      return res;
    }

    Stmt visit(const For* f) {
      if (isInnermostLoop(f)) {
        cout << "Found innermost loop with var: " << f->name << endl;
        Stmt lBody = f->body;
        for (int i = ((int) letStack.size()) - 1; i >= 0; i--) {
          auto let = letStack[i];
          lBody = LetStmt::make(let->name, let->value, lBody);
        }

        return For::make(f->name, f->min, f->extent, f->for_type, f->device_api, lBody);
      } else {
        return IRMutator::visit(f);
      }
    }
};

class InstructionCollector : public IRGraphVisitor {
  public:
    std::map<std::string, HWInstr*> vars;
    HWFunction f;
    HWInstr* lastValue;
    HWInstr* currentPredicate;
    
    Scope<std::vector<std::string> > activeRealizations;
    
    InstructionCollector() : lastValue(nullptr), currentPredicate(nullptr) {}

    HWInstr* newI() {
      auto ist = f.newI();
      ist->predicate = currentPredicate;
      return ist;
    }

    void pushInstr(HWInstr* instr) {
      f.body.push_back(instr);
    }

    void visit(const Realize* op) {
      if (ends_with(op->name, ".stencil")) {
        vector<std::string> fields;
        for (auto bnd : op->bounds) {
          fields.push_back(exprString(bnd.min));
          fields.push_back(exprString(bnd.extent));
        }
        activeRealizations.push(op->name, fields);
        IRGraphVisitor::visit(op);
        activeRealizations.pop(op->name);
        return;
      }
    
      IRGraphVisitor::visit(op);
    }

    HWInstr* varI(const std::string& name) {
      auto nI = newI();
      nI->tp = HWINSTR_TP_VAR;
      nI->name = name;
      return nI;
    }

    void visit(const UIntImm* imm) {
      auto ist = newI();
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = 16;
      ist->constValue = std::to_string(imm->value);
      lastValue = ist;
    }

    void visit(const IntImm* imm) {
      auto ist = newI();
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = 16;
      ist->constValue = std::to_string(imm->value);
      lastValue = ist;
    }

    void visit(const Cast* c) {
      auto ist = newI();
      ist->name = "cast";
      auto operand = codegen(c->value);
      ist->operands = {operand};
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const FloatImm* imm) {
      auto ist = newI();
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = 16;
      ist->constValue = std::to_string(imm->value);
      lastValue = ist;
    }

    void visit(const StringImm* imm) {
      auto ist = newI();
      ist->tp = HWINSTR_TP_STR;
      ist->strConst = imm->value;
      lastValue = ist;
    }

    void visit(const Provide* p) {

      vector<HWInstr*> operands;
      auto nameConst = newI();
      //nameConst->strConst = p->name;
      nameConst->name = p->name;
      nameConst->tp = HWINSTR_TP_VAR;
      operands.push_back(nameConst);
      for (size_t i = 0; i < p->values.size(); i++) {
        auto v = codegen(p->values[i]);
        //p->values[i].accept(this);
        internal_assert(v != nullptr) << " provide value is null?\n";
        operands.push_back(v);
      }
      for (size_t i = 0; i < p->args.size(); i++) {
        auto a = codegen(p->args[i]);
        internal_assert(a != nullptr) << " provide arg is null?\n";
        operands.push_back(a);
      }
      auto ist = newI(); 
      ist->name = "provide";
      ist->operands = operands;
      pushInstr(ist);
      lastValue = ist;
    }

    void visit(const Ramp* r) {

      IRGraphVisitor::visit(r);

      auto ist = newI();
      ist->name = "ramp";
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const Variable* v) {

      if (CoreIR::contains_key(v->name, vars)) {
        lastValue = CoreIR::map_find(v->name, vars);
        return;
      }

      cout << "Creating hwinstruction variable " << v->name << " that is not currently in vars" << endl;
      IRGraphVisitor::visit(v);
      auto ist = newI();
      ist->name = v->name;
      ist->tp = HWINSTR_TP_VAR;
      vars[v->name] = ist;

      lastValue = ist;
    }


    void visit(const GE* a) {
      visit_binop("gte", a->a, a->b);
    }

    void visit(const LE* a) {
      visit_binop("lte", a->a, a->b);
    }
    
    void visit(const LT* a) {
      visit_binop("lt", a->a, a->b);
    }

    void visit(const GT* a) {
      visit_binop("gt", a->a, a->b);
    }
    
    void visit(const And* a) {
      visit_binop("and", a->a, a->b);
    }

    HWInstr* codegen(const Expr e) {
      lastValue = nullptr;
      e.accept(this);

      internal_assert(lastValue) << "Codegen did not produce an LLVM value for " << e << "\n";
      return lastValue;
    }

    void visit(const Select* sel) {
      auto c = codegen(sel->condition);
      auto tv = codegen(sel->true_value);
      auto fv = codegen(sel->false_value);
      auto ist = newI();
      ist->name = "sel";
      ist->operands = {c, tv, fv};
      lastValue = ist;
      pushInstr(lastValue);
      //instrs.push_back(lastValue);
    }

    void visit_binop(const std::string& name, const Expr a, const Expr b) {
      auto aV = codegen(a);
      auto bV = codegen(b);
      auto ist = newI();
      ist->name = name;
      ist->operands = {aV, bV};
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const LetStmt* l) {

      auto ev = codegen(l->value);

      vars[l->name] = ev;

      l->body->accept(this);
      //IRGraphVisitor::visit(l->body);
      //auto lv = codegen(l->body);
      //internal_assert(lv) << "let body did not produce a value\n";
      //lastValue = lv;

      //vars.erase(l->name);
      
    }
    void visit(const Let* l) {

      auto ev = codegen(l->value);

      vars[l->name] = ev;

      auto lv = codegen(l->body);
      internal_assert(lv) << "let body did not produce a value\n";
      lastValue = lv;

      vars.erase(l->name);
      
    }

    HWInstr* strConstI(const std::string& name) {
      auto ist = newI();
      ist->tp = HWINSTR_TP_STR;
      ist->strConst = name;
      return ist;
    }

    void visit(const Load* ld) {
      auto ist = newI();
      ist->name = "load";
      vector<HWInstr*> operands;
      operands.push_back(strConstI(ld->name));
      operands.push_back(codegen(ld->predicate));
      operands.push_back(codegen(ld->index));
      ist->operands = operands;
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const Store* st) {
      auto ist = newI();
      vector<HWInstr*> operands;
      operands.push_back(strConstI(st->name));
      operands.push_back(codegen(st->predicate));
      operands.push_back(codegen(st->value));
      operands.push_back(codegen(st->index));
      ist->name = "store";
      ist->operands = operands;
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const Min* m) {
      visit_binop("min", m->a, m->b);
    }
    
    void visit(const Max* m) {
      visit_binop("max", m->a, m->b);
    }

    void visit(const Mod* d) {
      visit_binop("mod", d->a, d->b);
    }

    void visit(const Div* d) {
      visit_binop("div", d->a, d->b);
    }

    void visit(const Add* a) {
      visit_binop("add", a->a, a->b);
    }

    void visit(const EQ* a) {
      visit_binop("eq", a->a, a->b);
    }
    
    void visit(const NE* a) {
      visit_binop("neq", a->a, a->b);
    }
    
    void visit(const Mul* b) {
      visit_binop("mul", b->a, b->b);
    }

    void visit(const Sub* b) {
      visit_binop("sub", b->a, b->b);
    }

    HWInstr* andHW(HWInstr* a, HWInstr* b) {
      auto andOp = newI();
      andOp->name = "and";
      andOp->operands = {a, b};
      pushInstr(andOp);
      //instrs.push_back(andOp);
      return andOp;
    }
    
    void visit(const Call* op) {
      vector<HWInstr*> callOperands;
      //cout << "Processing call: " << op->name << endl;
      for (size_t i = 0; i < op->args.size(); i++) {
        op->args[i].accept(this);
        //cout << "Processing argument " << i << ": " << op->args[i] << endl;
        internal_assert(lastValue != nullptr) << "Error: In call lastValue is null\n";
        callOperands.push_back(lastValue);
        lastValue = nullptr;
      }

      auto ist = newI();
      if (op->name == "linebuffer") {
        ist->name = "linebuf_decl";
      } else if (op->name == "absd") {
        ist->name = "absd";
      } else if (op->name == "write_stream") {
        ist->name = "write_stream";
        assert(callOperands.size() > 1);

        string stencilName = exprString(op->args[1]);
        vector<string> bnds = activeRealizations.get(stencilName);
        vector<int> rngs = toInts(bnds);
        vector<int> windowDims = getDimRanges(rngs);
        ist->operandTypes.resize(callOperands.size());
        ist->operandTypes[1] = {16, windowDims};
      
      } else if (op->name == "read_stream") {
        ist->name = "read_stream";

        string stencilName = exprString(op->args[1]);
        vector<string> bnds = activeRealizations.get(stencilName);
        vector<int> rngs = toInts(bnds);
        vector<int> windowDims = getDimRanges(rngs);
        ist->operandTypes.resize(callOperands.size());
        ist->operandTypes[1] = {16, windowDims};

      } else if (ends_with(op->name, ".stencil")) {
        ist->name = "stencil_read";
        auto calledStencil = op->name;
        auto callOp = newI();
        callOp->tp = HWINSTR_TP_VAR;
        callOp->name = calledStencil;
        
        //callOp->strConst = calledStencil;
        callOperands.insert(std::begin(callOperands), callOp);

        //callOperands[0] = varI(callOperands[0]->compactString());
      } else if (ends_with(op->name, ".stencil_update")) {
        ist->name = "stencil_update";
      } else {
        ist->name = "other_instr";
        internal_assert(false) << "Unsupported call: " << op->name << "\n";
      }
      ist->operands = callOperands;
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }
};

//vector<HWInstr*> buildHWBody(const For* perfectNest) {

HWFunction buildHWBody(const std::string& name, const For* perfectNest) {

  InstructionCollector collector;
  perfectNest->accept(&collector);
  collector.f.name = name;
  //return collector.f.body;
  return collector.f;
}

class StencilInfoCollector : public IRGraphVisitor {
  public:

    StencilInfo info;
    //vector<DispatchInfo> dispatches;
    Scope<std::vector<std::string> > activeRealizations;

    // So when a realization happens

    void visit(const Call* op) {
      cout << "Stencil visiting call: " << op->name << endl;
      if (op->name == "dispatch_stream") {
        cout << "Found dispatch" << endl;
        cout << "\tName = " << op->args[0] << "\n";
        cout << "\t# dims = " << op->args[1] << "\n";

        vector<string> dinfo;
        for (int i = 1; i  < op->args.size(); i++) {
          Expr e = op->args[i];
          ostringstream ss;
          ss << e;

          string en = ss.str();
          dinfo.push_back(en);
        }

        ostringstream ss;
        ss << op->args[0];
        info.streamDispatches[ss.str()] = dinfo;
      } else if (op->name == "linebuffer") {
        info.linebuffers.push_back({});
        for (auto arg : op->args) {
          info.linebuffers.back().push_back(exprString(arg));
        }
      } else if (op->name == "read_stream") {
        string stencilDest = exprString(op->args[1]);

        internal_assert(CoreIR::contains_key(stencilDest, info.stencilRealizations)) << "no entry for " << stencilDest << "\n";
        auto realizeParams = CoreIR::map_find(stencilDest, info.stencilRealizations);
        info.streamReadRealize[exprString(op->args[0])] = realizeParams;
        
        info.streamReads[exprString(op->args[0])] = exprString(op->args[1]);

        info.streamReadCallRealizations[op] = realizeParams;
        info.streamReadCalls[exprString(op->args[0])].push_back(op);

      } else if (op->name == "write_stream") {
        string stencilDest = exprString(op->args[1]);

        auto realizeParams = activeRealizations.get(stencilDest);
        info.streamWriteRealize[exprString(op->args[0])] = realizeParams;
        info.streamWrites[exprString(op->args[0])] = exprString(op->args[1]);

        info.streamWriteCallRealizations[op] = realizeParams;
        info.streamWriteCalls[exprString(op->args[0])].push_back(op);

      }
    }

    void visit(const Realize* op)  {
      if (ends_with(op->name, ".stream")) {
        auto tps = op->types[0];
        auto bnds = op->bounds;
        cout << "Realizing " << op->name << " with type = " << tps << endl;
        cout << "and bounds..." << endl;
        for (auto bnd : bnds) {
          cout << "\t" << bnd.min << " with extend: " << bnd.extent << endl;
        }

        IRGraphVisitor::visit(op);

      } else if (ends_with(op->name, ".stencil")) {

        //if (CoreIR::contains_key(op->name, info.stencilRealizations)) {
          //IRGraphVisitor::visit(op);
          //return;
        //}

        
        //internal_assert(!CoreIR::contains_key(op->name, info.stencilRealizations)) << "Realizations already contains an entry for " << op->name << "\n";
        //
        info.stencilRealizations[op->name] = {};
        auto tps = op->types[0];
        auto bnds = op->bounds;
        cout << "Realizing " << op->name << " with type = " << tps << endl;
        cout << "and bounds..." << endl;
        for (auto bnd : bnds) {
          cout << "\t" << bnd.min << " with extend: " << bnd.extent << endl;
          info.stencilRealizations[op->name].push_back(exprString(bnd.min));
          info.stencilRealizations[op->name].push_back(exprString(bnd.extent));
        }

        activeRealizations.push(op->name, info.stencilRealizations[op->name]);
        IRGraphVisitor::visit(op);
        activeRealizations.pop(op->name);

      }

    }

};

void replaceAll( string &s, const string &search, const string &replace ) {
    for( size_t pos = 0; ; pos += replace.length() ) {
        // Locate the substring to replace
        pos = s.find( search, pos );
        if( pos == string::npos ) break;
        // Replace by erasing and inserting
        s.erase( pos, search.length() );
        s.insert( pos, replace );
    }
}

// Note that stencils will need to have
// multiple define (or set) stages
// Also note that inside a cycle different
// instructions may use the current value of an
// instruction or a later value
// Note: For each instruction in a pipeline
// its intermediate result register will only
// be stored to if the instruction predicate
// is true in that stage, and the stage is active
class PipelineInfo {
  public:
    HWInstr* instruction;
    int defStage;
    vector<int> useStages;
};

vector<int> getStencilDims(const std::string& name, StencilInfo& info) {
  if (CoreIR::contains_key(name, info.stencilRealizations)) {
    vector<string> rz = CoreIR::map_find(name, info.stencilRealizations);
    internal_assert(rz.size() % 2 == 0);
    vector<int> dims;
    for (int i = 0; i < rz.size() / 2; i++) {
      string min = rz[2*i];
      string ext = rz[2*i + 1];
      dims.push_back(stoi(min));
      dims.push_back(stoi(ext));

    }
    return dims;
  }

  internal_assert(false) << "No stencil dimensions for " << name << "\n";
}


vector<int> getStreamDims(const std::string& str, StencilInfo& info) {
  //internal_assert(CoreIR::contains_key(str, info.streamParams)) << " error: no stream named " << str << " in stream params\n";
  //<< info.streamParams << "\n";
  if (CoreIR::contains_key(str, info.streamParams)) {
    return toInts(CoreIR::map_find(str, info.streamParams));
  }

  // Assume this is a top-level input
  return {0, 1, 0, 1};

  //if (CoreIR::contains_key(str, info.streamWrites)) {

    //string inputStencil = info.streamWrites[str];
    //cout << "Stream " << str << " writes to stencil " << inputStencil << endl;
    //return getStencilDims(inputStencil, info);
  //}


  //if (CoreIR::contains_key(str, info.streamReads)) {

    //string inputStencil = info.streamReads[str];
    //cout << "Stream " << str << " reads from stencil " << inputStencil << endl;
    //return getStencilDims(inputStencil, info);
  //}

  //return {0, 1, 0, 1};
  //internal_assert(false) << "No stream dims for " << str << "\n";
}

#define COREMK(ctx, v) CoreIR::Const::make((ctx), (v))

std::string coreirSanitize(const std::string& str) {
  string san = "";
  for (auto c : str) {
    if (c == '.') {
      san += "_";
    } else {
      san += c;
    }
  }
  return san;
}

class UnitMapping {
  public:
    std::map<HWInstr*, CoreIR::Wireable*> instrValues;
    std::map<HWInstr*, vector<int> > stencilRanges;
    std::map<HWInstr*, CoreIR::Instance*> unitMapping;

    std::map<HWInstr*, int> productionStages;
    std::map<HWInstr*, std::map<int, CoreIR::Instance*> > pipelineRegisters;

    std::vector<HWInstr*> body;

    bool hasOutput(HWInstr* const arg) const {
      return CoreIR::contains_key(arg, instrValues);
    }

    CoreIR::Type* outputType(HWInstr* const arg) const {
      internal_assert(CoreIR::contains_key(arg, instrValues));
      return CoreIR::map_find(arg, instrValues)->getType();
    }

    bool isOutputArg(HWInstr* arg) const {
      return outputType(arg)->isInput();
    }
 
    CoreIR::Wireable* valueAt(HWInstr* const arg1, const int stageNo) {
      string iValStr = "{";
      for (auto kv : instrValues) {
        iValStr += "\t{" + kv.first->compactString() + " -> " + CoreIR::toString(*(kv.second)) + "}, " + "\n";
      }
      iValStr += "}";
      internal_assert(CoreIR::contains_key(arg1, instrValues)) << *arg1 << " is not in instrValues: " << iValStr << "\n";

      if (arg1->tp == HWINSTR_TP_CONST) {
        return CoreIR::map_find(arg1, instrValues);
      }

      if (arg1->tp == HWINSTR_TP_VAR && isOutputArg(arg1)) {
        return CoreIR::map_find(arg1, instrValues);
      }
      //if (arg1->tp == HWINSTR_TP_VAR || (arg1->tp == HWINSTR_TP_CONST)) {
        //return CoreIR::map_find(arg1, instrValues);
      //}

      internal_assert(CoreIR::contains_key(arg1, productionStages)) << *arg1 << " is not produced at any stage of the pipeline\n";

      int producedStage = CoreIR::map_find(arg1, productionStages);

      internal_assert(producedStage <= stageNo) << "Error: " << *arg1 << " is produced in stage " << producedStage << " but we try to consume it in stage " << stageNo << "\n";
      if (stageNo == CoreIR::map_find(arg1, productionStages)) {
        return CoreIR::map_find(arg1, instrValues);
      } else {
        internal_assert(CoreIR::contains_key(arg1, pipelineRegisters)) << "no pipeline register for " << *arg1 << "\n";
        auto pregs = CoreIR::map_find(arg1, pipelineRegisters);

        internal_assert(CoreIR::contains_key(stageNo, pregs)) << "no register for " << *arg1 << " at stage " << stageNo << "\n";

        return CoreIR::map_find(stageNo, pregs)->sel("out");
      }
    }
};

CoreIR::Instance* pipelineRegister(CoreIR::Context* context, CoreIR::ModuleDef* def, const std::string name, CoreIR::Type* type) {

  if (type == context->BitIn() || type == context->Bit()) {
    auto r = def->addInstance(name, "corebit.reg");
    return r;
  }
  auto r = def->addInstance(name, "commonlib.reg_array", {{"type", COREMK(context, type)}});
  return r;
}

UnitMapping createUnitMapping(StencilInfo& info, CoreIR::Context* context, HWLoopSchedule& sched, CoreIR::ModuleDef* def) {

  int defStage = 0;

  UnitMapping m;
  m.body = sched.body;
  auto& unitMapping = m.unitMapping;
  auto& instrValues = m.instrValues;
  auto& stencilRanges = m.stencilRanges;
  
  std::set<std::string> pipeVars;
  for (auto instr : sched.body) {
    if (instr->tp == HWINSTR_TP_INSTR) {
      string name = instr->name;
      //cout << "Instruction name = " << name << endl;
      if (name == "add") {
        auto adder = def->addInstance("add_" + std::to_string(defStage), "coreir.add", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = adder->sel("out");
        unitMapping[instr] = adder;
      } else if (name == "and_bv") {
        auto adder = def->addInstance("and_bv_" + std::to_string(defStage), "coreir.and", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = adder->sel("out");
        unitMapping[instr] = adder;

      } else if (name == "mul") {
        auto mul = def->addInstance("mul_" + std::to_string(defStage), "coreir.mul", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "absd") {
        auto mul = def->addInstance("absd" + std::to_string(defStage), "commonlib.abs", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      }else if (name == "mod") {

        // TODO: Replace this with real implementation of mod!!!
        auto mul = def->addInstance("mod" + std::to_string(defStage), "coreir.sub", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "sub") {
        auto mul = def->addInstance("sub_" + std::to_string(defStage), "coreir.sub", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "div") {
        auto mul = def->addInstance("div_" + std::to_string(defStage), "coreir.udiv", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "max") {
        auto mul = def->addInstance("max_" + std::to_string(defStage), "commonlib.smax", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "min") {
        auto mul = def->addInstance("min_" + std::to_string(defStage), "commonlib.smin", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "eq") {
        auto mul = def->addInstance("eq" + std::to_string(defStage), "coreir.eq", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "neq") {
        auto mul = def->addInstance("neq" + std::to_string(defStage), "coreir.neq", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;

      } else if (name == "lt") {
        auto mul = def->addInstance("lt_" + std::to_string(defStage), "coreir.slt", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "gt") {
        auto mul = def->addInstance("gt_" + std::to_string(defStage), "coreir.sgt", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "lte") {
        auto mul = def->addInstance("lte_" + std::to_string(defStage), "coreir.sle", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "and") {
        auto mul = def->addInstance("and_" + std::to_string(defStage), "corebit.and");
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "sel") {
        auto sel = def->addInstance("sel_" + std::to_string(defStage), "coreir.mux", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = sel->sel("out");
        unitMapping[instr] = sel;
      } else if (name == "cast") {
        auto cs = def->addInstance("wire_" + std::to_string(defStage), "coreir.wire", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = cs->sel("out");
        unitMapping[instr] = cs;
      } else if (name == "rd_stream") {
        auto dims = getStreamDims(instr->operands[0]->name, info);
        cout << "# of dims in " << instr->operands[0]->name << " = " << dims.size() << endl;
        for (auto d : dims) {
          cout << "Dim = " << d << endl;
        }

        vector<int> dimRanges = getDimRanges(dims);
        stencilRanges[instr] = dimRanges;
        auto rdStrm = def->addInstance("rd_stream_" + std::to_string(defStage), "halidehw.rd_stream", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", CoreIR::Const::make(context, dimRanges[0])}, {"ncols", CoreIR::Const::make(context, dimRanges[1])}});

        auto res = rdStrm->sel("out");
        instrValues[instr] = res;
        unitMapping[instr] = rdStrm;
      } else if (name == "write_stream") {

        internal_assert(instr->operandTypes.size() > 1);
        auto otherDims = instr->operandTypes[1];
        cout << "# of dims in write_stream to " << instr->getOperand(0)->compactString() << " = " << otherDims.dims.size() << endl;
        for (auto d : otherDims.dims) {
          cout << "\t" << d << endl;
        }
        
        auto dims = getStreamDims(instr->operands[0]->name, info);
        //vector<int> dimRanges = getDimRanges(dims);
        //auto dimRanges = getDimRanges(otherDims.dims);
        auto dimRanges = otherDims.dims;

        if (dimRanges.size() == 2) {
          auto wrStrm = def->addInstance("write_stream_" + std::to_string(defStage), "halidehw.write_stream", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}});
          unitMapping[instr] = wrStrm;
        } else {
          internal_assert(dimRanges.size() == 3);
          auto wrStrm = def->addInstance("write_stream_" + std::to_string(defStage), "halidehw.write_stream_3", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}, {"nchannels", COREMK(context, dimRanges[2])}});
          unitMapping[instr] = wrStrm;
        }
      } else if (starts_with(name, "init_stencil")) {
        int bnds = instr->getOperand(0)->toInt();
        vector<int> dims;
        for (int i = 1; i < 1 + bnds; i++) {
          dims.push_back(instr->getOperand(i)->toInt());
        }

        auto dimRanges = getDimRanges(dims);

        if (dimRanges.size() == 2) {
          stencilRanges[instr] = dimRanges;
          auto initS = def->addInstance("init_stencil_" + std::to_string(defStage), "halidehw.init_stencil", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}});
          instrValues[instr] = initS->sel("out");
          unitMapping[instr] = initS;
        } else {
          internal_assert(dimRanges.size() == 3);
          stencilRanges[instr] = dimRanges;
          auto initS = def->addInstance("init_stencil_" + std::to_string(defStage), "halidehw.init_stencil_3", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}, {"nchannels", COREMK(context, dimRanges[2])}});
          instrValues[instr] = initS->sel("out");
          unitMapping[instr] = initS;
        }
      } else if (starts_with(name, "create_stencil")) {


        auto dimRanges = CoreIR::map_find(instr->getOperand(0), stencilRanges);

        if (dimRanges.size() == 2) {
          int selRow = instr->getOperand(2)->toInt();
          int selCol = instr->getOperand(3)->toInt();
          auto cS = def->addInstance("create_stencil_" + std::to_string(defStage), "halidehw.create_stencil", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}});

          stencilRanges[instr] = dimRanges;
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        } else {
          internal_assert(dimRanges.size() == 3);

          int selRow = instr->getOperand(2)->toInt();
          int selCol = instr->getOperand(3)->toInt();
          int selChan = instr->getOperand(4)->toInt();
          auto cS = def->addInstance("create_stencil_" + std::to_string(defStage), "halidehw.create_stencil_3", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}, {"nchannels", COREMK(context, dimRanges[2])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}, {"b", COREMK(context, selChan)}});

          stencilRanges[instr] = dimRanges;
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        }

      } else if (starts_with(name, "stencil_read")) {
        vector<int> dimRanges = CoreIR::map_find(instr->getOperand(0), stencilRanges);
        internal_assert(dimRanges.size() > 1) << "dimranges has size: " << dimRanges.size() << "\n";

        int selRow = instr->getOperand(1)->toInt();
        int selCol = instr->getOperand(2)->toInt();
        auto cS = def->addInstance("stencil_read_" + std::to_string(defStage), "halidehw.stencil_read", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", COREMK(context, dimRanges[0])}, {"ncols", COREMK(context, dimRanges[1])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}});
        //auto cS = def->addInstance("stencil_read_" + std::to_string(defStage), "halidehw.stencil_read", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = cS->sel("out");
        unitMapping[instr] = cS;
      } else if (name == "ashr") {
        auto shr = def->addInstance("ashr" + std::to_string(defStage), "coreir.ashr", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = shr->sel("out");
        unitMapping[instr] = shr;
      } else {
        internal_assert(false) << "no functional unit generation code for " << *instr << "\n";
      }
    }

    int constNo = 0;
    for (auto op : instr->operands) {
      if (op->tp == HWINSTR_TP_CONST) {
        int width = op->constWidth;
        int value = stoi(op->constValue);
        BitVector constVal = BitVector(width, value);
        //cout << "Constant value for operand " << op->compactString() << " in instruction " << *instr << " is = " << constVal << ", as int = " << constVal.to_type<int>() << endl;
        auto cInst = def->addInstance("const_" + std::to_string(defStage) + "_" + std::to_string(constNo), "coreir.const", {{"width", CoreIR::Const::make(context, width)}},  {{"value", CoreIR::Const::make(context, BitVector(width, value))}});
        constNo++;
        instrValues[op] = cInst->sel("out");
      } else if (op->tp == HWINSTR_TP_VAR) {
        string name = op->name;
        if (CoreIR::elem(name, pipeVars)) {
          continue;
        }
        pipeVars.insert(name);
        //cout << "Finding argument value for " << name << endl;
        auto self = def->sel("self");
        auto val = self->sel(coreirSanitize(name));

        instrValues[op] = val;
        
        if (val->getType()->isOutput()) {
          m.productionStages[op] = 0;

          for (int stage = 0; stage < (int) sched.stages.size(); stage++) {
            m.pipelineRegisters[op][stage] = pipelineRegister(context, def, coreirSanitize(op->name) + "_reg_" + std::to_string(stage), m.outputType(op));
          }

          // Need to decide how to map pipeline stage numbers? Maybe start from stage 1?
          for (int stage = 0; stage < ((int) sched.stages.size()) - 1; stage++) {
            if (stage == 0) {
              def->connect(m.pipelineRegisters[op][stage + 1]->sel("in"), instrValues[op]);
            } else {
              def->connect(m.pipelineRegisters[op][stage + 1]->sel("in"), m.pipelineRegisters[op][stage]->sel("out"));
            }
          }
        }
      }
    }
    defStage++;
  }
  
  
  int uNum = 0;
  for (int i = 0; i < (int) sched.stages.size(); i++) {
    auto& stg = sched.stages[i];

    for (auto instr : stg) {
      m.productionStages[instr] = i;
    }

    for (auto instr : m.body) {
      if (m.hasOutput(instr)) {
        m.pipelineRegisters[instr][i] = pipelineRegister(context, def, "pipeline_reg_" + std::to_string(i) + "_" + std::to_string(uNum), m.outputType(instr));
        uNum++;
      }
    }
  }

  // Now: Wire up pipeline registers in chains, delete the unused ones and test each value produced in this code
  for (auto instr : sched.body) {
    if (m.hasOutput(instr)) {
      auto fstVal = CoreIR::map_find(instr, m.instrValues);
      int prodStage = CoreIR::map_find(instr, m.productionStages);

      CoreIR::Wireable* lastReg = fstVal;
      //auto lastReg = m.pipelineRegisters[instr][prodStage];
      //def->connect(lastReg->sel("in"), fstVal);
      for (int i = prodStage + 1; i < (int) sched.stages.size(); i++) {
        CoreIR::Instance* pipeReg = m.pipelineRegisters[instr][i];
        def->connect(pipeReg->sel("in"), lastReg);
         //lastReg->sel("out"));
        lastReg = pipeReg->sel("out");
        //lastReg = pipeReg;
      }
    }
  }
  return m;
}

void emitCoreIR(StencilInfo& info, CoreIR::Context* context, HWLoopSchedule& sched, CoreIR::ModuleDef* def) {
  assert(sched.II == 1);

  UnitMapping m = createUnitMapping(info, context, sched, def);
  auto& unitMapping = m.unitMapping;
  //auto& instrValues = m.instrValues;
  //auto& stencilRanges = m.stencilRanges;

  cout << "Building connections inside each cycle\n";
  for (int i = 0; i < (int) sched.stages.size(); i++) {

    int stageNo = i;
    auto& instrsInStage = sched.stages[i];
    for (auto instr : instrsInStage) {
      internal_assert(CoreIR::contains_key(instr, unitMapping));
      CoreIR::Instance* unit = CoreIR::map_find(instr, unitMapping);

      if (instr->name == "add" || (instr->name == "mul") || (instr->name == "div") || (instr->name == "sub") || (instr->name == "min") || (instr->name == "max") ||
          (instr->name == "lt") || (instr->name == "gt") || (instr->name == "lte") || (instr->name == "gte") || (instr->name == "and") || (instr->name == "mod") ||
          (instr->name == "eq") || (instr->name == "neq") || (instr->name == "and_bv")) {
        auto arg0 = instr->getOperand(0);
        auto arg1 = instr->getOperand(1);

        def->connect(unit->sel("in0"), m.valueAt(arg0, stageNo));
        def->connect(unit->sel("in1"), m.valueAt(arg1, stageNo));
      } else if (instr->name == "cast" || (instr->name == "absd")) {
        auto arg = instr->getOperand(0);
        //auto unit = CoreIR::map_find(instr, unitMapping);
        def->connect(unit->sel("in"), m.valueAt(arg, stageNo));
      } else if (instr->name == "rd_stream") {
        auto arg = instr->getOperand(0);
        def->connect(unit->sel("in"), m.valueAt(arg, stageNo));
      } else if (instr->name == "stencil_read") {
        auto arg = instr->getOperand(0);

        def->connect(unit->sel("in"), m.valueAt(arg, stageNo));
      } else if (starts_with(instr->name, "create_stencil")) {
        auto srcStencil = instr->getOperand(0);
        auto newVal = instr->getOperand(1);

        def->connect(unit->sel("in_stencil"), m.valueAt(srcStencil, stageNo));
        def->connect(unit->sel("new_val"), m.valueAt(newVal, stageNo));
      } else if (instr->name == "write_stream") {
        auto strm = instr->getOperand(0);
        auto stencil = instr->getOperand(1);

        
        def->connect(unit->sel("stream"), m.valueAt(strm, stageNo));
        def->connect(unit->sel("stencil"), m.valueAt(stencil, stageNo));
        
      } else if (instr->name == "sel") {

        def->connect(unit->sel("sel"), m.valueAt(instr->getOperand(0), stageNo));
        def->connect(unit->sel("in1"), m.valueAt(instr->getOperand(1), stageNo));
        def->connect(unit->sel("in0"), m.valueAt(instr->getOperand(2), stageNo));

      } else if (starts_with(instr->name, "init_stencil")) {
        // No inputs
      } else if (instr->name == "ashr") {

        def->connect(unit->sel("in1"), m.valueAt(instr->getOperand(1), stageNo));
        def->connect(unit->sel("in0"), m.valueAt(instr->getOperand(0), stageNo));
      } else {
        internal_assert(false) << "no wiring procedure for " << *instr << "\n";
      }
    }
  }
}

CoreIR::Module* moduleForKernel(CoreIR::Context* context, StencilInfo& info, HWFunction& f) {
  auto& instrs = f.body;
  vector<std::pair<std::string, CoreIR::Type*> > tps;
  tps = {{"reset", context->BitIn()}, {"in_en", context->BitIn()}, {"valid", context->Bit()}};
  std::set<string> inStreams;
  std::set<string> outStreams;
  for (auto instr : instrs) {
    if (isCall("rd_stream", instr)) {
      inStreams.insert(instr->operands[0]->compactString());
    }

    if (isStreamWrite(instr)) {
      outStreams.insert(instr->operands[0]->compactString());
    }
  }

  for (auto v : f.controlVars) {
    string vName = coreirSanitize(v);
    tps.push_back({vName, context->BitIn()->Arr(16)});
  }

  //cout << "Current stencils..." << endl;
  //cout << stencils << endl;
  cout << "All input streams" << endl;
  for (auto is : inStreams) {
    cout << "\t\t" << is << endl;
    vector<string> dispatchInfo = CoreIR::map_find(is, info.streamDispatches);
    cout << "\tDispatch info..." << endl;
    //vector<int> windowDims = streamWindowDims(is, info);
    vector<int> windowRngs = getStreamDims(is, info);
    vector<int> windowDims = getDimRanges(windowRngs);
    CoreIR::Type* base = context->BitIn()->Arr(16);
    for (auto d : windowDims) {
      base = base->Arr(d);
    }

    string inName = is;
    replaceAll(inName, ".", "_");
    tps.push_back({inName, base});
  }
  cout << "All output streams" << endl;
  for (auto is : outStreams) {
    cout << "\t\t" << is << endl;

    vector<int> rgs = getStreamDims(is, info);
    vector<int> windowDims = getDimRanges(rgs);
    CoreIR::Type* base = context->Bit()->Arr(16);
    for (auto d : windowDims) {
      base = base->Arr(d);
    }

    string inName = is;
    replaceAll(inName, ".", "_");
    tps.push_back({inName, base});
  }

  tps.push_back({"in_en", context->BitIn()});
  tps.push_back({"valid", context->Bit()});
  CoreIR::Type* design_type = context->Record(tps);
 
  // TODO: Build moduledef when the HWFunction is initialized
  // then add modules for pre-bound operations to the hardware
  // and create a "hwinstruction input" function and output function
  // inside HWInstruction that will tell the RTL elaborator how to
  // build an instance of the instruction
  auto global_ns = context->getNamespace("global");
  auto design = global_ns->newModuleDecl(f.name, design_type);
  auto def = design->newModuleDef();
  auto self = def->sel("self");

  HWLoopSchedule sched;
  sched.body = instrs;
  sched.II = 1;
  sched.stages.push_back({});
  for (auto instr : instrs) {
    sched.stages[0].push_back(instr);
    //sched.stages.push_back({instr});
  }

  int nStages = sched.stages.size();
  cout << "Number of stages = " << nStages << endl;
  CoreIR::Wireable* inEn = self->sel("in_en");
  for (int i = 0; i < nStages - 1; i++) {
    //auto vR = pipelineRegister(context, def, "valid_delay_reg_" + std::to_string(i), context->Bit());
    auto vR = def->addInstance("valid_delay_reg_" + std::to_string(i), "corebit.reg");
    def->connect(inEn, vR->sel("in"));
    inEn = vR->sel("out");
  }
  def->connect(inEn, self->sel("valid"));

  cout << "# of stages in loop schedule = " << sched.stages.size() << endl;
  emitCoreIR(info, context, sched, def);

  design->setDef(def);
  return design;
}

bool isLoad(HWInstr* instr) {
  return isCall("load", instr);
}

bool isStore(HWInstr* instr) {
  return isCall("store", instr);
}
bool isConstant(HWInstr* instr) {
  return instr->tp == HWINSTR_TP_CONST;
}

bool operator==(const HWInstr& a, const HWInstr& b) {
  if (a.tp != b.tp) {
    return false;
  }

  if (a.tp == HWINSTR_TP_STR) {
    return a.strConst == b.strConst;
  }

  if (a.tp == HWINSTR_TP_VAR) {
    return a.name == b.name;
  }

  if (a.tp == HWINSTR_TP_CONST) {
    return a.constWidth == b.constWidth && a.constValue == b.constValue;
  }

  if (a.tp == HWINSTR_TP_INSTR) {
    return a.uniqueNum == b.uniqueNum;
  }

  assert(false);
}


void replaceOperand(HWInstr* toReplace, HWInstr* replacement, HWInstr* instr) {
  int i = 0;
  for (auto op : instr->operands) {
    if (*op == *toReplace) {
      instr->operands[i] = replacement;
    }
    i++;
  }
}

void replaceAllUsesWith(HWInstr* toReplace, HWInstr* replacement, vector<HWInstr*>& body) {
  for (auto* instr : body) {
    replaceOperand(toReplace, replacement, instr);
  }
}

void removeBadStores(StoreCollector& storeCollector, vector<HWInstr*>& body) {
  vector<HWInstr*> constLoads;
  std::map<HWInstr*, HWInstr*> loadsToConstants;
  int pos = 0;
  for (auto instr : body) {
    if (isLoad(instr)) {
      auto location = instr->operands[2];
      //cout << "Load " << *instr << " from location: " << location->compactString() << endl;
      if (isConstant(location)) {
        cout << "Getting value for store to " << instr->getOperand(0)->compactString() << ", " << instr->getOperand(1)->compactString() << "[" << location->toInt() << "]" << endl;
        int newValue = map_get(location->toInt(), map_get(instr->getOperand(0)->strConst, storeCollector.constStores));

        cout << "Replacing load from " << instr->getOperand(0)->compactString() << " " << location->compactString() << " with value " << newValue << endl;
        HWInstr* lastStoreToLoc = new HWInstr();
        lastStoreToLoc->tp = HWINSTR_TP_CONST;
        lastStoreToLoc->constWidth = 16;
        lastStoreToLoc->constValue = std::to_string(newValue);
        constLoads.push_back(instr);

        if (lastStoreToLoc) {
          loadsToConstants[instr] = lastStoreToLoc;
        }
      }
    }
  }
  pos++;

  //cout << "# of const loads = " << constLoads.size() << endl;
  for (auto ldNewVal : loadsToConstants) {
    //cout << "Replace " << *(ldNewVal.first) << " with " << ldNewVal.second->compactString() << endl;
    replaceAllUsesWith(ldNewVal.first, ldNewVal.second, body);
  }

  for (auto ldNewVal : loadsToConstants) {
    CoreIR::remove(ldNewVal.first, body);
  }

  CoreIR::delete_if(body, [](HWInstr* instr) { return isStore(instr); });

  cout << "Allocate ROMs..." << endl;
  std::map<std::string, vector<HWInstr*> > romLoads;
  for (auto instr : body) {
    if (isCall("load", instr)) {
      cout << "Found load..." << *instr << endl;
      romLoads[instr->getOperand(0)->compactString()].push_back(instr);
    }
  }

  cout << "All loads..." << endl;
  for (auto m : romLoads) {
    cout << "\tTo rom: " << m.first << endl;
    for (auto ld : m.second) {
      cout << "\t\t" << *ld << endl;
    }
  }

  // Now: Need to add code to HWInstr that will allow use to bind a specific functional unit to it
  // A given instruction needs to have a hardware module associated with it by name, and needs to have
  // a latency (eventually), a port which is the output of the instruction, and an identifier for
  // the module which it will be bound to, and a procedure for wiring up the operands of the
  // instruction to names of ports on the target module
  internal_assert(false) << "Stopping here so dillon can view loads\n";
}

void insert(const int i, HWInstr* instr, vector<HWInstr*>& body) {
  body.insert(std::begin(body) + i, instr);
}

int instructionPosition(HWInstr* instr, vector<HWInstr*>& body) {
  for (int pos = 0; pos < (int) body.size(); pos++) {
    if (body[pos] == instr) {
      return pos;
    }
  }
  return -1;
}
void insertAt(HWInstr* instr, HWInstr* refresh, vector<HWInstr*>& body) {
  int position = instructionPosition(instr, body);
  assert(position >= 0);
  insert(position, refresh, body);
}

void replaceAllUsesAfter(HWInstr* refresh, HWInstr* toReplace, HWInstr* replacement, vector<HWInstr*>& body) {
  int startPos = instructionPosition(refresh, body);
  for (int i = startPos + 1; i < (int) body.size(); i++) {
    replaceOperand(toReplace, replacement, body[i]);
 }
}

vector<HWInstr*> allInstructions(HWFunction& f) {
  return f.body;
}

void removeWriteStreamArgs(StencilInfo& info, HWFunction& f) {
  for (auto instr : allInstructions(f)) {
    if (isCall("write_stream", instr)) {
      instr->operands = {instr->operands[0], instr->operands[1]};
    }
  }
}

void valueConvertStreamReads(StencilInfo& info, HWFunction& f) {
//vector<HWInstr*>& body) {
  auto& body = f.body;
  std::map<HWInstr*, HWInstr*> replacements;
  for (auto instr : body) {
    if (isCall("read_stream", instr)) {
      auto callRep = f.newI();
      //auto callRep = new HWInstr();
      //callRep->uniqueNum = 999;
      callRep->name = "rd_stream";
      callRep->operands = {instr->operands[0]};
      auto targetStencil = instr->operands[1];
      replaceAllUsesWith(targetStencil, callRep, body);
      replacements[instr] = callRep;
    }
  }

  for (auto rp : replacements) {
    insertAt(rp.first, rp.second, body);
  }
  CoreIR::delete_if(body, [replacements](HWInstr* ir) { return CoreIR::contains_key(ir, replacements); });
}

bool allConst(const int start, const int end, vector<HWInstr*>& hwInstr) {
  //for (auto instr : hwInstr) {
  internal_assert(start >= 0);
  internal_assert(end <= (int) hwInstr.size());
  for (int i = start; i < end; i++) {
    if (hwInstr[i]->tp != HWINSTR_TP_CONST) {
      cout << "Instruction " << *(hwInstr[i]) << " is not const!" << endl;
      return false;
    }
  }

  return true;
}

std::set<std::string> streamsThatUseStencil(const std::string& name, StencilInfo& info) {
  std::set<std::string> users;
  for (auto wr : info.streamReadCallRealizations) {
    users.insert(exprString(wr.first->args[0]));
  }

  for (auto wr : info.streamWriteCallRealizations) {
    users.insert(exprString(wr.first->args[0]));
  }
  return users;
}

vector<int> stencilDimsInBody(StencilInfo& info, HWFunction &f, const std::string& stencilName) {
  std::set<std::string> streamUsers = streamsThatUseStencil(stencilName, info);
  std::set<std::string> streamsInF = allStreamNames(f);
  std::set<std::string> streamUsersInF = CoreIR::intersection(streamUsers, streamsInF);
  internal_assert(streamUsersInF.size() > 0) << " no streams that use " << stencilName << " in hardware kernel that contains it\n";
  auto user = *std::begin(streamUsersInF);
  return toInts(map_get(user, info.streamParams));
}

void valueConvertProvides(StencilInfo& info, HWFunction& f) {
  auto& body = f.body;
  std::map<string, vector<HWInstr*> > provides;
  std::map<string, HWInstr*> stencilDecls;
  for (auto instr : body) {
    if (isCall("provide", instr)) {
      string target = instr->operands[0]->compactString();
      provides[target].push_back(instr);
      stencilDecls[target] = instr;
    }
  }

  // What is the right way to optimize this?
  // Find the initialization value of the provides
  // Create an instruction to initialize the provide
  // Replace all references to stencil after a given
  // provide with the result of the new provide call
  std::map<std::string, HWInstr*> initProvides;
  cout << "Provides" << endl;
  for (auto pr : provides) {
    auto provideValue = CoreIR::map_find(pr.first, stencilDecls);
    auto provideName = provideValue->operands[0]->compactString();
    // What to do?
    //vector<int> dims = getStencilDims(provideName, info);

    vector<int> dims = stencilDimsInBody(info, f, provideName);
    vector<HWInstr*> initialSets;
    for (auto instr : pr.second) {
      auto operands = instr->operands;
      if (allConst(1, operands.size(), operands)) {
        initialSets.push_back(instr);
      } else {
        break;
      }
    }

    HWInstr* initInstr = f.newI();
    initInstr->name = "init_stencil_" + pr.first;
    initInstr->operands = {};

    initInstr->operands.push_back(f.newConst(32, dims.size()));
    cout << "Dims of " << provideName << endl;
    for (auto c : dims) {
      cout << "\t" << c << endl;
      initInstr->operands.push_back(f.newConst(32, c));
    }

    for (auto initI : initialSets) {
      for (int i = 1; i < (int) initI->operands.size(); i++) {
        initInstr->operands.push_back(initI->operands[i]);
      }
    }

    insert(0, initInstr, body);
    HWInstr* activeProvide = initInstr;
    replaceAllUsesWith(provideValue->operands[0], activeProvide, body);
    cout << "done with set values..." << endl;
    int provideNum = 0;
    //for (auto instr : pr.second) {
    for (int i = initialSets.size(); i < (int) pr.second.size(); i++) {
      auto instr = pr.second[i];
      cout << "\t\t" << *instr << endl;
      //HWInstr* refresh = new HWInstr();
      auto refresh = f.newI();
      //refresh->uniqueNum = provideNum + 100;
      refresh->operands = instr->operands;
      refresh->name = "create_stencil_" + pr.first + "_" + std::to_string(provideNum);
      insertAt(instr, refresh, body);
      replaceAllUsesAfter(refresh, activeProvide, refresh, body);
      activeProvide = refresh;
      provideNum++;
    }
  }

  for (auto pr : provides) {
    for (auto instr : pr.second) {
      CoreIR::remove(instr, body);
    }
  }
}

std::set<CoreIR::Wireable*> allConnectedWireables(CoreIR::Wireable* w) {
  std::set<CoreIR::Wireable*> allC;
  for (auto c : w->getConnectedWireables()) {
    allC.insert(c);
  }

  for (auto r : w->getSelects()) {
    for (auto wc : allConnectedWireables(r.second)) {
      allC.insert(wc);
    }
  }
  return allC;
}

std::set<CoreIR::Wireable*> allOutputConnections(CoreIR::Wireable* w) {
  auto allConnections = allConnectedWireables(w);

  std::set<CoreIR::Wireable*> outConnections;
  for (auto w : allConnections) {
    CoreIR::Type* tp = w->getType();
    if (tp->hasInput()) {
      outConnections.insert(w);
    }
  }
  return outConnections;
}

CoreIR::Wireable* getBase(CoreIR::Wireable* const w) {
  if (CoreIR::isa<CoreIR::Instance>(w)) {
    return w;
  }

  if (CoreIR::isa<CoreIR::Interface>(w)) {
    return w;
  }

  //cout << "Getting base of " << CoreIR::toString(*w) << endl;

  internal_assert(CoreIR::isa<CoreIR::Select>(w));
  auto s = static_cast<CoreIR::Select*>(w);
  return getBase(s->getParent());
}

CoreIR::Instance* pickNextInstance(CoreIR::ModuleDef* def, std::set<CoreIR::Wireable*>& alreadyDone) {
  std::set<CoreIR::Instance*> instances;
  for (auto d : alreadyDone) {
    CoreIR::Wireable* base = getBase(d);
    if (CoreIR::isa<CoreIR::Instance>(base)) {
      instances.insert(static_cast<CoreIR::Instance*>(base));
    }
  }
  //cout << "Getting next instance" << endl;
  for (auto inst : def->getInstances()) {
    auto instV = inst.second;
    //cout << "Checking instance " << instV->getInstname() << endl;
    if (!CoreIR::elem(instV, instances)) {
      return instV;
    }
  }

  return nullptr;
}

void removeUnusedInstances(CoreIR::ModuleDef* def) {
  bool foundUnused = true;
  while (foundUnused) {
    foundUnused = false;

    //cout << "Finding unused instances" << endl;
    CoreIR::Instance* unused = nullptr;
    for (auto instV : def->getInstances()) {
      auto inst = instV.second;
      //cout << "All connections for " << CoreIR::toString(*inst) << endl;
      //for (auto c : allConnectedWireables(inst)) {
        //cout << "\t" << CoreIR::toString(*c) << endl;
      //}
      auto allOutputs = allOutputConnections(inst);
      //cout << CoreIR::toString(*inst) << " has " << allOutputs.size() << " output connections" << endl;

      if (allOutputs.size() == 0) {
        foundUnused = true;
        unused = inst;
        break;
      }
    }

    if (foundUnused) {
      def->disconnect(unused);
      def->removeInstance(unused);
    }
  }
}

void modToShift(HWFunction& f) {
  std::set<HWInstr*> toErase;
  std::map<HWInstr*, HWInstr*> replacements;
  for (auto instr : f.body) {
    if (isCall("mod", instr)) {
      //cout << "Found mod" << endl;
      if (isConstant(instr->getOperand(1))) {
        //cout << "\tMod by constant = " << instr->getOperand(1)->compactString() << endl;
        auto constVal = instr->getOperand(1)->toInt();
        if (CoreIR::isPower2(constVal)) {
          //cout << "\t\tand it is a power of 2" << endl;
          int value = std::ceil(std::log2(constVal));
          //cout << "\t\tpower of 2 = " << value << endl;
          internal_assert(value > 0);
          // Procedure: and with 0 ^ (1 << (value - 1))
          auto shrInstr = f.newI();
          shrInstr->name = "and_bv";
          shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, 1 << (value - 1))};
          replacements[instr] = shrInstr;
        }
      }
    }
  }

  for (auto r : replacements) {
    insertAt(r.first, r.second, f.body);
    replaceAllUsesWith(r.first, r.second, f.body);
    toErase.insert(r.first);
  }

  for (auto i : toErase) {
    CoreIR::remove(i, f.body);
  }
}
void divToShift(HWFunction& f) {
  std::set<HWInstr*> toErase;
  std::map<HWInstr*, HWInstr*> replacements;
  for (auto instr : f.body) {
    if (isCall("div", instr)) {
      //cout << "Found div" << endl;
      if (isConstant(instr->getOperand(1))) {
        //cout << "\tDividing by constant = " << instr->getOperand(1)->compactString() << endl;
        auto constVal = instr->getOperand(1)->toInt();
        if (CoreIR::isPower2(constVal)) {
          //cout << "\t\tand it is a power of 2" << endl;
          int value = std::ceil(std::log2(constVal));
          //cout << "\t\tpower of 2 = " << value << endl;
          auto shrInstr = f.newI();
          shrInstr->name = "ashr";
          shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, value)};
          replacements[instr] = shrInstr;
        }
      }
    }
  }

  for (auto r : replacements) {
    insertAt(r.first, r.second, f.body);
    replaceAllUsesWith(r.first, r.second, f.body);
    toErase.insert(r.first);
  }

  for (auto i : toErase) {
    CoreIR::remove(i, f.body);
  }
}
void removeUnconnectedInstances(CoreIR::ModuleDef* m) {

  std::vector<std::set<CoreIR::Wireable*> > components;
  std::set<CoreIR::Wireable*> visited;

  while (true) {
    auto instF = pickNextInstance(m, visited);
    if (instF == nullptr) {
      //cout << "Next instance is a nullptr!" << endl;
      break;
    }

    std::set<CoreIR::Wireable*> component;
    std::deque<CoreIR::Wireable*> toVisit{instF};

    //cout << "Removing unconnected wires" << endl;
    while (toVisit.size() > 0) {
      auto val = toVisit.front();
      visited.insert(val);
      //cout << "Next value to get local connections for = " << CoreIR::toString(*(val)) << endl;
      toVisit.pop_front();

      std::set<CoreIR::Wireable*> allConnected =
        allConnectedWireables(val);

      for (auto w : allConnected) {
        //cout << "\t" << CoreIR::toString(*w) << endl;
        if (!CoreIR::elem(getBase(w), visited)) {
          toVisit.push_back(getBase(w));
        }

        component.insert(w);
      }

    }

    //cout << "Inserting component with " << component.size() << " entries" << endl;
    components.push_back(component);

  }

  std::set<CoreIR::Instance*> toDel;
  //cout << "Connected components..." << endl;
  for (auto component : components) {
    //cout << "\tComponent" << endl;
    bool containsInterface = false;
    for (auto w : component) {
      //cout << "\t\t" << CoreIR::toString(*w) << endl;
      auto b = getBase(w);
      if (CoreIR::isa<CoreIR::Interface>(b)) {
        containsInterface = true;
      }
    }

    if (!containsInterface) {
      //cout << "\tNOTE: Unused component: Should be deleted" << endl;
      for (auto w : component) {
        auto b = getBase(w);
        if (CoreIR::isa<CoreIR::Instance>(b)) {
          toDel.insert(static_cast<CoreIR::Instance*>(b));
        }
      }
    }
  }

  for (auto d : toDel) {
    m->disconnect(d);
    m->removeInstance(d);
  }
}

class HWVarExtractor : public IRGraphVisitor {
  public:
    vector<std::string> hwVars;
    std::set<std::string> defined;
    HWFunction* f;

    void visit(const Variable* v) {
      if (starts_with(v->name, "_")) {
        addVar(v->name);
      }
    }

    void addVar(const std::string& name) {
      if (!CoreIR::elem(name, hwVars) && !CoreIR::elem(name, defined)) {
        hwVars.push_back(name);
      }
    }

    //void visit(const LetStmt* l) {
      //addVar(l->name);
      ////hwVars.push_back(l->name);
      //IRGraphVisitor::visit(l);
    //}

    void visit(const For* lp) {
      addVar(lp->name);
      //hwVars.push_back(lp->name);

      IRGraphVisitor::visit(lp);
    }
};

class DefinedVarExtractor : public IRGraphVisitor {
  public:
    std::set<std::string> defined;

    void visit(const LetStmt* let) {
      defined.insert(let->name);
      IRGraphVisitor::visit(let);
    }

    void visit(const Let* let) {
      defined.insert(let->name);
      IRGraphVisitor::visit(let);
    }

    // TODO: Add stencil calls
};

std::set<std::string> getDefinedVars(const For* f) {
  DefinedVarExtractor ex;
  f->accept(&ex);
  return ex.defined;
}

vector<std::string> extractHardwareVars(const For* lp, HWFunction& f) {
  std::set<std::string> vars = getDefinedVars(lp);
  HWVarExtractor ex;
  ex.defined = vars;
  ex.f = &f;
  lp->accept(&ex);
  return ex.hwVars;
}

class KernelControlPath {
  public:
    std::vector<std::string> controlVars;
    CoreIR::Module* m;
};

class ForInfo {
  public:
    std::string name;
    int min;
    int extent;
};

class LoopNestInfo {
  public:
    std::vector<ForInfo> loops;
};

class LoopNestInfoCollector : public IRGraphVisitor {
  public:

    LoopNestInfo info;


    void visit(const For* lp) {
      ForInfo forInfo;
      forInfo.name = lp->name;
      forInfo.min = getConstInt(lp->min);
      forInfo.extent = getConstInt(lp->extent);
      info.loops.push_back(forInfo);

      if (!isInnermostLoop(lp)) {
        IRGraphVisitor::visit(lp);
      }
    }
};

CoreIR::Instance* mkConst(CoreIR::ModuleDef* def, const std::string& name, const int width, const int val) {
  return def->addInstance(name, "coreir.const", {{"width", COREMK(def->getContext(), width)}}, {{"value", COREMK(def->getContext(), BitVector(width, val))}});
}

CoreIR::Wireable* andVals(CoreIR::ModuleDef* def, CoreIR::Wireable* a, CoreIR::Wireable* b) {
  auto c = def->getContext();
  auto ad = def->addInstance("and_all", "corebit.and");
  def->connect(ad->sel("in0"), a);
  def->connect(ad->sel("in1"), b);

  return ad->sel("out");
}

CoreIR::Wireable* andList(CoreIR::ModuleDef* def, const std::vector<CoreIR::Wireable*>& vals) {
  CoreIR::Wireable* val = nullptr;
  if (vals.size() == 0) {
    return def->addInstance("and_all", "corebit.const", {{"value", COREMK(def->getContext(), true)}});
  }

  val = vals[0];
  for (int i = 1; i < ((int) vals.size()) - 1; i++) {
    val = andVals(def, val, vals[i]);
  }
  return val;
}

KernelControlPath controlPathForKernel(CoreIR::Context* c, StencilInfo& info, HWFunction& f, const For* lp) {
  LoopNestInfoCollector cl;
  lp->accept(&cl);
  LoopNestInfo loopInfo = cl.info;
  cout << "# of levels in loop for control path = " << loopInfo.loops.size() << endl;

  KernelControlPath cp;
  std::set<std::string> streamNames = allStreamNames(f);
  auto globalNs = c->getNamespace("global");
  vector<std::pair<std::string, CoreIR::Type*> > tps{{"reset", c->BitIn()}, {"in_en", c->BitIn()}};
  std::set<HWInstr*> vars;
  for (auto instr : f.body) {
    for (auto op : instr->operands) {
      if (op->tp == HWINSTR_TP_VAR) {
        if (!elem(op->name, streamNames)) {
          vars.insert(op);
        }
      }
    }
  }

  for (auto var : vars) {
    int width = 16;
    cp.controlVars.push_back(coreirSanitize(var->compactString()));
    tps.push_back({coreirSanitize(var->compactString()), c->Bit()->Arr(width)});
  }
  CoreIR::Module* controlPath = globalNs->newModuleDecl(f.name + "_control_path", c->Record(tps));

  auto def = controlPath->newModuleDef();


  int width = 16;

  // What is the right way to create this control path?
  // - Counter for each loop index variable
  // - Connect reset to counter reset, and connect the counter out to the control output
  // - Each level in the nest increments when all levels below it are at their max?
  // - Each level resets when it reaches its max? (but maybe this is default counter behavior?)
  std::vector<CoreIR::Wireable*> loopLevelCounters;
  std::vector<CoreIR::Wireable*> levelAtMax;
  std::set<std::string> loopVarNames;
  auto self = def->sel("self");
  for (auto l : loopInfo.loops) {
    int min_value = l.min;
    int max_value = min_value + l.extent - 1;
    int inc_value = 1;

    CoreIR::Values args = {{"width",CoreIR::Const::make(c, width)},
      {"min",CoreIR::Const::make(c, min_value)},
      {"max",CoreIR::Const::make(c, max_value)},
      {"inc",CoreIR::Const::make(c, inc_value)}};

    loopVarNames.insert(l.name);
    string varName = coreirSanitize(l.name);
    CoreIR::Wireable* counter_inst = def->addInstance(varName, "commonlib.counter", args);
    loopLevelCounters.push_back(counter_inst);

    // If this loop variable is actually used in the kernel then connect it to the outside world
    if (self->canSel(varName)) {
      def->connect(counter_inst->sel("out"), self->sel(varName));
    }
    def->connect(counter_inst->sel("reset"), def->sel("self")->sel("reset"));
    //def->connect(counter_inst->sel("en"), def->sel("self")->sel("in_en"));

    auto maxValConst = mkConst(def, varName + "_max_value", width, max_value);
    auto atMax = def->addInstance(varName + "_at_max", "coreir.eq", {{"width", COREMK(c, width)}});
    def->connect(atMax->sel("in0"), maxValConst->sel("out"));
    def->connect(atMax->sel("in1"), counter_inst->sel("out"));

    levelAtMax.push_back(atMax);

  }

  internal_assert(levelAtMax.size() == loopLevelCounters.size());

  cout << "Wiring up counter enables" << endl;

  for (int i = 0; i < ((int) loopLevelCounters.size()) - 1; i++) {
    vector<CoreIR::Wireable*> below;
    for (int j = i + 1; j < (int) loopLevelCounters.size(); j++) {
      below.push_back(levelAtMax[j]->sel("out"));
    }
    CoreIR::Wireable* shouldInc = andList(def, below);
    def->connect(loopLevelCounters[i]->sel("en"), shouldInc);
     //shouldInc->sel("out"));
  }
  def->connect(loopLevelCounters.back()->sel("en"), self->sel("in_en"));

  //int min_value = 0;
  //int max_value = 16;
  //int inc_value = 1;
  //CoreIR::Values args = {{"width",CoreIR::Const::make(c, width)},
    //{"min",CoreIR::Const::make(c, min_value)},
    //{"max",CoreIR::Const::make(c, max_value)},
    //{"inc",CoreIR::Const::make(c, inc_value)}};

  //string varName = "clamped_x___scan_dim_0";
  //string xName = "x_var_counter";
  //CoreIR::Wireable* counter_inst = def->addInstance(xName, "commonlib.counter", args);

  //auto self = def->sel("self");
  //def->connect(counter_inst->sel("reset"), def->sel("self")->sel("reset"));
  //def->connect(counter_inst->sel("en"), def->sel("self")->sel("in_en"));


  //CoreIR::Values argsY = {{"width",CoreIR::Const::make(c, width)},
    //{"min",CoreIR::Const::make(c, min_value)},
    //{"max",CoreIR::Const::make(c, max_value)},
    //{"inc",CoreIR::Const::make(c, inc_value)}};

  //string yName = "y_var_counter";
  //CoreIR::Wireable* y_counter_inst = def->addInstance(yName, "commonlib.counter", argsY);
  //def->connect(y_counter_inst->sel("reset"), def->sel("self")->sel("reset"));
  //def->connect(y_counter_inst->sel("en"), def->sel("self")->sel("in_en"));

  for (auto var : vars) {
    int width = 16;
    if (elem(var->name, loopVarNames)) {

      // Do nothing
      //if (varName == coreirSanitize(var->name) && self->canSel(varName)) {
      //def->connect(counter_inst->sel("out"), self->sel(varName));
    } else {
      auto dummyVal = def->addInstance(coreirSanitize(var->name) + "_dummy_val", "coreir.const", {{"width", COREMK(c, width)}}, {{"value", COREMK(c, BitVector(width, 0))}});
      def->connect(dummyVal->sel("out"), def->sel("self")->sel(coreirSanitize(var->name)));
    }
  }
  
  controlPath->setDef(def);
  
  cp.m = controlPath;
  return cp;
}

CoreIR::Module* createCoreIRForStmt(CoreIR::Context* context,
    Stmt stmt,
    const std::string& name,
    const vector<CoreIR_Argument>& args) {

  int bitwidth = 16;
    cout << "All args" << endl;
    for (auto a : args) {
      cout << "\t" << a.name << endl;
    }
    LetPusher pusher;
    stmt = pusher.mutate(stmt);
    cout << "After let pushing..." << endl;
    cout << stmt << endl;

    LetEraser letEraser;
    stmt = letEraser.mutate(stmt);

    cout << "After let erasure..." << endl;
    cout << stmt << endl;
    //internal_assert(false) << "Stopping here for dillon to view\n";

    StoreCollector stCollector;
    stmt.accept(&stCollector);
    for (auto s : stCollector.stores) {
      cout << "Store to " << s->name << " with value " << s->value << " at " << s->index << endl;
    }

    for (auto s : stCollector.constStores) {
      cout << "Constant stores to " << s.first << endl;
      for (auto sc : s.second) {
        cout << "\t[" << sc.first << "] = " << sc.second << endl;
      }
    }

    //internal_assert(false) << "Stopping here for dillon to view\n";

    cout << "Emitting kernel for " << name << endl;
    cout << "\tStmt is = " << endl;
    cout << stmt << endl;
    NestExtractor extractor;
    stmt.accept(&extractor);

    StencilInfoCollector scl;
    stmt.accept(&scl);

    cout << "----- Stream info" << endl;
    cout << "------------ Stream reads" << endl;
    for (auto streamRead : scl.info.streamReadCalls) {
      cout << "\tReads from " << streamRead.first << endl;
      vector<std::string> newParams = {};
      for (auto c : streamRead.second) {
        cout << "\t\t" << c->args[0] << ", " << c->args[1] << ": has dims: " << scl.info.streamReadCallRealizations[c] << endl;
        newParams = scl.info.streamReadCallRealizations[c];
      }

      if (!CoreIR::contains_key(streamRead.first, scl.info.streamParams)) {
        scl.info.streamParams[streamRead.first] = newParams;
      }

      auto oldParams = scl.info.streamParams[streamRead.first];
      internal_assert(oldParams == newParams);
    }
    
    cout << "------------ Stream writes" << endl;
    for (auto streamRead : scl.info.streamWriteCalls) {
      cout << "\tWrites from " << streamRead.first << endl;
      vector<std::string> newParams = {};
      for (auto c : streamRead.second) {
        cout << "\t\t" << c->args[0] << ", " << c->args[1] << ": has dims: " << scl.info.streamWriteCallRealizations[c] << endl;
        newParams = scl.info.streamWriteCallRealizations[c];
      }

      if (!CoreIR::contains_key(streamRead.first, scl.info.streamParams)) {
        scl.info.streamParams[streamRead.first] = newParams;
      }

      auto oldParams = scl.info.streamParams[streamRead.first];
      internal_assert(oldParams == newParams);
    }

    cout << "-------------- Inferred stream parameters..." << endl;
    for (auto sp : scl.info.streamParams) {
      cout << "\t" << sp.first << " " << sp.second << endl;
    }
    //internal_assert(false) << "Stopping here to let Dillon view stream info\n";
    
    //cout << "Stencil info" << endl;
    StencilInfo info = scl.info;
    //cout << "Dispatches" << endl;
    //for (auto ds : info.streamDispatches) {
      //cout << ds.first << endl;
      //for (auto r : ds.second) {
        //cout << "\t" << r << endl;
      //}
    //}

    //cout << "Stencils" << endl;
    //for (auto ds : info.stencilRealizations) {
      //cout << ds.first << endl;
      //for (auto r : ds.second) {
        //cout << "\t" << r << endl;
      //}
    //}


    // TODO: Connect the different loop kernels using the structure contained
    // in stream in / stream out?
    cout << "\tAll " << extractor.loops.size() << " loops in design..." << endl;
    int kernelN = 0;

    std::map<const For*, CoreIR::Module*> kernelModules;
    //std::map<const For*, CoreIR::Module*> kernelControlPaths;
    std::map<const For*, KernelControlPath> kernelControlPaths;
    std::map<const For*, HWFunction> functions;
    for (const For* lp : extractor.loops) {
      cout << "\t\tLOOP" << endl;
      cout << "Original body.." << endl;
      cout << lp->body << endl;

      HWFunction f = buildHWBody("compute_kernel_" + std::to_string(kernelN), lp);
      auto hwVars = extractHardwareVars(lp, f);
      cout << "All hardware vars.." << endl;
      for (auto hv : hwVars) {
        cout << "\t" << hv << endl;
      }
      f.controlVars = hwVars;
      auto& body = f.body;

      removeBadStores(stCollector, body);
      valueConvertProvides(scl.info, f);
      valueConvertStreamReads(scl.info, f);
      removeWriteStreamArgs(scl.info, f);
      divToShift(f);
      modToShift(f);
      cout << "After stream read conversion..." << endl;
      for (auto instr : body) {
        cout << "\t\t\t" << *instr << endl;
      }

      CoreIR::Module* m = moduleForKernel(context, scl.info, f);
      kernelModules[lp] = m;
      auto cp = controlPathForKernel(context, scl.info, f, lp);
      cout << "Control path is..." << endl;
      cp.m->print();
      kernelControlPaths[lp] = cp;
      functions[lp] = f;

      context->runPasses({"rungenerators", "flatten", "deletedeadinstances"});
      removeUnconnectedInstances(m->getDef());
      removeUnusedInstances(m->getDef());

      cout << "Module after optimization" << endl;
      m->print();

      kernelN++;
    }    

    cout << "Done creating kernels..." << endl;
    uint num_inouts = 0;

    // Keep track of the inputs, output, and taps for this module
    std::vector<std::pair<string, CoreIR::Type*>> input_types;
    std::map<string, CoreIR::Type*> tap_types;
    CoreIR::Type* output_type = context->Bit();

    std::map<std::string, std::string> inputAliases;
    std::map<std::string, std::string> outputAliases;
    string output_name = "";
    string output_name_real = "";
    for (size_t i = 0; i < args.size(); i++) {
      string arg_name = "arg_" + std::to_string(i);
      string arg_name_real = args[i].name;
      cout << "Arg " << i << " has name " << arg_name_real << endl;
      //coreirSanitize(args[i].name);

      inputAliases[arg_name_real] = arg_name;

      if (args[i].is_stencil) {
        CodeGen_CoreIR_Base::Stencil_Type stype = args[i].stencil_type;

        vector<uint> indices;
        for(const auto &range : stype.bounds) {
          internal_assert(is_const(range.extent));
          indices.push_back(func_id_const_value(range.extent));
        }

        if (args[i].is_output && args[i].stencil_type.type == CodeGen_CoreIR_Base::Stencil_Type::StencilContainerType::AxiStream) {
          // add as the outputrg
          uint out_bitwidth = c_inst_bitwidth(stype.elemType.bits());
          if (out_bitwidth > 1) { output_type = output_type->Arr(out_bitwidth); }
          for (uint i=0; i<indices.size(); ++i) {
            output_type = output_type->Arr(indices[i]);
          }
          //hw_output_set.insert(arg_name);
          output_name = "out";
          output_name_real = coreirSanitize(args[i].name);

        } else if (!args[i].is_output && args[i].stencil_type.type == CodeGen_CoreIR_Base::Stencil_Type::StencilContainerType::AxiStream) {
          // add another input
          uint in_bitwidth = c_inst_bitwidth(stype.elemType.bits());
          CoreIR::Type* input_type = in_bitwidth > 1 ? context->BitIn()->Arr(in_bitwidth) : context->BitIn();
          for (uint i=0; i<indices.size(); ++i) {
            input_type = input_type->Arr(indices[i]);
          }
          input_types.push_back({arg_name, input_type});

        } else {
          // add another array of taps (configuration changes infrequently)
          uint in_bitwidth = c_inst_bitwidth(stype.elemType.bits());
          CoreIR::Type* tap_type = context->Bit()->Arr(in_bitwidth);
          for (uint i=0; i<indices.size(); ++i) {
            tap_type = tap_type->Arr(indices[i]);
          }
          tap_types[args[i].name] = tap_type;
        }

        num_inouts++;

      } else {
        // add another tap (single value)
        uint in_bitwidth = c_inst_bitwidth(args[i].scalar_type.bits());
        CoreIR::Type* tap_type = context->BitIn()->Arr(in_bitwidth);
        tap_types[arg_name] = tap_type;
      }

    }

    CoreIR::Type* design_type;

    design_type = context->Record({
        {"in", context->Record(input_types)},
        {"reset", context->BitIn()},
        {output_name, output_type},
        {"valid", context->Bit()},
        {"in_en", context->BitIn()}
        });

    CoreIR::Type* topType = design_type;
    auto global_ns = context->getNamespace("global");
    CoreIR::Module* topMod = global_ns->newModuleDecl("DesignTop", topType);
    cout << "Before creating definition.." << endl;
    topMod->print();
    auto def = topMod->newModuleDef();

    std::map<const For*, CoreIR::Instance*> kernels;
    std::map<const For*, CoreIR::Instance*> controlPaths;
    for (auto k : kernelModules) {
      auto kI = def->addInstance("compute_module_" + k.second->getName(), k.second);
      kernels[k.first] = kI;

      KernelControlPath cpM = map_get(k.first, kernelControlPaths);
      auto controlPath = def->addInstance("control_path_module_" + k.second->getName(), cpM.m);
      controlPaths[k.first] = controlPath;
      def->connect(def->sel("self")->sel("reset"), controlPath->sel("reset"));
      for (auto v : cpM.controlVars) {
        auto vn = coreirSanitize(v);
        def->connect(controlPath->sel(vn), kI->sel(vn));
      }

    }

    std::map<string, CoreIR::Instance*> linebufferResults;
    std::map<string, CoreIR::Instance*> linebufferInputs;
    for (auto lb : scl.info.linebuffers) {
      string inName = lb[0];
      string outName = lb[1];


      string lb_name = "lb_" + coreirSanitize(inName) + "_to_" + coreirSanitize(outName);
      vector<int> params;
      for (int i = 2; i < (int) lb.size(); i++) {
        params.push_back(stoi(lb[i]));
      }

      cout << "Linebuffer from " << inName << " to " << outName << " with params: ";
      for (auto p : params) {
        cout << p << ", ";
      }
      cout << endl;

      // Need to create: input_type, output_type, image_type, has_valid (assume true)

      uint num_dims = params.size();
      CoreIR::Type* input_type = context->BitIn()->Arr(bitwidth);
      CoreIR::Type* output_type = context->Bit()->Arr(bitwidth);
      CoreIR::Type* image_type = context->Bit()->Arr(bitwidth);


      vector<int> inRanges = getStreamDims(inName, info);
      vector<int> outRanges = getStreamDims(outName, info);
      uint input_dims [num_dims];
      for (uint i=0; i<num_dims; ++i) {
        input_dims[i] = inRanges[2*i + 1] - inRanges[2*i];
        input_type = input_type->Arr(input_dims[i]);
      }

      uint output_dims [num_dims];
      for (uint i=0; i<num_dims; ++i) {
        output_dims[i] = outRanges[2*i + 1] - outRanges[2*i];
        output_type = output_type->Arr(output_dims[i]);
      }

      uint image_dims [num_dims];
      for (uint i=0; i<num_dims; ++i) {
        image_dims[i] = params[i];
        image_type = image_type->Arr(image_dims[i]);
      }

      CoreIR::Values lb_args = {{"input_type", CoreIR::Const::make(context,input_type)},
        {"output_type", CoreIR::Const::make(context,output_type)},
        {"image_type", CoreIR::Const::make(context,image_type)},
        {"has_valid",CoreIR::Const::make(context,true)}};

      //CoreIR::Instance* coreir_lb = def->addInstance(lb_name, gens["linebuffer"], lb_args);
      CoreIR::Instance* coreir_lb = def->addInstance(lb_name, "commonlib.linebuffer", lb_args);
      def->connect(coreir_lb->sel("reset"), def->sel("self")->sel("reset"));
      linebufferResults[outName] = coreir_lb;
      linebufferInputs[inName] = coreir_lb;
    }

    // Create inter-kernel connections
    // Q: What is a good intermediate representation?
    // A: Nodes (linebuffers or kernels) with a map from names to inputs / outputs
    for (auto in : linebufferInputs) {
      string inName = in.first;
      auto lb = in.second;

      bool foundInput = false;
      for (auto v : args) {
        if (coreirSanitize(v.name) == coreirSanitize(inName)) {

          internal_assert(CoreIR::contains_key(inName, inputAliases));
          string coreirName = CoreIR::map_find(inName, inputAliases);
          cout << "Name for " << inName << " = " << coreirName << endl;
          def->connect(lb->sel("in"), def->sel("self")->sel("in")->sel(coreirName));
          cout << "Connected " << inName << " to linebuffer" << endl;
          def->connect(lb->sel("wen"), def->sel("self")->sel("in_en"));
          foundInput = true;
          break;
        }
      }

      // Could not find an input for this linebuffer in module inputs
      if (!foundInput) {
        for (auto f : functions) {
          for (auto output : outputStreams(f.second)) {
            if (coreirSanitize(output->name) == coreirSanitize(inName)) {
              def->connect(lb->sel("in"), map_find(f.first, kernels)->sel(coreirSanitize(output->name)));
              def->connect(lb->sel("wen"), map_find(f.first, kernels)->sel("valid")); 
              foundInput = true;
              break;
            }
          }

          if (foundInput) {
            break;
          }
        }
      }
      internal_assert(foundInput) << "Could not find input for " << inName << "\n";
    }

    bool foundOut = false;
    // Wire up output
    for (auto f : functions) {
      for (auto out : outputStreams(f.second)) {
        if (coreirSanitize(out->name) == coreirSanitize(output_name_real)) {
          def->connect(map_find(f.first, kernels)->sel(coreirSanitize(out->name)), def->sel("self")->sel(output_name));
          def->connect(map_find(f.first, kernels)->sel("valid"), def->sel("self")->sel("valid"));
          foundOut = true;
          break;
        }
      }
    }
    internal_assert(foundOut) << "Could not find output for " << output_name_real << "\n";

    for (auto f : functions) {
      // Collect a map from input names to output wireables?
      // And together all inputs?
      //
      std::map<CoreIR::Wireable*, CoreIR::Wireable*> inputMap;
      std::vector<CoreIR::Wireable*> allEnables;
      for (auto input : inputStreams(f.second)) {
        cout << "Function " << f.second.name << " has input " << *input << endl;
        CoreIR::Wireable* inPort = map_get(f.first, kernels)->sel(coreirSanitize(input->name));

        if (CoreIR::contains_key(input->name, linebufferResults)) {
          auto lb = linebufferResults[input->name];
          inputMap[inPort] = lb->sel("out");
          allEnables.push_back(lb->sel("valid"));

        } else {
          // The input is a top-level module input?
          bool foundProducer = false;
          for (auto otherF : functions) {
            for (auto output : outputStreams(otherF.second)) {
              if (output->name == input->name) {
                cout << input->name << " is produced by " << otherF.second.name << endl;
                inputMap[inPort] = map_get(otherF.first, kernels)->sel(coreirSanitize(output->name));
                allEnables.push_back(map_get(otherF.first, kernels)->sel("valid"));

                foundProducer = true;
                break;
              }
            }

            if (foundProducer) {
              break;
            }
          }

          if (!foundProducer) {
            // Assume that this must be a top-level input
            auto self = def->sel("self");
            // TODO: Use hardware input map
            cout << "Trying to find default producer for " << CoreIR::toString(*inPort) << " in " << CoreIR::toString(*(self->sel("in"))) << endl;

            auto selTp = self->sel("in")->getType();
            internal_assert(CoreIR::isa<CoreIR::RecordType>(selTp)) << "select self.in must be a record\n";
            auto rtp = static_cast<CoreIR::RecordType*>(selTp);
            auto recs = rtp->getFields();
            internal_assert(recs.size() > 0);

            //internal_assert(self->sel("in")->getSelects().size() > 0) << "no selects in " << CoreIR::toString(*(self->sel("in"))) << "\n";

            auto argSel = self->sel("in")->sel(*begin(recs));

            //auto argSel = std::begin(self->sel("in")->getSelects())->first;
            inputMap[inPort] = argSel;
             //self->sel("in")->sel(argSel);
            //self->sel("in")->sel(argStr);
            allEnables.push_back(self->sel("in_en"));
            //internal_assert(foundProducer) << "Could not find producer for " << input->name << "\n";
          }
        }
      }

      cout << "Done getting function inputs..." << endl;

      // Actually connect sources to enables
      for (auto in : inputMap) {
        def->connect(in.first, in.second);
      }

      cout << "Done connecting inputMap" << endl;
      auto fKernel = map_find(f.first, kernels);
      auto cPaths = map_find(f.first, controlPaths);
      if (allEnables.size() == 0) {
        // Do nothing
        auto c1 = def->addInstance(fKernel->getInstname() + "_const_valid", "corebit.const", {{"value", COREMK(context, true)}});
        def->connect(fKernel->sel("in_en"), c1->sel("out"));
        def->connect(cPaths->sel("in_en"), c1->sel("out"));
      } else if (allEnables.size() == 1) {
        def->connect(allEnables[0], fKernel->sel("in_en"));
        def->connect(allEnables[0], cPaths->sel("in_en"));
      } else {
        auto v0 = allEnables[0];
        for (int i = 1; i < (int) allEnables.size(); i++) {
          auto and0 = def->addInstance("v_and_" + fKernel->getInstname() + "_" + std::to_string(i), "corebit.and");
          def->connect(and0->sel("in0"), v0);
          def->connect(and0->sel("in1"), allEnables[i]);
          v0 = and0->sel("out");
        }

        def->connect(fKernel->sel("in_en"), v0);
        def->connect(cPaths->sel("in_en"), v0);
      }

      cout << "Done setting enables" << endl;
    }

    cout << "Setting definition of topMod..." << endl;

    topMod->setDef(def);


    cout << "Top module" << endl;
    topMod->print();

    if (!saveToFile(global_ns, "conv_3_3_app.json")) {
      cout << "Could not save global namespace" << endl;
      context->die();
    }

    return topMod;
}

//float id_fconst_value(const Expr e) {
  //if (const FloatImm* e_float = e.as<FloatImm>()) {
    //return e_float->value;

  //} else {
    ////internal_error << "invalid constant expr\n";
    //return -1;
  //}
//}

//int get_const_bitwidth(const Expr e) {
  //if (const IntImm* e_int = e.as<IntImm>()) {
    //return e_int->type.bits();
      
  //} else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    //return e_uint->type.bits();

  //} else if (const FloatImm* e_float = e.as<FloatImm>()) {
    //return e_float->type.bits();
    
  //} else {
    //internal_error << "invalid constant expr\n";
    //return -1;
  //}
//}

}
}

