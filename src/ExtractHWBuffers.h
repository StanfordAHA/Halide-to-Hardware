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

struct Stride {
  int stride;
  bool is_inverse;
  Stride(int stride=0, bool is_inverse=false) :
    stride(stride), is_inverse(is_inverse) {}

};

struct MergedDimSize {
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

struct InputDimSize {
  std::string loop_name;
  Expr input_chunk;
  Expr input_block;
};

struct InputStream {
  std::string name;
  std::vector<InputDimSize> idims;
  Stmt input_access_pattern;
  std::map<std::string, Stride> stride_map;
};

struct OutputDimSize {
  std::string loop_name;
  Expr output_stencil;
  Expr output_block;
  Expr output_min_pos;
  Expr output_max_pos;
};

struct HWBuffer; // forward declare
struct OutputStream {
  std::string name;
  std::vector<OutputDimSize> odims;
  Stmt output_access_pattern;
  std::map<std::string, Stride> stride_map;
  std::shared_ptr<HWBuffer> hwref;
};

struct InOutDimSize {
  std::string loop_name;

  Expr input_chunk;
  Expr input_block;

  Expr output_stencil;
  Expr output_block;
  Expr output_min_pos;
  Expr output_max_pos;
};

struct LogicalDimSize {
  Expr var_name;
  Expr logical_size;
  Expr logical_min;
};

std::vector<MergedDimSize> create_hwbuffer_sizes(std::vector<int> logical_size,
                                                 std::vector<int> output_stencil,
                                                 std::vector<int> output_block,
                                                 std::vector<int> input_chunk,
                                                 std::vector<int> input_block);
  
struct HWBuffer {
  std::string name;
  std::string store_level;
  std::string compute_level;
  std::vector<std::string> streaming_loops;
  //LoopLevel store_looplevel;
  //LoopLevel compute_looplevel = LoopLevel::inlined();

  Stmt my_stmt;
  Function func;
  bool is_inlined = false;
  bool is_output = false;
  
  // old parameters for the HWBuffer
  std::shared_ptr<SlidingStencils> input_stencil;
  std::vector<InOutDimSize> dims;
  Stmt input_access_pattern;
  Stmt output_access_pattern;
  std::map<std::string, std::shared_ptr<HWBuffer>> consumer_buffers;   // used for transforming call nodes and inserting dispatch calls
  std::vector<std::string> input_streams;  // used when inserting read_stream calls
  std::map<std::string, Stride> stride_map;

  // dimensions for the hwbuffer
  std::vector<LogicalDimSize> ldims;
  std::map<std::string, InputStream> istreams;
  std::map<std::string, OutputStream> ostreams;
  
  // Constructors
  HWBuffer() : input_stencil(nullptr) { }

  HWBuffer(std::string name, std::vector<MergedDimSize> mdims, std::vector<std::string> loops,
           int store_index, int compute_index, bool is_inlined, bool is_output) :
    name(name), store_level(store_index < 0 ? "" : loops[store_index]),
    compute_level(compute_index < 0 ? "" : loops[compute_index]),
    is_inlined(is_inlined), is_output(is_output) {
    loops.erase(loops.begin());
    streaming_loops = loops;

    ldims = std::vector<LogicalDimSize>(mdims.size());
    for (size_t i=0; i<mdims.size(); ++i) {
      ldims[i].logical_size = mdims[i].logical_size;
      ldims[i].logical_min = mdims[i].logical_min;
    }

    InputStream istream;
    istream.idims = std::vector<InputDimSize>(mdims.size());
    istreams["input"] = istream;
    
    OutputStream ostream;
    ostream.odims = std::vector<OutputDimSize>(mdims.size());
    ostreams["output"] = ostream;
    
    dims = std::vector<InOutDimSize>(mdims.size());
    for (size_t i=0; i<mdims.size(); ++i) {
      istreams["input"].idims.at(i).loop_name       = mdims.at(i).loop_name;
      istreams["input"].idims.at(i).input_chunk     = mdims.at(i).input_chunk;
      istreams["input"].idims.at(i).input_block     = mdims.at(i).input_block;
      
      ostreams["output"].odims.at(i).loop_name      = mdims.at(i).loop_name;
      ostreams["output"].odims.at(i).output_stencil = mdims.at(i).output_stencil;
      ostreams["output"].odims.at(i).output_block   = mdims.at(i).output_block;
      ostreams["output"].odims.at(i).output_min_pos = mdims.at(i).output_min_pos;
      ostreams["output"].odims.at(i).output_max_pos = mdims.at(i).output_max_pos;
    }

    // old way
    dims = std::vector<InOutDimSize>(mdims.size());
    for (size_t i=0; i<mdims.size(); ++i) {
      dims[i].loop_name      = mdims[i].loop_name;
      dims[i].input_chunk    = mdims[i].input_chunk;
      dims[i].input_block    = mdims[i].input_block;
      dims[i].output_stencil = mdims[i].output_stencil;
      dims[i].output_block   = mdims[i].output_block;
      dims[i].output_min_pos = mdims[i].output_min_pos;
      dims[i].output_max_pos = mdims[i].output_max_pos;
    }
    
  };

  
};

std::ostream& operator<<(std::ostream& os, const std::vector<Expr>& vec);
std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer);

std::map<std::string, HWBuffer> extract_hw_buffers(Stmt s, const std::map<std::string, Function> &env,
                                                   const std::vector<std::string> &streaming_loop_names);


struct HWXcel {
  std::string name;
  Function func;
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
