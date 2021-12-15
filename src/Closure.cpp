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
    }
    s.accept(this);
}

void Closure::visit(const Let *op) {
    op->value.accept(this);
    //std::cout << "let: add to ignore: " << op->name << std::endl;
    ScopedBinding<> p(ignore, op->name);
    op->body.accept(this);
}

void Closure::visit(const LetStmt *op) {
    op->value.accept(this);
        //std::cout << "letstmt: add to ignore: " << op->name << std::endl;
    ScopedBinding<> p(ignore, op->name);
    op->body.accept(this);
}

void Closure::visit(const For *op) {
      //std::cout << "for: add to ignore: " << op->name << std::endl;
    ScopedBinding<> p(ignore, op->name);
    op->min.accept(this);
    op->extent.accept(this);
    op->body.accept(this);
}

void Closure::found_buffer_ref(const string &name, Type type,
                               bool read, bool written, Halide::Buffer<> image) {
  //std::cout << "looking at " << name << " in closure where ignore=" <<
  //ignore.contains(name) << "\n";
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
    }
}

void Closure::visit(const Call *op) {
  //std::cout << op->name << " has call type " << op->call_type << std::endl;
  //if ((op->call_type == Call::Intrinsic || op->call_type == Call::Halide || op->call_type == Call::Image) &&
  if ((op->call_type == Call::Intrinsic) &&
      (ends_with(op->name, ".stencil") || ends_with(op->name, ".stencil_update"))) {
    // consider call to stencil and stencil_update
    debug(3) << "visit call " << op->name << ": ";
    if(!ignore.contains(op->name)) {
      debug(3) << "adding to closure.\n";
      //std::cout << "adding to closure through call: " << op->name << "\n";
      vars[op->name] = Type();
    } else {
      debug(3) << "not adding to closure.\n";
    }
  }
  IRVisitor::visit(op);
}
  
void Closure::visit(const Load *op) {
    op->predicate.accept(this);
    op->index.accept(this);
    found_buffer_ref(op->name, op->type, true, false, op->image);
}

void Closure::visit(const Store *op) {
    op->predicate.accept(this);
    op->index.accept(this);
    op->value.accept(this);
    found_buffer_ref(op->name, op->value.type(), false, true, Halide::Buffer<>());
}

void Closure::visit(const Allocate *op) {
    if (op->new_expr.defined()) {
        op->new_expr.accept(this);
    }
    //std::cout << "allocate: add to ignore: " << op->name << std::endl;
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
        //std::cout << "Adding " << op->name << " to closure through variable\n";
        vars[op->name] = op->type;
    }
}

void Closure::visit(const Realize *op) {
  for (size_t i = 0; i < op->bounds.size(); i++) {
    op->bounds[i].min.accept(this);
    op->bounds[i].extent.accept(this);
  }
  op->condition.accept(this);

      //std::cout << "realize: add to ignore: " << op->name << std::endl;
      
  ScopedBinding<> p(ignore, op->name);
  op->body.accept(this);
}

}  // namespace Internal
}  // namespace Halide
