#include "Closure.h"
#include "Debug.h"

namespace Halide {
namespace Internal {

using std::map;
using std::pair;
using std::string;
using std::vector;

Closure::Closure(Stmt s, const string &loop_variable) {
    if (!loop_variable.empty()) {
        ignore.push(loop_variable);
        std::cout << "adding to ignore: " << loop_variable << "\n";
    }
    s.accept(this);
}

void Closure::visit(const Let *op) {
    op->value.accept(this);
    ScopedBinding<> p(ignore, op->name);
    op->body.accept(this);
}

void Closure::visit(const LetStmt *op) {
    op->value.accept(this);
    ScopedBinding<> p(ignore, op->name);
    op->body.accept(this);
}

void Closure::visit(const For *op) {
    ScopedBinding<> p(ignore, op->name);
    op->min.accept(this);
    op->extent.accept(this);
    op->body.accept(this);
}

void Closure::found_buffer_ref(const string &name, Type type,
                               bool read, bool written, Halide::Buffer<> image) {
    if (!ignore.contains(name)) {
        debug(3) << "Adding buffer " << name << " to closure\n";
        std::cout << "Adding buffer " << name << " to closure\n";
        Buffer &ref = buffers[name];
        ref.type = type.element_of(); // TODO: Validate type is the same as existing refs?
        ref.read = ref.read || read;
        ref.write = ref.write || written;

        // If reading an image/buffer, compute the size.
        if (image.defined()) {
            ref.size = image.size_in_bytes();
            ref.dimensions = image.dimensions();
        }
    } else {
        debug(3) << "Not adding " << name << " to closure\n";
        std::cout << "Not adding " << name << " to closure\n";
    }
}

void Closure::visit(const Call *op) {
  if (op->call_type == Call::Intrinsic &&
      (ends_with(op->name, ".stencil") || ends_with(op->name, ".stencil_update"))) {
    // consider call to stencil and stencil_update
    debug(3) << "visit call " << op->name << ": ";
    std::cout << "visit call " << op->name << ": ";
    if(!ignore.contains(op->name)) {
      debug(3) << "adding to closure.\n";
      std::cout << "adding var " << op->name << " to closure from Call.\n";
      vars[op->name] = Type();
    } else {
      debug(3) << "not adding to closure.\n";
      std::cout << "not adding to closure.\n";
    }
  }
  IRVisitor::visit(op);
}
  
void Closure::visit(const Load *op) {
  std::cout << "found a load for " << op->name << std::endl;
    op->predicate.accept(this);
    op->index.accept(this);
    found_buffer_ref(op->name, op->type, true, false, op->image);
}

void Closure::visit(const Store *op) {
  std::cout << "found a store for " << op->name << std::endl;

    op->predicate.accept(this);
    op->index.accept(this);
    op->value.accept(this);
    found_buffer_ref(op->name, op->value.type(), false, true, Halide::Buffer<>());
}

void Closure::visit(const Allocate *op) {
    if (op->new_expr.defined()) {
        op->new_expr.accept(this);
    }
    ScopedBinding<> p(ignore, op->name);
    for (size_t i = 0; i < op->extents.size(); i++) {
        op->extents[i].accept(this);
    }
    op->condition.accept(this);
    op->body.accept(this);
}

void Closure::visit(const Variable *op) {
    if (ignore.contains(op->name)) {
        debug(3) << "Not adding " << op->name << " to closure\n";
    } else {
        debug(3) << "Adding " << op->name << " to closure\n";
        std::cout << "Adding " << op->name << " variable to closure in Var\n";
        vars[op->name] = op->type;
    }
}

void Closure::visit(const Realize *op) {
  for (size_t i = 0; i < op->bounds.size(); i++) {
    op->bounds[i].min.accept(this);
    op->bounds[i].extent.accept(this);
  }
  op->condition.accept(this);

  ScopedBinding<> p(ignore, op->name);
  op->body.accept(this);
}

//  void Closure::visit(const ProducerConsumer *op) {
//    op->body.accept(this);
//  }

}  // namespace Internal
}  // namespace Halide
