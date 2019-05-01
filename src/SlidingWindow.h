#ifndef HALIDE_SLIDING_WINDOW_H
#define HALIDE_SLIDING_WINDOW_H

/** \file
 *
 * Defines the sliding_window lowering optimization pass, which avoids
 * computing provably-already-computed values.
 */

#include <map>
#include <vector>

#include "IR.h"

namespace Halide {
namespace Internal {

/** Perform sliding window optimizations on a halide
 * statement. I.e. don't bother computing points in a function that
 * have provably already been computed by a previous iteration.
 */
Stmt sliding_window(Stmt s, const std::map<std::string, Function> &env);


struct SlidingStencils {
  std::vector<Expr> output_stencil_extents;
  std::vector<Expr> input_chunk_extents;
};

std::map<std::string, SlidingStencils> extract_sliding_stencils(Stmt s, Function f);

}  // namespace Internal
}  // namespace Halide

#endif
