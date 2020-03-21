#include "HWBuffer.h"
#include "HWBufferUtils.h"

using std::string;
using std::vector;
using std::map;

namespace Halide {
namespace Internal {

std::ostream& operator<<(std::ostream& os, const std::vector<AccessDimSize>& vec) {
  vector<Expr> range, stride, dim_ref;
  for (const auto& dim : vec) {
    range.emplace_back(dim.range);
    stride.emplace_back(dim.stride);
    dim_ref.emplace_back(dim.dim_ref);
  }

  os << "  Range: " << range << std::endl
     << "  Stride: " << stride << std::endl
     << "  Dim Ref: " << dim_ref << std::endl;

  return os;
};

std::ostream& operator<<(std::ostream& os, const Port port) {
  os << port.stream_index << "=" << port.mem_indices << "=" << port.starting_addr;
  return os;
}

std::ostream& operator<<(std::ostream& os, const map<vector<Expr>, Port, ExprVecCompare> oports) {
  os << "Ports: {";

  // go through all ports
  for (const auto& port_pair : oports) {
    const auto& port = port_pair.second;
    //os << port.stream_index << "=" << port.mem_indices << ", ";
    os << port << ", ";
  }
  os << "}";

  return os;
}

std::ostream& operator<<(std::ostream& os, const HWBuffer& buffer) {
  vector<Expr> total_buffer_box, total_buffer_box_flatten,input_chunk_box, input_block_box;
  vector<Expr> output_stencil_box, output_block_box, output_min_pos;
  for (const auto& dim : buffer.ldims) {
    total_buffer_box.emplace_back(dim.logical_size);
    total_buffer_box_flatten.emplace_back(dim.logical_size_flatten);
  }
  for (const auto& dim : buffer.dims) {
    input_chunk_box.emplace_back(dim.input_chunk);
    input_block_box.emplace_back(dim.input_block);
    output_stencil_box.emplace_back(dim.output_stencil);
    output_block_box.emplace_back(dim.output_block);
    //output_min_pos.emplace_back(dim.output_min_pos);
  }

  vector<string> input_istreams, output_ostreams;
  map<string, vector<Expr> > ostream_output_mins;
  map<string, vector<Expr> > ostream_output_stencils;
  map<string, vector<Expr> > ostream_output_blocks;
  map<string, vector<AccessDimSize> > ostream_linear_accesses;
  for (const auto& istream_pair : buffer.istreams) {
    input_istreams.emplace_back(istream_pair.first);
  }
  for (const auto& ostream_pair : buffer.ostreams) {
    vector<Expr> consumer_output_min;
    vector<Expr> consumer_output_stencil;
    vector<Expr> consumer_output_block;
    output_ostreams.emplace_back(ostream_pair.first);
    for (const auto& dim : ostream_pair.second.odims) {
      consumer_output_min.emplace_back(dim.output_min_pos);
      consumer_output_stencil.emplace_back(dim.output_stencil);
      consumer_output_block.emplace_back(dim.output_block);
    }
    ostream_output_mins[ostream_pair.first] = consumer_output_min;
    ostream_output_stencils[ostream_pair.first] = consumer_output_stencil;
    ostream_output_blocks[ostream_pair.first] = consumer_output_block;
    ostream_linear_accesses[ostream_pair.first] = ostream_pair.second.linear_access;
  }

  //auto num_inputs = 0;//buffer.func.updates().size();
  //auto num_outputs = 0;//buffer.consumer_buffers.size();

  os << "HWBuffer: " << buffer.name << std::endl
     << "Logical Buffer: " << total_buffer_box << std::endl
     << "Logical Buffer flatten: " << total_buffer_box_flatten << std::endl
     << "Input Chunk: " << input_chunk_box << std::endl
     << "Input Block: " << input_block_box << std::endl
     << "Output Stencil: " << output_stencil_box << std::endl
     << "Output Block: " << output_block_box << std::endl
     << "Output Access Pattern:\n " << buffer.output_access_pattern << std::endl;
    //<< "Output Min Pos: " << output_min_pos << std::endl;

  //os << buffer.linear_addr << std::endl;

  for (const auto& omp_pair : ostream_output_mins) {
    os << "Ostream " << omp_pair.first << " Min Pos: "
       << omp_pair.second << std::endl;
  }
  for (const auto& osten_pair : ostream_output_stencils) {
    os << "Ostream " << osten_pair.first << " Output Stencil: "
       << osten_pair.second << std::endl;
  }
  for (const auto& osten_pair : ostream_output_blocks) {
    os << "Ostream " << osten_pair.first << " Output Block: "
       << osten_pair.second << std::endl;
  }
  for (const auto& olac_pair : ostream_linear_accesses) {
    os << "Ostream " << olac_pair.first << " Linear Access Pattern: " << std::endl
       << olac_pair.second;
  }

  os << "streaming loops: " << buffer.streaming_loops << std::endl
     << "compute level: " << buffer.compute_level << std::endl
     << "store level: " << buffer.store_level << std::endl
     << "is_inline=" << buffer.is_inlined << std::endl
     << "is_output=" << buffer.is_output << std::endl
     << "input_streams=" << input_istreams << std::endl
     << "output_streams=" << output_ostreams << std::endl;
  //<< "num_inputs=" << num_inputs << std::endl
  //<< "num_output=" << num_outputs << std::endl;


  return os;
};

HWBuffer::HWBuffer(string name, vector<MergedDimSize> mdims, vector<AccessDimSize> linear_addr,
                   vector<string> loops, int store_index, int compute_index, bool is_inlined, bool is_output,
                   string iname, string oname) :
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

  OutputStream ostream;
  ostream.odims = std::vector<OutputDimSize>(mdims.size());

  dims = std::vector<InOutDimSize>(mdims.size());
  for (size_t i=0; i<mdims.size(); ++i) {
    istream.idims.at(i).loop_name       = mdims.at(i).loop_name;
    istream.idims.at(i).input_chunk     = mdims.at(i).input_chunk;
    istream.idims.at(i).input_block     = mdims.at(i).input_block;

    ostream.odims.at(i).loop_name      = mdims.at(i).loop_name;
    ostream.odims.at(i).output_stencil = mdims.at(i).output_stencil;
    ostream.odims.at(i).output_block   = mdims.at(i).output_block;
    ostream.odims.at(i).output_min_pos = mdims.at(i).output_min_pos;
    ostream.odims.at(i).output_max_pos = mdims.at(i).output_max_pos;
  }

  ostream.linear_access = linear_addr;

  istreams[iname] = istream;
  ostreams[oname] = ostream;

  // old way
  dims = std::vector<InOutDimSize>(mdims.size());
  for (size_t i=0; i<mdims.size(); ++i) {
    dims[i].loop_name      = mdims[i].loop_name;
    dims[i].input_chunk    = mdims[i].input_chunk;
    dims[i].input_block    = mdims[i].input_block;
    dims[i].output_stencil = mdims[i].output_stencil;
    dims[i].output_block   = mdims[i].output_block;
    //dims[i].output_min_pos = mdims[i].output_min_pos;
    //dims[i].output_max_pos = mdims[i].output_max_pos;
  }

};

std::vector<MergedDimSize> create_hwbuffer_sizes(std::vector<int> logical_size,
                                                 std::vector<int> output_stencil, std::vector<int> output_block,
                                                 std::vector<int> input_chunk, std::vector<int> input_block) {
   internal_assert(logical_size.size() == output_stencil.size());
   internal_assert(logical_size.size() == output_block.size());
   internal_assert(logical_size.size() == input_chunk.size());
   internal_assert(logical_size.size() == input_block.size());

   std::vector<MergedDimSize> dims(logical_size.size());

   for (size_t i=0; i < logical_size.size(); ++i) {
     dims[i] = MergedDimSize({"loop" + std::to_string(i), Expr(logical_size.at(i)), Expr(0),
           Expr(input_chunk.at(i)), Expr(input_block.at(i)),
           Expr(output_stencil.at(i)), Expr(output_block.at(i)), Expr(0)});
   }

   return dims;
}

std::vector<AccessDimSize> create_linear_addr(std::vector<int> ranges,
                                              std::vector<int> strides,
                                              std::vector<int> dim_refs) {
   internal_assert(ranges.size() == strides.size());
   internal_assert(ranges.size() == dim_refs.size());

   std::vector<AccessDimSize> dims(ranges.size());

   for (size_t i=0; i < ranges.size(); ++i) {
     dims[i] = AccessDimSize({Expr(ranges.at(i)), Expr(strides.at(i)), Expr(dim_refs.at(i))});
   }

   return dims;
}


}  // namespace Internal
}  // namespace Halide

