#ifndef HALIDE_CODEGEN_COREHLS_H
#define HALIDE_CODEGEN_COREHLS_H

#include "HWTechLib.h"
#include "IR.h"
#include "PreprocessHWLoops.h"

#include "coreir.h"

namespace Halide {
  namespace Internal {

std::string coreirSanitize(const std::string& str);

enum HWInstrTp {
  HWINSTR_TP_INSTR,
  HWINSTR_TP_CONST,
  HWINSTR_TP_STR,
  HWINSTR_TP_ARG,
  HWINSTR_TP_VAR
};

class HWType {
  public:
    int baseWidth;
    std::vector<int> dims;
};

class LoopSpec {
  public:

    std::string name;
    Expr min;
    Expr extent;
};

class HWInstr {
  public:
    std::vector<LoopSpec> surroundingLoops;

    int uniqueNum;
    HWInstrTp tp;
    CoreIR::Module* opType;
    CoreIR::Instance* unit;
    
    bool preBound;
    std::string boundTargetName;
    
    int latency;

    std::string strConst;

    HWInstr* predicate;
    std::vector<HWInstr*> operands;
    std::vector<HWType> operandTypes;
    CoreIR::Type* retType;

    std::string name;
    int constWidth;
    std::string constValue;

    CoreIR::Type* resType;
    bool signedNum;

    HWInstr() : tp(HWINSTR_TP_INSTR), preBound(false), latency(0), predicate(nullptr), resType(nullptr), signedNum(false) {}

    bool isSigned() const {
      return signedNum;
    }

    void setSigned(const bool value) {
      signedNum = value;
    }

    CoreIR::Instance* getUnit() const {
      internal_assert(unit != nullptr) << "unit is null in HWInstr\n";
      return unit;
    }

    int toInt() const {
      internal_assert(tp == HWINSTR_TP_CONST);
      internal_assert(constWidth <= 32);
      return stoi(constValue);
    }

    HWInstr* getOperand(const int i) const {
      internal_assert(i < (int) operands.size());
      return operands.at(i);
    }

    std::string compactString() const {
      if (tp == HWINSTR_TP_STR) {
        return "\"" + strConst + "\"";
      }

      if (tp == HWINSTR_TP_VAR) {
        return name;
      }

      if (tp == HWINSTR_TP_CONST) {
        return std::to_string(constWidth) + "'d" + constValue;
      }

      return "%" + std::to_string(uniqueNum);
    }
};

class HWBlock {
  public:
    std::string name;
    int tripCount;
    std::vector<HWInstr*> instrs;
};

class HWFunction {

  protected:
    std::vector<HWBlock*> blocks;

  public:
    std::string name;
    int uniqueNum;
    std::vector<std::string> controlVars;
    CoreIR::Module* mod;

    std::vector<std::string> argNames() const {
      return controlVars;
    }

    bool isLocalVariable(const std::string& name) const {
      auto sName = coreirSanitize(name);
      std::vector<std::string> fds = mod->getType()->getFields();
      bool isLocal = !CoreIR::elem(sName, fds);

      if (!isLocal) {
        internal_assert(!isLoopIndexVar(name)) << name << " is not a local variable but it is a loop index variable of:\n" << name << "\n";
      }
      return isLocal;
    }

    bool isLoopIndexVar(const std::string& name) const {
      for (auto instr : allInstrs()) {
        for (auto lp : instr->surroundingLoops) {
          if (lp.name == name) {
            return true;
          }
        }
      }
      return false;
    }

    HWFunction() : uniqueNum(0), mod(nullptr) {
      newBlk();
    }

    HWBlock* newBlk() {
      HWBlock* n = new HWBlock();
      n->name = "blk_" + std::to_string(uniqueNum);
      uniqueNum++;
      blocks.push_back(n);

      return n;
    }

    std::vector<HWBlock*> getBlocks() const {
      return blocks;
    }

    std::vector<HWInstr*> structuredOrder() const {
      std::vector<HWInstr*> instrs;
      for (auto blk : getBlocks()) {
        for (auto instr : blk->instrs) {
          instrs.push_back(instr);
        }
      }
      return instrs;
    }

    void replaceAllUsesAfter(HWInstr* refresh, HWInstr* toReplace, HWInstr* replacement);
    void replaceAllUsesWith(HWInstr* toReplace, HWInstr* replacement);
    void deleteInstr(HWInstr* instr);

    void insertAt(HWInstr* pos, HWInstr* newInstr);
    void insertAfter(HWInstr* pos, HWInstr* newInstr);
    void insert(const int pos, HWInstr* newInstr);

    template<typename Cond>
    void deleteAll(Cond c) {
      for (auto& blk : blocks) {
        CoreIR::delete_if(blk->instrs, c);
      }
    }

    void pushInstr(HWInstr* instr) {
      blocks.back()->instrs.push_back(instr);
    }

    int numBlocks() const { return blocks.size(); }

    std::set<HWInstr*> allInstrs() const {
      std::set<HWInstr*> instrs;
      for (auto blk : getBlocks()) {
        for (auto instr : blk->instrs) {
          instrs.insert(instr);
        }
      }
      return instrs;
    }

    CoreIR::ModuleDef* getDef() const {
      auto def = mod->getDef();
      internal_assert(def != nullptr) << "module def is null\n";
      return def;
    }

    HWInstr* newConst(const int width, const int value) {
      auto ist = newI();
      ist->latency = 0;
      ist->tp = HWINSTR_TP_CONST;
      ist->constWidth = width;
      ist->constValue = std::to_string(value);
      return ist;
    }

    HWInstr* newVar(const std::string& name) {
      auto ist = newI();
      ist->latency = 0;
      ist->tp = HWINSTR_TP_VAR;
      ist->name = name;
      if (!isLocalVariable(name)) {
        if (contains_key(coreirSanitize(name), mod->getType()->getRecord())) {
          CoreIR::Type* tp = mod->getType()->getRecord().at(coreirSanitize(name));
          ist->resType = tp;
        }
      }
      return ist;
    }

    HWInstr* newI(HWInstr* instr) {
      auto i = newI();
      i->surroundingLoops = instr->surroundingLoops;
      i->resType = instr->resType;
      i->setSigned(instr->isSigned());
      i->name = instr->name;
      i->tp = instr->tp;
      return i;
    }

    HWInstr* newI() {
      auto in = new HWInstr();
      in->tp = HWINSTR_TP_INSTR;
      in->uniqueNum = uniqueNum;
      uniqueNum++;
      return in;
    }
};


void insertAt(HWInstr* instr, HWInstr* refresh, std::vector<HWInstr*>& body);

typedef std::pair<std::string, std::string> StreamWriteInfo;
typedef std::pair<std::string, std::string> StreamReadInfo;

class StencilInfo {
  public:
    std::map<std::string, std::vector<std::string> > stencilSizes;
    std::map<std::string, std::vector<std::string> > stencilRealizations;
    std::map<std::string, std::vector<std::string> > streamDispatches;
    std::map<std::string, std::string> streamWrites;
    std::map<std::string, std::string> streamReads;

    
    std::map<std::string, std::vector<std::string> > streamWriteRealize;
    std::map<std::string, std::vector<std::string> > streamReadRealize;
    
    std::vector<std::vector<std::string> > linebuffers;
    std::vector<std::vector<std::string> > hwbuffers;

    std::map<StreamWriteInfo, std::vector<std::string> > writeRealizations;
    std::map<StreamReadInfo, std::vector<std::string> > readRealizations;

    std::map<std::string, std::vector<const Call*> > streamReadCalls;
    std::map<std::string, std::vector<const Call*> > streamWriteCalls;

    std::map<const Call*, std::vector<std::string> > streamReadCallRealizations;
    std::map<const Call*, std::vector<std::string> > streamWriteCallRealizations;

    std::map<std::string, std::vector<std::string> > streamParams;
};

struct CoreIR_Argument;
    void loadHalideLib(CoreIR::Context* context);
    
    CoreIR::Module* createCoreIRForStmt(CoreIR::Context* context,
        HardwareInfo& info,
        Stmt stmt,
        const std::string& name,
        const std::vector<CoreIR_Argument>& args);
  }
}
#endif
