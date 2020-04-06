#ifndef HALIDE_HW_BUFFERS_H
#define HALIDE_HW_BUFFERS_H

/** \file
 *
 * Defines a hardware buffer, which has parameters for a  general buffer
 * that can then be outputted to a hardware backend.
 * These parameters provide specification for line buffers,  double
 * buffers, and general unified buffers.
 *
 */

#include <map>
#include <set>
#include <vector>
#include <iostream>

#include "IR.h"
#include "SlidingWindow.h"

namespace Halide {
namespace Internal {

struct HWBuffer; // forward declare

// used to make a map with a key of vector<Expr>
struct ExprVecCompare {
  bool operator()(const std::vector<Expr> &av, const std::vector<Expr> &bv) const {
    // loop through vectors
    auto len = std::min(av.size(), bv.size());
    for (size_t i=0; i<len; ++i) {
      auto a = av.at(i);
      auto b = bv.at(i);

      if (a.get() != b.get()) {
        return a.get() < b.get();
      }
    }

    // all are equal, so return false
    return false;
  }
};

struct Port {
  int stream_index;   // ports are labeled in order
  std::vector<Expr> mem_indices; // original 2d address
  int starting_addr;  // mem_indices * logical
};

struct Stride {
  int stride;
  bool is_inverse;
  Stride(int stride=0, bool is_inverse=false) :
    stride(stride), is_inverse(is_inverse) {}

};

struct AccessDimSize {
  Expr range;
  Expr stride;
  Expr dim_ref;
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
  Expr input_min_pos;
};

struct InputStream {
  std::string name;
  std::vector<InputDimSize> idims;
  Stmt input_access_pattern;
  std::map<std::string, Stride> stride_map;
  std::vector<AccessDimSize> linear_access; // use access pattern and stride map to construct this
  HWBuffer* hwref;
};

//struct UpdateStream {
//  std::string name;
//  // no dimension info needed, because same as input; perhaps needs min_pos?
//  Stmt update_access_pattern; // output of hwbuffer on each update
//  std::map<std::string, Stride> stride_map;
//  std::vector<AccessDimSize> linear_access;
//};

struct OutputDimSize {
  std::string loop_name;
  Expr output_stencil;
  Expr output_block;
  Expr output_min_pos;
  Expr output_max_pos;
};

struct OutputStream {
  std::string name;
  std::vector<OutputDimSize> odims;
  std::map<std::vector<Expr>, Port, ExprVecCompare> oports;
  Stmt output_access_pattern;
  int num_stencil_acc_dim; // which loop level corresponds to the compute level
  std::map<std::string, Stride> stride_map;
  std::vector<AccessDimSize> linear_access;
  //std::shared_ptr<HWBuffer> hwref;
  HWBuffer* hwref;
};

//struct RMWStream {
//  std::string name;
//  std::vector<InputDimSize> idims;
//  std::vector<OutputDimSize> mdims;
//  std::vector<OutputDimSize> odims;
//  Stmt input_access_pattern;
//  Stmt modify_access_pattern;
//  Stmt output_access_pattern;
//  std::map<std::string, Stride> stride_map;
//  //std::shared_ptr<HWBuffer> ohwref;
//  HWBuffer* ohwref;
//};

//struct StreamBundle {
//  std::string name;
//  InputStream istream;
//  std::vector<UpdateStream> ustreams;
//  std::vector<OutputStream> ostreams;
//
//};

struct InOutDimSize {
  std::string loop_name;

  Expr input_chunk;    // replace stencilfor, dispatch, need_hwbuffer, add_hwbuffer,
                       // transform_hwkernel, xcel inserthwbuffers
  Expr input_block;    // add_hwbuffer
  Expr input_min_pos;

  Expr output_stencil; // add_hwbuffer, dispatch
  Expr output_block;   // add_hwbuffer
  //Expr output_min_pos; // in provide+call shifts
  //Expr output_max_pos; // not used yet?
};

struct LogicalDimSize {
  Expr var_name;
  Expr logical_size;
  Expr logical_size_flatten;
  Expr logical_min;
};

std::vector<MergedDimSize> create_hwbuffer_sizes(std::vector<int> logical_size,
                                                 std::vector<int> output_stencil,
                                                 std::vector<int> output_block,
                                                 std::vector<int> input_chunk,
                                                 std::vector<int> input_block);

std::vector<AccessDimSize> create_linear_addr(std::vector<int> range,
                                              std::vector<int> stride,
                                              std::vector<int> dim_ref);

std::ostream& operator<<(std::ostream& os, const std::vector<AccessDimSize>& vec);
std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer);
std::ostream& operator<<(std::ostream& os, const std::map<std::vector<Expr>, Port, ExprVecCompare> oports);
std::ostream& operator<<(std::ostream& os, const Port port);

struct HWBuffer {
  std::string name;
  std::string store_level;
  std::string compute_level;
  std::vector<std::string> streaming_loops;

  Stmt my_stmt;
  Function func;
  bool is_inlined = false;
  bool is_output = false;
  int num_accum_iters = 0;

  // old parameters for the HWBuffer
  std::vector<InOutDimSize> dims;
  Stmt output_access_pattern;
  //Stmt input_access_pattern;
  //std::map<std::string, HWBuffer*> producer_buffers;
  //std::vector<std::string> input_streams;  // used when inserting read_stream calls; should make a set?
  //std::map<std::string, HWBuffer*> consumer_buffers;   // used for transforming call nodes and inserting dispatch calls
  //std::map<std::string, Stride> stride_map;
  //std::vector<AccessDimSize> linear_addr;

  // dimensions for the unassociated hwbuffer streams
  std::vector<LogicalDimSize> ldims;
  std::map<std::string, InputStream> istreams;
  std::vector<std::vector<Expr> > up_min_pos;
  //std::map<std::string, OutputStream> ustreams;
  std::map<std::string, OutputStream> ostreams;

  // dimensions for accumulation hwbuffer
  //std::map<std::string, StreamBundle> streams;

  // Constructors
  HWBuffer() { }

  //HWBuffer(const HWBuffer &b) = delete;

  HWBuffer(std::string name, std::vector<MergedDimSize> mdims, std::vector<AccessDimSize> linear_addr,
           std::vector<std::string> loops, int store_index, int compute_index, bool is_inlined, bool is_output,
           std::string iname="input", std::string oname="output");

};

}  // namespace Internal
}  // namespace Halide

#endif
