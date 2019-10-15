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
#include "SlidingWindow.h"

namespace Halide {
namespace Internal {

struct BufferDimSize {
  std::string loop_name;
  Expr logical_size;
  Expr logical_min;

  Expr input_chunk;
  Expr input_block;

  Expr output_stencil;
  Expr output_block;
  Expr output_min_pos;
  Expr output_max_pos;
};

struct Stride {
  int stride;
  bool is_inverse;
  Stride(int stride=0, bool is_inverse=false) :
    stride(stride), is_inverse(is_inverse) {}

};

std::vector<BufferDimSize> create_hwbuffer_sizes(std::vector<int> logical_size,
                                                 std::vector<int> output_stencil, std::vector<int> output_block,
                                                 std::vector<int> input_chunk, std::vector<int> input_block);
  
struct HWBuffer {
  std::string name;
  std::string store_level;
  std::string compute_level;
  LoopLevel store_looplevel;
  LoopLevel compute_looplevel = LoopLevel::inlined();
  
  // Parameters for the HWBuffer
  std::vector<BufferDimSize> dims;
  std::shared_ptr<SlidingStencils> input_stencil;

  Stmt input_access_pattern;
  Stmt output_access_pattern;
  Stmt my_stmt;

  // Parameters used during optimization
  bool is_inlined = false;
  bool is_output = false;
  std::map<std::string, std::shared_ptr<HWBuffer>> consumer_buffers;   // used for transforming call nodes and inserting dispatch calls
  std::vector<std::string> input_streams;  // used when inserting read_stream calls
  Function func;
  std::vector<std::string> streaming_loops;
  std::map<std::string, Stride> stride_map;

  HWBuffer(size_t num_dims, SlidingStencils &is) :
    dims(std::vector<BufferDimSize>(num_dims)), input_stencil(std::make_shared<SlidingStencils>(is)) { }
  HWBuffer() : input_stencil(nullptr) { }

  HWBuffer(std::string name, std::vector<BufferDimSize> dims, bool is_inlined, bool is_output) :
    name(name), dims(dims), is_inlined(is_inlined), is_output(is_output) { };
  HWBuffer(std::string name, std::vector<BufferDimSize> dims, std::vector<std::string> loops,
           int store_index, int compute_index, bool is_inlined, bool is_output) :
  //std::map<std::string, std::shared_ptr<HWBuffer>> consumer_map) :
    name(name), store_level(store_index < 0 ? "" : loops[store_index]),
    compute_level(compute_index < 0 ? "" : loops[compute_index]),
    dims(dims), is_inlined(is_inlined), is_output(is_output) {
    loops.erase(loops.begin());
    streaming_loops = loops;
  };

};

std::ostream& operator<<(std::ostream& os, const std::vector<Expr>& vec);
std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer);

std::map<std::string, HWBuffer> extract_hw_buffers(Stmt s, const std::map<std::string, Function> &env,
                                                   const std::vector<std::string> &streaming_loop_names);


struct HWXcel {
  std::string name;
  LoopLevel store_level;
  LoopLevel compute_level;
  
  std::vector<std::string> streaming_loop_levels;  // store (exclusive) to compute (inclusive)
  std::set<std::string> input_streams; // might be wrong?
  std::map<std::string, HWBuffer> hwbuffers;
  std::map<std::string, HWBuffer*> consumer_buffers; // used for transforming call nodes and inserting dispatch calls
  //std::map<std::string, HWTap> input_taps;
};

std::vector<HWXcel> extract_hw_accelerators(Stmt s, const std::map<std::string, Function> &env,
                                            const std::vector<BoundsInference_Stage> &inlined_stages);


std::ostream& operator<<(std::ostream& os, const std::vector<string>& vec);
int id_const_value(const Expr e);
std::vector<std::string> get_tokens(const std::string &line, const std::string &delimiter);


}  // namespace Internal
}  // namespace Halide



#endif
