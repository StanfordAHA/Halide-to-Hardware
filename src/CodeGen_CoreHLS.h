#ifndef HALIDE_CODEGEN_COREHLS_H
#define HALIDE_CODEGEN_COREHLS_H

#include "IR.h"

#include "coreir.h"

namespace Halide {
  namespace Internal {

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

class HWInstr {
  public:
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

    HWInstr() : tp(HWINSTR_TP_INSTR), preBound(false), latency(0), predicate(nullptr), resType(nullptr) {}

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

class HWFunction {
  public:
    std::string name;
    int uniqueNum;
    std::vector<HWInstr*> body;
    std::vector<std::string> controlVars;
    CoreIR::Module* mod;

    HWFunction() : uniqueNum(0), mod(nullptr) {}

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
        Stmt stmt,
        const std::string& name,
        const std::vector<CoreIR_Argument>& args);
  }
}
#endif
