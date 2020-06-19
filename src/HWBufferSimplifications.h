#ifndef HALIDE_SIMPLIFICATIONS_H
#define HALIDE_SIMPLIFICATIONS_H

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

enum HWMemoryType {
  NOT_FOUND,               // no loads or stores
  INOUT_REALIZATION,       // only read or only written
  CONSTS_REALIZATION,      // replace with const registers
  ROM_REALIZATION,         // const values with variable index load
  RMW_REALIZATION,         // variable reads and writes, but writes are to same address
  SRAM_REALIZATION,        // variable reads and variable writes
  UNKNOWN_REALIZATION
};

HWMemoryType identify_realization(Stmt s, std::string realizename);

}  // namespace Internal
}  // namespace Halide

#endif
