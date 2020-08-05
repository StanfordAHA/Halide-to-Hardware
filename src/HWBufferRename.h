#ifndef HALIDE_HWBUFFER_RENAME_H
#define HALIDE_HWBUFFER_RENAME_H

/** \file
 * Defines the lowering pass that renames allocations by appending with .stream
 * This allows 2D addresses to be passed
 */

#include "IR.h"

namespace Halide {
namespace Internal {

Stmt rename_hwbuffers(Stmt s, const std::map<std::string, Function> &env);

}  // namespace Internal
}  // namespace Halide

#endif
