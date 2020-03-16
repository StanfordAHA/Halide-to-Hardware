#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>

#include "CodeGen_CoreHLS.h"
#include "CodeGen_Internal.h"
#include "CodeGen_CoreIR_Target.h"
#include "CoreIR_Libs.h"
#include "Debug.h"
#include "HWBufferUtils.h"
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
#include "lakelib.h"

namespace Halide {
namespace Internal {

using std::ostream;
using std::pair;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::set;
using std::ostringstream;
using std::ofstream;
using std::cout;

using CoreIR::group_unary;
using CoreIR::DirectedGraph;
using CoreIR::ModuleDef;
using CoreIR::vdisc;
using CoreIR::Interface;
using CoreIR::Instance;
using CoreIR::isa;
using CoreIR::Wireable;
using CoreIR::edisc;
using CoreIR::map_find;
using CoreIR::elem;
using CoreIR::contains_key;

vector<int> getStencilDims(const std::string& name, StencilInfo& info);

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

void insert(vector<HWInstr*>& instrs, const int i, HWInstr* instr) {
  instrs.insert(std::begin(instrs) + i, instr);
}

void HWFunction::insert(const int i, HWInstr* instr) {
  blocks[0]->instrs.insert(std::begin(blocks[0]->instrs) + i, instr);
}

int instructionPosition(HWInstr* instr, vector<HWInstr*>& body) {
  for (int pos = 0; pos < (int) body.size(); pos++) {
    if (body[pos] == instr) {
      return pos;
    }
  }
  return -1;
}

void HWFunction::insertAfter(HWInstr* pos, HWInstr* newInstr) {
  for (auto blk : blocks) {
    if (elem(pos, blk->instrs)) {
      int position = instructionPosition(pos, blk->instrs);
      assert(position >= 0);
      Halide::Internal::insert(blk->instrs, position + 1, newInstr);
    }
  }
}
void HWFunction::insertAt(HWInstr* pos, HWInstr* newInstr) {
  for (auto blk : blocks) {
    if (elem(pos, blk->instrs)) {
      int position = instructionPosition(pos, blk->instrs);
      assert(position >= 0);
      Halide::Internal::insert(blk->instrs, position, newInstr);
    }
  }
}

void HWFunction::deleteInstr(HWInstr* instr) {
  for (auto blk : blocks) {
    CoreIR::remove(instr, blk->instrs);
  }

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

void HWFunction::replaceAllUsesWith(HWInstr* toReplace, HWInstr* replacement) {
  for (auto* instr : allInstrs()) {
    replaceOperand(toReplace, replacement, instr);
  }
}

void HWFunction::replaceAllUsesAfter(HWInstr* refresh, HWInstr* toReplace, HWInstr* replacement) {
  for (auto blk : blocks) {
    int startPos = instructionPosition(refresh, blk->instrs);
    for (int i = startPos + 1; i < (int) blk->instrs.size(); i++) {
      replaceOperand(toReplace, replacement, blk->instrs[i]);
    }
  }
}

template<typename T>
std::set<HWInstr*> allInstrs(const std::string& name, const T& program) {
  set<HWInstr*> vars;
  for (auto instr : program) {
    if (instr->name == name) {
      vars.insert(instr);
    }
  }
  return vars;
}

template<typename T>
std::set<HWInstr*> allValuesDefined(const T& program) {
  set<HWInstr*> vars;
  for (auto instr : program) {
    vars.insert(instr);
  }
  return vars;
}

template<typename T>
std::set<HWInstr*> allValuesUsed(const T& program) {
  set<HWInstr*> vars;
  for (auto instr : program) {
    for (auto op : instr->operands) {
      vars.insert(op);
    }
  }
  return vars;
}

template<typename T>
HWInstr* getUser(HWInstr* op, const T& program) {
  for (auto instr : program) {
    for (auto v : instr->operands) {
      if (v == op) {
        return instr;
      }
    }
  }
  return nullptr;
}

template<typename T>
std::set<HWInstr*> allVarsUsed(const T& program) {
  set<HWInstr*> vars;
  for (auto v : allValuesUsed(program)) {
    if (v->tp == HWINSTR_TP_VAR) {
      vars.insert(v);
    }
  }
  return vars;
}

HWInstr* head(const std::vector<HWInstr*>& instrs) {
  internal_assert(instrs.size() > 0);
  return instrs[0];
}

std::vector<std::string> loopNames(const std::vector<HWInstr*>& instrs) {
  vector<string> names;
  for (auto l : head(instrs)->surroundingLoops) {
    names.push_back(l.name);
  }
  return names;
}

int numLoops(const std::vector<HWInstr*>& instrs) {
  internal_assert(instrs.size() > 0);
  return instrs[0]->surroundingLoops.size();
}

std::string coreirSanitize(const std::string& str) {
  string san = "";
  for (auto c : str) {
    if (c == '.') {
      san += "_";
    } else if (c == '\"') {
      san += "_q_";
    } else {
      san += c;
    }
  }
  return san;
}


std::ostream& operator<<(std::ostream& out, const HWInstr& instr) {
  if (instr.surroundingLoops.size() > 0) {
    for (auto lp : instr.surroundingLoops) {
      out << lp.name << " : [" << lp.min << ", " << exprString(simplify(lp.extent + lp.min - 1)) << "] ";
    }
  }
  if (instr.tp == HWINSTR_TP_VAR) {
    out << instr.name;
  } else {
    out << (instr.predicate == nullptr ? "T" : instr.predicate->compactString()) << ": ";
    out << (instr.isSigned() ? "S" : "U") << ": ";
    out << (instr.resType == nullptr ? "<UNK>" : coreStr(instr.resType)) << ": ";
    out << ("%" + std::to_string(instr.uniqueNum)) << " = " << instr.name << "(";
    int opN = 0;
    for (auto op : instr.operands) {
      out << op->compactString();
      if (opN < ((int) instr.operands.size()) - 1) {
        out << ", ";
      }
      opN++;
    }
    out << ");";
  }
  return out;
}

class IBlock {
  public:
    bool entry;
    std::vector<HWInstr*> instrs;

    IBlock(const bool entry_) : entry(entry_) {}
    IBlock() : entry(false) {}

    IBlock(vector<HWInstr*>& instrs_) : entry(false), instrs(instrs_) {}

    bool isEntry() const { return entry; }
};

std::ostream& operator<<(std::ostream& out, const IBlock& blk) {
  if (!blk.isEntry()) {
    out << "--- Blk" << endl;
    for (auto instr : blk.instrs) {
      out << "\t" << *instr << endl;
    }
  } else {
    out << "--- Entry" << endl;
  }
  return out;
}

std::vector<std::string> loopNames(const IBlock& blk) {
  if (blk.isEntry()) {
    return {};
  }
  return loopNames(blk.instrs);
}

HWInstr* lastInstr(const IBlock& b) {
  if (b.isEntry()) {
    return nullptr;
  }
  return b.instrs.back();
}

HWInstr* head(const IBlock& b) {
  if (b.isEntry()) {
    return nullptr;
  }
  return head(b.instrs);
}

bool operator==(const IBlock& b, const IBlock c) {
  return head(b) == head(c);
}

bool operator!=(const IBlock& b, const IBlock c) {
  return !(b == c);
}

bool operator<(const IBlock& b, const IBlock c) {
  return head(b) < head(c);
}

vector<IBlock> getIBlockList(HWFunction& f) {
  auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });
  vector<IBlock> blks{{true}};
  for (auto ig : instrGroups) {
    blks.push_back({ig});
  }
  return blks;
}

set<IBlock> getIBlocks(HWFunction& f) {
  auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });
  set<IBlock> blks{{true}};
  for (auto ig : instrGroups) {
    blks.insert({ig});
  }
  return blks;
}

vector<IBlock> loopHeaders(HWFunction& f) {
  auto blks = getIBlockList(f);
  vector<IBlock> headers;

  set<vector<string> > prefixes;
  for (auto blk : blks){
    if (loopNames(blk).size() > 0 && !elem(loopNames(blk.instrs), prefixes)) {
      headers.push_back(blk);
      prefixes.insert(loopNames(blk.instrs));
    }
  }
  //cout << "# of headers = " << headers.size() << endl;
  return headers;
}

vector<IBlock> loopTails(HWFunction& f) {
  auto blks = getIBlockList(f);
  vector<IBlock> tails;

  set<vector<string> > prefixes;
  CoreIR::reverse(blks);
  for (auto blk : blks) {
    if (loopNames(blk).size() > 0 && !elem(loopNames(blk.instrs), prefixes)) {
      tails.push_back(blk);
      prefixes.insert(loopNames(blk.instrs));
    }
  }
  //cout << "# of tails = " << tails.size() << endl;
  return tails;
}

bool isHeader(const IBlock& blk, HWFunction& f) {
  auto h = loopHeaders(f);
  return elem(blk, h);
}

bool isTail(const IBlock& blk, HWFunction& f) {
  auto t = loopTails(f);
  return elem(blk, t);
}

bool isEntry(const IBlock& blk, HWFunction& f) {
  return blk.isEntry();
}

IBlock loopTail(const IBlock& blk, HWFunction& f) {
  internal_assert(isHeader(blk, f));

  for (auto tailBlock : loopTails(f)) {
    if (loopNames(tailBlock) == loopNames(blk)) {
      return tailBlock;
    }
  }

  internal_assert(false);
  return blk;
}

IBlock nextBlock(const IBlock& blk, HWFunction& f) {
  internal_assert(!isEntry(blk, f));

  auto blks = getIBlockList(f);
  if (blk == blks[0]) {
    return IBlock(true);
  }

  for (int i = 0; i < ((int) blks.size()) - 1; i++) {
    if (blks[i] == blk) {
      return blks[i + 1];
    }
  }

  internal_assert(false);
  return blk;
}

IBlock priorBlock(const IBlock& blk, HWFunction& f) {
  internal_assert(!isEntry(blk, f));

  auto blks = getIBlockList(f);
  if (blk == blks[0]) {
    return IBlock(true);
  }

  for (int i = 1; i < (int) blks.size(); i++) {
    if (blks[i] == blk) {
      return blks[i - 1];
    }
  }

  internal_assert(false);
  return blk;
}

set<IBlock> predecessors(const IBlock& blk, HWFunction& f) {
  if (isEntry(blk, f)) {
    return {};
  }

  if (!isHeader(blk, f)) {
    return {priorBlock(blk, f)};
  }

  internal_assert(isHeader(blk, f));
  internal_assert(!isEntry(blk, f));

  return {priorBlock(blk, f), loopTail(blk, f)};
}

std::map<IBlock, std::set<IBlock> > blockDominators(HWFunction& f) {
  auto blocks = getIBlockList(f);
  std::map<IBlock, std::set<IBlock> > dominators;
  for (size_t i = 0; i < blocks.size(); i++) {
    dominators[blocks[i]] = {};
    for (size_t j = 0; j <= i; j++) {
      dominators[blocks[i]].insert(blocks[j]);
    }
  }

  return dominators;
}

IBlock idom(const IBlock& blk, HWFunction& f) {
  return priorBlock(blk, f);
}

namespace {


bool fromGenerator(const std::string& genName, Instance* inst) {
  if (!inst->getModuleRef()->isGenerated()) {
    return false;
  }

  auto gen = inst->getModuleRef()->getGenerator();
  return gen->getRefName() == genName;
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

bool isComputeKernel(CoreIR::Wireable* v) {
  auto b = getBase(v);
  if (!isa<Instance>(b)) {
    return false;
  }

  // Simplify
  return !fromGenerator("lakelib.linebuffer", static_cast<Instance*>(b)) &&
    !fromGenerator("halidehw.shift_register", static_cast<Instance*>(b)) &&
    !fromGenerator("halidehw.stream_trimmer", static_cast<Instance*>(b));
}

bool isTrimmer(Wireable* destBase) {
  if (isa<Instance>(destBase)) {
    auto instBase = static_cast<CoreIR::Instance*>(destBase);
    return fromGenerator("halidehw.stream_trimmer", instBase);
  }

  return false;
}

bool isLinebuffer(Wireable* destBase) {
  if (isa<Instance>(destBase)) {
    auto instBase = static_cast<CoreIR::Instance*>(destBase);
    return fromGenerator("lakelib.linebuffer", instBase);
  }

  return false;
}

class ContainsCall : public IRGraphVisitor {
  public:

    using IRGraphVisitor::visit;

    std::set<const Call*> possible;
    bool found;

    ContainsCall() : found(false) {}

    void visit(const Call* c) {
      if (elem(c, possible)) {
        found = true;
      }
      IRGraphVisitor::visit(c);
    }
};

class CallRemover : public IRMutator {
  public:
    using IRMutator::visit;

    std::set<const Call*> toErase;

    Stmt visit(const Evaluate* e) override {
      Expr b = e->value;
      ContainsCall cc;
      cc.possible = toErase;
      e->accept(&cc);
      if (cc.found) {
        return Evaluate::make(0);
        //internal_assert(false);
      }
      return IRMutator::visit(e);
    }

};

class HWVarExtractor : public IRGraphVisitor {
  public:
    vector<std::string> hwVars;
    std::set<std::string> defined;
  protected:

    using IRGraphVisitor::visit;

    void visit(const Variable* v) override {
      if (starts_with(v->name, "_")) {
        addVar(v->name);
      }
    }

    void addVar(const std::string& name) {
      if (!CoreIR::elem(name, hwVars) && !CoreIR::elem(name, defined)) {
        hwVars.push_back(name);
      }
    }

    void visit(const For* lp) override {
      //addVar(lp->name);

      IRGraphVisitor::visit(lp);
    }
};

class DefinedVarExtractor : public IRGraphVisitor {
  public:
    std::set<std::string> defined;

  protected:
    using IRGraphVisitor::visit;
    void visit(const LetStmt* let) override {
      defined.insert(let->name);
      IRGraphVisitor::visit(let);
    }

    void visit(const Let* let) override {
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

std::set<std::string> getDefinedVars(const Stmt& f) {
  DefinedVarExtractor ex;
  f.accept(&ex);
  return ex.defined;
}

vector<std::string> extractHardwareVars(const Stmt& stmt) {
  std::set<std::string> vars = getDefinedVars(stmt);
  HWVarExtractor ex;
  ex.defined = vars;
  stmt.accept(&ex);
  return ex.hwVars;
}

vector<std::string> extractHardwareVars(const For* lp) {
  std::set<std::string> vars = getDefinedVars(lp);
  HWVarExtractor ex;
  ex.defined = vars;
  //ex.f = &f;
  lp->accept(&ex);
  return ex.hwVars;
}


  int func_id_const_value(const Expr e) {
    if (const IntImm* e_int = e.as<IntImm>()) {
      return e_int->value;

    } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
      return e_uint->value;

    } else {
      return -1;
    }
  }
  
template<typename K>
bool subset(K& a, K& b) {
  return CoreIR::intersection(a, b).size() == a.size();
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

  protected:
    using IRGraphVisitor::visit;
    void visit(const Store* st) override {
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
  for (auto instr : f.allInstrs()) {
    if (isCall("write_stream", instr)) {
      ins.push_back(instr->getOperand(0));
    }
  }

  return ins;
}

std::vector<HWInstr*> inputStreams(HWFunction& f) {
  vector<HWInstr*> ins;
  for (auto instr : f.allInstrs()) {
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

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::set<T>& strs) {
  out << "{";
  for (auto str : strs) {
    out << str << ", ";
  }
  out << "}";
  return out;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& strs) {
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

//class ContainForLoop : public IRVisitor {

  //protected:
    //using IRVisitor::visit;
    //void visit(const For *op) override {
      //found = true;
      //varnames.push_back(op->name);
    //}

//public:
  //bool found;
  //vector<string> varnames;
  //ContainForLoop() : found(false) {}
//};

//// identifies for loops in code statement
//bool contain_for_loop(Stmt s) {
  //ContainForLoop cfl;
  //s.accept(&cfl);
  //return cfl.found;
//}

//// Identifies for loop name in code statement.
////  Gives name of first for loop
//string name_for_loop(Stmt s) {
  //ContainForLoop cfl;
  //s.accept(&cfl);
  //return cfl.varnames[0];
//}

//// Identifies all for loop names in code statement.
//vector<string> contained_for_loop_names(Stmt s) {
  //ContainForLoop cfl;
  //s.accept(&cfl);
  //return cfl.varnames;
//}


//class UsesVariable : public IRVisitor {
  //using IRVisitor::visit;
  //void visit(const Variable *op) override {
    //if (op->name == varname) {
      //used = true;
    //}
    //return;
  //}

  //void visit(const Call *op) override {
    //// only go first two variables, not loop bound checks
    //if (op->name == "write_stream" && op->args.size() > 2) {
      //op->args[0].accept(this);
      //op->args[1].accept(this);
    //} else {
      //IRVisitor::visit(op);
    //}
  //}

//public:
  //bool used;
  //string varname;
  //UsesVariable(string varname) : used(false), varname(varname) {}
//};

//// identifies target variable string in code statement
//bool variable_used(Stmt s, string varname) {
  //UsesVariable uv(varname);
  //s.accept(&uv);
  //return uv.used;
//}

//class ROMInit : public IRVisitor {
  //using IRVisitor::visit;

  //bool is_const(const Expr e) {
    //if (e.as<IntImm>() || e.as<UIntImm>()) {
      //return true;
    //} else {
      //return false;
    //}
  //}

  //int id_const_value(const Expr e) {
    //if (const IntImm* e_int = e.as<IntImm>()) {
      //return e_int->value;

    //} else if (const UIntImm* e_uint = e.as<UIntImm>()) {
      //return e_uint->value;

    //} else {
      //return -1;
    //}
  //}
  
  //void visit(const Store *op) override {
    //if (op->name == allocname) {
      //auto value_expr = op->value;
      //auto index_expr = op->index;
      //internal_assert(is_const(value_expr) && is_const(index_expr));

      //int index = id_const_value(index_expr);
      //int value = id_const_value(value_expr);
      //init_values["init"][index] = value;
      ////init_values["init"][index] = std::to_string(value);
      ////init_values["init"].emplace_back(std::to_string(value));
    //}
  //}

//public:
  //nlohmann::json init_values;
  //string allocname;
  //ROMInit(string allocname) : allocname(allocname) {}
//};

//// returns a map with all the initialization values for a rom
//nlohmann::json rom_init(Stmt s, string allocname) {
  //ROMInit rom_init(allocname);
  //s.accept(&rom_init);
  //return rom_init.init_values;
//}

  

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

  protected:
    using IRGraphVisitor::visit;
    void visit(const For* l) override {
      loops.push_back(l);
      //if (inFor) {
        //return;
      //}

      //loops.push_back(l);
      //inFor = true;

      //l->min.accept(this);
      //l->extent.accept(this);
      //l->body.accept(this);

      //inFor = false;
    }
};

// Now: Schedule and collect the output
// Q: What is the schedule going to look like?
// A: List of stages containing instructions + II
//    And: Resource mapping for each operation
class HWLoopSchedule {
  public:
    vector<HWInstr*> body;
    //int II;

    std::map<HWInstr*, int> endStages;
    std::map<HWInstr*, int> startStages;

    void print() {
      cout << "Schedule" << endl;
      for (int i = 0; i < numStages(); i++) {
        cout << "Stage " << i << endl;
        for (auto instr : instructionsStartingInStage(i)) {
          cout << "\tstart: " << *instr << endl;
        }
        for (auto instr : instructionsEndingInStage(i)) {
          cout << "\tend  : " << *instr << endl;
        }
      }
    }

    bool isScheduled(HWInstr* instr) const {
      return contains_key(instr, endStages) && contains_key(instr, startStages);
    }

    std::set<HWInstr*> instructionsStartingInStage(const int stage) const {
      std::set<HWInstr*> instrs;
      for (auto instr : body) {
        if (getStartTime(instr) == stage) {
          instrs.insert(instr);
        }
      }
      return instrs;
      //std::set<HWInstr*> instr;
      //for (auto i : startStages) {
        //if (i.second == stage) {
          //instr.insert(i.first);
        //}
      //}
      //return instr;
    }

    std::set<HWInstr*> instructionsEndingInStage(const int stage) const {
      std::set<HWInstr*> instrs;
      for (auto instr : body) {
        if (getEndTime(instr) == stage) {
          instrs.insert(instr);
        }
      }
      return instrs;
      //std::set<HWInstr*> instr;
      //for (auto i : endStages) {
        //if (i.second == stage) {
          //instr.insert(i.first);
        //}
      //}
      //return instr;
    }

    int numStages() const {
      int nStages = 0;
      for (auto es : endStages) {
        if (es.second >= nStages) {
          nStages = es.second;
        }
      }
      return nStages + 1;
    }

    int getEndTime(HWInstr* instr) const {
      // Variables and constants are always scheduled at the start of a design
      if (instr->tp != HWINSTR_TP_INSTR) {
        return 0;
      }
      internal_assert(isScheduled(instr)) << " getting end time of unscheduled instruction: " << *instr << "\n";
      return map_get(instr, endStages);
    }

    int getStartTime(HWInstr* instr) const {
      internal_assert(isScheduled(instr)) << " getting start time of unscheduled instruction: " << *instr << "\n";
      return map_get(instr, startStages);
    }

    void setStartTime(HWInstr* instr, const int stage) {
      startStages[instr] = stage;
    }

    void setEndTime(HWInstr* instr, const int stage) {
      endStages[instr] = stage;
    }

    int cycleLatency() const {
      return numStages() - 1;
    }
};

class InstructionCollector : public IRGraphVisitor {
  public:
    using IRGraphVisitor::visit;
    std::map<std::string, HWInstr*> vars;
    HWFunction f;
    HWInstr* lastValue;
    HWInstr* currentPredicate;
    HWBlock* activeBlock;

    std::vector<LoopSpec> activeLoops;
    Scope<std::vector<std::string> > activeRealizations;
    
    InstructionCollector() : lastValue(nullptr), currentPredicate(nullptr), activeBlock(nullptr) {}

    HWInstr* newI() {
      auto ist = f.newI();
      ist->predicate = currentPredicate;
      ist->surroundingLoops = activeLoops;
      return ist;
    }

    void pushInstr(HWInstr* instr) {
      if (activeBlock == nullptr) {
        internal_assert(false);
      } else {
        activeBlock->instrs.push_back(instr);
      }
    }

  protected:

    HWInstr* newBr() {
      auto i = f.newI();
      i->name = "br";
      return i;
    }

    void visit(const For* lp) override {

      activeLoops.push_back({lp->name, lp->min, lp->extent});
      //auto toLoop = newBr();
      //auto fromLoop = newBr();
      
      //pushInstr(toLoop);

      //auto loopBlk = f.newBlk();
      //activeBlock = loopBlk;
      IRGraphVisitor::visit(lp);
     
      //pushInstr(fromLoop);
      
      activeLoops.pop_back();
      //auto nextBlk = f.newBlk();
      
      //toLoop->operands.push_back(f.newVar(loopBlk->name));
      
      //fromLoop->operands.push_back(f.newVar(loopBlk->name));
      //fromLoop->operands.push_back(f.newVar(nextBlk->name));


      //activeBlock = nextBlk;
      //internal_assert(false) << "code generation assumes the loop nest for each kernel is perfect already, but we encountered a for loop: " << lp->name << "\n";
    }
    
    void visit(const Realize* op) override {
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
      return f.newVar(name);
      //auto nI = newI();
      //nI->tp = HWINSTR_TP_VAR;
      //nI->name = name;
      //return nI;
    }

    void visit(const UIntImm* imm) override {
      auto ist = newI();
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = 16;
      ist->constValue = std::to_string(imm->value);
      ist->setSigned(false);
      lastValue = ist;
    }

    void visit(const IntImm* imm) override {
      auto ist = newI();
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = 16;
      ist->constValue = std::to_string(imm->value);
      ist->setSigned(true);
      ist->resType = f.mod->getContext()->Bit()->Arr(16);
      lastValue = ist;
    }

    void visit(const Cast* c) override {
      auto ist = newI();
      ist->name = "cast";
      auto operand = codegen(c->value);
      ist->operands = {operand};
      ist->setSigned(!(c->type.is_uint()));
      ist->resType = f.mod->getContext()->Bit()->Arr(16);
      pushInstr(ist);
      lastValue = ist;
    }

    void visit(const FloatImm* imm) override {
      auto ist = newI();
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = 16;
      ist->constValue = std::to_string(imm->value);
      lastValue = ist;
    }

    void visit(const StringImm* imm) override {
      auto ist = newI();
      ist->tp = HWINSTR_TP_STR;
      ist->strConst = imm->value;
      lastValue = ist;
    }

    void visit(const Provide* p) override {

      vector<HWInstr*> operands;
      auto nameConst = f.newVar(p->name);
      //internal_assert(nameConst->resType != nullptr) << nameConst->compactString() << " has null result type\n";
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

    void visit(const Ramp* r) override {

      IRGraphVisitor::visit(r);

      auto ist = newI();
      ist->name = "ramp";
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const Variable* v) override {

      if (CoreIR::contains_key(v->name, vars)) {
        lastValue = CoreIR::map_find(v->name, vars);
        return;
      }

      //cout << "Creating hwinstruction variable " << v->name << " that is not currently in vars" << endl;
      IRGraphVisitor::visit(v);
      auto ist = f.newVar(v->name);

      //ist->name = v->name;
      //ist->tp = HWINSTR_TP_VAR;
      ist->setSigned(!(v->type.is_uint()));
      vars[v->name] = ist;

      lastValue = ist;
    }


    void visit(const GE* a) override {
      visit_binop("gte", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }

    void visit(const LE* a) override {
      visit_binop("lte", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }
    
    void visit(const LT* a) override {
      visit_binop("lt", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }

    void visit(const GT* a) override {
      visit_binop("gt", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }
    
    void visit(const And* a) override {
      visit_binop("and", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }

    HWInstr* codegen(const Expr e) {
      lastValue = nullptr;
      e.accept(this);

      internal_assert(lastValue) << "Codegen did not produce an LLVM value for " << e << "\n";
      return lastValue;
    }

    void visit(const Select* sel) override {
      auto c = codegen(sel->condition);
      auto tv = codegen(sel->true_value);
      auto fv = codegen(sel->false_value);
      auto ist = newI();
      ist->name = "sel";
      ist->operands = {c, tv, fv};
      lastValue = ist;
      internal_assert(tv->resType != nullptr) << *tv << " has null result type\n";
      lastValue->resType = tv->resType;
      pushInstr(lastValue);
      //instrs.push_back(lastValue);
    }

    void visit_binop(const std::string& name, const Expr a, const Expr b) {
      auto aV = codegen(a);
      auto bV = codegen(b);
      auto ist = newI();
      ist->name = name;
      ist->operands = {aV, bV};
      if (a.type().is_uint() && b.type().is_uint()) {
        ist->setSigned(true);
      } else {
        ist->setSigned(false);
      }
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const LetStmt* l) override {

      auto ev = codegen(l->value);

      vars[l->name] = ev;

      l->body->accept(this);
      //IRGraphVisitor::visit(l->body);
      //auto lv = codegen(l->body);
      //internal_assert(lv) << "let body did not produce a value\n";
      //lastValue = lv;

      //vars.erase(l->name);
      
    }
    void visit(const Let* l) override {

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

    void visit(const Load* ld) override {
      auto ist = newI();
      ist->name = "load";
      vector<HWInstr*> operands;
      operands.push_back(strConstI(ld->name));
      operands.push_back(codegen(ld->predicate));
      operands.push_back(codegen(ld->index));
      ist->operands = operands;
      ist->resType = f.mod->getContext()->Bit()->Arr(16);
      pushInstr(ist);
      //instrs.push_back(ist);
      lastValue = ist;
    }

    void visit(const Store* st) override {
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

    void visit(const Min* m) override {
      visit_binop("min", m->a, m->b);
      lastValue->setSigned(!(m->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }
    
    void visit(const Max* m) override {
      visit_binop("max", m->a, m->b);
      lastValue->setSigned(!(m->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }

    void visit(const Mod* d) override {
      visit_binop("mod", d->a, d->b);
      lastValue->setSigned(!(d->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }

    void visit(const Div* d) override {
      visit_binop("div", d->a, d->b);
      lastValue->setSigned(!(d->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }

    void visit(const Add* a) override {
      visit_binop("add", a->a, a->b);
      lastValue->setSigned(!(a->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }

    void visit(const EQ* a) override {
      visit_binop("eq", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }
    
    void visit(const NE* a) override {
      visit_binop("neq", a->a, a->b);
      lastValue->resType = f.mod->getContext()->Bit();
    }
    
    void visit(const Mul* b) override {
      visit_binop("mul", b->a, b->b);
      lastValue->setSigned(!(b->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }

    void visit(const Sub* b) override {
      visit_binop("sub", b->a, b->b);
      lastValue->setSigned(!(b->type.is_uint()));
      lastValue->resType = f.mod->getContext()->Bit()->Arr(16);
    }

    HWInstr* andHW(HWInstr* a, HWInstr* b) {
      auto andOp = newI();
      andOp->name = "and";
      andOp->operands = {a, b};
      lastValue->resType = f.mod->getContext()->Bit();
      pushInstr(andOp);
      //instrs.push_back(andOp);
      return andOp;
    }
    
    void visit(const Call* op) override {
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
        ist->resType = f.mod->getContext()->Bit()->Arr(16);
      } else if (op->name == "abs") {
        ist->name = "abs";
        ist->resType = f.mod->getContext()->Bit()->Arr(16);
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
        ist->setSigned(!(op->type.is_uint()));

      } else if (ends_with(op->name, ".stencil")) {
        ist->name = "stencil_read";
        auto calledStencil = op->name;
        auto callOp = f.newVar(op->name);
        //auto callOp = newI();
        //callOp->tp = HWINSTR_TP_VAR;
        callOp->name = calledStencil;
        //callOp->setSigned(!(op->type.is_uint()));
        //cout << "Read from: " << op->name << " has signed result ? " << callOp->isSigned() << endl;
       
        ist->setSigned(!(op->type.is_uint()));
        ist->resType = f.mod->getContext()->Bit()->Arr(16);
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

CoreIR::Type* moduleTypeForKernel(CoreIR::Context* context,
    HardwareInfo& hwInfo,
    StencilInfo& info, const Stmt& stmt, const vector<CoreIR_Argument>& args);

void modToShift(HWFunction& f);
void divToShift(HWFunction& f);
void removeBadStores(StoreCollector& st, HWFunction& f);

class HWTransition {
  public:
    HWInstr* srcBlk;
    HWInstr* dstBlk;
    int delay;
};

class NestSchedule {
  public:
    string name;
    int II;
    int L;
    int TC;

    int completionTime() const {
      return (II*(TC - 1)) + L;
    }
};

class FunctionSchedule {
  public:
    HWFunction* f;
    map<HWInstr*, HWLoopSchedule> blockSchedules;

    std::vector<NestSchedule> nestSchedules;
    std::vector<HWTransition> transitions;

    HWLoopSchedule& getContainerBlock(HWInstr* const sourceLocation) {
      return getScheduleFor(sourceLocation);
    }

    HWLoopSchedule& getScheduleFor(HWInstr* const sourceLocation) {
      for (auto& blkS : blockSchedules) {
        if (elem(sourceLocation, blkS.second.body)) {
          return blkS.second;
        }
      }
      internal_assert(false) << "No container for " << *sourceLocation << "\n";
      return begin(blockSchedules)->second;
    }

    int getStartStage(HWInstr* instr) {
      return getContainerBlock(instr).getStartTime(instr);
    }
    
    int getEndStage(HWInstr* instr) {
      return getContainerBlock(instr).getEndTime(instr);
    }

    // API for special case where the entire function is one basic block
    std::set<HWInstr*> instructionsStartingInStage(const int stage) {
      return onlySched().instructionsStartingInStage(stage);
    }

    std::set<HWInstr*> instructionsEndingInStage(const int stage) {
      return onlySched().instructionsEndingInStage(stage);
    }

    HWLoopSchedule& onlySched() {
      internal_assert(blockSchedules.size() == 1);
      return begin(blockSchedules)->second;
    }

    std::vector<HWInstr*> body() {
      return f->structuredOrder();
      //return onlySched().body;
    }

    
    int cycleLatency() {
      if (blockSchedules.size() == 0) {
        return 0;
      }
      return onlySched().cycleLatency();
    }

    int numStages() {
      return onlySched().numStages();
    }

    int getStartTime(HWInstr* instr) {
      return onlySched().getStartTime(instr);
    }

    int getEndTime(HWInstr* instr) {
      return onlySched().getEndTime(instr);
    }

};

IBlock containerBlock(HWInstr* instr, HWFunction& f) {
  for (auto b : getIBlocks(f)) {
    if (elem(instr, b.instrs)) {
      return b;
    }
  }

  internal_assert(false);
  return *(begin(getIBlocks(f)));
}

class ComputeKernel {
  public:
    CoreIR::Module* mod;
    FunctionSchedule sched;
};

std::ostream& operator<<(std::ostream& out, const HWFunction& f) {
  out << "@" << f.name << "(";
  for (auto v : f.controlVars) {
    out << v << ", ";
  }
  out << ")\n";
  for (auto blk : f.getBlocks()) {
    out << "--- Blk " << blk->name << endl;
    for (auto instr : blk->instrs) {
      out << "\t" << *instr << endl;
    }
  }
  return out;
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


  protected:
    using IRGraphVisitor::visit;
    void visit(const For* lp) override {
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

KernelControlPath controlPathForKernel(HWFunction& f);

void valueConvertStreamReads(StencilInfo& info, HWFunction& f);
void valueConvertProvides(StencilInfo& info, HWFunction& f);

void removeWriteStreamArgs(StencilInfo& info, HWFunction& f);
void addDynamicStencilReads(HWFunction& f) {
  for (auto instr : f.structuredOrder()) {
    if (instr->name == "stencil_read") {
      bool allConstant = true;
      for (size_t i = 1; i < instr->operands.size(); i++) {
        if (instr->getOperand(i)->tp != HWINSTR_TP_CONST) {
          allConstant = false;
          break;
        }
      }

      if (!allConstant) {
        instr->name = "dynamic_stencil_read";
      }
    }
  }
}

HWFunction buildHWBody(CoreIR::Context* context,
    HardwareInfo& hwInfo,
    StencilInfo& info, const std::string& name, const Stmt& perfectNest, const vector<CoreIR_Argument>& args, StoreCollector& stCollector) {

  InstructionCollector collector;
  collector.activeBlock = *std::begin(collector.f.getBlocks());
  collector.f.name = name;
  
  auto design_type = moduleTypeForKernel(context, hwInfo, info, perfectNest, args);
  //cout << "Module Type: " << coreStr(design_type) << endl;
  auto global_ns = context->getNamespace("global");
  auto design = global_ns->newModuleDecl(collector.f.name, design_type);
  auto def = design->newModuleDef();
  design->setDef(def);
  collector.f.mod = design;
  perfectNest.accept(&collector);

  auto f = collector.f;

  auto hwVars = extractHardwareVars(perfectNest);
  for (auto arg : args) {
    if (!arg.is_stencil) {
      hwVars.push_back(coreirSanitize(arg.name));
    }
  }

  //cout << "All hardware vars.." << endl;
  //for (auto hv : hwVars) {
    //cout << "\t" << hv << endl;
  //}

  f.controlVars = hwVars;

  //cout << "Before opts..." << endl;
  //cout << f << endl;

  for (auto instr : f.structuredOrder()) {
    for (auto op : instr->operands) {
      if (op->tp == HWINSTR_TP_VAR) {
        if (op->resType == nullptr) {
          if (ends_with(op->name, ".stencil")) {
            //cout << op->compactString() << " has null type\n";
            vector<int> dims = getStencilDims(op->name, info);
            vector<int> sizes = getDimRanges(dims);
            CoreIR::Type* tp = f.mod->getContext()->Bit()->Arr(16);
            for (auto d : sizes) {
              tp = tp->Arr(d);
            }
            op->resType = tp;
          }
        }
      }
    }
  }
  removeBadStores(stCollector, f);
  valueConvertStreamReads(info, f);
  //cout << "After valueconver stream reads..." << endl;
  //cout << f << endl;
  if (hwInfo.interfacePolicy == HW_INTERFACE_POLICY_TOP) {
    valueConvertProvides(info, f);
  }
  removeWriteStreamArgs(info, f);
  divToShift(f);
  modToShift(f);
  addDynamicStencilReads(f);
  return f;
}

class StencilInfoCollector : public IRGraphVisitor {
  public:

    StencilInfo info;
    //vector<DispatchInfo> dispatches;
    Scope<std::vector<std::string> > activeRealizations;

    // So when a realization happens

  protected:
    using IRGraphVisitor::visit;
    void visit(const Call* op) override {
      if (op->name == "dispatch_stream") {

        vector<string> dinfo;
        for (int i = 1; i  < (int) op->args.size(); i++) {
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
      } else if (op->name == "hwbuffer") {
        info.hwbuffers.push_back({});
        for (auto arg : op->args) {
          info.hwbuffers.back().push_back(exprString(arg));
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

    void visit(const Realize* op)  override {
      if (ends_with(op->name, ".stream")) {
        //auto tps = op->types[0];
        //auto bnds = op->bounds;
        //cout << "Realizing " << op->name << " with type = " << tps << endl;
        //cout << "and bounds..." << endl;
        //for (auto bnd : bnds) {
          //cout << "\t" << bnd.min << " with extend: " << bnd.extent << endl;
        //}

        IRGraphVisitor::visit(op);

      } else if (ends_with(op->name, ".stencil")) {

        //if (CoreIR::contains_key(op->name, info.stencilRealizations)) {
          //IRGraphVisitor::visit(op);
          //return;
        //}

        
        //internal_assert(!CoreIR::contains_key(op->name, info.stencilRealizations)) << "Realizations already contains an entry for " << op->name << "\n";
        //
        info.stencilRealizations[op->name] = {};
        //auto tps = op->types[0];
        auto bnds = op->bounds;
        //cout << "Realizing " << op->name << " with type = " << tps << endl;
        //cout << "and bounds..." << endl;
        for (auto bnd : bnds) {
          //cout << "\t" << bnd.min << " with extend: " << bnd.extent << endl;
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
    for (int i = 0; i < (int) (rz.size() / 2); i++) {
      string min = rz[2*i];
      string ext = rz[2*i + 1];
      dims.push_back(stoi(min));
      dims.push_back(stoi(ext));

    }
    return dims;
  }

  internal_assert(false) << "No stencil dimensions for " << name << "\n";
  return {};
}


vector<int> getStreamDims(const std::string& str, StencilInfo& info) {
  //internal_assert(CoreIR::contains_key(str, info.streamParams)) << " error: no stream named " << str << " in stream params\n";
  //<< info.streamParams << "\n";
  if (CoreIR::contains_key(str, info.streamParams)) {
    return toInts(CoreIR::map_find(str, info.streamParams));
  }

  // Assume this is a top-level input
  return {0, 1, 0, 1};
}

class UnitMapping {
  public:

    std::map<HWInstr*, std::map<HWInstr*, CoreIR::Wireable*> > hwStartValues;
    std::map<HWInstr*, std::map<HWInstr*, CoreIR::Wireable*> > hwEndValues;

    FunctionSchedule fSched;

    std::map<HWInstr*, CoreIR::Wireable*> instrValues;
    std::map<HWInstr*, vector<int> > stencilRanges;
    std::map<HWInstr*, CoreIR::Instance*> unitMapping;

    std::map<HWInstr*, std::map<int, CoreIR::Instance*> > pipelineRegisters;

    std::vector<HWInstr*> body;

    int getEndTime(HWInstr* instr) {
      return fSched.getEndTime(instr);
    }

    bool hasOutput(HWInstr* const arg) const {
      return CoreIR::contains_key(arg, instrValues);
    }

    void valueIsAlways(HWInstr* const arg1, CoreIR::Wireable* w) {
      for (auto instr : fSched.body()) {
        hwStartValues[arg1][instr] = w;
        hwEndValues[arg1][instr] = w;
      }
    }

    CoreIR::Wireable* valueAtStart(HWInstr* const arg1, HWInstr* const sourceLocation) {
      internal_assert(contains_key(arg1, hwStartValues)) << *arg1 << " is not in hwStartValues when getting its value at: " << *sourceLocation << "\n";
      internal_assert(contains_key(sourceLocation, map_get(arg1, hwStartValues))) << *sourceLocation << " is not in hwStartValues[" << *arg1 << "]\n";
      return map_get(sourceLocation, map_get(arg1, hwStartValues));
    }

    CoreIR::Wireable* valueAtEnd(HWInstr* const arg1, HWInstr* const sourceLocation) {
      internal_assert(contains_key(arg1, hwEndValues));
      internal_assert(contains_key(sourceLocation, hwEndValues[arg1]));
      return hwEndValues[arg1][sourceLocation];
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

void createFunctionalUnitsForOperations(StencilInfo& info, UnitMapping& m, FunctionSchedule& sched, ModuleDef* def, CoreIR::Instance* controlPath) {
  auto context = def->getContext();
  int defStage = 0;
  auto& unitMapping = m.unitMapping;
  auto& instrValues = m.instrValues;
  auto& stencilRanges = m.stencilRanges;

  for (auto instr : sched.body()) {
    if (instr->tp == HWINSTR_TP_INSTR) {
      string name = instr->name;
      //cout << "Creating unit for " << *instr << endl;
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
      } else if (name == "abs") {
        auto mul = def->addInstance("abs" + std::to_string(defStage), "commonlib.abs", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      } else if (name == "absd") {
        auto mul = def->addInstance("absd" + std::to_string(defStage), "commonlib.absd", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = mul->sel("out");
        unitMapping[instr] = mul;
      }else if (name == "mod") {
        auto mul = def->addInstance("mod" + std::to_string(defStage), "coreir.mod", {{"width", CoreIR::Const::make(context, 16)}});
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
        auto cs = def->addInstance("wire_" + std::to_string(defStage), "halidehw.cast", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = cs->sel("out");
        unitMapping[instr] = cs;
      } else if (name == "rd_stream") {
        auto dims = getStreamDims(instr->operands[0]->name, info);

        //cout << "# of dims in " << instr->operands[0]->name << " = " << dims.size() << endl;
        //for (auto d : dims) {
          //cout << "Dim = " << d << endl;
        //}

        vector<int> dimRanges = getDimRanges(dims);
        if (dimRanges.size() == 2) {
          stencilRanges[instr] = dimRanges;
          auto rdStrm = def->addInstance("rd_stream_" + std::to_string(defStage), "halidehw.rd_stream", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", CoreIR::Const::make(context, dimRanges[0])}, {"ncols", CoreIR::Const::make(context, dimRanges[1])}});

          auto res = rdStrm->sel("out");
          instrValues[instr] = res;
          unitMapping[instr] = rdStrm;
        } else {
          internal_assert(dimRanges.size() == 3);
          stencilRanges[instr] = dimRanges;
          auto rdStrm = def->addInstance("rd_stream_" + std::to_string(defStage), "halidehw.rd_stream_3", {{"width", CoreIR::Const::make(context, 16)}, {"nrows", CoreIR::Const::make(context, dimRanges[0])}, {"ncols", CoreIR::Const::make(context, dimRanges[1])}, {"nchannels", COREMK(context, dimRanges[2])}});

          auto res = rdStrm->sel("out");
          instrValues[instr] = res;
          unitMapping[instr] = rdStrm;
        }
      } else if (name == "write_stream") {

        internal_assert(instr->operandTypes.size() > 1);
        auto otherDims = instr->operandTypes[1];
        //cout << "# of dims in write_stream to " << instr->getOperand(0)->compactString() << " = " << otherDims.dims.size() << endl;
        //for (auto d : otherDims.dims) {
          //cout << "\t" << d << endl;
        //}
        
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
        //cout << "Making create stencil from " << *instr << endl;
        //cout << "Operand 0 = " << *(instr->getOperand(0)) << endl;

        internal_assert(instr->getOperand(0)->resType != nullptr);
        auto dimRanges = arrayDims(instr->getOperand(0)->resType);

        //auto dimRanges = CoreIR::map_find(instr->getOperand(0), stencilRanges);

        //cout << "dimRanges = " << dimRanges << endl;
        //internal_assert(false);
        if (dimRanges.size() == 3) {
          int selRow = instr->getOperand(2)->toInt();
          int selCol = instr->getOperand(3)->toInt();
          auto cS = def->addInstance("create_stencil_" + std::to_string(defStage), "halidehw.create_stencil", {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}});

          stencilRanges[instr] = dimRanges;
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        } else {
          internal_assert(dimRanges.size() == 4);

          int selRow = instr->getOperand(2)->toInt();
          int selCol = instr->getOperand(3)->toInt();
          int selChan = instr->getOperand(4)->toInt();
          auto cS = def->addInstance("create_stencil_" + std::to_string(defStage), "halidehw.create_stencil_3", {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"nchannels", COREMK(context, dimRanges[3])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}, {"b", COREMK(context, selChan)}});

          stencilRanges[instr] = dimRanges;
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        }
        //cout << "Built dimranges" << endl;

      } else if (starts_with(name, "dynamic_stencil_read")) {

        //cout << "Creating stencil read: " << *instr << endl;
        //cout << "\tread from: " << *(instr->getOperand(0)) << endl;
        internal_assert(instr->getOperand(0)->resType != nullptr);

        vector<int> dimRanges = arrayDims(instr->getOperand(0)->resType);
        internal_assert(dimRanges.size() > 1) << "dimranges has size: " << dimRanges.size() << "\n";

        if (dimRanges.size() == 3) {
          auto cS = def->addInstance("dynamic_stencil_read_" + std::to_string(defStage), "halidehw.dynamic_stencil_read", {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}});
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        } else {
          internal_assert(dimRanges.size() == 4);
          auto cS = def->addInstance("stencil_read_" + std::to_string(defStage),
              "halidehw.dynamic_stencil_read_3",
              {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"nchannels", COREMK(context, dimRanges[3])}});
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        }
      } else if (starts_with(name, "stencil_read")) {
        //cout << "Creating stencil read: " << *instr << endl;
        //cout << "\tread from: " << *(instr->getOperand(0)) << endl;
        internal_assert(instr->getOperand(0)->resType != nullptr);

        vector<int> dimRanges = arrayDims(instr->getOperand(0)->resType);
        //vector<int> dimRanges = CoreIR::map_find(instr->getOperand(0), stencilRanges);
        internal_assert(dimRanges.size() > 1) << "dimranges has size: " << dimRanges.size() << "\n";

        if (dimRanges.size() == 3) {
          internal_assert(instr->getOperand(1)->tp == HWINSTR_TP_CONST);
          internal_assert(instr->getOperand(2)->tp == HWINSTR_TP_CONST);
          //cout << "\tOperands 1 and 2 of " << *instr << " are constants" << endl;

          int selRow = instr->getOperand(1)->toInt();
          int selCol = instr->getOperand(2)->toInt();
          auto cS = def->addInstance("stencil_read_" + std::to_string(defStage), "halidehw.stencil_read", {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}});
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        } else {
          internal_assert(dimRanges.size() == 4);
          
          internal_assert(instr->getOperand(1)->tp == HWINSTR_TP_CONST);
          internal_assert(instr->getOperand(2)->tp == HWINSTR_TP_CONST);
          internal_assert(instr->getOperand(3)->tp == HWINSTR_TP_CONST);

          //cout << "\tOperands 1, 2and 3 of " << *instr << " are constants" << endl;
          
          int selRow = instr->getOperand(1)->toInt();
          int selCol = instr->getOperand(2)->toInt();
          int selChan = instr->getOperand(3)->toInt();
          auto cS = def->addInstance("stencil_read_" + std::to_string(defStage),
              "halidehw.stencil_read_3",
              {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"nchannels", COREMK(context, dimRanges[3])},
              {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}, {"b", COREMK(context, selChan)}});
          instrValues[instr] = cS->sel("out");
          unitMapping[instr] = cS;
        }
      } else if (name == "ashr") {
        auto shr = def->addInstance("ashr" + std::to_string(defStage), "coreir.ashr", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = shr->sel("out");
        unitMapping[instr] = shr;
      } else if (name == "lshr") {
        auto shr = def->addInstance("lshr" + std::to_string(defStage), "coreir.lshr", {{"width", CoreIR::Const::make(context, 16)}});
        instrValues[instr] = shr->sel("out");
        unitMapping[instr] = shr;
      } else if (name == "load") {
        internal_assert(instr->getOperand(0)->tp == HWINSTR_TP_CONST);
        int portNo = instr->getOperand(0)->toInt();
        unitMapping[instr] = instr->getUnit();
        //cout << "Connecting " << *instr << endl;
        internal_assert(instr->getUnit() != nullptr);
        //cout << "Connecting " << coreStr(instr->getUnit()) << " ROM rdata" << endl;
        internal_assert(instr->getUnit() != nullptr);
        internal_assert(isa<CoreIR::Instance>(instr->getUnit()));
        Instance* inst = static_cast<Instance*>(instr->getUnit());
        internal_assert(fromGenerator("halidehw.ROM", inst));
        instrValues[instr] = instr->getUnit()->sel("rdata")->sel(portNo);
        //cout << "Done." << endl;
      } else if (name == "phi") {
        // TODO: Replace this with real code for a multiplexer
        internal_assert(instr->resType != nullptr);
        auto sel = def->addInstance("phi" + std::to_string(defStage), "halidehw.passthrough", {{"type", COREMK(context, instr->resType)}});
        instrValues[instr] = sel->sel("out");
        unitMapping[instr] = sel;
      } else if (name == "delay") {
        auto tp = instr->resType;
        internal_assert(tp != nullptr);
        auto rname = "delay_reg_" + context->getUnique();
        Instance* inst = pipelineRegister(context, def, rname, tp);
        unitMapping[instr] = inst;
        instrValues[instr] = inst->sel("out");
      } else if (name == "provide") {

      } else {
        internal_assert(false) << "no functional unit generation code for " << *instr << "\n";
      }
    }

    defStage++;
  }

  // Constants and pre-bound instructions / variables
  // are always bound to the same wire
  //
  // Local variables can be bound at any place where they are provided
  // Loop index variables are trickier because they are bound to the output of a counter
  // at some location, but then they are defined somewhere else
  for (auto instr : sched.body()) {
    int constNo = 0;
    for (auto op : instr->operands) {
      if (op->tp == HWINSTR_TP_CONST) {
        int width = op->constWidth;
        int value = stoi(op->constValue);
        BitVector constVal = BitVector(width, value);
        auto cInst = def->addInstance("const_" + context->getUnique() + "_" + std::to_string(constNo), "coreir.const", {{"width", CoreIR::Const::make(context, width)}},  {{"value", CoreIR::Const::make(context, BitVector(width, value))}});
        constNo++;
        m.valueIsAlways(op, cInst->sel("out"));
      }
    }
  }

  for (auto v : instrValues) {
    m.hwEndValues[v.first][v.first] = v.second;
    auto op = v.first;
    auto val = v.second;
    int endStage = sched.getEndStage(op);
    for (auto instr : sched.instructionsStartingInStage(endStage)) {
      m.hwStartValues[op][instr] = val;
    }
  }
  
}

UnitMapping createUnitMapping(HWFunction& f, StencilInfo& info, FunctionSchedule& sched, CoreIR::ModuleDef* def, CoreIR::Instance* controlPath) {
  internal_assert(sched.blockSchedules.size() > 0);
  //cout << "--- Block schedules..." << endl;
  //for (auto& blk : sched.blockSchedules) {
    //blk.second.print();
  //}

  auto context = f.mod->getContext();

  UnitMapping m;
  m.fSched = sched;
  m.body = sched.body();
  cout << "Creating unit mapping for " << def->getModule()->getName() << "\n";
  createFunctionalUnitsForOperations(info, m, sched, def, controlPath);
  cout << "Created functional units" << endl;


  for (auto op : allVarsUsed(sched.body())) {
    string name = op->name;

    if (!f.isLocalVariable(name)) {
      cout << "Finding argument value for " << name << endl;
      auto self = def->sel("self");

      Wireable* val = nullptr;
      cout << "Checking if " << name << " is local" << endl;
      internal_assert(!f.isLocalVariable(name)) << name << " is a local variable, but we are selecting it from self\n";
      val = self->sel(coreirSanitize(name));
      internal_assert(val != nullptr);

      m.valueIsAlways(op, val);
    }
  }

  cout << "Populating pipeline registers..." << endl;
  int uNum = 0;
  for (auto instr : sched.body()) {
    int prodStage = sched.getEndStage(instr);
    //for (int i = 0; i < sched.getContainerBlock(instr).numStages(); i++) {
    for (int i = prodStage + 1; i < sched.getContainerBlock(instr).numStages(); i++) {
      if (m.hasOutput(instr)) {
        internal_assert(instr->tp == HWINSTR_TP_INSTR) << *instr << " is not of type instr\n";
        internal_assert(instr->resType != nullptr) << *instr << " has null restype\n";
        m.pipelineRegisters[instr][i] = pipelineRegister(context, def, "pipeline_reg_" + std::to_string(i) + "_" + std::to_string(uNum), instr->resType);
        for (auto otherInstr : sched.getContainerBlock(instr).instructionsStartingInStage(i)) {
          m.hwStartValues[instr][otherInstr] = m.pipelineRegisters[instr][i]->sel("out");
        }
        for (auto otherInstr : sched.getContainerBlock(instr).instructionsEndingInStage(i)) {
          m.hwEndValues[instr][otherInstr] = m.pipelineRegisters[instr][i]->sel("out");
        }
        //m.outputType(instr));
        uNum++;
      }
    }
  }

  cout << "Wired up non local variables" << endl;

  for (auto op : allVarsUsed(sched.body())) {
    if (op->tp == HWINSTR_TP_VAR) {
      cout << "Wiring up var..." << op->compactString() << endl;
      string name = op->name;

      if (f.isLoopIndexVar(name)) {
        auto instr = getUser(op, sched.body());
        if (instr != nullptr) {
          auto val = controlPath->sel(coreirSanitize(name));
          // Now: Need proper pipeline register wiring here. Better algorithm:
          //  1. Find all users of the var
          //  2. Group the users by block and state
          //  3. For each usergroup (or user state) find the piece of storage that is needed for that value
          //
          //  Need to find the header of the loop for this variable, and then set the hwStartValue of the loop
          //  for all instructions in that state to the value of the counter output
          m.hwStartValues[op][instr] = val;
          m.hwEndValues[op][op] = val;
          auto blk = containerBlock(instr, *(sched.f));
          int iStage = m.fSched.getStartStage(head(blk));
          for (auto instr : m.fSched.instructionsStartingInStage(iStage)) {
            m.hwStartValues[op][instr] = val;
          }

          //for (int stage = 0; stage < (int) sched.getContainerBlock(instr).numStages(); stage++) {
          for (int stage = 1; stage < (int) sched.getContainerBlock(instr).numStages(); stage++) {
            //internal_assert(op->resType != nullptr) << *op << " has null result type\n";
            //m.pipelineRegisters[op][stage] = pipelineRegister(context, def, coreirSanitize(op->name) + "_reg_" + std::to_string(stage), m.outputType(op));
            //m.pipelineRegisters[op][stage] = pipelineRegister(context, def, coreirSanitize(op->name) + "_reg_" + std::to_string(stage), op->resType);
            m.pipelineRegisters[op][stage] = pipelineRegister(context, def, coreirSanitize(op->name) + "_reg_" + std::to_string(stage), context->Bit()->Arr(16));
          }

          //for (int stage = 0; stage < sched.getContainerBlock(instr).numStages() - 1; stage++) {
            //cout << "stage = " << stage << endl;
            //if (stage == 0) {
              //auto prg = map_get(stage + 1, map_get(op, m.pipelineRegisters));
              //def->connect(prg->sel("in"), val); 
            //} else {
              //auto prg = map_get(stage + 1, map_get(op, m.pipelineRegisters));
              //def->connect(prg->sel("in"), m.pipelineRegisters[op][stage]->sel("out"));
            //}
          //}
        }
      } else {
        //internal_assert(!f.isLocalVariable(name)) << name << " is a local variable!\n";
      }
    }
  }

  cout << "Connecting register chains for variables" << endl;
  // Now: Wire up pipeline registers in chains, delete the unused ones and test each value produced in this code
  for (auto pr : m.pipelineRegisters) {
    auto instr = pr.first;
  //for (auto instr : sched.body()) {
    //cout << "Connecting registers for " << *instr << endl;
    //if (m.hasOutput(instr)) {
      cout << "\tGetting value at end" << endl;
      auto fstVal = m.valueAtEnd(instr, instr);
      cout << "\tGetting prod stage" << endl;
      //int prodStage = sched.getEndStage(instr);

      //CoreIR::Wireable* lastReg = fstVal;
      //for (int i = prodStage + 1; i < sched.getContainerBlock(instr).numStages(); i++) {
      //int fstIndex = m.pipelineRegisters[instr].size();
      //for (auto elem : m.pipelineRegisters[instr]) {
        //if (elem.first < fstIndex) {
          //fstIndex = elem.first;
        //}
      //}
      //cout << "fstIndex for " << *instr << " = " << fstIndex << endl;
      ////internal_assert(false);
      //for (size_t i = (size_t) fstIndex; i < m.pipelineRegisters[instr].size(); i++) {
        //cout << "\ti = " << i << endl;
      for (auto pReg : m.pipelineRegisters[instr]) {
        int index = pReg.first;
        auto pipeReg = pReg.second;
        if (contains_key(index - 1, m.pipelineRegisters[instr])) {
          def->connect(pipeReg->sel("in"), m.pipelineRegisters[instr][index - 1]->sel("out"));
        } else {
          def->connect(pipeReg->sel("in"), fstVal);
        }
        //internal_assert(contains_key((int) i, m.pipelineRegisters[instr])) << i << " is not an index of a pipeline register\n";
        //CoreIR::Instance* pipeReg = m.pipelineRegisters[instr][i];
        //def->connect(pipeReg->sel("in"), lastReg);
        //lastReg = pipeReg->sel("out");
      }
    //}
  }
 
  cout << "Done connecting register chains" << endl;
  return m;
}

Expr loopLatency(const std::vector<std::string>& prefixVars, const IBlock& blk, FunctionSchedule& sched) {
  return 234;
}

IBlock innermostLoopContainerHeader(HWInstr* instr, HWFunction& f) {
  auto blk = containerBlock(instr, f);
  for (auto possibleHeader : getIBlocks(f)) {
    if (isHeader(possibleHeader, f) && loopNames(possibleHeader) == loopNames(blk)) {
      return possibleHeader;
    }
  }
  internal_assert(false);
  return blk;
}

Expr delayFromIterationStartToInstr(HWInstr* instr, FunctionSchedule& sched) {
  return 0;
  //cout << "Getting delay from iteration start to instr for: " << *instr << endl;
  //auto& f = *(sched.f);
  //IBlock header = innermostLoopContainerHeader(instr, f);
  
  //cout << "Header..." << endl;
  //cout << header << endl;
  
  //IBlock container = containerBlock(instr, f);
  
  //cout << "Container..." << endl;
  //cout << header << endl;
  
  //internal_assert(loopNames(header) == loopNames(container));

  //Expr delay = 0;
  //IBlock activeBlock = header;
  //vector<string> excludedVars = loopNames(header);
  //while (activeBlock != container) {
    //delay += loopLatency(excludedVars, activeBlock, sched);
    //activeBlock = nextBlock(activeBlock, f);
  //}
  //return delay;
}

Expr containerIterationStart(HWInstr* instr, FunctionSchedule& sched) {
  return 0;
  //// What is the container iteration start time?
  //// Time from root of the program to the x, yth iteration of the loop containing this
  //// expression
  //Expr s = 0;
  //// Assume initiation interval of 1
  //for (auto lp : instr->surroundingLoops) {
    //s += 1*Variable::make(Int(32), lp.name);
  //}
  //return s;
}

Expr endTime(HWInstr* instr, FunctionSchedule& sched) {
  Expr cs = containerIterationStart(instr, sched);
  Expr bd = delayFromIterationStartToInstr(instr, sched);
  return cs + bd + sched.getEndStage(instr);
}

Expr startTime(HWInstr* instr, FunctionSchedule& sched) {
  Expr cs = containerIterationStart(instr, sched);
  Expr bd = delayFromIterationStartToInstr(instr, sched);
  return cs + bd + sched.getStartStage(instr);
}

void emitCoreIR(HWFunction& f, StencilInfo& info, FunctionSchedule& sched) {
  internal_assert(sched.blockSchedules.size() > 0);

  auto def = f.mod->getDef();
  internal_assert(def != nullptr);

  CoreIR::Context* context = def->getContext();
 
  // Create control path
  auto cpM = controlPathForKernel(f);
  cout << "Control path module..." << endl;
  cpM.m->print();
  auto controlPath = def->addInstance("control_path_module_" + f.name, cpM.m);
  def->connect(def->sel("self")->sel("reset"), controlPath->sel("reset"));
  cout << "Wiring up def in enable and control path in_en" << endl;
  def->connect(def->sel("self")->sel("in_en"), controlPath->sel("in_en"));

  // In this mapping I want to assign values that are 
  UnitMapping m = createUnitMapping(f, info, sched, def, controlPath);
  auto& unitMapping = m.unitMapping;

  auto self = def->sel("self");
  cout << "Wiring up enables" << endl;
  set<HWInstr*> streamReads = allInstrs("rd_stream", sched.body());
  set<HWInstr*> streamWrites = allInstrs("write_stream", sched.body());

  int validDelay = 0;
  internal_assert(streamWrites.size() == 1 ||
      streamWrites.size() == 0);
  if (streamWrites.size() == 1) {
    HWInstr* read = *begin(streamReads);
    HWInstr* write = *begin(streamWrites);
    Expr latency = endTime(write, sched) - startTime(read, sched);
    cout << "Read  = " << *read << endl;
    cout << "Write = " << *write << endl;
    cout << "\tSymbolic latency  : " << latency << endl;
    Expr simplifiedLatency = simplify(latency);
    cout << "\tSimplified latency: " << simplifiedLatency << endl;
    //internal_assert(false);

    validDelay = func_id_const_value(simplifiedLatency);
  } else {
    validDelay = 0;
  }
  internal_assert(validDelay >= 0);
  //internal_assert(validDelay == 0) << "Valid delay: " << validDelay << "\n";

  //int validDelay = sched.numStages() - 1;
  cout << "Got valid delay" << endl;
  CoreIR::Wireable* inEn = self->sel("in_en");
  for (int i = 0; i < validDelay; i++) {
    auto vR = def->addInstance("valid_delay_reg_" + std::to_string(i), "corebit.reg");
    def->connect(inEn, vR->sel("in"));
    inEn = vR->sel("out");
  }
  def->connect(inEn, self->sel("valid"));

  cout << "Building connections inside each cycle\n";
  for (auto instr : sched.body()) {
    if (instr->name == "provide") {
      internal_assert(instr->getOperand(0)->tp == HWINSTR_TP_VAR); 
      string output_port = coreirSanitize(instr->operands[0]->name);
      auto pt = def->sel("self")->sel(output_port);
      for (size_t i = instr->operands.size() - 1; i >= 2; i--) {
        pt = pt->sel(instr->getOperand(i)->toInt());
      }
      def->connect(pt, m.valueAtStart(instr->operands.at(1), instr));
      continue;
    }
    internal_assert(CoreIR::contains_key(instr, unitMapping)) << "no unit mapping for " << *instr << "\n";
    CoreIR::Instance* unit = CoreIR::map_find(instr, unitMapping);

    if (instr->name == "add" || (instr->name == "mul") || (instr->name == "div") || (instr->name == "sub") || (instr->name == "min") || (instr->name == "max") ||
        (instr->name == "lt") || (instr->name == "gt") || (instr->name == "lte") || (instr->name == "gte") || (instr->name == "and") || (instr->name == "mod") ||
        (instr->name == "eq") || (instr->name == "neq") || (instr->name == "and_bv") || (instr->name == "absd")) {
      auto arg0 = instr->getOperand(0);
      auto arg1 = instr->getOperand(1);

      def->connect(unit->sel("in0"), m.valueAtStart(arg0, instr));
      def->connect(unit->sel("in1"), m.valueAtStart(arg1, instr));

    } else if (instr->name == "abs") {
      auto arg = instr->getOperand(0);
      def->connect(unit->sel("in"), m.valueAtStart(arg, instr));
    } else if (instr->name == "delay") {
      auto arg = instr->getOperand(0);
      def->connect(unit->sel("in"), m.valueAtStart(arg, instr));
    } else if (instr->name == "cast") {
      auto arg = instr->getOperand(0);
      def->connect(unit->sel("in"), m.valueAtStart(arg, instr));
    } else if (instr->name == "rd_stream") {
      auto arg = instr->getOperand(0);
      def->connect(unit->sel("in"), m.valueAtStart(arg, instr));
    } else if (instr->name == "stencil_read") {
      auto arg = instr->getOperand(0);
      def->connect(unit->sel("in"), m.valueAtStart(arg, instr));
    } else if (starts_with(instr->name, "create_stencil")) {
      auto srcStencil = instr->getOperand(0);
      auto newVal = instr->getOperand(1);

      def->connect(unit->sel("in_stencil"), m.valueAtStart(srcStencil, instr));
      def->connect(unit->sel("new_val"), m.valueAtStart(newVal, instr));

    } else if (instr->name == "write_stream") {
      auto strm = instr->getOperand(0);
      auto stencil = instr->getOperand(1);


      def->connect(unit->sel("stream"), m.valueAtStart(strm, instr));
      def->connect(unit->sel("stencil"), m.valueAtStart(stencil, instr));

    } else if (instr->name == "sel") {

      def->connect(unit->sel("sel"), m.valueAtStart(instr->getOperand(0), instr));
      def->connect(unit->sel("in1"), m.valueAtStart(instr->getOperand(1), instr));
      def->connect(unit->sel("in0"), m.valueAtStart(instr->getOperand(2), instr));

    } else if (starts_with(instr->name, "init_stencil")) {
      // No inputs
    } else if ((instr->name == "ashr") || (instr->name == "lshr")) {
      def->connect(unit->sel("in1"), m.valueAtStart(instr->getOperand(1), instr));
      def->connect(unit->sel("in0"), m.valueAtStart(instr->getOperand(0), instr));

    } else if (instr->name == "load") {
      internal_assert(instr->getOperand(0)->tp == HWINSTR_TP_CONST);
      int portNo = instr->getOperand(0)->toInt();
      def->connect(unit->sel("raddr")->sel(portNo), m.valueAtStart(instr->getOperand(2), instr));
      def->connect(unit->sel("ren")->sel(portNo), def->addInstance("ld_bitconst_" + context->getUnique(), "corebit.const", {{"value", COREMK(context, true)}})->sel("out"));

    } else if (instr->name == "phi") {
      // TODO: Replace with real mux code
      def->connect(unit->sel("in"), m.valueAtStart(instr->getOperand(0), instr));
    } else {
      internal_assert(false) << "no wiring procedure for " << *instr << "\n";
    }
  }
  cout << "Done building connections in body" << endl;
}

CoreIR::Type* moduleTypeForKernel(CoreIR::Context* context,
    HardwareInfo& hwInfo,
    StencilInfo& info, const Stmt& stmt, const vector<CoreIR_Argument>& args) {

  vector<std::pair<std::string, CoreIR::Type*> > tps;
  tps = {{"reset", context->BitIn()}, {"in_en", context->BitIn()}, {"valid", context->Bit()}};

  if (hwInfo.interfacePolicy == HW_INTERFACE_POLICY_TOP) {
    StencilInfoCollector lpInfo;
    stmt.accept(&lpInfo);

    std::set<string> inStreams;
    std::set<string> outStreams;

    for (auto v : lpInfo.info.streamReads) {
      inStreams.insert(v.first);
    }
    for (auto v : lpInfo.info.streamWrites) {
      outStreams.insert(v.first);
    }

    for (auto v : extractHardwareVars(stmt)) {
      string vName = coreirSanitize(v);
      tps.push_back({vName, context->BitIn()->Arr(16)});
    }

    for (auto arg : args) {
      if (!arg.is_stencil) {
        string vName = coreirSanitize(arg.name);
        if (!arg.is_output) {
          tps.push_back({vName, context->Bit()->Arr(16)});
        } else {
          tps.push_back({vName, context->BitIn()->Arr(16)});
        }
      }
    }

    //cout << "Current stencils..." << endl;
    //cout << stencils << endl;
    cout << "All input streams" << endl;
    for (auto is : inStreams) {
      cout << "\t\t" << is << endl;
      vector<string> dispatchInfo = CoreIR::map_find(is, info.streamDispatches);
      cout << "\tDispatch info..." << endl;
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
  } else {
    internal_assert(HW_INTERFACE_POLICY_COMPUTE_UNIT); 
    for (auto arg : args) {
      //internal_assert(arg.is_stencil) << arg.name << " is not a stencil\n";
     
      if (arg.is_stencil) {
        auto stype = arg.stencil_type;

        vector<uint> indices;
        for(const auto &range : stype.bounds) {
          internal_assert(is_const(range.extent));
          indices.push_back(func_id_const_value(range.extent));
          info.streamParams[arg.name].push_back(std::to_string(func_id_const_value(range.min)));
          info.streamParams[arg.name].push_back(std::to_string(func_id_const_value(range.extent)));
        }

        if (arg.is_output) {
          uint out_bitwidth = c_inst_bitwidth(stype.elemType.bits());
          internal_assert(out_bitwidth > 0);

          CoreIR::Type* output_type = out_bitwidth > 1 ? context->Bit()->Arr(out_bitwidth) : context->Bit();
          for (uint i=0; i<indices.size(); ++i) {
            output_type = output_type->Arr(indices[i]);
          }
          string output_name_real = coreirSanitize(arg.name);
          tps.push_back({output_name_real, output_type});

        } else {
          uint in_bitwidth = c_inst_bitwidth(stype.elemType.bits());
          CoreIR::Type* input_type = in_bitwidth > 1 ? context->BitIn()->Arr(in_bitwidth) : context->BitIn();
          for (uint i=0; i<indices.size(); ++i) {
            input_type = input_type->Arr(indices[i]);
          }
          tps.push_back({coreirSanitize(arg.name), input_type});
        }
      } else {
        // TODO: Actually handle non-stencil arguments
      }
    }
  }

  tps.push_back({"in_en", context->BitIn()});
  tps.push_back({"valid", context->Bit()});
  CoreIR::Type* design_type = context->Record(tps);
  return design_type;
}

std::set<HWInstr*> instrsUsedBy(HWInstr* instr) {
  std::set<HWInstr*> instrs;
  for (auto i : instr->operands) {
    if (i->tp == HWINSTR_TP_INSTR) {
      instrs.insert(i);
    }
  }
  return instrs;
}

std::set<HWInstr*> dependencies(HWInstr* toSchedule,
    map<HWInstr*, vdisc> iNodes,
    DirectedGraph<HWInstr*, int>& depGraph) {
  set<HWInstr*> deps;
  auto v = map_find(toSchedule, iNodes);
  auto inEdges = depGraph.inEdges(v);
  for (auto e : inEdges) {
    auto src = depGraph.source(e);
    deps.insert(depGraph.getNode(src));
  }
  return deps;
}

HWLoopSchedule asapSchedule(std::vector<HWInstr*>& instrs) {
  HWLoopSchedule sched;
  sched.body = instrs;
  // TODO: Actually compute this later on
  //sched.II = 1;

  std::map<HWInstr*, int> activeToTimeRemaining;
  std::set<HWInstr*> finished;
  std::set<HWInstr*> remaining(begin(sched.body), end(sched.body));

  DirectedGraph<HWInstr*, int> blockGraph;
  map<HWInstr*, vdisc> iNodes;
  for (auto instr : sched.body) {
  //for (auto instr : f.allInstrs()) {
    auto v = blockGraph.addVertex(instr);
    iNodes[instr] = v;
  }
  for (auto instr : sched.body) {
    auto v = map_get(instr, iNodes);
    for (auto op : instr->operands) {
      if (op->tp == HWINSTR_TP_INSTR) {
        if (contains_key(op, iNodes)) {
          if (instr->name == "phi") {
            bool definedBefore = false;
            for (auto iVal : instrs) {
              if (*iVal == *op) {
                definedBefore = true;
                break;
              }
              if (*iVal == *instr) {
                break;
              }
            }
            if (definedBefore) {
              auto depV = map_get(op, iNodes);
              blockGraph.addEdge(depV, v);
            } else {
              cout << "Not adding " << *op << " as dependence of " << *instr << " because it finishes lexically later" << endl;
            }
          } else {
            auto depV = map_get(op, iNodes);
            blockGraph.addEdge(depV, v);
          }
        } else {
          // The dependence is on an instruction outside of the given set of instructions,
          // which is assumed to have completed before this instruction block begins
          finished.insert(op);
          internal_assert(!elem(op, sched.body)) << "no iNode for instruction: " << *op << ", which should be schedueld as part of instrs\n";
        }
      }
    }
  }

  auto sortedNodes = topologicalSort(blockGraph);
  cout << "Already finished..." << endl;
  for (auto i : finished) {
    cout << "\t" << *i << ", latency: " << i->latency << endl;
  }
  cout << "Instruction sort..." << endl;
  for (auto v : sortedNodes) {
    cout << "\t" << *blockGraph.getNode(v) << ", latency: " << (blockGraph.getNode(v))->latency << endl;
  }
  //internal_assert(false);
  int currentTime = 0;
  while (remaining.size() > 0) {
    //cout << "Current time = " << currentTime << endl;
    //cout << "\t# Finished = " << finished.size() << endl;
    //cout << "\tActive = " << activeToTimeRemaining << endl;
    //cout << "\tRemain = " << remaining << endl;
    bool foundNextInstr = false;
    for (auto toSchedule : remaining) {
      //std::set<HWInstr*> deps = instrsUsedBy(toSchedule);
      std::set<HWInstr*> deps = dependencies(toSchedule, iNodes, blockGraph);
      //cout << "Instr: " << *toSchedule << " has " << deps.size() << " deps: " << endl;
      if (subset(deps, finished)) {
        //cout << "Scheduling " << *toSchedule << " in time " << currentTime << endl;
        sched.setStartTime(toSchedule, currentTime);
        if (toSchedule->latency == 0) {
          sched.setEndTime(toSchedule, currentTime);
          finished.insert(toSchedule);
          //cout << "Finishing " << *toSchedule << " in time " << currentTime << endl;
        } else {
          activeToTimeRemaining[toSchedule] = toSchedule->latency;
        }
        remaining.erase(toSchedule);
        foundNextInstr = true;
        break;
      } else {
        //cout << "\tUnfinished deps..." << endl;
        //for (auto d : deps) {
          //if (!elem(d, finished)) {
            //cout << "\t\t" << *d << endl;
          //}
        //}
      }
    }

    if (!foundNextInstr) {
      internal_assert(activeToTimeRemaining.size() > 0) << "cannot find new instruction to schedule i, but no instructions are in progress...\n";
      currentTime++;
      std::set<HWInstr*> doneThisCycle;
      for (auto& instr : activeToTimeRemaining) {
        activeToTimeRemaining[instr.first]--;
        if (activeToTimeRemaining[instr.first] == 0) {
          sched.setEndTime(instr.first, currentTime);
          finished.insert(instr.first);
          doneThisCycle.insert(instr.first);
        } else {
        }
      }

      for (auto instr : doneThisCycle) {
        activeToTimeRemaining.erase(instr);
        finished.insert(instr);
      }
    }
  }

  cout << "Final Schedule" << endl;
  for (int i = 0; i < sched.numStages(); i++) {
    cout << "Stage " << i << endl;
    for (auto instr : sched.instructionsStartingInStage(i)) {
      cout << "\tstart: " << *instr << endl;
    }
    for (auto instr : sched.instructionsEndingInStage(i)) {
      cout << "\tend  : " << *instr << endl;
    }
  }

  internal_assert((instrs.size() == 0) || sched.numStages() > 0) << "error, 0 stages in schedule\n";
  internal_assert(sched.startStages.size() == sched.endStages.size()) << "not every instruction with a start has an end\n";
  for (auto instr : instrs) {
    internal_assert(sched.isScheduled(instr)) << "instruction: " << *instr << " is not scheduled!\n";
    internal_assert((sched.getEndTime(instr) - sched.getStartTime(instr)) == instr->latency) << "latency in schedule does not match for " << *instr << "\n";
  }

  return sched;
}

HWLoopSchedule asapSchedule(HWFunction& f) {
  auto cpy = f.structuredOrder();
  auto sched = asapSchedule(cpy);
  return sched;
}

int tripCountInt(const std::string& var, HWFunction& f) {
  for (auto instr : f.allInstrs()) {
    for (auto lp : instr->surroundingLoops) {
      if (lp.name == var) {
        Expr tc = simplify(lp.extent);
        return func_id_const_value(tc);
      }
    }
  }

  return 1;
}

int headerLatencyInt(const std::string& name, HWFunction& f, FunctionSchedule& fSched) {
  return 0;
}

int tailLatencyInt(const std::string& name, HWFunction& f, FunctionSchedule& fSched) {
  return 0;
}

FunctionSchedule buildFunctionSchedule(HWFunction& f) {
  auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });
  // Check if we are in a perfect loop nest
  FunctionSchedule fSched;
  fSched.f = &f;
  for (auto group : instrGroups) {
    HWLoopSchedule sched = asapSchedule(group);
    fSched.blockSchedules[head(group)] = sched;
  }

  // Compute IIs here
  // How? first find loop variable order or the instruction with largest
  // number of loop variables
  // Then: iterate backward over this loop variable set computing IIs?

  IBlock deepest(true);
  for (auto blk : getIBlocks(f)) {
    if (loopNames(blk).size() >= loopNames(deepest).size()) {
      deepest = blk;
    }
  }

  cout << "Deepest loop nest block:" << endl;
  cout << deepest << endl;

  vector<string> nestVars = loopNames(deepest);
  if (nestVars.size() == 0) {
    return fSched;
  }

  CoreIR::reverse(nestVars);

  vector<NestSchedule> schedules;
  int tc0 = tripCountInt(nestVars[0], f);
  int latency0 = fSched.getScheduleFor(head(deepest)).cycleLatency();
  schedules.push_back({nestVars[0], 1, latency0, tc0});

  for (int i = 1; i < (int) nestVars.size(); i++) {
    // Create nest schedule

    int tc = tripCountInt(nestVars[i], f);
    int headerLatency = headerLatencyInt(nestVars[i], f, fSched);
    int tailLatency = tailLatencyInt(nestVars[i], f, fSched);
    int II = headerLatency + schedules.back().completionTime() + tailLatency;
    int L = II; // Execute outer loops sequentially;

    schedules.push_back({nestVars[i], II, L, tc});
  }
  cout << "Nest schedules..." << endl;
  for (auto sched : schedules) {
    cout << "\t" << sched.name << endl;
    cout << "\t\tII = " << sched.II << endl;
    cout << "\t\tTC = " << sched.TC << endl;
    cout << "\t\tL  = " << sched.L << endl;
    cout << "\t\tC  = " << sched.completionTime() << endl;
  }

  fSched.nestSchedules = schedules;
  internal_assert(fSched.blockSchedules.size() > 0);

  if (f.allInstrs().size() == 0) {
    internal_assert(false);
    return fSched;
  }

  // Transitions?
  internal_assert(instrGroups.size() > 0);
  internal_assert(instrGroups[0].size() > 0);
  for (int i = 0; i < (int) (instrGroups.size() - 1); i++) {
    auto current = instrGroups[i];
    auto next = instrGroups[i + 1];

    if (numLoops(current) < numLoops(next)) {
      cout << "Entering loop" << endl;
      fSched.transitions.push_back({head(current), head(next), 0});
    }
    if (numLoops(current) > numLoops(next)) {
      cout << "Exiting loop" << endl;
      fSched.transitions.push_back({head(current), head(next), 0});
    }
    internal_assert(numLoops(current) != numLoops(next));
  }

  // Find companion blocks for each loop nest, then do I want to
  // have a transition from one block to another or from one loop to itsef?
  set<vector<string> > alreadySeenLoops;
  for (auto group : instrGroups) {
    auto prefix = loopNames(group);
    if (!elem(prefix, alreadySeenLoops)) {
      // By default assume all loops have II = 1
      fSched.transitions.push_back({head(group), head(group), 1});
      alreadySeenLoops.insert(prefix);
    }
  }

  cout << "Transitions in schedule" << endl;
  for (auto t : fSched.transitions) {
    cout << "\t" << *(t.srcBlk) << " -> " << *(t.dstBlk) << ", delay: " << t.delay << endl;
  }

  internal_assert(fSched.blockSchedules.size() > 0);
  return fSched;
}

ComputeKernel moduleForKernel(StencilInfo& info, HWFunction& f) {
  internal_assert(f.mod != nullptr) << "no module in HWFunction\n";

  // Check that all instructions resTypes
  //for (auto instr : f.structuredOrder()) {
    //internal_assert(instr->resType != nullptr) << *instr << " has no resType\n";
  //}

  auto design = f.mod;
  auto def = design->getDef();

  internal_assert(def != nullptr) << "module definition is null!\n";
  //if (f.allInstrs().size() == 0) {
    ////auto sched = asapSchedule(f);
    //return {f.mod, {}};
  //}

  cout << "Hardware function is..." << endl;
  cout << f << endl;

  FunctionSchedule fSched = buildFunctionSchedule(f);
  internal_assert(fSched.blockSchedules.size() > 0);

  emitCoreIR(f, info, fSched);

  design->setDef(def);
  return {design, fSched};
  
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

void replaceAll(std::map<HWInstr*, HWInstr*>& loadsToConstants, HWFunction& f) {
  for (auto ldNewVal : loadsToConstants) {
    //cout << "Replace " << *(ldNewVal.first) << " with " << ldNewVal.second->compactString() << endl;
    if (!(ldNewVal.second->tp == HWINSTR_TP_CONST)) {
      f.insertAt(ldNewVal.first, ldNewVal.second);
    }
    f.replaceAllUsesWith(ldNewVal.first, ldNewVal.second);
  }

  for (auto ldNewVal : loadsToConstants) {
    f.deleteInstr(ldNewVal.first);
  }

  f.deleteAll([](HWInstr* instr) { return isStore(instr); });
}

void removeBadStores(StoreCollector& storeCollector, HWFunction& f) {
  cout << "Allocate ROMs..." << endl;
  std::map<std::string, vector<HWInstr*> > romLoads;
  for (auto instr : f.allInstrs()) {
    if (isCall("load", instr)) {
      cout << "Found load..." << *instr << endl;
      romLoads[instr->getOperand(0)->compactString()].push_back(instr);
    }
  }

  //cout << "All loads..." << endl;
  auto def = f.getDef();
  auto context = def->getContext();

  std::map<HWInstr*, HWInstr*> loadsToReplacements;
  for (auto m : romLoads) {
    //cout << "\tTo rom: " << m.first << endl;
    //cout << "StoredValues = " << storeCollector.constStores << endl;
    //auto values = map_get("\"" + (m.first) + "\"", storeCollector.constStores);
    string curveName = m.first.substr(1, m.first.size() - 2);
    //cout << "Getting value for " << curveName << endl;
    auto values = map_get(curveName, storeCollector.constStores);
    //Json romVals;
    nlohmann::json romVals;
    for (int i = 0; i < (int) values.size(); i++) {
      //cout << "Getting " << i << " from " << values << endl;
      int val = map_get(i, values);
      //romVals["init"][i] = val;
      romVals["init"].emplace_back(val);
      //romVals["init"].emplace_back(200);
    }
    CoreIR::Values vals{{"width", COREMK(context, 16)}, {"depth", COREMK(context, romVals["init"].size())}, {"nports", COREMK(context, m.second.size())}};
    auto rom = def->addInstance(coreirSanitize(m.first), "halidehw.ROM", vals, {{"init", COREMK(context, romVals)}});
    internal_assert(fromGenerator("halidehw.ROM", rom)) << "Did not produce a ROM in load optimization\n";
    int portNo = 0;
    for (auto ld : m.second) {
      cout << "\t\t" << *ld << endl;
      auto rLoad = f.newI(ld);
      rLoad->name = "load";
      rLoad->latency = 1;
      rLoad->operands.push_back(f.newConst(32, portNo));
      for (size_t i = 1; i < ld->operands.size(); i++) {
        rLoad->operands.push_back(ld->getOperand(i));
      }
      rLoad->unit = rom;
      loadsToReplacements[ld] = rLoad;
      portNo++;
    }
  }

  replaceAll(loadsToReplacements, f);

  // Now: Need to add code to HWInstr that will allow use to bind a specific functional unit to it
  // A given instruction needs to have a hardware module associated with it by name, and needs to have
  // a latency (eventually), a port which is the output of the instruction, and an identifier for
  // the module which it will be bound to, and a procedure for wiring up the operands of the
  // instruction to names of ports on the target module
  cout << "Module def..." << endl;
  f.mod->print();
}

void replaceAllUsesAfter(HWInstr* refresh, HWInstr* toReplace, HWInstr* replacement, vector<HWInstr*>& body) {
  int startPos = instructionPosition(refresh, body);
  for (int i = startPos + 1; i < (int) body.size(); i++) {
    replaceOperand(toReplace, replacement, body[i]);
 }
}

void removeWriteStreamArgs(StencilInfo& info, HWFunction& f) {
  for (auto instr : f.allInstrs()) {
    if (isCall("write_stream", instr)) {
      instr->operands = {instr->operands[0], instr->operands[1]};
    }
  }
}

void valueConvertStreamReads(StencilInfo& info, HWFunction& f) {
  auto body = f.structuredOrder();
  std::map<HWInstr*, HWInstr*> replacements;
  for (auto instr : body) {
  //for (auto instr : f.allInstrs()) {
    if (isCall("read_stream", instr)) {
      auto callRep = f.newI(instr);
      callRep->setSigned(instr->isSigned());
      callRep->name = "rd_stream";
      callRep->operands = {instr->operands[0]};
      auto targetStencil = instr->operands[1];
      auto dims = getStreamDims(instr->operands[0]->name, info);
      callRep->resType = f.mod->getContext()->Bit()->Arr(16);
      vector<int> dimRanges = getDimRanges(dims);
      for (auto d : dimRanges) {
        callRep->resType = callRep->resType->Arr(d);
      }

      f.replaceAllUsesWith(targetStencil, callRep);
      replacements[instr] = callRep;
    }
  }

  for (auto rp : replacements) {
    f.insertAt(rp.first, rp.second);
  }
  f.deleteAll([replacements](HWInstr* ir) { return CoreIR::contains_key(ir, replacements); });
}

bool allConst(const int start, const int end, vector<HWInstr*>& hwInstr) {
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
  cout << "Getting streams that use " << name << endl;
  std::set<std::string> users;
  for (auto wr : info.streamReadCallRealizations) {
    string rdName = exprString(wr.first->args[1]);
    cout << "\trdName = " << rdName << endl;
    if (rdName == name) {
      users.insert(exprString(wr.first->args[0]));
      //users.insert(rdName);
    }
  }

  for (auto wr : info.streamWriteCallRealizations) {
    string wrName = exprString(wr.first->args[1]);
    cout << "\twrName = " << wrName << endl;
    if (wrName == name) {
      users.insert(exprString(wr.first->args[0]));
    }
  }
  return users;
}

vector<int> stencilDimsInBody(StencilInfo& info, HWFunction &f, const std::string& stencilName) {
  cout << "Getting stencilDimsInBody of " << stencilName << endl;
  std::set<std::string> streamUsers = streamsThatUseStencil(stencilName, info);
  std::set<std::string> streamsInF = allStreamNames(f);
  std::set<std::string> streamUsersInF = CoreIR::intersection(streamUsers, streamsInF);
  //internal_assert(streamUsersInF.size() > 0) << " no streams that use " << stencilName << " in hardware kernel that contains it\n";
  if (streamUsersInF.size() == 0) {
    return getStreamDims(stencilName, info);
  }
  cout << "Streams that use " << stencilName << "..." << endl;
  for (auto user : streamUsersInF) {
    cout << "\t" << user << endl;
  }
  auto user = *std::begin(streamUsersInF);
  return toInts(map_get(user, info.streamParams));
}

template<typename T>
set<HWInstr*> allVarUsers(const std::string& name, const T& program) {
  set<HWInstr*> users;
  for (auto instr : program) {
    for (auto op : instr->operands) {
      if (op->tp == HWINSTR_TP_VAR) {
        if (op->name == name) {
          users.insert(instr);
        }
      }
    }
  }
  return users;
}

void valueConvertProvides(StencilInfo& info, HWFunction& f) {
  std::map<string, vector<HWInstr*> > provides;
  std::map<string, HWInstr*> stencilDecls;
  for (auto instr : f.structuredOrder()) {
    if (isCall("provide", instr)) {
      string target = instr->operands[0]->name;
      provides[target].push_back(instr);
      stencilDecls[target] = instr;
    }
  }

  cout << "Provides" << endl;
  for (auto pr : provides) {
    auto provideValue = CoreIR::map_find(pr.first, stencilDecls);
    //auto provideName = provideValue->operands[0]->compactString();
    auto provideName = provideValue->operands[0]->name;

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
    vector<int> ranges = getDimRanges(dims);
    CoreIR::Type* tp = f.mod->getContext()->Bit()->Arr(16);
    for (auto d : ranges) {
      tp = tp->Arr(d);
    }
    initInstr->resType = tp;

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

    initInstr->surroundingLoops = f.structuredOrder()[0]->surroundingLoops;
    f.insert(0, initInstr);
    // Assume that initInstr has same containing loops as the first instruction
    // in the HWFunction
    internal_assert(f.structuredOrder().size() > 0);
    HWInstr* activeProvide = initInstr;
    f.replaceAllUsesWith(provideValue->operands[0], activeProvide);
    cout << "done with set values..." << endl;
    int provideNum = 0;
    for (int i = initialSets.size(); i < (int) pr.second.size(); i++) {
      auto instr = pr.second[i];
      cout << "\t\t" << *instr << endl;
      auto refresh = f.newI(instr);
      refresh->operands = instr->operands;
      refresh->name = "create_stencil_" + pr.first + "_" + std::to_string(provideNum);
      refresh->resType = instr->getOperand(0)->resType;
      f.insertAt(instr, refresh);
      f.replaceAllUsesAfter(refresh, activeProvide, refresh);
      activeProvide = refresh;
      provideNum++;
    }
  }

  for (auto pr : provides) {
    for (auto instr : pr.second) {
      f.deleteInstr(instr);
    }
  }

  cout << "After cleanup..." << endl;
  cout << f << endl;

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

Wireable* replaceBase(Wireable* toReplace, Wireable* newBase) {
  auto b = getBase(toReplace);
  if (b == toReplace) {
    return newBase;
  }

  internal_assert(CoreIR::isa<CoreIR::Select>(toReplace));

  auto s = static_cast<CoreIR::Select*>(toReplace);
  return replaceBase(s->getParent(), newBase)->sel(s->getSelStr());
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
  //std::map<HWInstr*, HWInstr*> replacements;
  std::vector<std::pair<HWInstr*, HWInstr*> > replacements;
  //for (auto instr : f.allInstrs()) {
  for (auto instr : f.structuredOrder()) {
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
          auto shrInstr = f.newI(instr);
          shrInstr->name = "and_bv";
          shrInstr->resType = f.mod->getContext()->Bit();
          //shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, 1 << (value - 1))};
          shrInstr->setSigned(instr->isSigned());

          shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, constVal - 1)};

          replacements.push_back({instr, shrInstr});
        }
      }
    }
  }

  CoreIR::reverse(replacements);
  for (auto r : replacements) {
    f.insertAt(r.first, r.second);
    f.replaceAllUsesWith(r.first, r.second);
    toErase.insert(r.first);
  }

  for (auto i : toErase) {
    f.deleteInstr(i);
  }
}

void divToShift(HWFunction& f) {
  cout << "Div to shift for:" << endl;
  cout << f << endl;

  std::set<HWInstr*> toErase;
  std::vector<std::pair<HWInstr*, HWInstr*> > replacements;
  for (auto instr : f.structuredOrder()) {
    if (isCall("div", instr)) {
      cout << "Found div: " << *instr << endl;
      if (isConstant(instr->getOperand(1))) {
        auto constVal = instr->getOperand(1)->toInt();
        if (CoreIR::isPower2(constVal)) {
          cout << "\t\tand it is a power of 2" << endl;
          int value = std::ceil(std::log2(constVal));
          cout << "\t\tpower of 2 = " << value << endl;
          auto shrInstr = f.newI(instr);
          if (instr->getOperand(0)->isSigned()) {
            shrInstr->name = "ashr";
            cout << "Operand 0 signed" << endl;
          } else {
            shrInstr->name = "lshr";
            cout << "Operand 0 unssigned" << endl;
          }
          shrInstr->setSigned(instr->isSigned());
          shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, value)};
          replacements.push_back({instr, shrInstr});
        }
      }
    }
  }

  CoreIR::reverse(replacements);
  for (auto r : replacements) {
    f.insertAt(r.first, r.second);
    f.replaceAllUsesWith(r.first, r.second);
    toErase.insert(r.first);
  }

  for (auto i : toErase) {
    f.deleteInstr(i);
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

// Maybe first change toward a inner loop handling should be to run instruction collection on outer loops
// and thus save an activeloop list. Then generate a loopnestinfo data structure directly from that so that
// lp is not passed around as a parameter?
KernelControlPath controlPathForKernel(HWFunction& f) {

  // TODO: Do real traversal of instructions
  LoopNestInfo loopInfo;
  if (f.allInstrs().size() > 0) {
    HWInstr* fst = f.structuredOrder()[0];
    cout << "Adding surrounding loops from: " << *fst << endl;
    for (auto lp : fst->surroundingLoops) {
      loopInfo.loops.push_back({lp.name, func_id_const_value(lp.min), func_id_const_value(lp.extent)});
    }
  } else {
    cout << "Error: HWFunction..." << endl;
    cout << f << endl;
    cout << "has no instructions!" << endl;
    internal_assert(false);
  }
  cout << "# of levels in loop for control path = " << loopInfo.loops.size() << endl;
  
  auto c = f.mod->getContext();

  KernelControlPath cp;
  std::set<std::string> streamNames = allStreamNames(f);
  auto globalNs = c->getNamespace("global");
  vector<std::pair<std::string, CoreIR::Type*> > tps{{"reset", c->BitIn()}, {"in_en", c->BitIn()}};
  std::set<string> vars;
  for (auto instr : f.allInstrs()) {
    for (auto lp : instr->surroundingLoops) {
      vars.insert(lp.name);
    }
  }

  for (auto var : vars) {
    int width = 16;
    cp.controlVars.push_back(coreirSanitize(var));
    tps.push_back({coreirSanitize(var), c->Bit()->Arr(width)});
  }
  CoreIR::Module* controlPath = globalNs->newModuleDecl(f.name + "_control_path", c->Record(tps));

  auto def = controlPath->newModuleDef();

  int width = 16;

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

    auto maxValConst = mkConst(def, varName + "_max_value", width, max_value);
    auto atMax = def->addInstance(varName + "_at_max", "coreir.eq", {{"width", COREMK(c, width)}});
    def->connect(atMax->sel("in0"), maxValConst->sel("out"));
    def->connect(atMax->sel("in1"), counter_inst->sel("out"));

    levelAtMax.push_back(atMax);

  }

  internal_assert(levelAtMax.size() == loopLevelCounters.size());

  cout << "Wiring up counter enables for " << loopLevelCounters.size() << " loop levels" << endl;

  if (loopLevelCounters.size() > 0) {
    for (int i = 0; i < ((int) loopLevelCounters.size()) - 1; i++) {
      vector<CoreIR::Wireable*> below;
      for (int j = i + 1; j < (int) loopLevelCounters.size(); j++) {
        below.push_back(levelAtMax[j]->sel("out"));
      }
      CoreIR::Wireable* shouldInc = andList(def, below);
      def->connect(loopLevelCounters[i]->sel("en"), shouldInc);
    }
    def->connect(loopLevelCounters.back()->sel("en"), self->sel("in_en"));
  }

  controlPath->setDef(def);
  
  cp.m = controlPath;
  return cp;
}

void inferStreamTypes(StencilInfoCollector& scl) {
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
    
}

std::string lbName(const std::vector<std::string>& lb) {
  internal_assert(lb.size() > 1);
  return "lb_" + coreirSanitize(lb[0]) + "_to_" + coreirSanitize(lb[1]);
}

Instance* createLinebuffer(CoreIR::Context* context,
    CoreIR::ModuleDef* def,
    StencilInfo& info,
    const std::vector<std::string>& lb) {

  int bitwidth = 16;

  string inName = lb[0];
  string outName = lb[1];

  //string lb_name = "lb_" + coreirSanitize(inName) + "_to_" + coreirSanitize(outName);
  string lb_name = lbName(lb);
  //"lb_" + coreirSanitize(inName) + "_to_" + coreirSanitize(outName);
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
  uint output_dims [num_dims];
  uint image_dims [num_dims];
  for (uint i=0; i<num_dims; ++i) {
    input_dims[i] = inRanges[2*i + 1] - inRanges[2*i];
    input_type = input_type->Arr(input_dims[i]);

    output_dims[i] = outRanges[2*i + 1] - outRanges[2*i];
    output_type = output_type->Arr(output_dims[i]);
    
    image_dims[i] = params[i];
    image_type = image_type->Arr(image_dims[i]);
}

CoreIR::Values lb_args = {{"input_type", CoreIR::Const::make(context,input_type)},
  {"output_type", CoreIR::Const::make(context,output_type)},
  {"image_type", CoreIR::Const::make(context,image_type)},
  {"has_valid",CoreIR::Const::make(context,true)}};

CoreIR::Instance* coreir_lb = def->addInstance(lb_name, "lakelib.linebuffer", lb_args);
return coreir_lb;
}

void createLinebuffers(CoreIR::Context* context,
    CoreIR::ModuleDef* def,
    const int bitwidth,
    std::map<string, CoreIR::Instance*>& linebufferResults,
    std::map<string, CoreIR::Instance*>& linebufferInputs,
    std::set<CoreIR::Instance*>& linebuffers,
    StencilInfoCollector& scl) {

  auto& info = scl.info;
  for (auto lb : scl.info.linebuffers) {
    string inName = lb[0];
    string outName = lb[1];
    auto coreir_lb = createLinebuffer(context, def, info, lb);
    def->connect(coreir_lb->sel("reset"), def->sel("self")->sel("reset"));
    linebufferResults[outName] = coreir_lb;
    linebufferInputs[inName] = coreir_lb;
    linebuffers.insert(coreir_lb);
  }
}

class KernelEdge {
  public:

    CoreIR::Wireable* dataSrc;
    CoreIR::Wireable* valid;

    CoreIR::Wireable* dataDest;
    CoreIR::Wireable* en;

    std::string toString() const {
      return CoreIR::toString(*dataSrc) + " : " + CoreIR::toString(*valid) + " -> " +
        CoreIR::toString(*dataDest) + " : " + CoreIR::toString(*en);
    }
};


class AppGraph {
  public:
    CoreIR::DirectedGraph<CoreIR::Wireable*, KernelEdge> appGraph;
    std::map<CoreIR::Wireable*, vdisc> values;
    std::map<edisc, KernelEdge> edgeLabels;
    std::map<edisc, Instance*> edgeDelays;
    std::map<edisc, int> extraDelaysNeeded;
    std::map<const For*, ComputeKernel> kernelModules;

    int edgeDelay(const edisc e) const {
      if (contains_key(e, extraDelaysNeeded)) {
        return map_get(e, extraDelaysNeeded);
      }
      return 0;
    }
    
    bool hasInputs(Wireable* w) const {
      return appGraph.inEdges(map_get(w, values)).size() > 0;
    }

    Wireable* inEnable(Wireable* w) const {
      std::set<Wireable*> inEnables;
      for (auto inEdge : appGraph.inEdges(map_get(w, values))) {
        inEnables.insert(map_get(inEdge, edgeLabels).en);
      }

      cout << "In enables for " << CoreIR::toString(*w) << endl;
      for (auto e : inEnables) {
        cout << "\t" << CoreIR::toString(*e) << endl;
      }

      if (inEnables.size() == 0) {
        if (isComputeKernel(getBase(w))) {
          return getBase(w)->sel("in_en");
        }
      }

      internal_assert(inEnables.size() == 1) << "inEnables.size() == " << inEnables.size() << "\n";

      return *std::begin(inEnables);
    }

    std::vector<Wireable*> allInputValids(CoreIR::Wireable* w) const {
      vector<Wireable*> ens;
      auto v = map_get(w, values);
      for (auto inEdge : appGraph.inEdges(v)) {
        ens.push_back(map_get(inEdge, edgeLabels).valid);
      }
      return ens;
    }

    Wireable* getNode(vdisc v) const {
      return appGraph.getNode(v);
    }

    std::vector<Wireable*> allVertices() const {
      vector<Wireable*> wVerts;
      for (auto v : appGraph.getVerts()) {
        wVerts.push_back(appGraph.getNode(v));
      }
      return wVerts;
    }

    bool destIsSelf(KernelEdge& e) {
      auto destBase = getBase(e.dataDest);
      if (isa<Interface>(destBase)) {
        return true;
      }

      return false;
    }

    Wireable* target(const edisc e) {
      return appGraph.getNode(appGraph.target(e));
    }
    
    Wireable* source(const edisc e) {
      return appGraph.getNode(appGraph.source(e));
    }

    bool destIsLinebuffer(KernelEdge& e) {
      cout << "Getting base of " << CoreIR::toString(e.dataDest) << endl;
      auto destBase = getBase(e.dataDest);
      if (isa<Instance>(destBase)) {
        auto instBase = static_cast<CoreIR::Instance*>(destBase);
        return fromGenerator("lakelib.linebuffer", instBase);
      }

      return false;
    }

    std::vector<KernelEdge> allEdges() const {
      std::vector<KernelEdge> eds;
      for (auto el : edgeLabels) {
        eds.push_back(el.second);
      }
      return eds;
    }

    edisc addEdge(CoreIR::Wireable* src, CoreIR::Wireable* dest) {
      auto v = map_get(src, values);
      auto d = map_get(dest, values);
      auto ed = appGraph.addEdge(v, d);
      return ed;
    }

    void addEdgeLabel(edisc ed, KernelEdge& e) {
      internal_assert(!contains_key(ed, edgeLabels));
      edgeLabels[ed] = e;
    }

    vdisc addVertex(CoreIR::Wireable* w) {
      auto vd = appGraph.addVertex(w);
      values[w] = vd;
      return vd;
    }

    bool hasLabel(const edisc ed) {
      return contains_key(ed, edgeLabels);
    }

    KernelEdge getLabel(const edisc ed) {
      return map_get(ed, edgeLabels);
    }

    vdisc getVdisc(CoreIR::Wireable* w) const {
      return map_get(w, values);
    }

    std::string toString() {
      std::string str = "";
      str += "---- App graph info\n";
      for (auto v : allVertices()) {
        cout << "Getting info for " << coreStr(v) << endl;
        str += "\t-- Vertex: " + CoreIR::toString(*v) + "\n";
        for (auto inEdge : appGraph.inEdges(getVdisc(v))) {
          cout << "In edge name = " << inEdge << endl;
          internal_assert(hasLabel(inEdge));
          cout << "has label" << endl;
          cout << "In edge = " << getLabel(inEdge).toString() << endl;
          str += "\t\t" + getLabel(inEdge).toString() + "\n";
        }
      }
      return str;
    }

    std::vector<edisc> inEdges(Wireable* w) const {
      return appGraph.inEdges(getVdisc(w));
    }
};

int arrivalTime(edisc e, const int index, AppGraph& g);

edisc firstInputEdge(Wireable* producer, AppGraph& g) {
  //cout << "Getting input edges for " << coreStr(producer) << endl;
  internal_assert(g.inEdges(producer).size() > 0);
  return *std::begin(g.inEdges(producer));
}

int productionTime(Wireable* producer, const int outputIndex, AppGraph& g);

vector<int> inImageDims(Wireable* ld) {
  auto param = getGenArgs(ld).at("image_type")->get<CoreIR::Type*>();
  //cout << "Image type = " << coreStr(param) << endl;
  internal_assert(isa<CoreIR::ArrayType>(param));

  vector<int> arrayLens = arrayDims(param);
  vector<int> imgDims;
  //cout << "Array types..." << endl;
  //for (auto v : arrayLens) {
    //cout << "\t" << v << endl;
  //}

  for (size_t i = 1; i < arrayLens.size(); i++) {
    imgDims.push_back(arrayLens[i]);
  }
  return imgDims;
}

int numOutWindowRows(Wireable* ld) 
{
  auto param = getGenArgs(ld).at("output_type")->get<CoreIR::Type*>();
  //cout << "Out image cols = " << coreStr(param) << endl;
  return arrayDims(param)[1];
}

int numOutWindowCols(Wireable* ld) 
{
  auto param = getGenArgs(ld).at("output_type")->get<CoreIR::Type*>();
  //cout << "Out image cols = " << coreStr(param) << endl;
  return arrayDims(param)[2];
}

int numInImageCols(Wireable* ld) {
  auto param = getGenArgs(ld).at("image_type")->get<CoreIR::Type*>();
  //cout << "Out image cols = " << coreStr(param) << endl;
  return arrayDims(param)[2];
}

int numInImageRows(Wireable* ld) {
  auto param = getGenArgs(ld).at("image_type")->get<CoreIR::Type*>();
  //cout << "Out image cols = " << coreStr(param) << endl;
  return arrayDims(param)[1];
}

int numOutImageRows(Wireable* ld) {
  return numInImageRows(ld) - (numOutWindowRows(ld) - 1);
}
int numOutImageCols(Wireable* ld) {
  return numInImageCols(ld) - (numOutWindowCols(ld) - 1);
}

int linebufferDelay(Wireable* ld) {
  internal_assert(inImageDims(ld).size() == 2);
  return (numOutWindowCols(ld) - 1)*numInImageCols(ld) + (numOutWindowRows(ld) - 1);
}

int linebufferFilledSize(Wireable* ld) {
  return linebufferDelay(ld);
}

// Maybe the proper closed form is an expression for
// difference(pr(i + 1), pr(i)) for all i?
//
// Or function from t_reset, ar(i) for inputs?

// Will this relative production time need to become recursive when we move to
// multi-rate computation?
int relativeProductionTime(Wireable* producer, const int outputIndex, AppGraph& g) {
  if (isa<Interface>(producer)) {
    return outputIndex;
  }
  
  if (isComputeKernel(producer)) {
    if (outputIndex == 0) {
      for (auto k : g.kernelModules) {
        if (k.second.mod == toInstance(producer)->getModuleRef()) {
          return k.second.sched.cycleLatency();
        }
      }
      internal_assert(false) << "No kernel module for " << coreStr(producer) << "\n";
    } else {
      return relativeProductionTime(producer, outputIndex - 1, g);
    }
  }

  internal_assert(isLinebuffer(producer));

  return outputIndex % numOutImageCols(producer) +
    std::floor(outputIndex / numOutImageCols(producer)) * numInImageCols(producer);
  //int delay = 0;
  //if (outputIndex == 0) {
    //delay = 0;
    //return 0;
    ////return linebufferDelay(producer);
  //} else if (outputIndex % numOutImageCols(producer) != 0) {
    //delay = relativeProductionTime(producer, outputIndex - 1, g) + 1;
  //} else {
    //delay = relativeProductionTime(producer, outputIndex - 1, g) + 1 + (numInImageCols(producer) - numOutImageCols(producer));
  //}
  ////cout << "Delay for " << coreStr(producer) << "'s " << outputIndex << "(th) element = " << delay << endl;
  //return delay;
}

int productionTime(Wireable* producer, const int outputIndex, AppGraph& g) {

  //cout << "Getting production time of " << outputIndex << "th output of " << coreStr(producer) << endl;
  if (isa<Interface>(getBase(producer))) {
    return outputIndex;
  }

  if (isComputeKernel(getBase(producer))) {
    for (auto k : g.kernelModules) {
      if (k.second.mod == toInstance(producer)->getModuleRef()) {
        return k.second.sched.cycleLatency() + arrivalTime(firstInputEdge(producer, g), outputIndex, g);
      }
    }
    internal_assert(false) << "No kernel module for " << coreStr(producer) << "\n";
  }

  if (isTrimmer(producer)) {

    auto delay = getGenArgs(producer).at("delay")->get<int>();
    return outputIndex + 2*delay;
  }
  internal_assert(isLinebuffer(producer));
  return arrivalTime(firstInputEdge(producer, g), linebufferDelay(producer), g) + relativeProductionTime(producer, outputIndex, g);
}

// Time at which the ith value from inputSource reaches dest
int arrivalTime(edisc e, const int index, AppGraph& g) {
  KernelEdge edgeLabel = g.getLabel(e);
  Wireable* inputSource = edgeLabel.dataSrc;
  return productionTime(getBase(inputSource), index, g) + g.edgeDelay(e);
}

int cycleDelay(edisc e, AppGraph& appGraph) {
  int aTime = arrivalTime(e, 0, appGraph);

  return aTime;
}

void wireUpAppGraph(AppGraph& appGraph,
    CoreIR::ModuleDef* def) {

  cout << "Setting data sigals on compute kernels" << endl;
  for (auto e : appGraph.allEdges()) {
    def->connect(e.dataDest, e.dataSrc);
  }

  cout << "Wiring up function input valids" << endl;
  for (auto v : appGraph.allVertices()) {
    cout << "Wiring up valids for " << coreStr(v) << endl;
    if (!appGraph.hasInputs(v)) {
      if (isa<Interface>(getBase(v))) {
        continue;
      }
    }

    cout << "Wiring up enables for " << coreStr(v) << endl;

    vector<Wireable*> allEnables =
      appGraph.allInputValids(v);

    Wireable* inEnable = appGraph.inEnable(v);

    cout << "Creating and list for enables" << endl;
    auto wEn = andList(def, allEnables);
    cout << "Connecting " << coreStr(inEnable) << " to " << coreStr(wEn) << endl;
    def->connect(inEnable, wEn);
  }

}

class MappedDAG {
  public:
    CoreIR::Module* mappedModule;
    std::map<std::string, int> operationCycleLatencies;
};

// Right now this function is a no-op, eventually it will
// serialize each un-mapped module to coreir json, call the
// mapper for the CGRA to figure out the mappeddag and cycle latencies
std::vector<MappedDAG> callHWMapper(std::vector<CoreIR::Module*>& unmappedModules) {
  // TODO: Serialize unmapped
  // TODO: Call python mapper on mapped file
  // TODO: De-serialize resulting mapped coreir
  return {};
}

std::map<const For*, HWFunction> mapFunctionsToCGRA(std::map<const For*, HWFunction>& functions) {
  // TODO: Build map from strings to functions, create naive coreir for each function
  // then emit that naive coreir to a json file, invoke the mapper, call the python code for
  // the mapper, deserialize the mapper results from json, and return the mapped applications
  return functions;
}

void printCollectedStores(StoreCollector& stCollector) {
    for (auto s : stCollector.stores) {
      cout << "Store to " << s->name << " with value " << s->value << " at " << s->index << endl;
    }

    for (auto s : stCollector.constStores) {
      cout << "Constant stores to " << s.first << endl;
      for (auto sc : s.second) {
        cout << "\t[" << sc.first << "] = " << sc.second << endl;
      }
    }
}

class AcceleratorInterface {
  public:
    CoreIR::Type* designType;

    std::map<std::string, std::string> inputAliases;
    std::map<std::string, std::string> outputAliases;
    string output_name_real;
    string output_name;
};

// How much of this do I need in order to build a test rig that
// can handle multi-dimensional outputs and inputs
AcceleratorInterface topLevelType(CoreIR::Context* context,
    HardwareInfo& hwInfo,
    const std::vector<CoreIR_Argument>& args) {

    uint num_inouts = 0;

    // Keep track of the inputs, output, and taps for this module
    std::vector<std::pair<string, CoreIR::Type*>> input_types;
    std::map<string, CoreIR::Type*> tap_types;
    CoreIR::Type* output_type = context->Bit();

    AcceleratorInterface interface;
    string output_name = "";
    string output_name_real = "";
    std::map<std::string, std::string> inputAliases;
    std::map<std::string, std::string> outputAliases;

    if (hwInfo.interfacePolicy == HW_INTERFACE_POLICY_TOP) {
      for (size_t i = 0; i < args.size(); i++) {
        string arg_name = "arg_" + std::to_string(i);
        string arg_name_real = args[i].name;
        cout << "Arg " << i << " has name " << arg_name_real << endl;

        if (!(args[i].is_output)) {
          inputAliases[arg_name_real] = arg_name;
        }

        if (args[i].is_stencil) {
          Stencil_Type stype = args[i].stencil_type;

          vector<uint> indices;
          for(const auto &range : stype.bounds) {
            internal_assert(is_const(range.extent));
            indices.push_back(func_id_const_value(range.extent));
          }

          if (args[i].is_output && args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream) {
            // add as the outputrg
            uint out_bitwidth = c_inst_bitwidth(stype.elemType.bits());
            if (out_bitwidth > 1) { output_type = output_type->Arr(out_bitwidth); }
            for (uint i=0; i<indices.size(); ++i) {
              output_type = output_type->Arr(indices[i]);
            }
            //hw_output_set.insert(arg_name);
            output_name = "out";
            output_name_real = coreirSanitize(args[i].name);

          } else if (!args[i].is_output && args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream) {
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

      interface.output_name_real = output_name_real;
      interface.output_name = output_name;
      interface.inputAliases = inputAliases;
      interface.outputAliases = outputAliases;
      interface.designType = design_type;

      return interface;
    } else {
      internal_assert(hwInfo.interfacePolicy == HW_INTERFACE_POLICY_COMPUTE_UNIT);
      vector<pair<string, CoreIR::Type* > > tps;

      for (auto arg : args) {
        if (arg.is_stencil) {
          auto stype = arg.stencil_type;

          vector<uint> indices;
          for(const auto &range : stype.bounds) {
            internal_assert(is_const(range.extent));
            indices.push_back(func_id_const_value(range.extent));
          }

          if (arg.is_output) {
            uint out_bitwidth = c_inst_bitwidth(stype.elemType.bits());
            internal_assert(out_bitwidth > 0);

            CoreIR::Type* output_type = out_bitwidth > 1 ? context->Bit()->Arr(out_bitwidth) : context->Bit();
            for (uint i=0; i<indices.size(); ++i) {
              output_type = output_type->Arr(indices[i]);
            }
            string output_name_real = coreirSanitize(arg.name);
            tps.push_back({output_name_real, output_type});

          } else {
            uint in_bitwidth = c_inst_bitwidth(stype.elemType.bits());
            CoreIR::Type* input_type = in_bitwidth > 1 ? context->BitIn()->Arr(in_bitwidth) : context->BitIn();
            for (uint i=0; i<indices.size(); ++i) {
              input_type = input_type->Arr(indices[i]);
            }
            tps.push_back({coreirSanitize(arg.name), input_type});
          }
        } else {
          // TODO: Actually handle non-stencil arguments
        }
      }
      
      CoreIR::Type* design_type;

      design_type = context->Record(tps);
      interface.output_name_real = output_name_real;
      interface.output_name = output_name;
      interface.inputAliases = inputAliases;
      interface.outputAliases = outputAliases;
      interface.designType = design_type;

      return interface;
    }

}

AppGraph insertDelays(AppGraph& appGraph) {

  CoreIR::Context* context = std::begin(appGraph.values)->first->getContext();
  CoreIR::ModuleDef* def = std::begin(appGraph.values)->first->getContainer();
  AppGraph delayedGraph;
  for (auto w : appGraph.values) {
    delayedGraph.addVertex(w.first);
  }

  //map<edisc, Instance*> edgeDelays;
  for (auto n : appGraph.extraDelaysNeeded) {
    if (n.second != 0) {
      auto kEdge = appGraph.getLabel(n.first);

      auto delay = def->addInstance("delay_" + context->getUnique(),
          "halidehw.shift_register",
          {{"type", COREMK(context, kEdge.dataSrc->getType())}, {"delay", COREMK(context, n.second)}});
          //{{"type", COREMK(context, kEdge.dataSrc->getType())}, {"delay", COREMK(context, 0)}});
      cout << "\tCreated delay = " << CoreIR::toString(*delay) << endl;
      delayedGraph.addVertex(delay);
      appGraph.edgeDelays[n.first] = delay;
    }
  }

  // Now: For each edge in the original graph:
  // If the edge needs a delay that is not zero:
  // add an edge from current graph to delay, and
  // from delay to next node
  // Else: Copy the edge from appGraph to delayedGraph
  for (auto e : appGraph.appGraph.getEdges()) {
    int delay = map_get(e, appGraph.extraDelaysNeeded);
    auto origSrc = appGraph.source(e);
    auto origDst = appGraph.target(e);
    auto origEdge = appGraph.getLabel(e);

    auto newSrcV = delayedGraph.getVdisc(origSrc);
    auto newDstV = delayedGraph.getVdisc(origDst);
    
    auto newSrc = delayedGraph.getNode(newSrcV);
    auto newDst = delayedGraph.getNode(newDstV);
    
    if (delay == 0) {
      // Copy edge
      auto newEdge = delayedGraph.addEdge(newSrc, newDst);
      KernelEdge labelCopy;
      labelCopy = origEdge;

      delayedGraph.addEdgeLabel(newEdge, labelCopy);
    } else {
      cout << "Finding delay for " << e << endl;
      Wireable* newD = map_find(e, appGraph.edgeDelays);
      cout << "Found delay for " << e << endl;

      auto s2d = delayedGraph.addEdge(newSrc, newD);
      auto d2d = delayedGraph.addEdge(newD, newDst);

      cout << "Added new edges" << endl;

      KernelEdge srcToDelay;
      KernelEdge delayToDst;
      
      srcToDelay.dataSrc = origEdge.dataSrc;
      srcToDelay.valid = origEdge.valid;
      srcToDelay.dataDest = newD->sel("in_data");
      srcToDelay.en = newD->sel("in_en");

      delayToDst.dataSrc = newD->sel("out_data");
      delayToDst.valid = newD->sel("valid");
      delayToDst.dataDest = origEdge.dataDest;
      delayToDst.en = origEdge.en;

      delayedGraph.addEdgeLabel(s2d, srcToDelay);
      delayedGraph.addEdgeLabel(d2d, delayToDst);
    }
  }

  return delayedGraph;
}

enum StreamSource {
  STREAM_SOURCE_ARG,
  STREAM_SOURCE_LOOP,
  STREAM_SOURCE_LB
};

class StreamNode {
  public:
    StreamSource ss;
    const For* lp;
    CoreIR_Argument arg;
    std::string lbName;
    CoreIR::Wireable* wire;

    StreamNode() : wire(nullptr) {}

    void setWireable(Wireable* w) {
      wire = w;
    }

    bool isLoopNest() const { return ss == STREAM_SOURCE_LOOP; }
    bool isArgument() const { return ss == STREAM_SOURCE_ARG; }

    CoreIR::Wireable* getWireable() const {
      internal_assert(wire != nullptr) << "wire is null in getWireable\n";
      if (ss == STREAM_SOURCE_ARG) {
        return wire;
      }

      if (ss == STREAM_SOURCE_LOOP) {
        return wire;
      }

      if (ss == STREAM_SOURCE_LB) {
        return wire;
      }
      internal_assert(false);
      return nullptr;
    }

    std::string toString() const {
      if (ss == STREAM_SOURCE_ARG) {
        return arg.name;
      }

      if (ss == STREAM_SOURCE_LOOP) {
        return lp->name;
      }

      if (ss == STREAM_SOURCE_LB) {
        return lbName;
      }
      internal_assert(false);
      return "ERROR";
    }
};

StreamNode lbNode(const std::string& name) {
  StreamNode node;
  node.ss = STREAM_SOURCE_LB;
  node.lbName = name;
  return node;
}

StreamNode argNode(CoreIR_Argument& arg) {
  StreamNode node;
  node.ss = STREAM_SOURCE_ARG;
  node.arg = arg;
  return node;
}

StreamNode nestNode(const For* lp) {
  StreamNode node;
  node.ss = STREAM_SOURCE_LOOP;
  node.lp = lp;
  return node;
}

bool operator==(const StreamNode& x, const StreamNode& y) {
  if (x.ss != y.ss) {
    return false;
  }
  if (x.ss == STREAM_SOURCE_LOOP) {
    return x.lp == y.lp;
  }
  if (x.ss == STREAM_SOURCE_ARG) {
    return x.arg.name == y.arg.name;
  }
  if (x.ss == STREAM_SOURCE_LB) {
    return x.lbName == y.lbName;
  }
  internal_assert(false) << "Cannot compare 2 StreamNodes\n";
  return false;
}

class StreamEdge {
  public:
    std::string streamName;
    vector<string> dispatchParams;
};

class CallCollector : public IRGraphVisitor {
  public:
    std::string targetName;
    std::set<const Call*> calls;

    CallCollector(const std::string target_) : targetName(target_) {}

  protected:
    using IRGraphVisitor::visit;
    void visit(const Call* c) override {
      if (c->name == targetName) {
        calls.insert(c);
      }
    }
};

Wireable* dataValid(StreamNode& src) {
  if (isComputeKernel(src.getWireable())) {
    return src.getWireable()->sel("valid");
  }

  if (isLinebuffer(src.getWireable())) {
    return src.getWireable()->sel("valid");
  }

  internal_assert(isa<Interface>(getBase(src.getWireable())));
  return getBase(src.getWireable())->sel("in_en");
}

Wireable* dataEn(StreamNode& dest, const std::string& stream) {
  if (isComputeKernel(dest.getWireable())) {
    return dest.getWireable()->sel("in_en");
  }
  if (isLinebuffer(dest.getWireable())) {
    return dest.getWireable()->sel("wen");
  }

  return getBase(dest.getWireable())->sel("valid");
  //internal_assert(false);
}

Wireable* dataIn(StreamNode& src, const std::string& stream) {
  if (isComputeKernel(src.getWireable())) {
    auto inst = static_cast<Instance*>(src.getWireable());
    return inst->sel(coreirSanitize(stream));
  }

  if (isLinebuffer(src.getWireable())) {
    return src.getWireable()->sel("in");
  }

  return src.getWireable();
  //internal_assert(false);
}

Wireable* dataOut(StreamNode& src, std::string& stream) {
  if (isLinebuffer(src.getWireable())) {
    return src.getWireable()->sel("out");
  }

  if (isComputeKernel(src.getWireable())) {
    cout << "Selecting " << stream << endl;
    return src.getWireable()->sel(coreirSanitize(stream));
  }

  return src.getWireable();
}

std::set<const Call*> collectCalls(const std::string& name, const Stmt& stmt) {
  CallCollector c(name);
  stmt.accept(&c);
  return c.calls;
}

std::string stripQuotes(const std::string& str) {
  string nStr = "";
  for (auto c : str) {
    if (c == '\"') {
    } else {
      nStr += c;
    }
  }
  return nStr;
}

class StreamSubset {
  public:
    int numDims;
    vector<int> usedExtents;
    vector<int> actualExtents;

    vector<int> usedOffsets() const {
      vector<int> off;
      for (int i = 0; i < numDims; i++) {
        off.push_back(usedExtents[2*i]);
      }
      return off;
    }

    vector<int> getUsedDims() const {
      vector<int> dims;
      for (size_t i = 0; i < usedExtents.size(); i += 2) {
        dims.push_back(usedExtents.at(i + 1));
      }
      return dims;
    }

    vector<int> getActualDims() const {
      vector<int> dims;
      for (size_t i = 2; i < actualExtents.size(); i += 3) {
        dims.push_back(actualExtents.at(i));
      }
      return dims;
    }
};

StreamSubset findDispatch(std::string& streamStr, const std::string& dispatchName, StencilInfo& info) {
  auto sd = map_get(streamStr, info.streamDispatches);
  internal_assert(sd.size() > 0);
  int numDims = stoi(sd[0]);
  int numDimParams = 1 + 3*numDims;
  vector<int> dimParams;
  for (int i = 1; i < numDimParams; i++) {
    dimParams.push_back(stoi(sd[i]));
  }

  int numReceivers = stoi(sd[numDimParams]);
  //cout << "\t\t# receivers = " << numReceivers << endl;
  int paramsPerReceiver = 1 + 1 + 2*numDims;
  int receiverOffset = numDimParams + 1;
  //cout << "\t\t# receiver params = " << paramsPerReceiver << endl;
  for (int r = 0; r < numReceivers; r++) {
    int rStart = receiverOffset + r*paramsPerReceiver;
    //cout << "\t\t\tReceiver " << r << " is " << sd.second[rStart] << ", with buffer size = " << sd.second[rStart + 1];
    if (sd[rStart] == dispatchName) {
      vector<int> params;
      for (int p = rStart + 2; p < rStart + paramsPerReceiver; p += 2) {
        cout << "Getting dispatch parameter " << exprString(sd[p]) << endl;
        params.push_back(stoi(stripQuotes(exprString(sd[p]))));
        params.push_back(stoi(stripQuotes(exprString(sd[p + 1]))));
      }
      return {numDims, params, dimParams};
    }
  }

  internal_assert(false) << "No dispatch for " << streamStr << " to " << dispatchName << "\n";
  return {};
   //" in " << info.streamDispatches << "\n";
}

class StreamUseInfo {
  public:
    vector<pair<StreamNode, StreamSubset> > readers;
    StreamNode writer;
};


vdisc getStreamNode(StreamNode& target, DirectedGraph<StreamNode, StreamSubset>& streamGraph) {
  for (auto node : streamGraph.getVertNames()) {
    if (node.second == target) {
      return node.first;
    }
  }
  cout << "Error: No node for " << target.toString() << endl;
  internal_assert(false);
  return 0;
}

map<string, StreamUseInfo> createStreamUseInfo(std::map<const For*, HWFunction>& functions,
    std::map<const For*, ComputeKernel>& kernelModules,
    std::map<const For*, CoreIR::Instance*>& kernels,
    const std::vector<CoreIR_Argument>& args,
    AcceleratorInterface& ifc,
    StencilInfoCollector& scl,
    DirectedGraph<StreamNode, StreamSubset>& streamGraph) {
  map<string, StreamUseInfo> streamUseInfo;
  for (auto a : args) {
    if (a.is_stencil) {
      cout << "\t" << a.name << endl;
      StreamNode aN = argNode(a);
      streamGraph.addVertex(aN);
      if (!a.is_output) {
        streamUseInfo[a.name].writer = argNode(a);
      } else {
        streamUseInfo[a.name].readers.push_back({argNode(a), {}});
      }
    }
  }
  std::map<const For*, std::set<const Call*> > streamReads;
  std::map<const For*, std::set<const Call*> > streamWrites;
  for (auto f : functions) {
    std::set<const Call*> readStreams = collectCalls("read_stream", f.first->body);
    std::set<const Call*> writeStreams = collectCalls("write_stream", f.first->body);
    streamReads[f.first] = readStreams;
    streamWrites[f.first] = writeStreams;
    StreamNode forNode = nestNode(f.first);
    streamGraph.addVertex(forNode);
  }

  for (auto lb : scl.info.linebuffers) {
    StreamNode lbN = lbNode(lbName(lb));
    //lb[0] + "_to_" + lb[1]);
    streamGraph.addVertex(lbN);
  }

  cout << "Stream reads" << endl;
  for (auto r : streamReads) {
    cout << "\tLoop..." << endl;
    for (auto rd : r.second) {
      auto stencilName = rd->args[1];
      auto streamName = rd->args[0];
      auto dispatchString = rd->args[2];
      cout << "\t\tstream = " << streamName << ", dispatch str = " << dispatchString << endl;
      string dispatchName = exprString(dispatchString);
      string streamStr = exprString(streamName);
      StreamSubset dispatchParams = findDispatch(streamStr, dispatchName, scl.info);
      StreamNode node = nestNode(r.first);
      streamUseInfo[streamStr].readers.push_back({node, dispatchParams});
    }
  }

  cout << "Stream writes" << endl;
  for (auto r : streamWrites) {
    cout << "\tLoop..." << endl;
    for (auto wr : r.second) {
      cout << "\t\tstream = " << wr->args[0] << endl;
      streamUseInfo[exprString(wr->args[0])].writer = nestNode(r.first);
    }
  }

  cout << "Stream writes by linebuffers" << endl;
  for (auto lb : scl.info.linebuffers) {
    cout << "\t" << lb[1] << endl;
    streamUseInfo[lb[1]].writer = lbNode(lbName(lb));
    cout << "\t" << lb[0] << endl;
    streamUseInfo[lb[0]].readers.push_back({lbNode(lbName(lb)), {}});
  }

  cout << "Checking dispatch statements" << endl;
  for (auto sd : scl.info.streamDispatches) {
    cout << "\t" << sd.first << " -> " << sd.second << endl;
    internal_assert(sd.second.size() > 0);
    int numDims = stoi(sd.second[0]);
    cout << "\t\t# dims = " << numDims << endl;
    int numDimParams = 1 + 3*numDims;
    vector<int> dimParams;
    for (int i = 1; i < numDimParams; i++) {
      dimParams.push_back(stoi(sd.second[i]));
    }
    cout << "\t\tdim params = " << dimParams << endl;

    int numReceivers = stoi(sd.second[numDimParams]);
    cout << "\t\t# receivers = " << numReceivers << endl;
    int paramsPerReceiver = 1 + 1 + 2*numDims;
    int receiverOffset = numDimParams + 1;
    cout << "\t\t# receiver params = " << paramsPerReceiver << endl;
    for (int r = 0; r < numReceivers; r++) {
      int rStart = receiverOffset + r*paramsPerReceiver;
      cout << "\t\t\tReceiver " << r << " is " << sd.second[rStart] << ", with buffer size = " << sd.second[rStart + 1];
      for (int p = rStart + 2; p < rStart + paramsPerReceiver; p += 2) {
        cout << " offset = " << sd.second[p] << ", extent = " << sd.second[p + 1];
      }
      cout << endl;
    }
  }


  return streamUseInfo;
}

// Now: I want to incorporate information about how streams are dispatched
// (offsets and extents) so that I can insert hedgetrimmer elements that
// strip portions of the stream output away.
AppGraph buildAppGraph(std::map<const For*, HWFunction>& functions,
    std::map<const For*, ComputeKernel>& kernelModules,
    std::map<const For*, CoreIR::Instance*>& kernels,
    const std::vector<CoreIR_Argument>& args,
    AcceleratorInterface& ifc,
    StencilInfoCollector& scl) {

  DirectedGraph<StreamNode, StreamSubset> streamGraph;

  cout << "All args" << endl;

  map<string, StreamUseInfo> streamUseInfo =
    createStreamUseInfo(functions, kernelModules, kernels, args, ifc, scl, streamGraph);
  
  auto inputAliases = ifc.inputAliases;
  auto output_name = ifc.output_name;
  auto output_name_real = ifc.output_name_real;
  int bitwidth = 16;

  internal_assert(kernels.size() > 0) << "kernels size = " << kernels.size() << "\n";

  CoreIR::ModuleDef* def = std::begin(kernels)->second->getContainer();
  CoreIR::Context* context = def->getContext();
  // Challenges for refactoring here:
  // No mapping from linebuffers to linebuffer nodes (solved)
  // Creating new edges is a tedious process
  // Im not sure how we are going to compute new delays from streamsubsets
  // DAG data structure API is not exactly what I want
  std::map<string, CoreIR::Instance*> linebufferResults;
  std::map<string, CoreIR::Instance*> linebufferInputs;
  std::set<CoreIR::Instance*> linebuffers;
  createLinebuffers(context, def, bitwidth, linebufferResults, linebufferInputs, linebuffers, scl);

  for (auto& streamUse : streamUseInfo) {
    auto& info = streamUse.second;
    auto& writer = info.writer;

    Wireable* base = nullptr;
    if (writer.ss == STREAM_SOURCE_ARG) {
      if (!writer.arg.is_output) {
        string coreirName = CoreIR::map_find(writer.arg.name, inputAliases);
        auto s = def->sel("self")->sel("in")->sel(coreirName);
        base = s;
      } else {
        auto s = def->sel("self")->sel(output_name);
        base = s;
      }
    } else if (writer.ss == STREAM_SOURCE_LB) {
      for (auto inst : linebuffers) {
        if (inst->getInstname() == writer.lbName) {
          base = static_cast<Wireable*>(inst);
          break;
        }
      }
      internal_assert(base != nullptr) << "No linebuffer instance for " << writer.lbName << "\n";
    } else if (writer.ss == STREAM_SOURCE_LOOP) {
      // Add kernel instance
      base = map_find(writer.lp, kernels);
    }
    internal_assert(base != nullptr);

    writer.setWireable(base);
   
    for (auto& readers : info.readers) {
      auto& writer = readers.first;
      Wireable* base = nullptr;
      if (writer.ss == STREAM_SOURCE_ARG) {
        if (!writer.arg.is_output) {
          string coreirName = CoreIR::map_find(writer.arg.name, inputAliases);
          auto s = def->sel("self")->sel("in")->sel(coreirName);
          base = s;
        } else {
          auto s = def->sel("self")->sel(output_name);
          base = s;
        }
      } else if (writer.ss == STREAM_SOURCE_LB) {
        for (auto inst : linebuffers) {
          if (inst->getInstname() == writer.lbName) {
            base = static_cast<Wireable*>(inst);
            break;
          }
        }
        internal_assert(base != nullptr) << "No linebuffer instance for " << writer.lbName << "\n";
      } else if (writer.ss == STREAM_SOURCE_LOOP) {
        // Add kernel instance
        base = map_find(writer.lp, kernels);
      }
      internal_assert(base != nullptr);

      writer.setWireable(base);
    }
  }

  AppGraph appGraph;
  appGraph.kernelModules = kernelModules;
  std::map<vdisc, Wireable*> streamNodeMap;
  for (auto node : streamGraph.getVertNames()) {
    vdisc v = node.first;
    StreamNode n = node.second;
    if (n.ss == STREAM_SOURCE_ARG) {
      if (!n.arg.is_output) {
        string coreirName = CoreIR::map_find(n.arg.name, inputAliases);
        auto s = def->sel("self")->sel("in")->sel(coreirName);
        appGraph.addVertex(s);
        streamNodeMap[v] = s;
      } else {
        auto s = def->sel("self")->sel(output_name);
        appGraph.addVertex(s);
        //def->sel("self")->sel(output_name));
        streamNodeMap[v] = s;
      }
    } else if (n.ss == STREAM_SOURCE_LB) {
      // Create linebuffer
      Wireable* w = nullptr;
      for (auto inst : linebuffers) {
        if (inst->getInstname() == n.lbName) {
          w = static_cast<Wireable*>(inst);
          break;
        }
      }
      internal_assert(w != nullptr) << "No linebuffer instance for " << n.lbName << "\n";
     
      streamNodeMap[v] = w;
      appGraph.addVertex(w);
    } else if (n.ss == STREAM_SOURCE_LOOP) {
      // Add kernel instance
      streamNodeMap[v] = map_find(n.lp, kernels);
      appGraph.addVertex(map_find(n.lp, kernels));
    }
  }

  for (auto streamInfo : streamUseInfo) {
    string stream = streamInfo.first;
    auto writerNode = streamInfo.second.writer;
    for (auto reader : streamInfo.second.readers) {
      StreamNode readerNode = reader.first;
      StreamSubset subset = reader.second;

      // TODO: Check stream subsets and see if the connecting edge
      // needs to use a trimmer
      internal_assert(subset.numDims == 0 || subset.numDims == 2 || subset.numDims == 3);
      Wireable* src = writerNode.getWireable();
      Wireable* dest = readerNode.getWireable();

      cout << "Edge from " << coreStr(src) << " to " << coreStr(dest) << endl;
      cout << "\tOriginal extents = " << subset.actualExtents << endl;
      cout << "\tUsed extents     = " << subset.usedExtents << endl;

      // Need to check whether the edge uses all data produced by the streamnode
      bool needTrimmer = false;
      if (subset.numDims == 0) {
      } else {
        // Check if all dims are used
        vector<int> fullExtents = subset.getActualDims();
        vector<int> usedExtents = subset.getUsedDims();
        cout << "Full extents: " << fullExtents << ", used extents " << usedExtents << endl;
        if (fullExtents != usedExtents) {
          vector<int> diffs;
          for (size_t i = 0; i < fullExtents.size(); i++) {
            diffs.push_back(fullExtents.at(i) - usedExtents.at(i));
          }
          cout << "\tDiffs = " << diffs << endl;
          vector<int> offsets = subset.usedOffsets();
          cout << "\tOffsets = " << offsets << endl;
          bool allZero = true;
          for (auto off : offsets) {
            if (off != 0) {
              allZero = false;
              break;
            }
          }
          if (!allZero) {
            needTrimmer = true;
          }
        }
      }

      if (needTrimmer) {
        auto trimInTp = dataOut(writerNode, stream)->getType();
        // TODO: Actually compute the delay value from diffs, extents, and traversal order
        int delayNeeded = 20;
        auto trimmer = def->addInstance("trimmer_" + def->getContext()->getUnique(), "halidehw.stream_trimmer", {{"type", COREMK(context, trimInTp)}, {"delay", COREMK(context, delayNeeded)}});
        appGraph.addVertex(trimmer);
        
        KernelEdge toTrimmer;
        toTrimmer.dataSrc = dataOut(writerNode, stream);
        toTrimmer.valid = dataValid(writerNode);
        toTrimmer.dataDest = trimmer->sel("in");
        toTrimmer.en = trimmer->sel("en");

        KernelEdge fromTrimmer;
        fromTrimmer.dataSrc = trimmer->sel("out");
        fromTrimmer.valid = trimmer->sel("valid");
        fromTrimmer.dataDest = dataIn(readerNode, stream);
        fromTrimmer.en = dataEn(readerNode, stream);

        auto ed = appGraph.addEdge(src, trimmer);
        cout << "Trimmer edge ed: " << ed << endl;
        appGraph.addEdgeLabel(ed, toTrimmer);

        auto ed1 = appGraph.addEdge(trimmer, dest);
        cout << "Trimmer edge ed1: " << ed1 << endl;
        appGraph.addEdgeLabel(ed1, fromTrimmer);

        internal_assert(appGraph.hasLabel(ed1));
      } else {
        KernelEdge e;
        e.dataSrc = dataOut(writerNode, stream);
        e.valid = dataValid(writerNode);

        e.dataDest = dataIn(readerNode, stream);
        e.en = dataEn(readerNode, stream);

        auto ed = appGraph.addEdge(src, dest);
        appGraph.addEdgeLabel(ed, e);
      }
    }
  }

  cout << "Application graph..." << endl;
  cout << appGraph.toString() << endl;

  return appGraph;
}

void computeDelaysForAppGraph(AppGraph& appGraph) {

  auto topSort = topologicalSort(appGraph.appGraph);
  
  // Map from nodes to the times at which first valid from that node is emitted
  std::map<vdisc, int> nodesToDelays;
  for (auto v : topSort) {
    vector<edisc> inEdges = appGraph.appGraph.inEdges(v);

    int maxDist = 0;
    edisc maxEdge = 0;
    bool setMaxEdge = false;
    for (auto e : inEdges) {
      int distToSrc = cycleDelay(e, appGraph);

      if (distToSrc >= maxDist) {
        maxDist = distToSrc;
        maxEdge = e;
        setMaxEdge = true;
      }
      
    }

    bool allDelaysSame = true;
    if (inEdges.size() == 0) {
      // All delays are trivially the same
    } else {
      for (size_t i = 0; i < inEdges.size() - 1; i++) {
        int distToSrc = cycleDelay(inEdges[i], appGraph);
        
        int distToSrc1 = cycleDelay(inEdges[i + 1], appGraph);
        if (distToSrc != distToSrc1) {
          allDelaysSame = false;
          break;
        }
      }
    }

    if (allDelaysSame) {
      for (auto e : inEdges) {
        appGraph.extraDelaysNeeded[e] = 0;
      }
    } else {

      for (auto e : inEdges) {
        if (setMaxEdge && (e == maxEdge)) {
          appGraph.extraDelaysNeeded[e] = 0;
        } else {
          auto srcVert = appGraph.appGraph.source(e);
          int distToSrc = map_get(srcVert, nodesToDelays) + cycleDelay(e, appGraph);
          int filler = maxDist - distToSrc;
          appGraph.extraDelaysNeeded[e] = filler;
        }
      }

    }

    nodesToDelays[v] = maxDist;
  }

  internal_assert(nodesToDelays.size() == topSort.size()) << "some nodes did not get a computed delay\n";

}

class UselessReadRemover : public IRMutator {
  public:
    using IRMutator::visit;

    Stmt visit(const For* lp) {
      // This code section is a hack to deal with insertion of
      // provides in unified buffer code that should not be there
      CallCollector cc("read_stream");
      lp->body.accept(&cc);

      ProvideCollector pc;
      lp->body.accept(&pc);

      set<const Call*> callsToRemove;
      for (auto rd : cc.calls) {
        string name = exprString(rd->args[1]);
        cout << "\tRead: " << name << endl;

        bool remove = false;
        for (auto provide : pc.provides) {
          if (provide->name == name) {
            remove = true;
            break;
          }
        }
        if (remove) {
          callsToRemove.insert(rd);
        }
      }

      cout << "Should remove..." << endl;
      for (auto c : callsToRemove) {
        cout << "\t" << c->name << "(" << c->args[0] << ", " << c->args[1] << ")" << endl;
      }

      CallRemover cr;
      cr.toErase = callsToRemove;
      auto newBody = cr.mutate(lp->body);
      cout << "New body.." << endl;
      cout << newBody << endl;

      return For::make(lp->name, lp->min, lp->extent, lp->for_type, lp->device_api, newBody);

    }
};

void flattenExcluding(ModuleDef* def, vector<string>& generatorNames) {
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto instP : def->getInstances()) {
      bool fromAnyGen = false;
      for (auto g : generatorNames) {
        if (fromGenerator(g, instP.second)) {
          fromAnyGen = true;
          break;
        }
      }

      if (!fromAnyGen) {
        //cout << "Instance: " << coreStr(instP.second) << " is not any of: " << generatorNames << endl;
        changed = inlineInstance(instP.second);
        if (changed) {
          break;
        }
      }
    }
  }

}

void flattenExcluding(CoreIR::Context* c, vector<string>& generatorNames) {
  for (auto ns : c->getNamespaces()) {
    for (auto m : ns.second->getModules()) {
      if (m.second->hasDef()) {
        flattenExcluding(m.second->getDef(), generatorNames);
        //cout << m.first << "After selective flattening..." << endl;
        //m.second->print();
      }
    }
  }
}

// Heuristic method to hit the critical path: Add a register in front of
// every operation that has a critical path that is "large", meaning at
// least 1/3 of the critical path in this case
void insertCriticalPathTargetRegisters(HardwareInfo& hwInfo, HWFunction& f) {
  if (!hwInfo.hasCriticalPathTarget) {
    return;
  }
  int cp = hwInfo.criticalPathTarget;
  set<HWInstr*> delayInsertionSites;
  for (auto instr : f.structuredOrder()) {
    if (hwInfo.criticalPath(instr->name) > (cp / 3)) {
      delayInsertionSites.insert(instr);
    }
  }

  std::map<HWInstr*, HWInstr*> delayRegister;
  for (auto s : delayInsertionSites) {
    auto delay = f.newI();
    delay->name = "delay";
    delay->latency = 1;
    delay->surroundingLoops = s->surroundingLoops;
    delay->resType = f.mod->getContext()->Bit()->Arr(16);
    delayRegister[s] = delay;
  }

  for (auto s : delayRegister) {
    f.insertAfter(s.first, s.second);
    f.replaceAllUsesWith(s.first, s.second);
  }

  for (auto s : delayRegister) {
    s.second->operands.push_back(s.first);
  }

  // Now: For each delay register: insert it after its replacement
  // then: replace all uses of target with delay register
  // then: connect each delay

  if (delayRegister.size() > 0) {
    cout << "Function after delay register insertion..." << endl;
    cout << f << endl;
  }
}

CoreIR::Module* createCoreIRForStmt(CoreIR::Context* context,
    HardwareInfo& hwInfo,
    Stmt stmt,
    const std::string& name,
    const vector<CoreIR_Argument>& args) {

  cout << "Creating coreir for" << endl;
  cout << stmt << endl;

  cout << "All args" << endl;
  for (auto a : args) {
    cout << "\t" << a.name << endl;
  }

  // Build top level interface
  AcceleratorInterface ifc = topLevelType(context, hwInfo, args);
  auto inputAliases = ifc.inputAliases;
  auto output_name_real = ifc.output_name_real;
  auto output_name = ifc.output_name;
  auto topType = ifc.designType;
  auto global_ns = context->getNamespace("global");

  // Here: Find all external vars and then replace them with dummies
  // TODO: Move to preprocess hardware loops, and eliminate when we
  // need to handle rea parameter passing
  std::map<string, Expr> dummyVars;
  for (auto a : args) {
    if (!a.is_stencil) {
      dummyVars[a.name] = IntImm::make(a.scalar_type, 0);
    }
  }
  stmt = substitute(dummyVars, stmt);

  if (hwInfo.interfacePolicy == HW_INTERFACE_POLICY_COMPUTE_UNIT) {
    CoreIR::Module* topMod = global_ns->newModuleDecl(name, topType);
    cout << "Emitting kernel for " << name << endl;
    cout << "\tStmt is = " << endl;
    cout << stmt << endl;
    NestExtractor extractor;
    stmt.accept(&extractor);
    internal_assert(extractor.loops.size() == 0);


    auto def = topMod->newModuleDef();

    StencilInfoCollector scl;
    StoreCollector stCollector;

    HWFunction f =
      buildHWBody(context,
          hwInfo,
          scl.info,
          "compute_kernel",
          stmt,
          args,
          stCollector);

    ComputeKernel compK = moduleForKernel(scl.info, f);

    auto compute_mod = def->addInstance("compute_kernel", compK.mod);
    topMod->setDef(def);

    auto self = def->sel("self");
    for (auto a : args) {
      if (a.is_stencil) {
        def->connect(self->sel(coreirSanitize(a.name)), compute_mod->sel(coreirSanitize(a.name)));
      }
    }
    cout << "Top module for compute kernel..." << endl;
    topMod->print();

    cout << "Compute unit..." << endl;
    compK.mod->print();
    topMod->setDef(def);

    //internal_assert(false);
    return topMod;
  } else {
    CoreIR::Module* topMod = global_ns->newModuleDecl("DesignTop", topType);
    cout << "Before creating definition.." << endl;
    topMod->print();
    // Maybe what I should do is build a json file here which
    // stores the alias maps as well as the stencil types
    // and then use that to build a test case which automatically
    // wraps up the input and output types?
    Json aliasInfo;
    auto& a = aliasInfo["aliasMap"];
    for (auto al : ifc.inputAliases) {
      a[al.first] = al.second;
    }

    auto& oname = aliasInfo["outName"];
    oname[output_name_real] = output_name;

    auto& ranges = aliasInfo["ranges"];
    for (auto arg : args) {
      if (arg.is_stencil) {
        string name = arg.name;
        auto& val = ranges[name];
        auto ranges = arg.stencil_type.bounds;
        for (auto r : ranges) {
          val.emplace_back(getConstInt(r.min));
          val.emplace_back(getConstInt(r.extent));
        }
      }
    }
    ofstream interfaceInfo("accel_interface_info.json");
    interfaceInfo << aliasInfo;
    interfaceInfo.close();

    stmt = preprocessHWLoops(stmt);

    cout << "After substitution..." << endl;
    cout << stmt << endl;

    //internal_assert(dummyVars.size() == 0);
    UselessReadRemover readRemover;
    stmt = readRemover.mutate(stmt);

    StoreCollector stCollector;
    stmt.accept(&stCollector);
    printCollectedStores(stCollector);

    cout << "Emitting kernel for " << name << endl;
    cout << "\tStmt is = " << endl;
    cout << stmt << endl;
    NestExtractor extractor;
    stmt.accept(&extractor);

    StencilInfoCollector scl;
    stmt.accept(&scl);

    inferStreamTypes(scl);

    //cout << "Stencil info" << endl;
    StencilInfo info = scl.info;


    cout << "\tAll " << extractor.loops.size() << " loops in design..." << endl;
    int kernelN = 0;
    std::map<const For*, ComputeKernel> kernelModules;
    std::map<const For*, HWFunction> functions;
    for (const For* lp : extractor.loops) {
      cout << "\t\tLOOP" << endl;
      cout << "Original body.." << endl;
      cout << lp->body << endl;

      Stmt lpB = For::make(lp->name, lp->min, lp->extent, lp->for_type, lp->device_api, lp->body);
      // Actual scheduling here
      HWFunction f = buildHWBody(context, hwInfo, scl.info, "compute_kernel_" + std::to_string(kernelN), lpB, args, stCollector);

      functions[lp] = f;

      kernelN++;
    }    

    functions = mapFunctionsToCGRA(functions);
    auto def = topMod->newModuleDef();

    // Connects up all control paths in the design
    std::map<const For*, CoreIR::Instance*> kernels;
    for (auto fp : functions) {
      auto lp = fp.first;
      HWFunction& f = fp.second;
      insertCriticalPathTargetRegisters(hwInfo, f);
      ComputeKernel compK = moduleForKernel(scl.info, f);
      auto m = compK.mod;
      cout << "Created module for kernel.." << endl;
      kernelModules[lp] = compK;

      cout << "Module before optimization" << endl;
      m->print();

      removeUnconnectedInstances(m->getDef());
      removeUnusedInstances(m->getDef());

      cout << "Module after optimization" << endl;
      m->print();
      auto kI = def->addInstance("compute_module_" + m->getName(), m);
      //k.second.mod);
      def->connect(kI->sel("reset"), def->sel("self")->sel("reset"));
      kernels[lp] = kI;
    }

    context->runPasses({"rungenerators"});
    vector<string> generatorNames{"lakelib.unified_buffer", "lakelib.linebuffer", "commonlib.linebuffer", "commonlib.rom2", "memory.rom2"};
    flattenExcluding(context, generatorNames);
    context->runPasses({"deletedeadinstances"});
    cout << "Kernels size before buildAppGraph = " << kernels.size() << endl;
    AppGraph appGraph = buildAppGraph(functions, kernelModules, kernels, args, ifc, scl);
    computeDelaysForAppGraph(appGraph);
    appGraph = insertDelays(appGraph);
    wireUpAppGraph(appGraph, def);

    cout << "Setting definition of topMod..." << endl;
    topMod->setDef(def);
    
    cout << "Top module before inlining" << endl;
    topMod->print();

    context->runPasses({"rungenerators"});

    flattenExcluding(context, generatorNames);
    context->runPasses({"deletedeadinstances"});

    if (!saveToFile(global_ns, "conv_3_3_app.json")) {
      cout << "Could not save global namespace" << endl;
      context->die();
    }

    return topMod;
  }

}

}
}
