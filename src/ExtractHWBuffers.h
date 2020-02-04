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

#include "HWBuffer.h"

#include "Bounds.h"
#include "IR.h"

namespace Halide {
namespace Internal {

#ifndef HALIDE_BOUNDS_INFERENCE_STAGE
#define HALIDE_BOUNDS_INFERENCE_STAGE
struct BoundsInference_Stage {
    std::string name;
    size_t stage;
    std::vector<int> consumers;
    std::map<std::pair<std::string, int>, Box> bounds;
};
#endif

struct HWXcel {
  std::string name;
  Function func;
  LoopLevel store_level;
  LoopLevel compute_level;
  
  std::vector<std::string> streaming_loop_levels;  // store (exclusive) to compute (inclusive)
  std::set<std::string> input_streams; // might be wrong?
  std::map<std::string, HWBuffer> hwbuffers; // delete default copy constructor?
  std::map<std::string, HWBuffer*> consumer_buffers; // used for transforming call nodes and inserting dispatch calls

  //std::map<std::string, HWTap> input_taps;
};



std::map<std::string, HWBuffer> extract_hw_buffers(Stmt s, const std::map<std::string, Function> &env,
                                                   const std::vector<std::string> &streaming_loop_names);

std::vector<HWXcel> extract_hw_accelerators(Stmt s, const std::map<std::string, Function> &env,
                                            const std::vector<BoundsInference_Stage> &inlined_stages);


}  // namespace Internal
}  // namespace Halide



#endif
