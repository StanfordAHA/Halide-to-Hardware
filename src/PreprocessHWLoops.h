#ifndef HALIDE_PREPROCESS_HW_LOOPS_H
#define HALIDE_PREPROCESS_HW_LOOPS_H

#include "IROperator.h"
#include "IR.h"
#include "IRVisitor.h"
#include "Substitute.h"

namespace Halide {
  namespace Internal {

class MemOp {
  public:

    std::vector<const For*> surroundingLoops;
    const Store* store;
    const Load* load;

    MemOp() : store(nullptr), load(nullptr) {}
    MemOp(std::vector<const For*>& lps_,
        const Store* st_,
        const Load* ld_) : surroundingLoops(lps_), store(st_), load(ld_) {}

    std::string getBufferName() const {
      if (isLoad()) {
        return load->name;
      }
      return store->name;
    }

    bool isLoad() const { return load != nullptr; }
    bool isStore() const { return store != nullptr; }
    

    void print(std::ostream& out) const {
      for (auto lp : surroundingLoops) {
        out << lp->name << " : " << lp->min << ", " << lp->extent << "; ";
      }
      if (isLoad()) {
        out << "ld: " << load->name << "[" << load->index << "]";
      } else {
        assert(isStore());
        //out << "st: " << store->name << "[" << store->index << "] =  " << store->value;
        out << "st: " << store->name << "[" << store->index << "]";
      }
    }
};

static inline
std::ostream& operator<<(std::ostream& out, const MemOp& op) {
  op.print(out);
  return out;
}

class MemoryInfoCollector : public IRGraphVisitor {
  public:

    using IRGraphVisitor::visit;

    std::vector<const For*> activeLoops;
    std::vector<MemOp> memOps;
    std::map<std::string, Expr> letValues;

    std::set<std::string> allBufferNames() const {
      std::set<std::string> names;
      for (auto op : memOps) {
        names.insert(op.getBufferName());
      }

      return names;
    }

    std::vector<MemOp> allOpsOn(const std::string& buf) const {
      std::vector<MemOp> ops;
      for (auto op : memOps) {
        if (op.getBufferName() == buf) {
          ops.push_back(op);
        }
      }
      return ops;
    }

    int romValue(const std::string& name, const int addr);

    bool isROM(const std::string& name) const;

    std::set<std::string> roms() const {
      // Find all memories that are only stored to at constant indexes outside of loop nests
      std::set<std::string> memNames = allBufferNames();
      std::set<std::string> roms;
      for (auto buf : memNames) {
        std::vector<MemOp> ops = allOpsOn(buf);
        bool allStoresToConstants = true;
        int numStores = 0;
        for (auto op : ops) {
          if (op.isStore()) {
            numStores++;
            const Expr& ind = op.store->index;
            if (!is_const(ind) ||
                !is_const(op.store->value)) {
              allStoresToConstants = false;
              break;
            }
          }
        }
        if (numStores > 0 && allStoresToConstants) {
          roms.insert(buf);
        }
      }
      return roms;
    }

    void visit(const Let* lt) {
      letValues[lt->name] = lt->value;
      IRGraphVisitor::visit(lt);
      letValues.erase(lt->name);
    }

    void visit(const LetStmt* lt) {
      letValues[lt->name] = lt->value;
      IRGraphVisitor::visit(lt);
      letValues.erase(lt->name);
    }

    void visit(const Load* ld) {
      IRGraphVisitor::visit(ld);
      Expr addr = substitute(letValues, ld->index);

      //cout << "Old = " << ld->index << ", New addr = " << addr << endl;

      memOps.push_back({activeLoops, nullptr, ld});
    }

    void visit(const Store* st) {
      IRGraphVisitor::visit(st);
      Expr addr = substitute(letValues, st->index);
      memOps.push_back({activeLoops, st, nullptr});
    }

    void visit(const For* lp) {
      activeLoops.push_back(lp);


      IRGraphVisitor::visit(lp);

      activeLoops.pop_back();
    }

};

    bool isInnermostLoop(const For* f);
    Stmt preprocessHWLoops(const Stmt& stmt);

  }
}

#endif

