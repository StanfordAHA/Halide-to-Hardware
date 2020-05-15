#ifndef HALIDE_SIMPLICATIONS_H
#define HALIDE_SIMPLICATIONS_H

/** \file
 * Defines the lowering pass that unrolls loops marked as such
 */

#include "IR.h"

namespace Halide {
namespace Internal {

/** Take a statement with for loops marked for unrolling, and convert
 * each into several copies of the innermost statement. I.e. unroll
 * the loop. */
Stmt inline_memory_constants(Stmt);

}  // namespace Internal
}  // namespace Halide

#endif
