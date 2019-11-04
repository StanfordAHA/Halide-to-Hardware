#include "PreprocessHWLoops.h"

#include "IRMutator.h"

#include "coreir.h"

using namespace std;

namespace Halide {

  namespace Internal {


    bool MemoryInfoCollector::isROM(const std::string& name) const {
      return CoreIR::elem(name, roms());
    }

    class InnermostLoopChecker : public IRGraphVisitor {
      public:
        bool foundSubLoop;

        InnermostLoopChecker() : foundSubLoop(false) {}

      protected:

        using IRGraphVisitor::visit;

        void visit(const For* f) override {
          foundSubLoop = true;
        }
    };

bool isInnermostLoop(const For* f) {
  InnermostLoopChecker c;
  f->body->accept(&c);
  return !c.foundSubLoop;
}


class LetPusher : public IRMutator {
  public:

    vector<const LetStmt*> letStack;

  protected:
    using IRMutator::visit;

    Stmt visit(const LetStmt* let) override {
      letStack.push_back(let);
      auto res = IRMutator::visit(let);
      letStack.pop_back();

      return res;
    }

    Stmt visit(const For* f) override {
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

class LetEraser : public IRMutator {

  public:

  protected:

    using IRMutator::visit;
    
    Stmt visit(const LetStmt* let) override {
      auto newBody = this->mutate(let->body);
      return newBody;
    }

    Stmt visit(const For* f) override {
      if (isInnermostLoop(f)) {
        return f;
      } else {
        return IRMutator::visit(f);
      }
    }
};

    int MemoryInfoCollector::romValue(const std::string& name, const int addr) {
      internal_assert(memOps.size() > 0);
      for (int i = ((int) memOps.size()) - 1; i >= 0; i--) {
        auto op = memOps[i];
        if (op.isStore() && is_const(op.store->index)) {
          int a = *as_const_int(op.store->index);
          if (a == addr) {
            internal_assert(is_const(op.store->value));
            return *as_const_int(op.store->value);
          }
        }
      }
      internal_assert(false);
      return 0;
    }

class ROMReadOptimizer : public IRMutator {
  public:

    MemoryInfoCollector mic;

    using IRMutator::visit;

    Expr visit(const Load* ld) {
      if (mic.isROM(ld->name) &&
          is_const(ld->index)) {
        int addr = *as_const_int(ld->index);
        int value = mic.romValue(ld->name, addr);
        return IntImm::make(Int(32), value);
      }
      
      return IRMutator::visit(ld);
    }
};


  Stmt preprocessHWLoops(const Stmt& stmt) {
    LetPusher pusher;
    Stmt pushed = pusher.mutate(stmt);
    LetEraser letEraser;
    Stmt r = letEraser.mutate(pushed);
    ROMReadOptimizer romReadOptimizer;

    MemoryInfoCollector mic;
    r.accept(&mic);
    romReadOptimizer.mic = mic;
    auto romOpt = romReadOptimizer.mutate(r);
    return romOpt;
  }

}
}
