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
  NOT_FOUND,               // no loads or stores. Shouldn't be possible
  INOUT_REALIZATION,       // only read or only written. Shouldn't be possible
  CONSTS_REALIZATION,      // replace with const registers
  ROM_REALIZATION,         // const values with variable index load
  RMW_REALIZATION,         // variable reads and writes, but writes are to same address
  RMW_DYNAMIC_REALIZATION, // data-dependent reads and writes, where writes are to same address
  ITERATIVE_REALIZATION,   // variable reads and writes, writes coming from the same memory but different address
  SRAM_REALIZATION,        // variable reads and variable writes
  SRAM_DYNAMIC_REALIZATION,// variable reads and writes, including data-dependent index calculation
  UNKNOWN_REALIZATION      
};

constexpr const char* HWMemoryTypeToString(HWMemoryType e) {
  switch (e) {
  case HWMemoryType::NOT_FOUND: return "NOT_FOUND";       
  case HWMemoryType::INOUT_REALIZATION: return "INOUT_REALIZATION";      
  case HWMemoryType::CONSTS_REALIZATION: return "CONSTS_REALIZATION";     
  case HWMemoryType::ROM_REALIZATION: return "ROM_REALIZATION";        
  case HWMemoryType::RMW_REALIZATION: return "RMW_REALIZATION";        
  case HWMemoryType::RMW_DYNAMIC_REALIZATION: return "RMW_DYNAMIC_REALIZATION";
  case HWMemoryType::ITERATIVE_REALIZATION: return "ITERATIVE_REALIZATION";  
  case HWMemoryType::SRAM_REALIZATION: return "SRAM_REALIZATION";       
  case HWMemoryType::SRAM_DYNAMIC_REALIZATION: return "SRAM_DYNAMIC_REALIZATION";
  case HWMemoryType::UNKNOWN_REALIZATION: return "UNKNOWN_REALIZATION";
  default: return "NOT IMPLEMENTED: " + e;
  }
};

HWMemoryType identify_realization(Stmt s, std::string realizename);

}  // namespace Internal
}  // namespace Halide

#endif
