#include "PreprocessHWLoops.h"

#include "IRMutator.h"

using namespace std;

namespace Halide {

  namespace Internal {


  bool is_const(const Expr e) {
    if (e.as<IntImm>() || e.as<UIntImm>()) {
      return true;
    } else {
      return false;
    }
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


  Stmt preprocessHWLoops(const Stmt& stmt) {
    LetPusher pusher;
    Stmt pushed = pusher.mutate(stmt);
    LetEraser letEraser;
    return letEraser.mutate(pushed);
  }

}
}
