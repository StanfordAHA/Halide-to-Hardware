#ifndef HALIDE_CODEGEN_RDAI_H
#define HALIDE_CODEGEN_RDAI_H

/** \file
 *
 * Defines the code-generator for producing RDAI-compatible wrappers
 */
#include <iostream>
#include <string>
#include <vector>

#include "Bounds.h"
#include "CodeGen_C.h"
#include "Module.h"
#include "Scope.h"

namespace Halide::Internal {

using std::string;
using std::ostream;
using std::vector;

struct RDAI_Info
{
    struct DeviceInfo;

    string platform_type;
    string platform_name;
    vector<DeviceInfo> devices;
   
    struct DeviceInfo
    {
        string vendor;
        string library;
        string name;
        int version;
        size_t num_inputs;
    };
};


struct HW_Stencil_Type
{
    enum class StencilContainerType { Stencil, Stream, AxiStream };
    
    StencilContainerType type;
    Type elemType;
    Region bounds;
    int depth;
};

struct HW_Arg
{
    string name;
    bool is_stencil;
    bool is_output;
    Type scalar_type;
    HW_Stencil_Type stencil_type;
    vector<Expr> args;
    Box box;
};

struct RDAI_TargetGenLike
{
    virtual void add_kernel(Stmt stmt, const string& kernel_name, const vector<HW_Arg>& args) = 0;
    virtual void set_output_folder(const string& out_dir) = 0;
};

class CodeGen_RDAI : public CodeGen_C {
public:
    CodeGen_RDAI(std::ostream &pipeline_stream, const Target& target, string pipeline_name);

    void set_output_folder(const string& out_folder);

    virtual RDAI_TargetGenLike *get_target_codegen() = 0;

    virtual ~CodeGen_RDAI();

    using CodeGen_C::compile;

protected:
    using CodeGen_C::visit;

    void visit(const Call *op) override;
    void visit(const ProducerConsumer *op) override;
    void visit(const Provide *op) override;
    void visit(const Realize *op) override;


    void compile(const LoweredFunc &func) override;
    
    std::string print_name(const string& name);

protected:
    Scope<HW_Stencil_Type> stencils;
    RDAI_Info rdai_info;

protected:
    const Target& target;
    string pipeline_name;
    string output_directory;
    vector<LoweredArgument> func_args;
    bool inserted_host_buf_calls = false;
    int num_xcels;
    int xcel_idx;
};

}

#endif // HALIDE_CODEGEN_RDAI_H
