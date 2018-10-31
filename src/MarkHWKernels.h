#ifndef MARK_HW_KERNELS_H
#define MARK_HW_KERNELS_H

/** \file
 *
 * Defines analyses to mark hardware kernels between inputs and the output
 * functions.
 */
#include <set>
#include <map>

#include "IR.h"

namespace Halide {
namespace Internal {

/** Mark the functions transitively called by Function f as
 * HW kernels. We traverse all the function directly called
 * by Function f, or indirectly in those function, recursively.
 * The recursion only terminates at functions with names included
 * by set inputs.
 */
void mark_hw_kernels(Function f, const std::set<std::string> &inputs,
                     const std::set<std::string> &taps);

Stmt mark_hw_accelerators(Stmt s, const std::map<std::string, Function> &env);

}
}

#endif
