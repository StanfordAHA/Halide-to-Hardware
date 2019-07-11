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
#include <set>
#include <vector>
#include <iostream>

#include "IR.h"
#include "ExtractHWKernelDAG.h"

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

  // Parameters used during optimization
  bool is_inlined;
  bool is_output;
  //std::map<std::string, std::vector<StencilDimSpecs> > consumer_stencils;   // used for transforming call nodes and inserting dispatch calls
  std::vector<std::string> input_streams;  // used when inserting read_stream calls
  Function func;

};

std::ostream& operator<<(std::ostream& os, const std::vector<Expr>& vec);
std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer);

std::map<std::string, HWBuffer> extract_hw_buffers(Stmt s, const std::map<std::string, Function> &env);


struct HWXcel {
  std::string name;
  LoopLevel store_level;
  LoopLevel compute_level;
  
  std::set<std::string> streaming_loop_levels;
  std::set<std::string> input_kernels;
  std::map<std::string, HWBuffer> hwbuffers;
  //std::map<std::string, std::vector<StencilDimSpecs> > consumer_stencils; // used for transforming call nodes and inserting dispatch calls
  //std::map<std::string, HWTap> input_taps;
};

std::vector<HWXcel> extract_hw_accelerators(Stmt s, const std::map<std::string, Function> &env,
                                            const std::vector<BoundsInference_Stage> &inlined_stages);


}  // namespace Internal
}  // namespace Halide



#endif
