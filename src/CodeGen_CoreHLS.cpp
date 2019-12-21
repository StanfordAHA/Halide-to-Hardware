#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>

#include "CodeGen_CoreHLS.h"
#include "CodeGen_Internal.h"
#include "CodeGen_CoreIR_Target.h"
#include "Debug.h"
#include "HWUtils.h"
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

#define COREMK(ctx, v) CoreIR::Const::make((ctx), (v))

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
using CoreIR::split_by;
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

std::string loopLevel(HWInstr* const instr) {
  internal_assert(instr->surroundingLoops.size() > 0);
  return instr->surroundingLoops.back().name;
}

std::string dbgName(HWInstr* const instr) {
  return "dbg_" + to_string(instr->uniqueNum);
}

std::string coreStr(const Wireable* w) {
  return CoreIR::toString(*w);
}

std::string coreStr(const CoreIR::Type* w) {
  return CoreIR::toString(*w);
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

int instructionPosition(HWInstr* instr, HWFunction& f) {
  auto ord = f.structuredOrder();
  return instructionPosition(instr, ord);
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
set<HWInstr*> getUsers(HWInstr* op, const T& program) {
  set<HWInstr*> users;
  for (auto instr : program) {
    for (auto v : instr->operands) {
      if (v == op) {
        users.insert(instr);
      }
    }
  }
  return users;
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

HWInstr* tail(const std::vector<HWInstr*>& instrs) {
  internal_assert(instrs.size() > 0);
  return instrs.back();
}

HWInstr* head(const std::vector<HWInstr*>& instrs) {
  internal_assert(instrs.size() > 0);
  return instrs[0];
}

std::vector<LoopSpec> loops(const std::vector<HWInstr*>& instrs) {
  vector<LoopSpec> loops;
  vector<string> names;
  for (auto i : instrs) {
    for (auto lp : i->surroundingLoops) {
      if (!elem(lp.name, names)) {
        loops.push_back(lp);
        names.push_back(lp.name);
      }
    }
  }
  return loops;
}

std::vector<std::string> loopNames(const std::vector<HWInstr*>& instrs) {
  vector<string> names;
  for (auto i : instrs) {
    for (auto lp : i->surroundingLoops) {
      if (!elem(lp.name, names)) {
        names.push_back(lp.name);
      }
    }
  }
  return names;
}

std::vector<std::string> loopNames(const HWInstr* instr) {
  vector<string> names;
  for (auto l : instr->surroundingLoops) {
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


class FunctionSchedule;

vector<SWTransition> buildSWTransitions(vector<ProgramPosition>& positions, HWFunction& f);
vector<SWTransition> hwTransitions(FunctionSchedule& sched);

template<typename TOut, typename T>
TOut* sc(T* p) {
  return static_cast<TOut*>(p);
}

CoreIR::Values getGenArgs(Wireable* p) {
  internal_assert(isa<Instance>(p));
  auto inst = toInstance(p);
  internal_assert(inst->getModuleRef()->isGenerated());
  return inst->getModuleRef()->getGenArgs();
}

vector<int> arrayDims(CoreIR::Type* tp) {
  internal_assert(isa<CoreIR::ArrayType>(tp));
  auto arrTp = sc<CoreIR::ArrayType>(tp);
  if (!isa<CoreIR::ArrayType>(arrTp->getElemType())) {
    vector<int> dims;
    dims.push_back(arrTp->getLen());
    return dims;
  } else {
    auto tps = arrayDims(arrTp->getElemType());
    tps.push_back(arrTp->getLen());
    return tps;
  }
}

vector<int> arrayDims(CoreIR::Wireable* w) {
  return arrayDims(w->getType());
}

enum StreamSource {
  STREAM_SOURCE_ARG,
  STREAM_SOURCE_LOOP,
  STREAM_SOURCE_LB
};

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
    bool isLinebuffer() const { return ss == STREAM_SOURCE_ARG; }

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

class StreamUseInfo {
  public:
    vector<pair<StreamNode, StreamSubset> > readers;
    StreamNode writer;
};


map<string, StreamUseInfo> createStreamUseInfo(std::map<const For*, HWFunction>& functions,
    const std::vector<CoreIR_Argument>& args,
    StencilInfo& scl);

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

HWInstr* tail(const IBlock& b) {
  if (b.isEntry()) {
    return nullptr;
  }
  return tail(b.instrs);
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

IBlock containerBlock(HWInstr* instr, HWFunction& f) {
  for (auto b : getIBlocks(f)) {
    if (elem(instr, b.instrs)) {
      return b;
    }
  }

  internal_assert(false);
  return *(begin(getIBlocks(f)));
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

bool lastBlock(const IBlock& blk, HWFunction& f) {
  return blk == getIBlockList(f).back();
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
  return !fromGenerator("commonlib.linebuffer", static_cast<Instance*>(b)) &&
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
    return fromGenerator("commonlib.linebuffer", instBase);
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
        //cout << "Found call: " << e->value << endl;
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

}

vector<Wireable*> getSelects(CoreIR::Wireable* w) {
  auto dims = arrayDims(w);
  if (dims.size() == 1) {
    return {w};
  }
  vector<Wireable*> sels;
  for (int i = 0; i < dims.back(); i++) {
    sels.push_back(w->sel(i));
  }
  return sels;
}

vector<Wireable*> flatSelects(CoreIR::Wireable* w) {
  if (arrayDims(w).size() == 1) {
    return {w};
  }

  vector<Wireable*> lSels = getSelects(w);
  vector<Wireable*> finalSels;
  for (auto s : lSels) {
    for (auto sr : flatSelects(s)) {
      finalSels.push_back(sr);
    }
  }
  return finalSels;
}

int bitWidth(CoreIR::Wireable* w) {
  auto tp = w->getType();
  internal_assert(isa<CoreIR::ArrayType>(tp));
  auto arrTp = sc<CoreIR::ArrayType>(tp);
  return arrTp->getLen();
}

void loadHalideLib(CoreIR::Context* context) {
  auto hns = context->newNamespace("halidehw");

  {
    CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("unpack", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto tIn = args.at("type")->get<CoreIR::Type*>();
        int totalWidth = 1;
        vector<int> output_dims = arrayDims(tIn);
        for (auto d : output_dims) {
        totalWidth *= d;
        }
        return c->Record({
            {"in", c->BitIn()->Arr(totalWidth)},
            {"out", tIn},
            });
        });
    auto srGen = hns->newGeneratorDecl("unpack", srTg, srParams);
    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto tIn = args.at("type")->get<CoreIR::Type*>();
        int totalWidth = 1;
        vector<int> output_dims = arrayDims(tIn);
        auto dims = output_dims;
        for (auto d : output_dims) {
        totalWidth *= d;
        }

        auto self = def->sel("self");
        if (totalWidth == dims[0]) {
        auto zeroThOut = self->sel("out");
        for (int i = 0; i < ((int) dims.size()) - 1; i++) {
        zeroThOut = zeroThOut->sel(0);
        }
        def->connect(zeroThOut, self->sel("in"));
        } else {

        vector<Wireable*> arrayElems = flatSelects(self->sel("out"));
        internal_assert(arrayElems.size() > 0);

        auto activeSel = self->sel("in");
        // Inverse of packing is to slice the value off in reverse order?
        for (int i = ((int) arrayElems.size()) - 1; i >= 0; i--) {
        int w = bitWidth(activeSel);
        int arrayW = bitWidth(arrayElems[i]);
        auto cc =
          def->addInstance("unpack_slice_" + c->getUnique(),
              "coreir.slice",
              {{"lo", COREMK(c, w - arrayW)}, {"hi", COREMK(c, w)}, {"width", COREMK(c, w)}});

        def->connect(cc->sel("in"), activeSel);
        def->connect(cc->sel("out"), arrayElems[i]);
        }
        }
        });
  }

  {
    CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("pack", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto t = args.at("type")->get<CoreIR::Type*>();
        int totalWidth = 1;
        vector<int> dims = arrayDims(t);
        for (auto d : dims) {
        totalWidth *= d;
        }
        return c->Record({
            {"in", t->getFlipped()},
            {"out", c->Bit()->Arr(totalWidth)}
            });
        });
    auto srGen = hns->newGeneratorDecl("pack", srTg, srParams);
    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto t = args.at("type")->get<CoreIR::Type*>();
        int totalWidth = 1;
        vector<int> dims = arrayDims(t);
        for (auto d : dims) {
        totalWidth *= d;
        }
        auto self = def->sel("self");
        if (totalWidth == dims[0]) {
        auto zeroThOut = self->sel("in");
        for (int i = 0; i < ((int) dims.size()) - 1; i++) {
        zeroThOut = zeroThOut->sel(0);
        }
        def->connect(zeroThOut, self->sel("out"));
        } else {
        vector<Wireable*> arrayElems = flatSelects(self->sel("in"));
        cout << "# of array elements: " << arrayElems.size() << endl;
        for (auto e : arrayElems) {
        cout << "\t" << coreStr(e) << endl;
        }
        internal_assert(arrayElems.size() > 0);
        auto activeSel = arrayElems[0];
        auto self = def->sel("self");
        for (int i = 1; i < ((int) arrayElems.size()); i++) {
        int w = bitWidth(activeSel);
        int arrayW = bitWidth(arrayElems[i]);
        auto cc =
          def->addInstance("pack_concat_" + c->getUnique(),
              "coreir.concat",
              {{"width0", COREMK(c, w)}, {"width1", COREMK(c, arrayW)}});

        def->connect(cc->sel("in0"), activeSel);
        def->connect(cc->sel("in1"), arrayElems[i]);
        activeSel = cc->sel("out");
        }
        //internal_assert(bitWidth(activeSel), bitWidth(self->sel("out"))) << "bit widths do not match\n";
        def->connect(activeSel, self->sel("out"));
        }
        });
  }

  {
    CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("reg", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto tIn = args.at("type")->get<CoreIR::Type*>();
        return c->Record({
            {"in", tIn->getFlipped()},
            {"clk", c->Named("coreir.clkIn")},
            {"en", c->BitIn()},
            {"out", tIn},
            });
        });

    auto srGen = hns->newGeneratorDecl("reg", srTg, srParams);

    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto tIn = args.at("type")->get<CoreIR::Type*>();
        auto self = def->sel("self");

        Instance* reg = nullptr;
        if (tIn == c->Bit()) {
        reg =
        def->addInstance("reg", "mantle.reg", {{"width", COREMK(c, 1)}, {"has_en", COREMK(c, true)}});
        def->connect(reg->sel("in")->sel(0), self->sel("in"));
        def->connect(reg->sel("out")->sel(0), self->sel("out"));
        } else {
          reg = def->addInstance("reg", "commonlib.reg_array", {{"type", COREMK(c, tIn)}, {"has_en", COREMK(c, true)}});

          def->connect(reg->sel("in"), self->sel("in"));
          def->connect(reg->sel("out"), self->sel("out"));
        }

        internal_assert(reg != nullptr);

        def->connect(reg->sel("en"), self->sel("en"));
        def->connect(reg->sel("clk"), self->sel("clk"));
    });
  }

  {
    CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("mux", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto tIn = args.at("type")->get<CoreIR::Type*>();
        int totalWidth = 1;
        vector<int> output_dims = arrayDims(tIn);
        for (auto d : output_dims) {
        totalWidth *= d;
        }
        return c->Record({
            {"in0", tIn->getFlipped()},
            {"in1", tIn->getFlipped()},
            {"sel", c->BitIn()},
            {"out", tIn},
            });
        });

    auto srGen = hns->newGeneratorDecl("mux", srTg, srParams);

    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto self = def->sel("self");
        auto pack0 = def->addInstance("pack0", "halidehw.pack", {{"type", COREMK(c, self->sel("in0")->getType())}});
        auto pack1 = def->addInstance("pack1", "halidehw.pack", {{"type", COREMK(c, self->sel("in1")->getType())}});
        auto unpack = def->addInstance("unpack", "halidehw.unpack", {{"type", COREMK(c, self->sel("in0")->getType())}});

        auto ad = arrayDims(unpack->sel("in")->getType());
        internal_assert(ad.size() == 1) << "Output type: " << coreStr(unpack->sel("in")->getType()) << " has " << ad.size() << " dims\n";

        int outWidth = ad[0];
        auto mux = def->addInstance("mux", "coreir.mux", {{"width", COREMK(c, outWidth)}});
        def->connect(mux->sel("in0"), pack0->sel("out"));
        def->connect(mux->sel("in1"), pack1->sel("out"));
        def->connect(mux->sel("out"), unpack->sel("in"));

        def->connect(self->sel("in0"), pack0->sel("in"));
        def->connect(self->sel("in1"), pack1->sel("in"));
        def->connect(self->sel("sel"), mux->sel("sel"));
        def->connect(self->sel("out"), unpack->sel("out"));
        });
  }

  {
    CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("passthrough", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto t = args.at("type")->get<CoreIR::Type*>();
        return c->Record({
            {"in", t->getFlipped()},
            {"out", t}
            });
        });
    auto srGen = hns->newGeneratorDecl("passthrough", srTg, srParams);
    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto self = def->sel("self");
        def->connect(self->sel("in"), self->sel("out"));
        });
  }
  {
    CoreIR::Params srParams{{"width", context->Int()}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("cast", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto width = args.at("width")->get<int>();
        return c->Record({
            {"in", c->BitIn()->Arr(width)},
            {"out", c->Bit()->Arr(width)}
            });
        });
    auto srGen = hns->newGeneratorDecl("cast", srTg, srParams);
    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto self = def->sel("self");
        def->connect(self->sel("in"), self->sel("out"));
        });
  }

  {
  CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}, {"delay", context->Int()}};
  CoreIR::TypeGen* srTg = hns->newTypeGen("stream_trimmer", srParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      auto t = args.at("type")->get<CoreIR::Type*>();
      return c->Record({
          {"clk", c->Named("coreir.clkIn")},
          {"in", t->getFlipped()},
          {"en", c->BitIn()},
          {"out", t},
          {"valid", c->Bit()}
          });
      });
  auto srGen = hns->newGeneratorDecl("stream_trimmer", srTg, srParams);
  srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      auto self = def->sel("self");
      int delay = args.at("delay")->get<int>();
      auto tp = args.at("type")->get<CoreIR::Type*>();
      auto sr = def->addInstance("sr", "halidehw.shift_register", {{"type", COREMK(c, tp)}, {"delay", COREMK(c, delay)}});
      //auto sr = def->addInstance("sr", "halidehw.shift_register", {{"type", COREMK(c, tp)}, {"delay", COREMK(c, 9*2 + 2)}});
      
      def->connect(sr->sel("in_data"), self->sel("in"));
      def->connect(sr->sel("in_en"), self->sel("en"));
      def->connect(sr->sel("valid"), self->sel("valid"));
      def->connect(sr->sel("out_data"), self->sel("out"));

      //def->connect(self->sel("in"), self->sel("out"));
      //def->connect(self->sel("en"), self->sel("valid"));
      });
  }

  CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}, {"delay", context->Int()}};
  CoreIR::TypeGen* srTg = hns->newTypeGen("shift_register", srParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      auto t = args.at("type")->get<CoreIR::Type*>();
      return c->Record({
          {"clk", c->Named("coreir.clkIn")},
          {"in_data", t->getFlipped()},
          {"in_en", c->BitIn()},
          {"out_data", t},
          {"valid", c->Bit()}
          });
      });
  auto srGen = hns->newGeneratorDecl("shift_register", srTg, srParams);
  srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      auto self = def->sel("self");
      CoreIR::Type* type = args.at("type")->get<CoreIR::Type*>();
      int delay = args.at("delay")->get<int>();

      Wireable* inData = self->sel("in_data");
      Wireable* inEn = self->sel("in_en");

      for (int i = 0; i < delay; i++) {
        auto delayReg = def->addInstance("delay_reg_" + std::to_string(i), "commonlib.reg_array", {{"type", COREMK(c, type)}});
        def->connect(delayReg->sel("clk"), self->sel("clk"));
        def->connect(delayReg->sel("in"), inData);

        inData = delayReg->sel("out");
        auto validReg = def->addInstance("valid_reg_" + std::to_string(i), "corebit.reg");
        def->connect(validReg->sel("in"), inEn);
        def->connect(validReg->sel("clk"), self->sel("clk"));
        inEn = validReg->sel("out");
      }

      def->connect(self->sel("out_data"), inData);
      def->connect(self->sel("valid"), inEn);
      });
  
  CoreIR::Params widthParams{{"width", context->Int()}};
  CoreIR::Params romParams{{"width", context->Int()}, {"depth", context->Int()}, {"nports", context->Int()}};
  CoreIR::Params widthDimParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}};
  CoreIR::Params widthDimParams3{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}, {"nchannels", context->Int()}};
  CoreIR::Params dynamicStencilReadParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}};
  CoreIR::Params stencilReadParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()},
    {"r", context->Int()}, {"c", context->Int()}};
  CoreIR::Params stencilReadParams3{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}, {"nchannels", context->Int()},
    {"r", context->Int()}, {"c", context->Int()}, {"b", context->Int()}};

  CoreIR::TypeGen* romTg = hns->newTypeGen("ROM", romParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      int addrWidth = 16;
      int dataWidth = args.at("width")->get<int>();
      auto nports = args.at("nports")->get<int>();
      return c->Record({{"reset", c->BitIn()}, {"clk", c->Named("coreir.clkIn")}, {"raddr", c->BitIn()->Arr(addrWidth)->Arr(nports)}, {"rdata", c->Bit()->Arr(dataWidth)->Arr(nports)},
          {"ren", c->BitIn()->Arr(nports)}});
      });
  auto romGen = hns->newGeneratorDecl("ROM", romTg, romParams);
  auto romGenModParamFun = [](CoreIR::Context* c, CoreIR::Values genargs) {
    CoreIR::Params modparams;
    CoreIR::Values defaultargs;
    modparams["init"] = CoreIR::JsonType::make(c);
    return std::pair<CoreIR::Params, CoreIR::Values>(modparams, defaultargs);
  };
  romGen->setModParamsGen(romGenModParamFun);

  romGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      auto self = def->sel("self");
      int width = args.at("width")->get<int>();
      int depth = args.at("depth")->get<int>();
      int nports = args.at("nports")->get<int>();

      auto vals = def->getModule()->getArg("init");
      for (int i = 0; i < nports; i++) {
      auto romBank = def->addInstance("rom_bank_" + std::to_string(i), "memory.rom2",
          {{"width", COREMK(c, width)}, {"depth", COREMK(c, depth)}},
          {{"init", vals}});
      def->connect(romBank->sel("raddr"), self->sel("raddr")->sel(i));
      def->connect(romBank->sel("rdata"), self->sel("rdata")->sel(i));
      def->connect(romBank->sel("ren"), self->sel("ren")->sel(i));
      def->connect(romBank->sel("clk"), self->sel("clk"));
      }
      });

  
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
    CoreIR::TypeGen* tg = hns->newTypeGen("rd_stream_3", widthDimParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto rdStreamGen = hns->newGeneratorDecl("rd_stream_3", tg, widthDimParams3);
    rdStreamGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        def->connect("self.in", "self.out");
        });
  }

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
    // How to implement this with coreir primitives?
    //  - Put concatenation of the whole thing in to a mux
    //    and split rows until the result is what we expect
    //    it to be?
    CoreIR::TypeGen* ws = hns->newTypeGen("dynamic_stencil_read", dynamicStencilReadParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto width = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(width)->Arr(nr)->Arr(nc)}, {"r", c->BitIn()->Arr(16)}, {"c", c->BitIn()->Arr(16)}, {"out", c->Bit()->Arr(width)}});
        });

    auto readStencil = hns->newGeneratorDecl("dynamic_stencil_read", ws, dynamicStencilReadParams);
    readStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {

        //def->connect(def->sel("self")->sel("in")->sel(col)->sel(row), def->sel("self")->sel("out"));
        //def->connect(def->sel("self")->sel("in")->sel(0)->sel(row), def->sel("self")->sel("out"));
        def->connect(def->sel("self")->sel("in")->sel(0)->sel(0), def->sel("self")->sel("out"));
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
    CoreIR::TypeGen* ws = hns->newTypeGen("stencil_read_3", stencilReadParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto width = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(width)->Arr(nr)->Arr(nc)->Arr(nb)}, {"out", c->Bit()->Arr(width)}});
        });
    auto readStencil = hns->newGeneratorDecl("stencil_read_3", ws, stencilReadParams3);
    readStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto row = args.at("r")->get<int>();
        auto col = args.at("c")->get<int>();
        auto chan = args.at("b")->get<int>();

        def->connect(def->sel("self")->sel("in")->sel(chan)->sel(col)->sel(row), def->sel("self")->sel("out"));
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
    }

    std::set<HWInstr*> instructionsEndingInStage(const int stage) const {
      std::set<HWInstr*> instrs;
      for (auto instr : body) {
        if (getEndTime(instr) == stage) {
          instrs.insert(instr);
        }
      }
      return instrs;
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
      auto nI = newI();
      nI->tp = HWINSTR_TP_VAR;
      nI->name = name;
      nI->resType = f.getContext()->Bit()->Arr(16);
      return nI;
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
      ist->resType = f.getContext()->Bit()->Arr(16);
      lastValue = ist;
    }

    void visit(const Cast* c) override {
      auto ist = newI();
      ist->name = "cast";
      auto operand = codegen(c->value);
      ist->operands = {operand};
      ist->setSigned(!(c->type.is_uint()));
      ist->resType = f.getContext()->Bit()->Arr(16);
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

      cout << "Creating hwinstruction variable " << v->name << " that is not currently in vars" << endl;
      IRGraphVisitor::visit(v);
      auto ist = newI();
      ist->name = v->name;
      ist->tp = HWINSTR_TP_VAR;
      ist->setSigned(!(v->type.is_uint()));
      vars[v->name] = ist;

      lastValue = ist;
    }


    void visit(const GE* a) override {
      visit_binop("gte", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
    }

    void visit(const LE* a) override {
      visit_binop("lte", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
    }
    
    void visit(const LT* a) override {
      visit_binop("lt", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
    }

    void visit(const GT* a) override {
      visit_binop("gt", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
    }
    
    void visit(const And* a) override {
      visit_binop("and", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
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
      ist->resType = f.getContext()->Bit()->Arr(16);
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
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }
    
    void visit(const Max* m) override {
      visit_binop("max", m->a, m->b);
      lastValue->setSigned(!(m->type.is_uint()));
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }

    void visit(const Mod* d) override {
      visit_binop("mod", d->a, d->b);
      lastValue->setSigned(!(d->type.is_uint()));
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }

    void visit(const Div* d) override {
      visit_binop("div", d->a, d->b);
      lastValue->setSigned(!(d->type.is_uint()));
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }

    void visit(const Add* a) override {
      visit_binop("add", a->a, a->b);
      lastValue->setSigned(!(a->type.is_uint()));
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }

    void visit(const EQ* a) override {
      visit_binop("eq", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
    }
    
    void visit(const NE* a) override {
      visit_binop("neq", a->a, a->b);
      lastValue->resType = f.getContext()->Bit();
    }
    
    void visit(const Mul* b) override {
      visit_binop("mul", b->a, b->b);
      lastValue->setSigned(!(b->type.is_uint()));
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }

    void visit(const Sub* b) override {
      visit_binop("sub", b->a, b->b);
      lastValue->setSigned(!(b->type.is_uint()));
      lastValue->resType = f.getContext()->Bit()->Arr(16);
    }

    HWInstr* andHW(HWInstr* a, HWInstr* b) {
      auto andOp = newI();
      andOp->name = "and";
      andOp->operands = {a, b};
      lastValue->resType = f.getContext()->Bit();
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
        ist->resType = f.getContext()->Bit()->Arr(16);
      } else if (op->name == "abs") {
        ist->name = "abs";
        ist->resType = f.getContext()->Bit()->Arr(16);
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
        auto callOp = newI();
        callOp->tp = HWINSTR_TP_VAR;
        callOp->name = calledStencil;
        //callOp->setSigned(!(op->type.is_uint()));
        //cout << "Read from: " << op->name << " has signed result ? " << callOp->isSigned() << endl;
       
        ist->setSigned(!(op->type.is_uint()));
        ist->resType = f.getContext()->Bit()->Arr(16);
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

CoreIR::Type* moduleTypeForKernel(CoreIR::Context* context, StencilInfo& info, const For* lp, const vector<CoreIR_Argument>& args);
vector<pair<string, CoreIR::Type*> > moduleFieldsForKernel(CoreIR::Context* context, StencilInfo& info, const For* lp, const vector<CoreIR_Argument>& args);

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

Expr endTime(HWInstr* instr, FunctionSchedule& sched);
Expr startTime(HWInstr* instr, FunctionSchedule& sched);
Expr startTime(const std::vector<ProgramPosition>& positions,
    const ProgramPosition& targetPos,
    FunctionSchedule& sched);

class FunctionSchedule {
  public:
    HWFunction* f;
    map<HWInstr*, HWLoopSchedule> blockSchedules;

    std::vector<NestSchedule> nestSchedules;
    std::vector<ProgramPosition> positions;
    std::vector<SWTransition> transitions;

    NestSchedule getNestSchedule(const std::string& loopName) const {
      for (auto s : nestSchedules) {
        if (s.name == loopName) {
          return s;
        }
      }
      internal_assert(false) << "no schedule for loop: " << loopName << "\n";
      return {};
    }
    int II(const std::string& loopName) const {
      for (auto n : nestSchedules) {
        if (n.name == loopName) {
          return n.II;
        }
      }
      internal_assert(false) << "No II for: " << loopName << "\n";
      return -1;
    }

    set<HWInstr*> allInstructionsInStage(const int stageNo) const {
      auto s = instructionsStartingInStage(stageNo);
      for (auto e : instructionsEndingInStage(stageNo)) {
        s.insert(e);
      }
      return s;
    }

    set<HWInstr*> instructionsEndingInStage(const int stageNo) const {
      internal_assert(blockSchedules.size() == 1);
      return begin(blockSchedules)->second.instructionsEndingInStage(stageNo);
    }

    set<HWInstr*> instructionsStartingInStage(const int stageNo) const {
      internal_assert(blockSchedules.size() == 1);
      return begin(blockSchedules)->second.instructionsStartingInStage(stageNo);
    }
    
    int numLinearStages() const {
      internal_assert(blockSchedules.size() == 1);
      return begin(blockSchedules)->second.numStages();
    }

    bool inSameBlock(HWInstr* const a, HWInstr* const b) {
      return head(containerBlock(a, *f)) == head(containerBlock(b, *f));
    }

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

    std::vector<HWInstr*> body() {
      return f->structuredOrder();
    }

    
    int cycleLatency() {
      if (blockSchedules.size() == 0) {
        return 0;
      }

      set<HWInstr*> streamReads = allInstrs("rd_stream", body());
      set<HWInstr*> streamWrites = allInstrs("write_stream", body());
      int validDelay = 0;

      internal_assert(streamWrites.size() == 1 ||
          streamWrites.size() == 0);

      if (streamWrites.size() == 1) {
        HWInstr* read = *begin(streamReads);
        HWInstr* write = *begin(streamWrites);
        Expr latency = endTime(write, *this) - startTime(read, *this);
        cout << "Read  = " << *read << endl;
        cout << "Write = " << *write << endl;
        cout << "\tSymbolic latency  : " << latency << endl;
        Expr simplifiedLatency = simplify(latency);
        cout << "\tSimplified latency: " << simplifiedLatency << endl;

        validDelay = func_id_const_value(simplifiedLatency);
      } else {
        cout << "# of stream writes: " << streamWrites.size() << endl;
        cout << "Function..." << endl;
        cout << *f << endl;
        validDelay = 0;
      }
      internal_assert(validDelay >= 0);

      return validDelay;

    }

};

class ComputeKernel {
  public:
    CoreIR::Module* mod;
    FunctionSchedule sched;
};

bool operator==(const ProgramPosition& a, const ProgramPosition& b) {
  if (!a.isOp() && !b.isOp()) {
    return a.loopLevel == b.loopLevel &&
      a.head == b.head &&
      a.tail == b.tail;
  }

  if (a.instr != b.instr) {
    return false;
  }
  if (a.loopLevel != b.loopLevel) {
    return false;
  }
  if (a.head != b.head) {
    return false;
  }
  if (a.tail != b.tail) {
    return false;
  }
  return true;
}

bool operator!=(const ProgramPosition& a, const ProgramPosition& b) {
  return !(a == b);
}

bool operator<(const ProgramPosition& a, const ProgramPosition& b) {
  if (a == b) {
    return false;
  }

  if (a.instr != b.instr) {
    return a.instr < b.instr;
  }
  if (a.loopLevel != b.loopLevel) {
    return a.loopLevel < b.loopLevel;
  }
  if (a.head != b.head) {
    return a.head < b.head;
  }
  return a.tail < b.tail;
}

std::ostream& operator<<(std::ostream& out, ProgramPosition& pos) {
  if (pos.isOp()) {
    out << "ll[" << pos.loopLevel << "] " << (pos.head ? "HEAD " : "") << (pos.tail ? "TAIL " : "") << *(pos.instr);
  } else {
    out << "ll[" << pos.loopLevel << "] " << (pos.head ? "HEAD " : "") << (pos.tail ? "TAIL " : "") << "<NOOP>";
  }
  return out;
}

ProgramPosition getHead(std::string& loopLevel, vector<ProgramPosition>& positions) {
  for (auto p : positions) {
    if (p.isHead() &&
        p.loopLevel == loopLevel) {
      return p;
    }
  }

  internal_assert(false) << "No head for instr\n";
  return {};
}

Expr endTime(const ProgramPosition& pos, FunctionSchedule& sched);
Expr startTime(const ProgramPosition& pos, FunctionSchedule& sched);

vector<ProgramPosition> buildProgramPositions(FunctionSchedule& sched);

bool operator==(const Condition& a, const Condition& b) {
  if (a.isUnconditional != b.isUnconditional) {
    return false;
  }
  if (a.loopCounter != b.loopCounter) {
    return false;
  }
  return a.atMax == b.atMax;
}

bool operator<(const Condition& a, const Condition& b) {
  if (a.isUnconditional != b.isUnconditional) {
    return a.isUnconditional < b.isUnconditional;
  }
  if (a.loopCounter != b.loopCounter) {
    return a.loopCounter < b.loopCounter;
  }
  return a.atMax < b.atMax;
}

Condition atMax(const std::string& n) {
  return {false, n, true};
}

Condition notAtMax(const std::string& n) {
  return {false, n, false};
}

Condition unconditional() {
  return {true, "", false};
}

std::ostream& operator<<(std::ostream& out, Condition& c) {
  if (c.isUnconditional) {
    out << "True";
  } else {
    out << (c.atMax ? "" : "!") << "atMax(" << c.loopCounter << ")";
  }
  return out;
}

bool operator==(const SWTransition& a, const SWTransition& b) {
  if (a.src != b.src) {
    return false;
  }

  if (a.dst != b.dst) {
    return false;
  }
  return a.cond == b.cond;
}

bool operator<(const SWTransition& a, const SWTransition& b) {
  if (a.src != b.src) {
    return a.src < b.src;
  }

  if (a.dst != b.dst) {
    return a.dst < b.dst;
  }
  return a.cond < b.cond;
}

std::ostream& operator<<(std::ostream& out, SWTransition& pos) {
  out << "if (" << pos.cond << ")\n\t" << pos.src << "\n\t" << pos.dst;
  return out;
}

bool lessThan(const std::string& ll0, const std::string& ll1, HWFunction& f) {
  if (ll0 == ll1) {
    return false;
  }

  for (auto instr : f.structuredOrder()) {
    for (auto lp : instr->surroundingLoops) {
      if (lp.name == ll1) {
        return false;
      }

      if (lp.name == ll0) {
        return true;
      }
      
    }
  }
  return false;
}

bool lessThanOrEqual(const std::string& ll0, const std::string& ll1, HWFunction& f) {
  return lessThan(ll0, ll1, f) || (ll0 == ll1);
}

set<string> earlierLevels(const std::string& ll, HWFunction& f) {
  set<string> earlier;
  for (auto instr : f.structuredOrder()) {
    for (auto lp : instr->surroundingLoops) {
      if (lessThan(lp.name, ll, f)) {
        earlier.insert(lp.name);
      }
    }
  }
  return earlier;
}

class IChunk {
  public:
    int stage;
    std::vector<ProgramPosition> instrs;

    bool containsInstr(HWInstr* instr) const {
      for (auto pos : instrs) {
        if (pos.isOp() && pos.instr == instr) {
          return true;
        }
      }
      return false;
    }

    bool containsPos(const ProgramPosition& pos) const {
      for (auto instr : instrs) {
        if (instr == pos) {
          return true;
        }
      }
      return false;
    }

    bool containsHeader() const {
      for (auto p : instrs) {
        if (p.isHead()) {
          return true;
        }
      }
      return false;
    }

    ProgramPosition getRep() const {
      internal_assert(instrs.size() > 0) << "No instructions in chunk for stage: " << stage << "\n";
      return instrs.at(0);
    }
};

map<int, vector<IChunk> > getChunks(const vector<ProgramPosition>& positions,
    FunctionSchedule& sched);

bool operator==(const IChunk& a, const IChunk& b) {
  return a.stage == b.stage && a.instrs == b.instrs;
}

std::ostream& operator<<(std::ostream& out, const IChunk& chunk) {
  out << chunk.stage << endl;
  for (auto i : chunk.instrs) {
    out << "\t" << i << endl;
  }
  return out;
}

int chunkIdx(const ProgramPosition& pos, const vector<IChunk>& chunks) {
  int ind = 0;
  for (auto sc : chunks) {
    for (auto other : sc.instrs) {
      if (other == pos) {
        return ind;
      }
    }
    ind++;
  }
  internal_assert(false) << "No chunk for: ";
  //<< pos << "\n";
  return -1;
}

int chunkIdx(const IChunk& c, const vector<IChunk>& chunks) {
  internal_assert(c.instrs.size() > 0);
  return chunkIdx(c.instrs.at(0), chunks);
}

IChunk getChunk(const ProgramPosition& pos, const vector<IChunk>& chunks) {
  return chunks.at(chunkIdx(pos, chunks));
}

class KernelControlPath {
  public:
    std::vector<std::string> controlVars;
    CoreIR::Module* m;
    std::map<int, string> stageIsActiveMap;
    std::map<int, string> chunkPhiInputMap;
    vector<IChunk> chunkList;

    std::string phiOutput(const IChunk& c) const {
      return map_get(chunkIdx(c, chunkList), chunkPhiInputMap);
    }

    std::string activeSignalOutput(const ProgramPosition& pos) const {
      return activeSignalOutput(getChunk(pos, chunkList));
    }

    std::string activeSignalOutput(const HWInstr* instr) const {
      for (auto c : chunkList) {
        for (auto p : c.instrs) {
          if (p.isOp() && p.instr == instr) {
            return activeSignalOutput(c);
          }
        }
      }
      internal_assert(false) << "no chunk for " << *instr << "\n";
      return "";
    }

    std::string activeSignalOutput(const IChunk& chunk) const {
      return map_get(chunkIdx(chunk, chunkList), stageIsActiveMap);
    }

    std::string activeSignal(const IChunk& chunk) const {
      return "self." + map_get(chunkIdx(chunk, chunkList), stageIsActiveMap);
    }

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

KernelControlPath controlPathForKernel(FunctionSchedule& f);

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

HWFunction buildHWBody(CoreIR::Context* context, StencilInfo& info, const std::string& name, const For* perfectNest, const vector<CoreIR_Argument>& args, StoreCollector& stCollector) {
  InstructionCollector collector;
  collector.activeBlock = *std::begin(collector.f.getBlocks());
  collector.f.name = name;
  collector.f.context = context;
  
  perfectNest->accept(&collector);

  auto& f = collector.f;

  auto hwVars = extractHardwareVars(perfectNest);
  for (auto arg : args) {
    if (!arg.is_stencil) {
      hwVars.push_back(coreirSanitize(arg.name));
    }
  }

  cout << "All hardware vars.." << endl;
  for (auto hv : hwVars) {
    cout << "\t" << hv << endl;
  }

  f.controlVars = hwVars;

  cout << "Before opts..." << endl;
  cout << f << endl;

  removeBadStores(stCollector, f);
  valueConvertStreamReads(info, f);
  valueConvertProvides(info, f);
  removeWriteStreamArgs(info, f);
  divToShift(f);
  modToShift(f);
  addDynamicStencilReads(f);
  
  auto global_ns = context->getNamespace("global");
  auto designFields = moduleFieldsForKernel(context, info, perfectNest, args);
  for (auto instr : f.structuredOrder()) {
    if (instr->resType != nullptr) {
      designFields.push_back({dbgName(instr), instr->resType});
    }
  }
  
  CoreIR::Type* design_type = context->Record(designFields);
  auto design = global_ns->newModuleDecl(collector.f.name, design_type);
  auto def = design->newModuleDef();
  design->setDef(def);
  f.mod = design;

  cout << "Returning HWFunction" << endl;
  cout << f << endl;

  internal_assert(f.mod != nullptr) << "mod is null after buildHWBody\n";

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
      //cout << "Stencil visiting call: " << op->name << endl;
      if (op->name == "dispatch_stream") {
        //cout << "Found dispatch" << endl;
        //cout << "\tName = " << op->args[0] << "\n";
        //cout << "\t# dims = " << op->args[1] << "\n";

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

    std::map<HWInstr*, vector<int> > stencilRanges;
    std::map<HWInstr*, CoreIR::Instance*> unitMapping;

    std::map<HWInstr*, std::map<int, CoreIR::Instance*> > pipelineRegisters;
    std::map<HWInstr*, CoreIR::Instance*> nonPipelineRegisters;

    std::vector<HWInstr*> body;

    int getEndTime(HWInstr* instr) {
      return fSched.getContainerBlock(instr).getEndTime(instr);
    }

    bool hasOutput(HWInstr* const arg) const {
      return arg->resType != nullptr;
      //return CoreIR::contains_key(arg, instrValues);
    }

    void valueIsAlways(HWInstr* const arg1, CoreIR::Wireable* w) {
      for (auto instr : fSched.body()) {
        hwStartValues[arg1][instr] = w;
        hwEndValues[arg1][instr] = w;
      }
    }

    CoreIR::Wireable* valueAtStart(HWInstr* const arg1, HWInstr* const sourceLocation) {
      internal_assert(contains_key(arg1, hwStartValues)) << *arg1 << " is not in hwStartValues\n";
      internal_assert(contains_key(sourceLocation, map_get(arg1, hwStartValues))) << *sourceLocation << " is not in hwStartValues(" << *arg1 << ")\n";
      return map_get(sourceLocation, map_get(arg1, hwStartValues));
    }

    CoreIR::Wireable* valueAtEnd(HWInstr* const arg1, HWInstr* const sourceLocation) {
      internal_assert(contains_key(arg1, hwEndValues));
      internal_assert(contains_key(sourceLocation, hwEndValues[arg1]));
      return hwEndValues[arg1][sourceLocation];
    }

};

CoreIR::Instance* pipelineRegisterEn(CoreIR::Context* context, CoreIR::ModuleDef* def, const std::string name, CoreIR::Type* type) {
  auto r = def->addInstance(name, "halidehw.reg", {{"type", COREMK(context, type)}});
  return r;
}

CoreIR::Instance* pipelineRegister(CoreIR::Context* context, CoreIR::ModuleDef* def, const std::string name, CoreIR::Type* type) {

  if (type == context->BitIn() || type == context->Bit()) {
    auto r = def->addInstance(name, "corebit.reg");
    return r;
  }
  auto r = def->addInstance(name, "commonlib.reg_array", {{"type", COREMK(context, type)}});
  return r;
}

vector<CoreIR::Instance*> pipelineRegisterChain(CoreIR::ModuleDef* def, const std::string name, CoreIR::Type* type, int depth) {
  auto context = def->getContext();
  if (depth == 0) {
    return {def->addInstance("passthrough_" + name, "halidehw.passthrough", {{"type", COREMK(context, type)}})};
  }
  
  internal_assert(depth > 0) << "trying to create pipeline register chain of depth " << depth << ", name: " << name << "\n";

  vector<Instance*> registers;
  auto activeReg = pipelineRegister(context, def, name + "_0", type);
  registers.push_back(activeReg);
  for (int i = 1; i < depth - 1; i++) {
    auto nextReg = pipelineRegister(context, def, name + "_" + to_string(i), type);
    def->connect(activeReg->sel("out"), nextReg->sel("in"));
    activeReg = nextReg;
    registers.push_back(activeReg);
  }
  return registers;
}

void createFunctionalUnitsForOperations(StencilInfo& info, UnitMapping& m, FunctionSchedule& sched, ModuleDef* def, CoreIR::Instance* controlPath) {
  cout << "# of instructions in body when creating functional units: " << sched.body().size() << endl;
  for (auto i : sched.body()) {
    cout << "\t" << *i << endl;
  }
  auto context = def->getContext();
  int defStage = 0;
  auto& unitMapping = m.unitMapping;
  //auto& instrValues = m.instrValues;
  map<HWInstr*, CoreIR::Wireable*> instrValues;
  auto& stencilRanges = m.stencilRanges;

  for (auto instr : sched.body()) {
    if (instr->tp == HWINSTR_TP_INSTR) {
      string name = instr->name;
      cout << "Creating unit for " << *instr << endl;
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

        cout << "# of dims in " << instr->operands[0]->name << " = " << dims.size() << endl;
        for (auto d : dims) {
          cout << "Dim = " << d << endl;
        }

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
        cout << "Making create stencil from " << *instr << endl;
        cout << "Operand 0 = " << *(instr->getOperand(0)) << endl;

        internal_assert(instr->getOperand(0)->resType != nullptr);
        auto dimRanges = arrayDims(instr->getOperand(0)->resType);

        //auto dimRanges = CoreIR::map_find(instr->getOperand(0), stencilRanges);

        cout << "dimRanges = " << dimRanges << endl;
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
        cout << "Built dimranges" << endl;

      } else if (starts_with(name, "dynamic_stencil_read")) {

        cout << "Creating stencil read: " << *instr << endl;
        cout << "\tread from: " << *(instr->getOperand(0)) << endl;
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
      } else if (starts_with(name, "dynamic_stencil_read")) {

        cout << "Creating dynamic stencil read: " << *instr << endl;
        cout << "\tread from: " << *(instr->getOperand(0)) << endl;
        internal_assert(instr->getOperand(0)->resType != nullptr);

        vector<int> dimRanges = arrayDims(instr->getOperand(0)->resType);
        internal_assert(dimRanges.size() > 1) << "dimranges has size: " << dimRanges.size() << "\n";

        Instance* cS = nullptr;
        if (dimRanges.size() == 3) {
          //internal_assert(instr->getOperand(1)->tp == HWINSTR_TP_CONST);
          //internal_assert(instr->getOperand(2)->tp == HWINSTR_TP_CONST);
          cout << "\tOperands 1 and 2 of " << *instr << " are constants" << endl;

          // TODO: Add real offsets
          int selRow = 0;
          int selCol = 0;
          cS = def->addInstance("stencil_read_" + std::to_string(defStage), "halidehw.stencil_read", {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}});
        } else {
          internal_assert(dimRanges.size() == 4);
          
          //internal_assert(instr->getOperand(1)->tp == HWINSTR_TP_CONST);
          //internal_assert(instr->getOperand(2)->tp == HWINSTR_TP_CONST);
          //internal_assert(instr->getOperand(3)->tp == HWINSTR_TP_CONST);

          cout << "\tOperands 1, 2and 3 of " << *instr << " are constants" << endl;
         
          // TODO: Add real offset wiring
          int selRow = 0;
          int selCol = 0;
          int selChan = 0;
          cS = def->addInstance("stencil_read_" + std::to_string(defStage),
              "halidehw.stencil_read_3",
              {{"width", CoreIR::Const::make(context, dimRanges[0])}, {"nrows", COREMK(context, dimRanges[1])}, {"ncols", COREMK(context, dimRanges[2])}, {"nchannels", COREMK(context, dimRanges[3])},
              {"r", COREMK(context, selRow)}, {"c", COREMK(context, selCol)}, {"b", COREMK(context, selChan)}});
        }
        internal_assert(cS != nullptr);
        instrValues[instr] = cS->sel("out");
        unitMapping[instr] = cS;
      } else if (starts_with(name, "stencil_read")) {
        cout << "Creating stencil read: " << *instr << endl;
        cout << "\tread from: " << *(instr->getOperand(0)) << endl;
        internal_assert(instr->getOperand(0)->resType != nullptr);

        vector<int> dimRanges = arrayDims(instr->getOperand(0)->resType);
        internal_assert(dimRanges.size() > 1) << "dimranges has size: " << dimRanges.size() << "\n";

        if (dimRanges.size() == 3) {
          internal_assert(instr->getOperand(1)->tp == HWINSTR_TP_CONST);
          internal_assert(instr->getOperand(2)->tp == HWINSTR_TP_CONST);
          cout << "\tOperands 1 and 2 of " << *instr << " are constants" << endl;

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

          cout << "\tOperands 1, 2and 3 of " << *instr << " are constants" << endl;
          
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

        if (!instr->realized()) {
          instr->realizeUnit(def);
        }
        auto unit = instr->getUnit();
        unitMapping[instr] = unit;
 
        cout << "Connecting " << *instr << endl;
        internal_assert(unit != nullptr);
        cout << "Connecting " << coreStr(instr->getUnit()) << " ROM rdata" << endl;
        internal_assert(isa<CoreIR::Instance>(unit));
        
        Instance* inst = static_cast<Instance*>(unit);
        
        internal_assert(fromGenerator("halidehw.ROM", inst));
        
        instrValues[instr] = unit->sel("rdata")->sel(portNo);

      } else if (name == "phi") {
        internal_assert(instr->resType != nullptr);
        auto sel = def->addInstance("phi_" + std::to_string(defStage), "halidehw.mux", {{"type", COREMK(context, instr->resType)}});
        instrValues[instr] = sel->sel("out");
        unitMapping[instr] = sel;
      } else if (name == "delay") {
        auto tp = instr->resType;
        internal_assert(tp != nullptr);
        auto rname = "delay_reg_" + context->getUnique();
        Instance* inst = pipelineRegister(context, def, rname, tp);
        unitMapping[instr] = inst;
        instrValues[instr] = inst->sel("out");
      } else {
        internal_assert(false) << "no functional unit generation code for " << *instr << "\n";
      }
    }

    defStage++;
  }

  for (auto v : instrValues) {
    m.hwEndValues[v.first][v.first] = v.second;
    auto op = v.first;
    auto val = v.second;
    int endStage = sched.getContainerBlock(op).getEndTime(op);
    for (auto instr : sched.getContainerBlock(op).instructionsStartingInStage(endStage)) {
      m.hwStartValues[op][instr] = val;
    }
  }
  // Constants and pre-bound instructions / variables
  // are always bound to the same wire
  //
  // Local variables can be bound at any place where they are provided
  // Loop index variables are trickier because they are bound to the output of a counter
  // at some location, but then they are defined somewhere else
  for (auto instr : sched.body()) {
    //cout << "Wiring up constants" << endl;
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

  
}

class LevelDiff {
  public:

    std::vector<LoopSpec> shared;
    std::vector<LoopSpec> producerOnly;
    std::vector<LoopSpec> consumerOnly;
};

LevelDiff splitLevels(const std::vector<LoopSpec>& producer,
    const std::vector<LoopSpec>& consumer) {
  LevelDiff diff;
  int minSize = std::min(producer.size(), consumer.size());
  int maxLoc = 0;
  for (int i = 0; i < minSize; i++) {
    if (producer[i].name == consumer[i].name) {
      diff.shared.push_back(producer[i]);
    } else {
      break;
    }
    maxLoc++;
  }

  for (int level = maxLoc; level < (int) producer.size(); level++) {
    diff.producerOnly.push_back(producer[level]);
  }
  
  for (int level = maxLoc; level < (int) consumer.size(); level++) {
    diff.consumerOnly.push_back(consumer[level]);
  }

  internal_assert(diff.shared.size() + diff.producerOnly.size() == producer.size());
  internal_assert(diff.shared.size() + diff.consumerOnly.size() == consumer.size());

  return diff;
}

UnitMapping createUnitMapping(HWFunction& f, StencilInfo& info, FunctionSchedule& sched, CoreIR::ModuleDef* def, CoreIR::Instance* controlPath, KernelControlPath& cpM) {
  internal_assert(sched.blockSchedules.size() > 0);
  //cout << "--- Block schedules..." << endl;
  //for (auto& blk : sched.blockSchedules) {
    //blk.second.print();
  //}

  auto context = f.getContext();

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

  vector<ProgramPosition> positions = buildProgramPositions(sched);
  map<int, vector<IChunk> > chunks = getChunks(positions, sched);
  vector<IChunk> chunkList;
  for (auto sc : chunks) {
    for (auto c : sc.second) {
      chunkList.push_back(c);
    }
  }

  map<HWInstr*, Wireable*> sourceWires;
  set<string> vars;
  for (auto op : allVarsUsed(sched.body())) {
    if (op->tp == HWINSTR_TP_VAR) {
      cout << "Wiring up var..." << op->compactString() << endl;
      string name = op->name;

      if (f.isLoopIndexVar(name) && !elem(name, vars)) {
        vars.insert(name);
        sourceWires[op] = controlPath->sel(coreirSanitize(name));

        string loopLevel = op->name;
        ProgramPosition p = headerPosition(loopLevel, positions);
        int prodStage = sched.getEndStage(p.instr);

        for (auto instr : sched.getContainerBlock(p.instr).instructionsStartingInStage(prodStage)) {
          m.hwStartValues[op][instr] = sourceWires[op];
        }
      }
    }
  }

  for (auto instr : sched.body()) {
    if (m.hasOutput(instr)) {
      sourceWires[instr] = m.valueAtEnd(instr, instr);
    }
  }

  //auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });

  int uNum = 0;
  for (auto instrW : sourceWires) {

    auto instr = instrW.first;
    Wireable* sourceWire = instrW.second;
    ProgramPosition pos;

    // Get production stage
    int prodStage = -1;
    CoreIR::Type* type = nullptr;
    if (instr->tp == HWINSTR_TP_INSTR) {
      prodStage = sched.getEndStage(instr);
      pos = getPosition(instr, positions);
      internal_assert(instr->resType != nullptr);
      type = instr->resType;
    } else {
      internal_assert(f.isLoopIndexVar(instr->name)) << "trying to create datapath for non loop index variable: " << instr->name << "\n";

      type = def->getContext()->Bit()->Arr(16);
      string loopLevel = instr->name;
      ProgramPosition p = headerPosition(loopLevel, positions);
      prodStage = sched.getEndStage(p.instr);
      pos = p;
    }

    internal_assert(prodStage >= 0);
    internal_assert(type >= 0);

    // Create storage for this instr
    cout << "Creating non pipeline registers for " << *instr << "..." << endl;

    m.nonPipelineRegisters[instr] = pipelineRegisterEn(context, def, "non_pipeline_reg" + context->getUnique(), type);
    def->connect(m.nonPipelineRegisters[instr]->sel("in"), sourceWire);
    def->connect(m.nonPipelineRegisters[instr]->sel("en"), controlPath->sel(cpM.activeSignalOutput(pos)));

    cout << "Creating pipeline registers for " << *instr << endl;
    for (int i = prodStage + 1; i < sched.numLinearStages(); i++) {
      m.pipelineRegisters[instr][i] = pipelineRegister(context, def, "pipeline_reg_" + std::to_string(i) + "_" + std::to_string(uNum), type);
      uNum++;
    }

    cout << "Getting start values for instr" << endl;

    auto& startValues = m.hwStartValues[instr];

    cout << "Got start values for instr" << endl;

    auto pipeRegs = m.pipelineRegisters[instr];

    auto users = getUsers(instr, f.structuredOrder());

    for (auto otherInstr : users) {
      if (otherInstr == instr) {
        continue;
      }

      Expr prodTimeFunc = endTime(pos, sched);
      Expr startTimeFunc = startTime(otherInstr, sched);

      cout << "Production time func: " << prodTimeFunc << endl;
      cout << "Start time func     : " << startTimeFunc << endl;

      LevelDiff ld = splitLevels(pos.instr->surroundingLoops,
          otherInstr->surroundingLoops);

      cout << "Shared loops..." << endl;
      for (auto l : ld.shared) {
        cout << "\t" << l.name << endl;
      }
      cout << "Producer only..." << endl;
      for (auto l : ld.producerOnly) {
        cout << "\t" << l.name << endl;
      }
      cout << "Consumer only..." << endl;
      for (auto l : ld.consumerOnly) {
        cout << "\t" << l.name << endl;
      }

      bool consumerForward =
        instructionPosition(instr, f) < instructionPosition(otherInstr, f);
      map<string, Expr> varMapping;
      cout << "Instr: " << *pos.instr << endl;
      cout << "User : " << *otherInstr << endl;
      if (consumerForward) {
        for (auto l : ld.shared) {
          varMapping[l.name] = Variable::make(Int(32), l.name);
        }

        for (auto l : ld.producerOnly) {
          auto min_value = l.min;
          Expr max_value = min_value + l.extent - 1;
          varMapping[l.name] = max_value;
        }
      } else {
        internal_assert(ld.shared.size() > 0);

        for (int i = 0; i < ((int) ld.shared.size()) - 1; i++) {
          auto l = ld.shared[i];
          varMapping[l.name] = Variable::make(Int(32), l.name);
        }

        varMapping[ld.shared.back().name] = Variable::make(Int(32), ld.shared.back().name) - 1;

        for (auto l : ld.producerOnly) {
          auto min_value = l.min;
          Expr max_value = min_value + l.extent - 1;
          varMapping[l.name] = max_value;
        }
      }
      cout << "VarMapping..." << endl;
      for (auto v : varMapping) {
        cout << "\t" << v.first << " -> " << v.second << endl;
      }

      Expr readFunc = substitute(varMapping, prodTimeFunc);
      Expr productionDelay = simplify(startTimeFunc - readFunc);
      cout << "Production delay    : " << productionDelay << endl;
      //internal_assert(consumerForward);
      internal_assert(ld.producerOnly.size() == 0);

      //internal_assert(ld.shared.size() == otherInstr->surroundingLoops.size());

      // What is a low effort thing I could do here?
      //  - Create leveldiff code and start printing out level difference info
      int otherStartStage = sched.getStartStage(otherInstr);
      if (prodStage == otherStartStage) {
      //if (is_zero(productionDelay)) {
        if (instructionPosition(instr, f) < instructionPosition(otherInstr, f)) {
          // otherInstr is lexically later in the same stage
          startValues[otherInstr] = sourceWire;
        } else {
          startValues[otherInstr] = m.nonPipelineRegisters[instr]->sel("out");
        }
      } else {
        if (loopLevel(otherInstr) == loopLevel(instr)) {
          if (is_negative_const(productionDelay)) {
            startValues[otherInstr] = m.nonPipelineRegisters[instr]->sel("out");
          } else {
            internal_assert(is_positive_const(productionDelay)) << productionDelay << " is not a positive constant!\n";
            
            int diff = func_id_const_value(productionDelay);

            internal_assert(diff >= 0) << *instr << " is used by " << *otherInstr << "before it is produced\n";

            startValues[otherInstr] = pipeRegs[prodStage + diff]->sel("out");
          }
        } else {
          startValues[otherInstr] = m.nonPipelineRegisters[instr]->sel("out");
        }
      }
    }
  } 

  cout << "Connecting register chains for variables" << endl;
  // Now: Wire up pipeline registers in chains, delete the unused ones and test each value produced in this code
  for (auto pr : m.pipelineRegisters) {
    auto instr = pr.first;
    cout << "\tGetting value at end" << endl;
    auto fstVal = map_get(instr, sourceWires);
    cout << "\tGetting prod stage" << endl;
    for (auto pReg : m.pipelineRegisters[instr]) {
      int index = pReg.first;
      auto pipeReg = pReg.second;
      if (contains_key(index - 1, m.pipelineRegisters[instr])) {
        def->connect(pipeReg->sel("in"), m.pipelineRegisters[instr][index - 1]->sel("out"));
      } else {
        def->connect(pipeReg->sel("in"), fstVal);
      }
    }
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
}

Expr containerIterationStart(HWInstr* instr, FunctionSchedule& sched) {
  return 0;
}

void emitCoreIR(HWFunction& f, StencilInfo& info, FunctionSchedule& sched) {
  internal_assert(sched.blockSchedules.size() > 0);

  vector<ProgramPosition> positions = buildProgramPositions(sched);
  map<int, vector<IChunk> > chunks = getChunks(positions, sched);
  vector<IChunk> chunkList;
  for (auto sc : chunks) {
    for (auto c : sc.second) {
      chunkList.push_back(c);
    }
  }

  cout << "Getting def" << endl;
  auto def = f.getDef();
  cout << "Got def" << endl;
  //auto def = f.mod->getDef();
  internal_assert(def != nullptr);

  CoreIR::Context* context = def->getContext();
 
  // Create control path
  auto cpM = controlPathForKernel(sched);
  cout << "Control path module..." << endl;
  cpM.m->print();
  auto controlPath = def->addInstance("control_path_module_" + f.name, cpM.m);
  def->connect(def->sel("self")->sel("reset"), controlPath->sel("reset"));
  cout << "Wiring up def in enable and control path in_en" << endl;
  def->connect(def->sel("self")->sel("in_en"), controlPath->sel("in_en"));

  // In this mapping I want to assign values that are 
  UnitMapping m = createUnitMapping(f, info, sched, def, controlPath, cpM);
  auto& unitMapping = m.unitMapping;

  cout << "Wiring up enables" << endl;

  set<HWInstr*> streamWrites = allInstrs("write_stream", sched.body());
  if (streamWrites.size() == 1) {
    HWInstr* writeInstr = *begin(streamWrites);
    //def->connect(controlPath->sel(cpM.activeSignalOutput(sched.getEndStage(writeInstr))), def->sel("self.valid"));
    def->connect(controlPath->sel(cpM.activeSignalOutput(writeInstr)), def->sel("self.valid"));
  } else {
    def->connect(def->sel("self.in_en"), def->sel("self.valid"));
  }

  cout << "Building connections inside each cycle\n";
  for (auto instr : sched.body()) {
    internal_assert(CoreIR::contains_key(instr, unitMapping)) << "no unit mapping for " << *instr << "\n";
    CoreIR::Instance* unit = CoreIR::map_find(instr, unitMapping);

    if (instr->resType != nullptr) {
      std::vector<std::string> fds = f.getMod()->getType()->getFields();
      if (elem(dbgName(instr), fds)) {
          def->connect(def->sel("self")->sel(dbgName(instr)), m.valueAtEnd(instr, instr));
      }
    }

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
    } else if (instr->name == "dynamic_stencil_read") {
      // TODO: Actually handle index wiring
      auto row = instr->getOperand(2);
      auto col = instr->getOperand(1);
      auto arg = instr->getOperand(0);
      def->connect(unit->sel("in"), m.valueAtStart(arg, instr));
      def->connect(unit->sel("r"), m.valueAtStart(row, instr));
      def->connect(unit->sel("c"), m.valueAtStart(col, instr));
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
      def->connect(unit->sel("in1"), m.valueAtStart(instr->getOperand(0), instr));
      def->connect(unit->sel("in0"), m.valueAtStart(instr->getOperand(1), instr));
      ProgramPosition p = getPosition(instr, positions);
      int instrPos = instructionPosition(instr, f);
      IChunk c = getChunk(getPosition(instr, positions), chunkList);
      Wireable* selectControl = controlPath->sel(cpM.phiOutput(c));
      if (instrPos > 0) {
        int priorPos = instrPos - 1;
        HWInstr* prior = f.structuredOrder()[priorPos];
        cout << "Prior instruction: " << *prior << endl;
        vector<string> priorLoops = loopNames(prior);
        string nextLoop = "";
        bool foundLoop = false;
        for (auto loop : instr->surroundingLoops) {
          if (!elem(loop.name, priorLoops)) {
            nextLoop = loop.name;
            foundLoop = true;
            break;
          }
        }

        if (foundLoop) {
          ProgramPosition headerPos = headerPosition(nextLoop, positions);
          IChunk headerChunk = getChunk(headerPos, chunkList);
          cout << "\tFound loop boundary for phi, setting to: " << nextLoop << endl;
          selectControl = controlPath->sel(cpM.phiOutput(headerChunk));
        }
      }
      
      def->connect(unit->sel("sel"), selectControl);
    } else {
      internal_assert(false) << "no wiring procedure for " << *instr << "\n";
    }
  }
  cout << "Done building connections in body" << endl;
}

vector<pair<string, CoreIR::Type*> > moduleFieldsForKernel(CoreIR::Context* context, StencilInfo& info, const For* lp, const vector<CoreIR_Argument>& args) {

  vector<std::pair<std::string, CoreIR::Type*> > tps;
  tps = {{"reset", context->BitIn()}, {"in_en", context->BitIn()}, {"valid", context->Bit()}};
  StencilInfoCollector lpInfo;
  lp->accept(&lpInfo);

  std::set<string> inStreams;
  std::set<string> outStreams;

  for (auto v : lpInfo.info.streamReads) {
    inStreams.insert(v.first);
  }
  for (auto v : lpInfo.info.streamWrites) {
    outStreams.insert(v.first);
  }

  for (auto v : extractHardwareVars(lp)) {
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

  tps.push_back({"in_en", context->BitIn()});
  tps.push_back({"valid", context->Bit()});

  return tps;
}

CoreIR::Type* moduleTypeForKernel(CoreIR::Context* context, StencilInfo& info, const For* lp, const vector<CoreIR_Argument>& args) {
  auto tps = moduleFieldsForKernel(context, info, lp, args);
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
    cout << "Current time = " << currentTime << endl;
    cout << "\t# Finished = " << finished.size() << endl;
    cout << "\tActive = " << activeToTimeRemaining << endl;
    //cout << "\tRemain = " << remaining << endl;
    bool foundNextInstr = false;
    for (auto toSchedule : remaining) {
      //std::set<HWInstr*> deps = instrsUsedBy(toSchedule);
      std::set<HWInstr*> deps = dependencies(toSchedule, iNodes, blockGraph);
      cout << "Instr: " << *toSchedule << " has " << deps.size() << " deps: " << endl;
      if (subset(deps, finished)) {
        cout << "Scheduling " << *toSchedule << " in time " << currentTime << endl;
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
        cout << "\tUnfinished deps..." << endl;
        for (auto d : deps) {
          if (!elem(d, finished)) {
            cout << "\t\t" << *d << endl;
          }
        }
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

Expr tripCount(const std::string& var, HWFunction& f) {
  for (auto instr : f.allInstrs()) {
    for (auto lp : instr->surroundingLoops) {
      if (lp.name == var) {
        Expr tc = simplify(lp.extent);
        return tc;
      }
    }
  }
  internal_assert(false);
  return -1;
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
  internal_assert(false);
  return -1;
}

int headerLatencyInt(const std::string& name, HWFunction& f, FunctionSchedule& fSched) {
  return 0;
}

int tailLatencyInt(const std::string& name, HWFunction& f, FunctionSchedule& fSched) {
  return 0;
}

vector<ProgramPosition> buildProgramPositions(FunctionSchedule& sched);

FunctionSchedule buildFunctionSchedule(map<string, StreamUseInfo>& streamUseInfo, HWFunction& f) {
  auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });
  // Check if we are in a perfect loop nest
  FunctionSchedule fSched;
  fSched.f = &f;
  fSched.positions = buildProgramPositions(fSched);

  //for (auto group : instrGroups) {
    //HWLoopSchedule sched = asapSchedule(group);
    //fSched.blockSchedules[head(group)] = sched;
  //}

  HWLoopSchedule sched = asapSchedule(f);
  fSched.blockSchedules[head(f.structuredOrder())] = sched;
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
  CoreIR::reverse(nestVars);

  vector<NestSchedule> schedules;
  int tc0 = tripCountInt(nestVars[0], f);
  int latency0 = fSched.getScheduleFor(head(deepest)).cycleLatency();
  schedules.push_back({nestVars[0], latency0 + 1, latency0, tc0});

  for (int i = 1; i < (int) nestVars.size(); i++) {
    // Create nest schedule

    int tc = tripCountInt(nestVars[i], f);
    int headerLatency = headerLatencyInt(nestVars[i], f, fSched);
    int tailLatency = tailLatencyInt(nestVars[i], f, fSched);
    int II = headerLatency + schedules.back().completionTime() + tailLatency + 1;
    int L = II - 1; // Execute outer loops sequentially;

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

  // Now: Check the producer of all stream reads
  // and then figure out the rate of production of each
  // stream read

  //cout << "All reads from producers: " << endl;
  //auto reads = allInstrs("rd_stream", f.structuredOrder());
  //// Now: for each read classify the sources of the read
  //for (auto rd : reads) {
    //string streamName = rd->getOperand(0)->compactString();
    //StreamUseInfo useInfo = map_get(streamName, streamUseInfo);
    //cout << "\tSource of read from: " << streamName << " = " << useInfo.writer.toString() << endl;
  //}

  fSched.nestSchedules = schedules;
  internal_assert(fSched.blockSchedules.size() > 0);

  if (f.allInstrs().size() == 0) {
    internal_assert(false);
    return fSched;
  }

  internal_assert(fSched.blockSchedules.size() > 0);

  fSched.transitions = hwTransitions(fSched);

  cout << "Returning schedule" << endl;
  return fSched;
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

// I want to be able to map to modules when coreir is emitted for a design
// Maybe just create a modulewrapper?
void removeBadStores(StoreCollector& storeCollector, HWFunction& f) {
  cout << "Allocate ROMs..." << endl;
  std::map<std::string, vector<HWInstr*> > romLoads;
  for (auto instr : f.allInstrs()) {
    if (isCall("load", instr)) {
      cout << "Found load..." << *instr << endl;
      romLoads[instr->getOperand(0)->compactString()].push_back(instr);
    }
  }

  //auto def = f.getDef();
  //auto context = def->getContext();
  auto context = f.getContext();

  std::map<HWInstr*, HWInstr*> loadsToReplacements;
  for (auto m : romLoads) {
    string curveName = m.first.substr(1, m.first.size() - 2);
    auto values = map_get(curveName, storeCollector.constStores);
    nlohmann::json romVals;
    for (int i = 0; i < (int) values.size(); i++) {
      int val = map_get(i, values);
      romVals["init"].emplace_back(std::to_string(val));
    }
    CoreIR::Values vals{{"width", COREMK(context, 16)}, {"depth", COREMK(context, romVals["init"].size())}, {"nports", COREMK(context, m.second.size())}};
    CoreIR::Values params{{"init", COREMK(context, romVals)}};

    InstanceWrapper romWrapper{coreirSanitize(m.first), "halidehw.ROM", vals, params};
    //auto rom = def->addInstance(coreirSanitize(m.first), "halidehw.ROM", vals, {{"init", COREMK(context, romVals)}});
    //internal_assert(fromGenerator("halidehw.ROM", rom)) << "Did not produce a ROM in load optimization\n";

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
      //rLoad->unit = rom;
      rLoad->inst = romWrapper;
      loadsToReplacements[ld] = rLoad;
      portNo++;
    }
  }

  replaceAll(loadsToReplacements, f);

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
      callRep->resType = f.getContext()->Bit()->Arr(16);
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
  internal_assert(streamUsersInF.size() > 0) << " no streams that use " << stencilName << " in hardware kernel that contains it\n";
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
  internal_assert(f.numBlocks() == 1) << "function:\n" << f << "\n has multiple blocks\n";

  auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });
  // We do not currently handle multiple instruction groups
  //if (instrGroups.size() != 1) {
    //return;
  //}
  std::map<string, vector<HWInstr*> > provides;
  std::map<string, HWInstr*> stencilDecls;
  for (auto instr : f.structuredOrder()) {
    if (isCall("provide", instr)) {
      string target = instr->operands[0]->compactString();
      provides[target].push_back(instr);
      stencilDecls[target] = instr;
    }
  }

  cout << "Provides..." << endl;
  for (auto p : provides) {
    cout << "\t" << p.first << endl;
    for (auto c : p.second) {
      cout << "\t\t" << *c << endl;
    }
    set<HWInstr*> users = allVarUsers(p.first, f.structuredOrder());
    set<HWInstr*> readers;
    cout << "\tUsers of: " << p.first << endl;
    for (auto u : users) {
      cout << "\t\t" << *u << endl;
      if (u->name != "provide") {
        readers.insert(u);
      }
    }

    cout << "Readers = " << readers << endl;
    set<IBlock> needPhi;
    set<HWInstr*> newPhis;
    map<IBlock, HWInstr*> headerPhis;
    for (auto blk : getIBlocks(f)) {
      if (predecessors(blk, f).size() >= 2) {
        cout << "Block...\n" << blk << endl << "\tneeds phi" << endl;
        needPhi.insert(blk);
        auto phiInstr = f.newI();
        phiInstr->name = "phi";
        phiInstr->surroundingLoops = head(blk)->surroundingLoops;
        headerPhis[blk] = phiInstr;
        newPhis.insert(phiInstr);
    }
  }

    cout << needPhi.size() << " blocks need a phi for " << p.first << endl;
    map<HWInstr*, HWInstr*> provideReplacements;
    set<HWInstr*> provideReplacementSet;
    int provideNum = 0;
    for (auto instr : p.second) {
      auto refresh = f.newI(instr);
      refresh->operands = instr->operands;
      refresh->name = "create_stencil_" + std::to_string(provideNum);
      provideNum++;
      provideReplacements[instr] = refresh;
      provideReplacementSet.insert(refresh);
    }
   
    // Modify to insert final phi instructions
    for (auto blk : headerPhis) {
      f.insertAt(head(blk.first), blk.second);
    }

    cout << "Replacing " << provideReplacements.size() << " provides..." << endl;
    // Modify to insert provide replacements
    for (auto replacementPair : provideReplacements) {
      f.insertAt(replacementPair.first, replacementPair.second);
    }

    auto baseInit = f.newI();
    {
      auto provideValue = CoreIR::map_find(p.first, stencilDecls);
      auto provideName = provideValue->operands[0]->compactString();

      vector<int> dims = stencilDimsInBody(info, f, provideName);
      vector<HWInstr*> initialSets;
      for (auto instr : p.second) {
        auto operands = instr->operands;
        if (allConst(1, operands.size(), operands)) {
          initialSets.push_back(instr);
        } else {
          break;
        }
      }

      baseInit->resType = f.getContext()->Bit()->Arr(16);
      for (size_t i = 0; i < dims.size(); i += 2) {
        auto d = dims[i + 1] - dims[i];
        baseInit->resType = baseInit->resType->Arr(d);
      }
      baseInit->operands.push_back(f.newConst(32, dims.size()));
      cout << "Dims of " << provideName << endl;
      for (auto c : dims) {
        cout << "\t" << c << endl;
        baseInit->operands.push_back(f.newConst(32, c));
      }

      for (auto initI : initialSets) {
        for (int i = 1; i < (int) initI->operands.size(); i++) {
          baseInit->operands.push_back(initI->operands[i]);
        }
      }

      baseInit->surroundingLoops = f.structuredOrder()[0]->surroundingLoops;
      baseInit->name = "init_stencil_" + p.first;

      f.insertAt(f.structuredOrder()[0], baseInit);
    }

    cout << "Function after phi and provide substitution..." << endl;
    cout << f << endl;

    auto currentDef = baseInit;
    auto provideVar = f.newVar(p.first);
    for (auto instr : f.structuredOrder()) {
      if (elem(instr, newPhis)) {
        instr->operands.push_back(f.newVar(p.first));
      }
      
      replaceOperand(provideVar, currentDef, instr);
      if (elem(instr, newPhis) || elem(instr, provideReplacementSet)) {
        currentDef = instr;
      }

    }

    // Set all phi output types
    for (auto instr : f.structuredOrder()) {
      if (elem(instr, newPhis)) {
        instr->resType = instr->getOperand(0)->resType;
        internal_assert(instr->resType != nullptr);
      }
      if (elem(instr, provideReplacementSet)) {
        instr->resType = instr->getOperand(0)->resType;
        internal_assert(instr->resType != nullptr);
      }
    }
    // Now: find all reverse phis
    for (auto blk : getIBlocks(f)) {
      for (auto instr : blk.instrs) {
        if (elem(instr, newPhis)) {
          cout << "Adding second operand to phi.." << endl;
          auto tail = loopTail(blk, f);
          auto pDef = baseInit;
          for (auto i : f.structuredOrder()) {
            if (elem(i, newPhis) || elem(i, provideReplacementSet)) {
              pDef = i;
            }

            if (i == lastInstr(tail)) {
              break;
            }
          }
          instr->operands.push_back(pDef);
        }
      }
    }

    cout << "After replacing references to " << p.first << endl;
    cout << f << endl;
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
  for (auto instr : f.allInstrs()) {
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
          shrInstr->resType = f.getContext()->Bit()->Arr(16);
          //shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, 1 << (value - 1))};
          shrInstr->setSigned(instr->isSigned());

          shrInstr->operands = {instr->getOperand(0), f.newConst(instr->getOperand(1)->constWidth, constVal - 1)};

          //1 << (value - 1))};
          replacements.push_back({instr, shrInstr});
          //replacements[instr] = shrInstr;
        }
      }
    }
  }

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
  for (auto instr : f.allInstrs()) {
    if (isCall("div", instr)) {
      cout << "Found div: " << *instr << endl;
      if (isConstant(instr->getOperand(1))) {
        //cout << "\tDividing by constant = " << instr->getOperand(1)->compactString() << endl;
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

CoreIR::Instance* mkConst(CoreIR::ModuleDef* def, const std::string& name, const int width, const int val) {
  return def->addInstance(name, "coreir.const", {{"width", COREMK(def->getContext(), width)}}, {{"value", COREMK(def->getContext(), BitVector(width, val))}});
}

CoreIR::Wireable* andVals(CoreIR::ModuleDef* def, CoreIR::Wireable* a, CoreIR::Wireable* b) {
  //auto c = def->getContext();
  auto ad = def->addInstance("and_all_" + def->getContext()->getUnique(), "corebit.and");
  def->connect(ad->sel("in0"), a);
  def->connect(ad->sel("in1"), b);

  return ad->sel("out");
}

CoreIR::Wireable* andList(CoreIR::ModuleDef* def, const std::vector<CoreIR::Wireable*>& vals) {
  CoreIR::Wireable* val = nullptr;
  if (vals.size() == 0) {
    return def->addInstance("and_all_" + def->getContext()->getUnique(), "corebit.const", {{"value", COREMK(def->getContext(), true)}})->sel("out");
  }

  if (vals.size() == 1) {
    return vals[0];
  }

  val = vals[0];
  for (int i = 1; i < ((int) vals.size()); i++) {
    val = andVals(def, val, vals[i]);
  }
  return val;
}

CoreIR::Wireable* andList(const std::vector<CoreIR::Wireable*>& vals) {
  internal_assert(vals.size() > 0) << "No values to and\n";

  auto def = vals[0]->getContainer();
  return andList(def, vals);
}

class FSMTransition {
  public:
    int src;
    int dst;
    int delay;
    std::string name;
};

bool operator<(const FSMTransition& a, const FSMTransition& b) {
  if (a.src != b.src) {
    return a.src < b.src;
  }

  if (a.dst != b.dst) {
    return a.dst < b.dst;
  }

  if (a.delay != b.delay) {
    return a.delay < b.delay;
  }

  return a.name < b.name;
}

class LoopCounters {
  public:
    std::vector<CoreIR::Wireable*> loopLevelCounters;
    std::vector<CoreIR::Wireable*> levelAtMax;
    std::map<std::string, Instance*> loopVarNames;
    std::map<string, Wireable*> loopVarAtMax;
    std::map<string, Wireable*> loopVarNotAtMax;
    std::vector<std::string> loopNames;

  int index(const std::string& name) const {
    int i = 0;
    for (auto l : loopNames) {
      if (l == name) {
        return i;
      }
      i++;
    }
    internal_assert(false) << "no index for " << name << "\n";
    return -1;
  }
};

Instance* buildCounter(CoreIR::ModuleDef* def, const std::string& name, int min_value, int max_value, int inc) {
  auto c = def->getContext();
  int width = 16;
  CoreIR::Values args = {{"width",CoreIR::Const::make(c, width)},
    {"min",CoreIR::Const::make(c, min_value)},
    {"max",CoreIR::Const::make(c, max_value)},
    {"inc",CoreIR::Const::make(c, inc)}};

  CoreIR::Instance* counter_inst = def->addInstance(name, "commonlib.counter", args);
  def->connect(counter_inst->sel("reset"), def->sel("self")->sel("reset"));
  return counter_inst;
}

//LoopCounters buildLoopCounters(CoreIR::ModuleDef* def, LoopNestInfo& loopInfo) {
LoopCounters buildLoopCounters(CoreIR::ModuleDef* def, HWFunction& f) {

  set<string> allLoopNames;
  LoopNestInfo loopInfo;
  for (auto instr : f.structuredOrder()) {
    for (auto lp : instr->surroundingLoops) {
      if (!elem(lp.name, allLoopNames)) {
        loopInfo.loops.push_back({lp.name, func_id_const_value(lp.min), func_id_const_value(lp.extent)});
        allLoopNames.insert(lp.name);
      }
    }
  }
  cout << "# of levels in loop for control path = " << loopInfo.loops.size() << endl;
  
  int width = 16;
  auto c = def->getContext();
  auto self = def->sel("self");

  LoopCounters counters;
  for (auto l : loopInfo.loops) {
    counters.loopNames.push_back(l.name);
    int min_value = l.min;
    int max_value = min_value + l.extent - 1;
    int inc_value = 1;

    string varName = coreirSanitize(l.name);
    auto counter_inst = buildCounter(def, varName, min_value, max_value, inc_value);
    //CoreIR::Values args = {{"width",CoreIR::Const::make(c, width)},
      //{"min",CoreIR::Const::make(c, min_value)},
      //{"max",CoreIR::Const::make(c, max_value)},
      //{"inc",CoreIR::Const::make(c, inc_value)}};

    //CoreIR::Instance* counter_inst = def->addInstance(varName, "commonlib.counter", args);
    counters.loopVarNames[l.name] = counter_inst;
    counters.loopLevelCounters.push_back(counter_inst);

    // If this loop variable is actually used in the kernel then connect it to the outside world
    if (self->canSel(varName)) {
      def->connect(counter_inst->sel("out"), self->sel(varName));
    }
    //def->connect(counter_inst->sel("reset"), def->sel("self")->sel("reset"));

    auto maxValConst = mkConst(def, varName + "_max_value", width, max_value);
    auto atMax = def->addInstance(varName + "_at_max", "coreir.eq", {{"width", COREMK(c, width)}});
    def->connect(atMax->sel("in0"), maxValConst->sel("out"));
    def->connect(atMax->sel("in1"), counter_inst->sel("out"));
    counters.loopVarAtMax[l.name] = atMax->sel("out");

    auto notAtMax = def->addInstance(varName + "_not_at_max", "corebit.not");
    def->connect(notAtMax->sel("in"), atMax->sel("out"));
    counters.loopVarNotAtMax[l.name] = notAtMax->sel("out");

    counters.levelAtMax.push_back(atMax);
  }

  internal_assert(counters.levelAtMax.size() == counters.loopLevelCounters.size());

  return counters;
}

vector<ProgramPosition> buildProgramPositions(FunctionSchedule& sched) {

  auto& f = *(sched.f);
  set<string> headLevelsSeen;
  set<string> tailLevelsSeen;
  vector<ProgramPosition> positions;
  for (auto blk : getIBlockList(f)) {
    if (blk.isEntry()) {
      continue;
    }

    bool h = isHeader(blk, f);
    bool t = isTail(blk, f);
    for (auto instr : blk.instrs) {
      //cout << "Creating positions for " << *instr << endl;

      internal_assert(instr->surroundingLoops.size() > 0);
      if (h && (head(blk) == instr)) {
        //cout << "\tIs head of header" << endl;
        for (auto lp : instr->surroundingLoops) {
          string loopName = lp.name;
          if (!elem(loopName, headLevelsSeen)) {
            positions.push_back({instr, loopName, true, false});
            headLevelsSeen.insert(loopName);
          }
        }
      }

      if (t && (tail(blk) == instr)) {
        //cout << "\tIs tail of tail" << endl;
        auto surrounding = instr->surroundingLoops;
        CoreIR::reverse(surrounding);
        for (auto lp : surrounding) {
          string loopName = lp.name;
          vector<string> nextLoops;
          if (!lastBlock(blk, f)) {
            nextLoops = loopNames(head(nextBlock(blk, f)));
          }
          if (elem(loopName, headLevelsSeen) &&
              !elem(loopName, nextLoops)) {
            positions.push_back({instr, loopName, false, true});
          }
        }
      }

      if (!(t && (tail(blk) == instr)) && !(h && (head(blk) == instr))) {
        //cout << "Not head of header or tail of tail" << endl;
        string ll = instr->surroundingLoops.back().name;
        positions.push_back({instr, ll, false, false});
      }
    }
  }

  //cout << "Program positions..." << endl;
  //for (auto p : positions) {
    //cout << p << endl;
  //}

  for (int i = 0; i < (int) positions.size(); i++) {
    auto p = positions[i];
    for (int j = 0; j < (int) positions.size(); j++) {
      auto other = positions[j];
      if (i != j) {
        internal_assert(other != p) << "Duplicate program positions\n";
        //cout << p << " is not equal to:\n" << other << endl;
      }
    }
  }

  for (auto instr : f.structuredOrder()) {
    bool found = false;
    for (auto p : positions) {
      if (p.instr == instr) {
        found = true;
        break;
      }
    }

    internal_assert(found) << "Did not create position containing: " << *instr << "\n";
  }
  return positions;
}

vector<SWTransition> buildSWTransitions(vector<ProgramPosition>& positions, HWFunction& f) {

  vector<SWTransition> transitions;
  for (int i = 0; i < ((int) positions.size() - 1); i++) {
    ProgramPosition current = positions[i];
    ProgramPosition next = positions[i + 1];
    if (current.isHead()) {
      transitions.push_back({current, current, notAtMax(current.loopLevel)});
    }

    if (lessThan(current.loopLevel, next.loopLevel, f)) {
      transitions.push_back({current, next, unconditional()});
    } else if (lessThan(next.loopLevel, current.loopLevel, f)) {
      transitions.push_back({current, next, atMax(current.loopLevel)});
    } else if (current.loopLevel == next.loopLevel) {
      // We will handle default transitions later on
    }
  }

  //cout << "Software transitions..." << endl;
  //for (auto t : transitions) {
    //cout << t << endl;
  //}

  return transitions;
}

ProgramPosition headerPosition(const std::string& level, const vector<ProgramPosition>& positions) {
  for (auto p : positions) {
    if (p.isHead() && p.loopLevel == level) {
      return p;
    }
  }

  internal_assert(false) << "No headerPosition for " << level << "\n";
  return *(begin(positions));
}

bool isEndPosition(const ProgramPosition& active, vector<ProgramPosition>& positions) {
  return positions.back() == active;
}

ProgramPosition nextPosition(const ProgramPosition& active, vector<ProgramPosition>& positions) {
  for (int i = 0; i < ((int) positions.size()) - 1; i++) {
    if (positions[i] == active) {
      return positions[i + 1];
    }
  }

  internal_assert(false) << " is the last position\n";
  return active;
}

vector<SWTransition> hwTransitions(FunctionSchedule& sched) {
  auto& f = *(sched.f);
  vector<ProgramPosition> positions = buildProgramPositions(sched);
  vector<SWTransition> transitions =
    buildSWTransitions(positions, f);

  map<int, vector<IChunk> > chunks;
  for (auto p : positions) {
    int s = sched.getStartStage(p.instr);
    if (contains_key(s, chunks)) {
      auto& chunkList = chunks.at(s);
      bool foundChunk = false;
      for (auto& chunk : chunkList) {
        internal_assert(chunk.instrs.size() > 0) << "chunk has no instructions\n";
        auto representative = chunk.instrs.at(0);
        if (representative.loopLevel == p.loopLevel &&
            (representative.isOp() || p.isOp())) {
          foundChunk = true;
          chunk.instrs.push_back(p);
        }
      }
      if (!foundChunk) {
        chunkList.push_back({s, {p}});
      }
    } else {
      chunks[s] = {{s, {p}}};
    }
  }
  //cout << "All instruction chunks..." << endl;
  //for (auto cs : chunks) {
    //for (auto chunk : cs.second) {
      //cout << "\t" << chunk.stage << endl;
      //for (auto pos : chunk.instrs) {
        //cout << "\t\t" << pos << endl;
      //}
    //}
  //}

  vector<IChunk> chunkList;
  for (auto sc : chunks) {
    for (auto c : sc.second) {
      chunkList.push_back(c);
    }
  }

  vector<SWTransition> relevantTransitions;
  for (auto t : transitions) {
    int startChunk = chunkIdx(t.src, chunkList);
    int endChunk = chunkIdx(t.dst, chunkList);
    if (startChunk != endChunk || (t.src == t.dst)) {
      relevantTransitions.push_back(t);
    }
  }

  vector<vector<ProgramPosition> > programBlocks =
    split_by(positions, [](const ProgramPosition& a, const ProgramPosition& b) {
        return a.loopLevel == b.loopLevel;
        });

  //cout << "Program blocks" << endl;
  //for (auto blk : programBlocks) {
    //cout << "##### Block" << endl;
    //for (auto instr : blk) {
      //cout << "\t" << instr << endl;
    //}
  //}

  // Add default transitions
  for (auto sc : chunks) {
    int stage = sc.first;
    vector<IChunk> chunksInStage = sc.second;
    int next = stage + 1;
    if (contains_key(next, chunks)) {
      vector<IChunk> nextChunks = map_get(next, chunks);

      for (auto c : chunksInStage) {
        for (auto n : nextChunks) {
          ProgramPosition rep = c.getRep();
          ProgramPosition nextRep = n.getRep();

          for (auto blk : programBlocks) {
            if (elem(rep, blk) &&
                elem(nextRep, blk)) {
              //cout << "Adding default transition from\n\t" << rep << "\nto\n\t" << nextRep << endl;
              relevantTransitions.push_back({rep, nextRep, unconditional()});
              break;
            }
          }
        }
      }
    }
  }

  return relevantTransitions;
}

Expr endTime(const ProgramPosition& pos, FunctionSchedule& sched) {
  return startTime(pos, sched) + pos.instr->latency;
}

Expr endTime(HWInstr* instr, FunctionSchedule& sched) {
  return startTime(instr, sched) + instr->latency;
}

Expr tripCount(std::string loop, HWInstr* instr) {
  for (auto lp : instr->surroundingLoops) {
    if (lp.name == loop) {
      return lp.extent;
    }
  }
  internal_assert(false) << "no trip count for " << loop << " in " << *instr << "\n";
  return Expr(0);
}

vector<SWTransition> outTransitions(const ProgramPosition& pos, const vector<SWTransition>& transitions) {
  vector<SWTransition> outs;
  for (auto t : transitions) {
    if (t.src == pos) {
      outs.push_back(t);
    }
  }
  return outs;
}

vector<SWTransition> forwardTransition(const IChunk& next, const vector<SWTransition>& transitions) {
  vector<SWTransition> outs;
  for (auto t : transitions) {
    if (next.containsPos(t.src) &&
        !next.containsPos(t.dst)) {
      outs.push_back(t);
    }
  }
  return outs;
}

Expr startTime(const std::vector<ProgramPosition>& positions,
    const ProgramPosition& targetPos,
    FunctionSchedule& sched) {
  string instrLoopLevel = targetPos.loopLevel;

  auto f = *(sched.f);
  auto& transitions = sched.transitions;
  //auto transitions = hwTransitions(sched);

  internal_assert(positions.size() > 0) << "no program positions\n";

  map<int, vector<IChunk> > chunks = getChunks(positions, sched);
  vector<IChunk> chunkList;
  for (auto sc : chunks) {
    for (auto c : sc.second) {
      chunkList.push_back(c);
    }
  }

  ProgramPosition pos = positions[0];
  IChunk next = getChunk(pos, chunkList);
  vector<IChunk> forwardPath;
  do {
    //cout << "Getting forward transitions for..." << endl;
    //cout << next << endl;

    vector<SWTransition> forward = forwardTransition(next, transitions);
    //cout << "Forward transitions..." << endl;
    //for (auto f : forward) {
      //cout << f << endl;
    //}

    internal_assert(forward.size() <= 1);
    if (forward.size() == 0) {
      internal_assert(next.containsPos(targetPos));
    }

    forwardPath.push_back(next);

    if (next.containsPos(targetPos)) {
      break;
    }

    next = getChunk(forward[0].dst, chunkList);
  } while (true);

  //cout << "Forward path length: " << forwardPath.size() << endl;
  // Now how do we compute startTime?
  // First: walk over checking for backedges and adding them
  // And: add delays on chunk -> chunk codes
  Expr startTime = Expr(0);
  for (int i = 0; i < (int) forwardPath.size(); i++) {
    IChunk c = forwardPath[i];
    if (c.containsHeader()) {
      if (lessThanOrEqual(c.getRep().loopLevel, instrLoopLevel, f)) {
        startTime += sched.II(c.getRep().loopLevel) * Variable::make(Int(32), c.getRep().loopLevel);
      } else {
        startTime += sched.II(c.getRep().loopLevel) * (tripCount(c.getRep().loopLevel, c.getRep().instr) - 1);
      }
    }

    if (i < ((int) forwardPath.size()) - 1) {
      IChunk nextChunk = forwardPath[i + 1];
      int stageDiff = nextChunk.stage - c.stage;
      internal_assert(0 <= stageDiff && stageDiff <= 1) << "stage diff: " << stageDiff << "\n";
      startTime += (nextChunk.stage - c.stage);
    }
  }
  //cout << "Start time for " << *instr << " = " << startTime << endl;

  return simplify(startTime);
}

Expr startTime(const ProgramPosition& targetPos,
    FunctionSchedule& sched) {
  //auto positions = buildProgramPositions(sched);
  return startTime(sched.positions, targetPos, sched);
}

ProgramPosition getPosition(HWInstr* instr, const vector<ProgramPosition>& positions) {
  for (auto p : positions) {
    if (p.isOp() && p.instr == instr) {
      return p;
    }
  }
  internal_assert(false) << "No position for: " << *instr << "\n";
  return positions.back();
}

Expr startTime(HWInstr* instr, FunctionSchedule& sched) {
  vector<ProgramPosition> positions = buildProgramPositions(sched);
  ProgramPosition pos = getPosition(instr, positions);
  return startTime(pos, sched);
}

map<int, vector<IChunk> > getChunks(const vector<ProgramPosition>& positions,
    FunctionSchedule& sched) {

  map<int, vector<IChunk> > chunks;
  for (auto p : positions) {
    int s = sched.getStartStage(p.instr);
    if (contains_key(s, chunks)) {
      auto& chunkList = chunks.at(s);
      bool foundChunk = false;
      for (auto& chunk : chunkList) {
        internal_assert(chunk.instrs.size() > 0) << "chunk has no instructions\n";
        auto representative = chunk.instrs.at(0);
        if (representative.loopLevel == p.loopLevel &&
            (representative.isOp() || p.isOp())) {
          foundChunk = true;
          chunk.instrs.push_back(p);
        }
      }
      if (!foundChunk) {
        chunkList.push_back({s, {p}});
      }
    } else {
      chunks[s] = {{s, {p}}};
    }
  }
  return chunks;
}

class ArithGenerator : public IRGraphVisitor {

  public:

    ModuleDef* def;
    Wireable* output;
    map<string, Wireable*> varWires;

    ArithGenerator(ModuleDef* def_, map<string, Wireable*>& varWires_) : def(def_), output(nullptr), varWires(varWires_) {}

    Wireable* getOutput() const {
      internal_assert(output != nullptr) << "Output is null\n";
      return output;
    }

  protected:

    using IRGraphVisitor::visit;
    
    void visit(const IntImm* v) {
      auto c = def->getContext();
      output =
        def->addInstance("arith_const" + c->getUnique(), "coreir.const", {{"width", COREMK(c, 16)}}, {{"value", COREMK(c, BitVector(16, v->value))}})->sel("out");
    }

    void visit(const Variable* var) {
      output = map_get(var->name, varWires);
    }

    void visit(const Mul* add) {
      add->a.accept(this);
      auto op0 = getOutput();

      add->b.accept(this);
      auto op1 = getOutput();

      auto c = def->getContext();
      auto val = 
        def->addInstance("mul" + c->getUnique(), "coreir.mul", {{"width", COREMK(c, 16)}});
      output =
        val->sel("out");
      def->connect(op0, val->sel("in0"));
      def->connect(op1, val->sel("in1"));
    }

    void visit(const Add* add) {
      add->a.accept(this);
      auto op0 = getOutput();

      add->b.accept(this);
      auto op1 = getOutput();

      auto c = def->getContext();
      auto val = 
        def->addInstance("add" + c->getUnique(), "coreir.add", {{"width", COREMK(c, 16)}});
      output =
        val->sel("out");
      def->connect(op0, val->sel("in0"));
      def->connect(op1, val->sel("in1"));
    }

};

// Now: Need to add handling for inner loop changes.
// the innermost loop should be used instead of the lexically
// first instruction, but even that will just patch the problem
KernelControlPath controlPathForKernel(FunctionSchedule& sched) {
  auto& f = *(sched.f);
  vector<ProgramPosition> positions = buildProgramPositions(sched);
  vector<SWTransition> transitions =
    buildSWTransitions(positions, f);

  map<int, vector<IChunk> > chunks = getChunks(positions, sched);
  vector<IChunk> chunkList;
  for (auto sc : chunks) {
    for (auto c : sc.second) {
      chunkList.push_back(c);
    }
  }
  
  cout << "All instruction chunks..." << endl;
  for (auto cs : chunks) {
    for (auto chunk : cs.second) {
      cout << "\tStage: " << chunk.stage << endl;
      for (auto pos : chunk.instrs) {
        cout << "\t\t" << pos << endl;
        //cout << "\t\t\tStart time: " << startTime(chunk.getRep(), sched) << endl;
      }
    }
  }

  auto c = f.getContext();
  KernelControlPath cp;
  vector<std::pair<std::string, CoreIR::Type*> > tps{{"reset", c->BitIn()}, {"in_en", c->BitIn()}, {"started", c->Bit()}, {"cycles_since_start", c->Bit()->Arr(16)}};
  for (int i = 0; i < (int) chunkList.size(); i++) {
    string s = "chunk_" + to_string(i) + "_active";
    tps.push_back({s, c->Bit()});
    cp.stageIsActiveMap[i] = s;

    {
      string s = "chunk_phi_" + to_string(i) + "_at_zero";
      tps.push_back({s, c->Bit()});
      cp.chunkPhiInputMap[i] = s;
    }
  }

  std::set<std::string> streamNames = allStreamNames(f);
  auto globalNs = c->getNamespace("global");
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
  cp.m = controlPath;

  cp.chunkList = chunkList;
  auto def = controlPath->newModuleDef();

  // What test should be used to decide if we need a real control
  // path?
  // - For now just check if all instructions have single loop level?

  auto instrGroups = group_unary(f.structuredOrder(), [](const HWInstr* i) { return i->surroundingLoops.size(); });
  if (instrGroups.size() > 1) {
  //if (false) {
  //if (true) {
    // Just to be safe for early tests, should really be 1 << 15 - 1
    const int MAX_CYCLES = ((1 << 14) - 1);
    cout << "MAX_CYCLES = " << MAX_CYCLES << endl;
    auto cyclesSinceStartCounter =
      buildCounter(def, "cycles_since_start", 0, MAX_CYCLES, 1);

    def->connect(def->sel("self.cycles_since_start"), cyclesSinceStartCounter->sel("out"));

    auto startedReg = def->addInstance("started_reg", "corebit.reg");
    auto startedBefore = def->addInstance("started_before", "corebit.or");

    def->connect(startedBefore->sel("in0"), startedReg->sel("out"));
    def->connect(startedBefore->sel("in1"), def->sel("self.in_en"));

    def->connect(startedReg->sel("in"), startedBefore->sel("out"));

    auto started = startedBefore->sel("out");

    //auto started = def->addInstance("started_const_dummy", "corebit.const", {{"value", COREMK(c, true)}})->sel("out");

    def->connect(cyclesSinceStartCounter->sel("en"), started);
    def->connect(def->sel("self.started"), started);

    // This is a map from loop variable names to the counter
    // for that variable which is created for the chunk that contains
    // the head instruction of that variables loop level
    map<string, Instance*> headLoopCounters;

    for (auto chunk : chunkList) {
      ProgramPosition pos = chunk.getRep();
      cout << "Creating control for chunk position: " << pos << endl;
      Expr st = startTime(pos, sched);

      Instance* doInc =
        def->addInstance("chunk_" + to_string(chunkIdx(chunk, chunkList)) + "_active",
            "halidehw.passthrough",
            {{"type", COREMK(c, c->Bit())}});

      map<string, Wireable*> countVals;
      map<string, Instance*> countInstances;
      for (auto lp : chunk.getRep().instr->surroundingLoops) {
        string loop = lp.name;
        if (lessThanOrEqual(loop, chunk.getRep().loopLevel, f)) {

          int min_value = func_id_const_value(lp.min);
          int max_value = min_value + func_id_const_value(lp.extent) - 1;
          cout << loop << " <= " << chunk.getRep().loopLevel << endl;
          auto loopVarCounter =
            buildCounter(def, coreirSanitize(loop) + "_value_counter" + c->getUnique(), min_value, max_value, 1);
          countVals[loop] = loopVarCounter->sel("out");
          countInstances[loop] = loopVarCounter;
        }
      }

      if (chunk.containsHeader()) {
        headLoopCounters[chunk.getRep().loopLevel] = map_get(chunk.getRep().loopLevel, countInstances);
      }

      cout << "Wiring up control counter enables" << endl;
      for (auto lp : chunk.getRep().instr->surroundingLoops) {
        string loop = lp.name;
        if (lessThanOrEqual(loop, chunk.getRep().loopLevel, f)) {

          vector<CoreIR::Wireable*> below;
          for (auto innerLoop : chunk.getRep().instr->surroundingLoops) {
            string name = innerLoop.name;
            if (!lessThanOrEqual(name, lp.name, f) &&
                (lessThanOrEqual(name, chunk.getRep().loopLevel, f))) {

              // Create atMax circuit
              internal_assert(contains_key(name, countVals)) << "Cannot find " << name << " in count values for " << lp.name << "\n";

              auto countVal = map_get(name, countVals);
              int min_value = func_id_const_value(innerLoop.min);
              int max_value = min_value + func_id_const_value(innerLoop.extent) - 1;
              auto maxConst =
                def->addInstance(coreirSanitize(name) + "_max_val" + c->getUnique(), "coreir.const", {{"width", COREMK(c, 16)}}, {{"value", COREMK(c, BitVec(16, max_value))}});
              auto atMax =
                def->addInstance(coreirSanitize(name) + "_at_max" + c->getUnique(), "coreir.eq", {{"width", COREMK(c, 16)}});
              def->connect(atMax->sel("in0"), countVal);
              def->connect(atMax->sel("in1"), maxConst->sel("out"));

              below.push_back(atMax->sel("out"));
            }
          }
          below.push_back(doInc->sel("out"));
          below.push_back(started);
          CoreIR::Wireable* shouldInc = andList(below);
          def->connect(map_get(lp.name, countInstances)->sel("en"), shouldInc);
        }
      }

      ArithGenerator arithGen(def, countVals);
      st.accept(&arithGen);

      Wireable* schedFValue = arithGen.getOutput();
      Instance* timeDiff =
        def->addInstance("is_active" + c->getUnique(), "coreir.sub", {{"width", COREMK(c, 16)}});
      def->connect(timeDiff->sel("in0"), cyclesSinceStartCounter->sel("out"));
      def->connect(timeDiff->sel("in1"), schedFValue);

      Wireable* zr =
        def->addInstance("zr" + c->getUnique(), "coreir.const", {{"width", COREMK(c, 16)}}, {{"value", COREMK(c, BitVec(16, 0))}})->sel("out");

      Instance* diffZero =
        def->addInstance("diff_zero" + c->getUnique(), "coreir.eq", {{"width", COREMK(c, 16)}});
      def->connect(diffZero->sel("in0"), timeDiff->sel("out"));
      def->connect(diffZero->sel("in1"), zr);

      auto incAnd = andList({diffZero->sel("out"), started});
      def->connect(incAnd, def->sel(cp.activeSignal(chunk)));
      def->connect(incAnd, doInc->sel("in"));
      //def->connect(diffZero->sel("out"), def->sel(cp.activeSignal(chunk)));
      //def->connect(diffZero->sel("out"), doInc->sel("in"));
    }

    cout << "Created value checks for chunks, now generting loop counters" << endl;

    for (auto loop : loops(f.structuredOrder())) {
      //auto loopVarCounter = map_get(loop, headLoopCounters);
      //cout << "Building counter for: " << loop << endl;
      int min_value = func_id_const_value(loop.min);
      int max_value = min_value + func_id_const_value(loop.extent) - 1;
      auto loopVarCounter =
        buildCounter(def, coreirSanitize(loop.name) + "_value_counter", min_value, max_value, 1);

      ProgramPosition headerPos = headerPosition(loop.name, positions);
      IChunk headerChunk = getChunk(headerPos, chunkList);
      def->connect(loopVarCounter->sel("en"), def->sel(cp.activeSignalOutput(headerChunk))->sel("out"));
      //cout << "Connecting counter for " << loop << " to " << coreStr(def->sel("self")->sel(coreirSanitize(loop))) << endl;

      def->connect(loopVarCounter->sel("out"), def->sel("self")->sel(coreirSanitize(loop.name)));

      auto eqz =
        def->addInstance(coreirSanitize(loop.name) + "_phi_use_one", "coreir.eq", {{"width", COREMK(c, 16)}});
      def->connect(eqz->sel("in0"), mkConst(def, "phi_one" + c->getUnique(), 16, 0)->sel("out"));
      def->connect(eqz->sel("in1"), loopVarCounter->sel("out"));

      string phiOutName = map_get(chunkIdx(headerChunk, chunkList), cp.chunkPhiInputMap);

      auto chunkActive = def->sel(cp.activeSignalOutput(headerChunk))->sel("out");
      auto phiActiveSignal =
        andList({eqz->sel("out"), chunkActive});
      def->connect(phiActiveSignal, def->sel("self")->sel(phiOutName));
      //def->connect(eqz->sel("out"), def->sel("self")->sel(phiOutName));
    }

    controlPath->setDef(def);

    cp.m = controlPath;

    cout << "Control path just before returning it: " << endl;
    cp.m->print();
    return cp;
  }

  // Now: build counters for each variable
  // Once that is done we need to wire each one
  // to the the and of enable and lexmax

  //internal_assert(false);

  //// Now: Delete transitions within chunks
  ////      For each transition across chunks, but inside of a stage we need comb logic
  ////      For each transition across chunks and across stages we need sequential logic
  ////      to delay the transition
  vector<SWTransition> relevantTransitions =
    hwTransitions(sched);

  // Delete transitions for valid loops -> valid loops?
  auto reads = allInstrs("rd_stream", f.structuredOrder());
  ProgramPosition readPos;
  if (reads.size() == 0) {
    internal_assert(f.structuredOrder().size() > 0) << "no instructions in function\n";
    readPos = headerPosition(f.structuredOrder()[0]->surroundingLoops.back().name, positions);
  } else {
    auto read = *(begin(reads));
    readPos = headerPosition(read->surroundingLoops.back().name, positions);
  }

  string enTriggeredLevel = readPos.loopLevel;
  set<string> earlier = earlierLevels(enTriggeredLevel, f);
  set<string> enTriggeredLevels = earlier;
  enTriggeredLevels.insert(enTriggeredLevel);

  // Now what do I want to do?
  // Split up enable triggered loops and internally triggered loops
  // Remove enable triggered loop backward transitions from transitions?
  vector<SWTransition> enTriggered;
  for (auto t : relevantTransitions) {
    if (t.src.isHead() &&
        t.dst.isHead() &&
        elem(t.src.loopLevel, enTriggeredLevels) &&
        elem(t.dst.loopLevel, enTriggeredLevels)) {
      enTriggered.push_back(t);
    }
  }

  CoreIR::subtract(relevantTransitions, enTriggered);

  // Now: classify transitions by start and end stage + delay
  map<SWTransition, int> srcStages;
  map<SWTransition, int> dstStages;
  map<SWTransition, int> delays;
  for (auto t : relevantTransitions) {
    int src = sched.getStartStage(t.src.instr);
    int dst = sched.getStartStage(t.dst.instr);
    int delay = dst - src;
    if (t.src == t.dst) {
      delay = sched.II(t.src.loopLevel);
    }
    internal_assert(delay >= 0) << delay << " is negative!\n";
    srcStages[t] = src;
    dstStages[t] = dst;
    delays[t] = delay;
  }

  cout << "Relevant transitions..." << endl;
  for (auto t : relevantTransitions) {
    cout << t << endl;
  }


  cout << "Creating activewires..." << endl;
  auto context = def->getContext();
  map<int, Instance*> isActiveWires;
  for (int i = 0; i < (int) chunkList.size(); i++) {
    isActiveWires[i] = def->addInstance("stage_" + to_string(i) + "_is_active", "halidehw.passthrough", {{"type", COREMK(context, context->Bit())}});
  }

  LoopCounters counters = buildLoopCounters(def, f);

  cout << "Creating transition wires..." << endl;
  map<SWTransition, Wireable*> transitionHappenedWires;
  for (auto t : relevantTransitions) {
    Wireable* transitionCondition = nullptr;

    // TODO: Use datapath in unitmapping to get the wire
    // for atMax / not atMax registers
    if (t.cond.isUnconditional) {
      transitionCondition =
        def->addInstance("unconditional_transition_" + def->getContext()->getUnique(), "corebit.const", {{"value", COREMK(context, true)}})->sel("out");
    } else if (t.cond.isAtMax()) {
      transitionCondition =
        map_get(t.src.loopLevel, counters.loopVarAtMax);
    } else {
      internal_assert(t.cond.isNotAtMax());
      transitionCondition =
        map_get(t.src.loopLevel, counters.loopVarNotAtMax);
    }

    internal_assert(transitionCondition != nullptr) << "transition condition is null\n";

    int delay = map_get(t, delays);
    vector<Instance*> regs =
      pipelineRegisterChain(def,
          "transition_" + def->getContext()->getUnique() + "_" + to_string(delay),
          def->getContext()->Bit(), delay);

    IChunk srcChunk = getChunk(t.src, chunkList);
    auto chunkActive =
      map_get(chunkIdx(t.src, chunkList), isActiveWires)->sel("out");
    vector<Wireable*> conds = {chunkActive, transitionCondition};
    auto doTransition = andList(conds);
    def->connect(regs[0]->sel("in"), doTransition);

    transitionHappenedWires[t] = regs.back()->sel("out");
  }

  cout << "Connecting stage active wires..." << endl;

  cout << "Wiring up counter enables for " << counters.loopLevelCounters.size() << " loop levels" << endl;
  auto self = def->sel("self");
  auto in_en = self->sel("in_en");

  for (auto c : chunkList) {
    cout << "Connecting stage for chunk: " << c.stage << ", " << c.instrs << endl;
    if (c.containsHeader()) {
      // TODO: Check if this is an outer loop level or not
      string name = c.getRep().loopLevel;
      vector<CoreIR::Wireable*> below;
      for (auto lp : loopNames(f.structuredOrder())) {
        if (lessThan(name, lp, f)) {
          below.push_back(counters.levelAtMax[counters.index(lp)]->sel("out"));
        }
      }

      //if (below.size() == 0) {
        below.push_back(in_en);
      //}
      CoreIR::Wireable* shouldInc = andList(def, below);
      int i = counters.index(name);
      def->connect(counters.loopLevelCounters[i]->sel("en"), shouldInc);
      def->connect(shouldInc, map_get(chunkIdx(c, chunkList), isActiveWires)->sel("in"));
    }  else {
      vector<Wireable*> transitionWires;
      for (auto t : relevantTransitions) {
        if (getChunk(t.dst, chunkList) == c) {
          transitionWires.push_back(map_get(t, transitionHappenedWires));
        }
      }
      cout << "Creating isActive value for chunk" << endl;
      cout << c << endl;
      internal_assert(transitionWires.size() > 0) << "No transition wires for chunk\n";
      auto isActive = andList(def, transitionWires);
      def->connect(isActive, map_get(chunkIdx(c, chunkList), isActiveWires)->sel("in"));

      internal_assert(contains_key(chunkIdx(c, chunkList), isActiveWires)) << chunkIdx(c, chunkList) << " is not in activeWires\n";
    }

    def->connect(map_get(chunkIdx(c, chunkList), isActiveWires)->sel("out"), def->sel(cp.activeSignal(c)));
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

CoreIR::Instance* coreir_lb = def->addInstance(lb_name, "commonlib.linebuffer", lb_args);
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
        return fromGenerator("commonlib.linebuffer", instBase);
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
    //return design_type;
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
    const std::vector<CoreIR_Argument>& args,
    StencilInfo& scl) {
  map<string, StreamUseInfo> streamUseInfo;
  for (auto a : args) {
    if (a.is_stencil) {
      StreamNode aN = argNode(a);
      //streamGraph.addVertex(aN);
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
    //streamGraph.addVertex(forNode);
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
      StreamSubset dispatchParams = findDispatch(streamStr, dispatchName, scl);
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
  for (auto lb : scl.linebuffers) {
    cout << "\t" << lb[1] << endl;
    streamUseInfo[lb[1]].writer = lbNode(lbName(lb));
    cout << "\t" << lb[0] << endl;
    streamUseInfo[lb[0]].readers.push_back({lbNode(lbName(lb)), {}});
  }

  cout << "Checking dispatch statements" << endl;
  for (auto sd : scl.streamDispatches) {
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

AppGraph buildAppGraph(std::map<const For*, HWFunction>& functions,
    std::map<const For*, ComputeKernel>& kernelModules,
    std::map<const For*, CoreIR::Instance*>& kernels,
    const std::vector<CoreIR_Argument>& args,
    AcceleratorInterface& ifc,
    StencilInfoCollector& scl) {

  DirectedGraph<StreamNode, StreamSubset> streamGraph;

  for (auto a : args) {
    if (a.is_stencil) {
      StreamNode aN = argNode(a);
      streamGraph.addVertex(aN);
    }
  }
  for (auto f : functions) {
    StreamNode forNode = nestNode(f.first);
    streamGraph.addVertex(forNode);
  }
  for (auto lb : scl.info.linebuffers) {
    StreamNode lbN = lbNode(lbName(lb));
    streamGraph.addVertex(lbN);
  }

  map<string, StreamUseInfo> streamUseInfo =
    createStreamUseInfo(functions, args, scl.info);
  
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
    delay->resType = f.getContext()->Bit()->Arr(16);
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

HWInstr* writeTo(const std::string& streamName, HWFunction& f) {
  for (auto instr : allInstrs("write_stream", f.structuredOrder())) {
    if (instr->getOperand(0)->compactString() == streamName) {
      return instr;
    }
  }

  internal_assert(false) << "No read from " << streamName << "\n";
  return nullptr;
}

HWInstr* readFrom(const std::string& streamName, HWFunction& f) {
  for (auto instr : allInstrs("rd_stream", f.structuredOrder())) {
    if (instr->getOperand(0)->compactString() == streamName) {
      return instr;
    }
  }

  internal_assert(false) << "No read from " << streamName << "\n";
  return nullptr;
}

class RateInfo {
  public:
    std::vector<NestSchedule> nestSchedules;
};

void adjustIIs(StencilInfo& stencilInfo, map<string, StreamUseInfo>& streamUseInfo, map<const For*, FunctionSchedule>& functionSchedules) {

  // TODO: Dont assume all reads and writes in the same nest
  // must be at the same rate, so that we can support upsample and
  // downsample
  //
  // TODO: Replace with real adjustment code
  // NOTE: This code does work for the conv examples
  //for (auto& fp : functionSchedules) {
    //auto& sched = fp.second;
    //for (auto& ns : sched.nestSchedules) {
      //if (ns.II == 2) {
        //ns.II = 4;
      //}
    //}
  //}

  //cout << "Populating rate info" << endl;
  //// For now: Assume each loop nest has a single II
  //// expression for all reads and writes
  //map<string, RateInfo> produceRates;
  //for (auto info : streamUseInfo) {
    //cout << "Getting info for " << info.first << endl;
    //string writer = info.second.writer.toString();
    //cout << "\twriter: " << writer << endl;
    //if (contains_key(writer, produceRates)) {
      //// Skip
    //} else {

      //if (info.second.writer.isLoopNest()) {
        //for (auto loop : functionSchedules) {
          //if (loop.first->name == info.second.writer.toString()) {
            //auto& sched = loop.second;
            //auto& f = *(sched.f);
            //HWInstr* writeInstr = writeTo(info.first, f);
            //RateInfo info;
            //for (auto lp : writeInstr->surroundingLoops) {
              //info.nestSchedules.push_back(sched.getNestSchedule(lp.name));
            //}

            //produceRates[writer] = info;
          //}
        //}
      //} else if (info.second.writer.isLinebuffer()) {
        //for (auto lb : stencilInfo.linebuffers) {
          //string n = lbName(lb);
          //if (n == writer) {
            //cout << "Found writer for " << writer << endl;
            //string inName = lb[0];
            //vector<int> dims = getStreamDims(inName, stencilInfo);
            //vector<int> dimRanges = getDimRanges(dims);
            //internal_assert(dimRanges.size() == 2);
            //RateInfo info;
            //info.nestSchedules.push_back({writer + "_x", dimRanges[0], 10, 10});
            //info.nestSchedules.push_back({writer + "_y", dimRanges[1], 10, 10});
            //produceRates[writer] = info;
          //}
        //}
      //} else if (info.second.writer.isArgument()) {
        //internal_assert(false) << info.second.writer.toString() << " is an argument, and should not have rates\n";
      //}
    //}

    //cout << "Getting readers for " << info.first << endl;
    //for (auto reader : info.second.readers) {

      //cout << "reader: " << reader.first.toString() << endl;

      //if (contains_key(reader.first.toString(), produceRates)) {
        //continue;
      //}

      //string rdString = reader.first.toString();
      //if (reader.first.isLoopNest()) {
        //for (auto loop : functionSchedules) {
          //if (loop.first->name == reader.first.toString()) {
            //auto& sched = loop.second;
            //auto& f = *(sched.f);
            //HWInstr* readInstr = readFrom(info.first, f);
            //RateInfo info;
            //for (auto lp : readInstr->surroundingLoops) {
              //info.nestSchedules.push_back(sched.getNestSchedule(lp.name));
            //}
            //produceRates[reader.first.toString()] = info;
          //}
        //}
      //} else if (reader.first.isLinebuffer()) {
        //vector<int> dims = getStreamDims(info.first, stencilInfo);
        //vector<int> dimRanges = getDimRanges(dims);
        //RateInfo info;
        //internal_assert(dimRanges.size() == 2);
        //info.nestSchedules.push_back({writer + "_x", dimRanges[0], 5, 5});
        //info.nestSchedules.push_back({writer + "_y", dimRanges[1], 5, 5});
        //produceRates[writer] = info;
      //} else if (reader.first.isArgument()) {
        //RateInfo info;
        //info.nestSchedules.push_back({rdString + "_x", 7, 8, 9});
        //info.nestSchedules.push_back({rdString + "_y", 7, 8, 9});
        //produceRates[writer] = info;
      //}

    //}
  //}

  //// TODO: Add rate information for inputs / outputs
  //cout << "Rate information" << endl;
  //for (auto info : produceRates) {
    //cout << "\t" << info.first << endl;
    //for (auto nest : info.second.nestSchedules) {
      //cout << "\t\t" << nest.name << endl;
      //cout << "\t\t\tII: " << nest.II << endl;
      //cout << "\t\t\tL : " << nest.L << endl;
      //cout << "\t\t\tTC : " << nest.TC << endl;
    //}
  //}

  //internal_assert(false);

  // Though I cannot really adjust the II of a linebuffer I guess I can
  // adjust it indirectly by changing the in_en pattern
  //
  // Simpler agorithm:
  // while (!changed) {
  //   for (each scheduled loop) {
  //     find all producers for the loop
  //     find all II writes to producers
  //     find all reads in the loop
  //     increase producer IIs to match if they do not
  //     adjust the IIs of downstream linebuffers to reflect new producer times
  //     changed = true
  //   }
  // }
  //
  // Q: How do we adjust the IIs of linebuffers?
  // A: The linebuffer is two loop nests, one writes to the cache, the other reads from the cache
  //    The write cache loop should have the same IIs as the producer
  //    The read from cache loop should be offset from the write to cache loop
  //     so that the first window is read when all data is available
  //
  //    Initially the write loop of the linebuffer is dense, so its IIs allow an
  //    inner loop write every single cycle
  //
  //    The read loop IIs are the same, but offset forward in time?
  //
  // Or maybe it would actually be better to go back and figure out how to
  // use the valid signal from the linebuffer inside of the compute unit,
  // instead of just using the start signal?

  //cout << "Stream Use Info After Adjustment" << endl;
  //for (auto info : streamUseInfo) {
    //cout << "\tInfo: " << info.first << endl;
    //cout << "\t\tWriter: " << info.second.writer.toString() << endl;
    ////map<HWInstr*, NestSchedule> schedulesToMatch;

    //if (info.second.writer.isLoopNest()) {
      //for (auto loop : functionSchedules) {
        //if (loop.first->name == info.second.writer.toString()) {
          //auto& sched = loop.second;
          //auto& f = *(sched.f);
          //cout << "\t\t\tStart time: " << startTime(writeTo(info.first, f), sched) << endl;
        //}
      //}
    //}

    //cout << "\t\tReaders..." << endl;
    //for (auto reader : info.second.readers) {
      //cout << "\t\t\t" << reader.first.toString() << endl;
      //if (reader.first.isLoopNest()) {
        //for (auto loop : functionSchedules) {
          //if (loop.first->name == reader.first.toString()) {
            //auto& sched = loop.second;
            //auto& f = *(sched.f);
            //cout << "\t\t\t\tStart time: " << startTime(readFrom(info.firstf), sched) << endl;
          //}
        //}
      //}
    //}

    // Each of these becomes a group whose IIs must match
    // Special cases for IIs: inputs / outputs / linebuffers

    // For each of these groups what has to match?
    // startTime of each read with all writes?
  //}

  //cout << "-----------------------" << endl;
  //for (auto& fp : functionSchedules) {
    ////auto& sched = fp.second;
    //auto& f = *(fp.second.f);
    
    //set<HWInstr*> reads = allInstrs("rd_stream", f.structuredOrder());
    //cout << "Stream reads..." << endl;
    //for (auto rd : reads) {
      //cout << "\t" << rd->getOperand(0)->compactString() << endl;
    //}

    //set<HWInstr*> writes = allInstrs("write_stream", f.structuredOrder());
    //cout << "Stream writes..." << endl;
    //for (auto wr : writes) {
      //cout << "\t" << wr->getOperand(0)->compactString() << endl;
    //}
  //}

  //internal_assert(false);
}

// Now: Need to print out arguments and their info, actually use the arguments to form
// the type of the outermost module?
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
  AcceleratorInterface ifc = topLevelType(context, args);
  auto inputAliases = ifc.inputAliases;
  auto output_name_real = ifc.output_name_real;
  auto output_name = ifc.output_name;
  auto topType = ifc.designType;
  auto global_ns = context->getNamespace("global");
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

  stmt = preprocessHWLoops(stmt);

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

  StencilInfo info = scl.info;

  cout << "\tAll " << extractor.loops.size() << " loops in design..." << endl;
  int kernelN = 0;
  std::map<const For*, ComputeKernel> kernelModules;
  std::map<const For*, HWFunction> functions;
  for (const For* lp : extractor.loops) {
    cout << "\t\tLOOP" << endl;
    cout << "Original body.." << endl;
    cout << lp->body << endl;

    // Actual scheduling here
    HWFunction f = buildHWBody(context, scl.info, "compute_kernel_" + std::to_string(kernelN), lp, args, stCollector);

    functions[lp] = f;

    kernelN++;
  }    

  functions = mapFunctionsToCGRA(functions);
  auto def = topMod->newModuleDef();

  auto streamUseInfo = createStreamUseInfo(functions, args, scl.info);
  map<const For*, FunctionSchedule> functionSchedules;
  for (auto& fp : functions) {
    HWFunction& f = fp.second;
    internal_assert(f.mod != nullptr) << "mod is null just before inserting critical path target registers\n";
    insertCriticalPathTargetRegisters(hwInfo, f);
    FunctionSchedule fSched = buildFunctionSchedule(streamUseInfo, f);
    functionSchedules[fp.first] = fSched;
  }

  adjustIIs(scl.info, streamUseInfo, functionSchedules);

  // Connects up all control paths in the design
  std::map<const For*, CoreIR::Instance*> kernels;

  for (auto& fp : functions) {
    auto lp = fp.first;
    HWFunction& f = fp.second;
    auto fSched = map_get(fp.first, functionSchedules);
    emitCoreIR(f, info, fSched);
    ComputeKernel compK{f.getMod(), fSched};

    aliasInfo["kernel_info"][f.getMod()->getName()].emplace_back(compK.sched.cycleLatency());
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
  
  flattenExcluding(context, generatorNames);
  cout << "Top module after inlining" << endl;
  topMod->print();
  
  context->runPasses({"rungenerators"});

  flattenExcluding(context, generatorNames);
  context->runPasses({"deletedeadinstances"});
  //cout << "Top module" << endl;
  //topMod->print();

  // Save accelerator metadata
  ofstream interfaceInfo("accel_interface_info.json");
  interfaceInfo << aliasInfo;
  interfaceInfo.close();

  if (!saveToFile(global_ns, "conv_3_3_app.json")) {
    cout << "Could not save global namespace" << endl;
    context->die();
  }

  return topMod;
}

}
}
