#ifndef HALIDE_EXTRACT_HW_BUFFERS_H
#define HALIDE_EXTRACT_HW_BUFFERS_H

/** \file
 *
 * Defines a hardware buffer extraction pass, which finds the parameters
 * for a general buffer that can then be outputted to a hardware backend.
 * These parameters provide specification for line buffers and double
 * buffers.
 * 
 */

#include <map>

#include "IR.h"

namespace Halide {
namespace Internal {



struct HWBuffer {
  std::string name;
  
  // Parameters for the HWBuffer
  std::vector<Expr> total_buffer_box;
  
  std::vector<Expr> input_chunk_box;
  std::vector<Expr> input_block_box;
  Stmt input_access_pattern;
  
  std::vector<Expr> output_stencil_box;
  std::vector<Expr> output_block_box;
  Stmt output_access_pattern;

};

  
Stmt extract_hw_buffers(Stmt s, const std::map<std::string, Function> &env);

}  // namespace Internal
}  // namespace Halide

#endif
