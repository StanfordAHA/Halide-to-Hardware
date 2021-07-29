#ifndef HALIDE_CODEGEN_CLOCKWORK_TARGET_H
#define HALIDE_CODEGEN_CLOCKWORK_TARGET_H

/** \file
 *
 * Defines an IRPrinter that emits Clockwork code.
 */

#include "CodeGen_Clockwork_Base.h"
#include "CodeGen_RDAI.h"
#include "Module.h"
#include "Scope.h"

#include "coreir.h"

namespace Halide {

namespace Internal {

struct ROM_data {
  std::string name;
  Stmt stmt;
  Stmt produce;
};

struct for_loop {
  std::string name;
  std::string parent_loopname;
  int min;
  int max;
  bool equal(for_loop other) {
    if (name != other.name) return false;
    if (parent_loopname != other.parent_loopname) return false;
    if (min != other.min) return false;
    if (max != other.max) return false;
    //if (!equal(min, other.min)) return false;
    return true;
  }
};

/** This class emits Clockwork compatible C++ code.
 */
class CodeGen_Clockwork_Target {
public:
    /** Initialize a C code generator pointing at a particular output
     * stream (e.g. a file, or std::cout) */
    CodeGen_Clockwork_Target(const std::string &name, const Target& target);
    virtual ~CodeGen_Clockwork_Target();

    void init_module();

    void add_kernel(Stmt stmt,
                    const std::string &xcel_name,
                    const std::vector<HW_Arg> &args);

    void dump();
    void set_output_folder(const std::string& folderpath) {
      output_base_path = folderpath;
      hdrc.set_output_path(folderpath);
      srcc.set_output_path(folderpath);
      clkc.set_output_path(folderpath);
    }

protected:
    class CodeGen_Clockwork_C : public CodeGen_Clockwork_Base {
    public:
      bool is_clockwork;
      
      //std::string mem_bodyname;
      std::vector<std::string> loop_list; // current stack of loops
      std::set<std::string> loops; // loops added to module so far
      std::set<std::string> buffers; // buffers added to module so far
      std::map<std::string, ROM_data> roms; // roms that have been identified
      std::vector<std::string> inputs; // inputs to the function
      std::map<std::string, std::vector<Expr> > realize_glb_indices; // offsets for glb
      std::string output;

      /** The stream we're outputting the memory on */
      std::ostringstream memory_oss;
      std::ostream& memory_stream;
      /** The stream we're outputting the compute on */
      std::ostringstream compute_oss;
      std::ostream& compute_stream;
      
      CoreIR::Context* context;
      
      CodeGen_Clockwork_C(std::ostream &s, Target target, OutputKind output_kind) :
        CodeGen_Clockwork_Base(s, target, output_kind), is_clockwork(false), memory_stream(memory_oss), compute_stream(compute_oss) { }
        //CodeGen_Clockwork_Base(s, target, output_kind), is_clockwork(false), memory_stream(std::cout), compute_stream(compute_oss) { }
        //CodeGen_Clockwork_Base(s, target, output_kind), is_clockwork(false), memory_stream(memory_oss), compute_stream(std::cout) { }
        //CodeGen_Clockwork_Base(s, target, output_kind), is_clockwork(false), memory_stream(std::cout), compute_stream(std::cout) { }

      void set_output_path(std::string pathname) {
        output_base_path = pathname;
      }
  
      void add_kernel(Stmt stmt,
                      const std::string &name,
                      const string &xcel_name,
                      const std::vector<HW_Arg> &args);

    protected:
      Scope<Expr> scope;
      std::string print_stencil_pragma(const std::string &name);
      std::string output_base_path;
      void add_buffer(const std::string& buffer_name, int width);
        
      using CodeGen_Clockwork_Base::visit;

      void visit(const Provide *op);
      //void visit(const Store *op);
      void visit(const ProducerConsumer *op);
      void visit(const For *op);
      void visit(const Allocate *op);
      void visit(const Realize *op);
      void visit(const Call *op);
      void visit(const LetStmt *op);
    };

    /** A name for the Clockwork target */
    std::string target_name;
    std::string output_base_path;
    bool enable_ponds;

    /* Arguments for each accelerator*/
    std::vector<std::string> xcel_names;
    std::map<std::string, std::vector<HW_Arg>> closure_args;

    /** String streams for building header and source files. */
    // @{
    std::ostringstream hdr_stream;
    std::ostringstream src_stream;
    std::ostringstream clk_stream;
    // @}

    /** Code generators for Clockwork target header and the source. */
    // @{
    CodeGen_Clockwork_C hdrc;
    CodeGen_Clockwork_C srcc;
    CodeGen_Clockwork_C clkc;
    // @}
};

}
}

#endif
