#include <iostream>//
#include <fstream>
#include <limits>
#include <algorithm>

#include "CodeGen_Internal.h"
#include "CodeGen_CoreIR_Target.h"
#include "CodeGen_CoreHLS.h"
#include "Substitute.h"
#include "IRMutator.h"
#include "IROperator.h"
#include "Param.h"
#include "Var.h"
#include "Lerp.h"
#include "Simplify.h"
#include "Debug.h"

#include "coreir.h"
#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"
#include "lakelib.h"

namespace Halide {
namespace Internal {

using std::ostream;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::ostringstream;
using std::ofstream;
using std::cout;

namespace {

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
  os << "[";
  for (size_t i=0; i<vec.size(); ++i) {
    os << vec.at(i);
    if (i < vec.size() - 1) {
      os << ",";
    }
  }
  os << "]";
  return os;
};

std::ostream& operator<<(std::ostream& os, const std::vector<size_t>& vec) {
  os << "[";
  for (size_t i=0; i<vec.size(); ++i) {
    os << vec.at(i);
    if (i < vec.size() - 1) {
      os << ",";
    }
  }
  os << "]";
  return os;
};


int id_const_value(const Expr e) {
  if (const IntImm* e_int = e.as<IntImm>()) {
    return e_int->value;

  } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    return e_uint->value;

  } else {
    return -1;
  }
}

float id_fconst_value(const Expr e) {
  if (const FloatImm* e_float = e.as<FloatImm>()) {
    return e_float->value;

  } else {
    //internal_error << "invalid constant expr\n";
    return -1;
  }
}

vector<CoreIR::Wireable*> get_wires(CoreIR::Wireable* base_wire, const vector<size_t> ports) {
  int num_ports = 1;
  for (const auto& port_length : ports) {
    num_ports *= port_length;
  }
//  cout << "we have " << num_ports << " ports\n";
  vector<CoreIR::Wireable*> all_wires(num_ports);

  vector<uint> port_idxs(ports.size());

  for (int idx = 0; idx < num_ports; ++idx) {
    // find the wire associated with the indices
    CoreIR::Wireable* cur_wire = base_wire;
    for (const auto& port_idx : port_idxs) {
      cur_wire = cur_wire->sel(port_idx);
    }

    // add the wire to our list
    all_wires.at(idx) = cur_wire;

    // increment  index
    port_idxs.at(0) += 1;
    for (size_t dim = 0; dim < port_idxs.size(); ++dim) {
      if (port_idxs.at(dim) >= ports.at(dim)) {
        port_idxs.at(dim) = 0;
        if (dim + 1 < port_idxs.size()) {
          port_idxs.at(dim+1) += 1;
        }
      }
    }
  }

  return all_wires;
}

vector<CoreIR::Wireable*> get_wires(CoreIR::Wireable* base_wire, const vector<size_t> ports, string wire_prefix) {
  int num_ports = 1;
  for (const auto& port_length : ports) {
    num_ports *= port_length;
  }

  vector<CoreIR::Wireable*> all_wires(num_ports);

  vector<uint> port_idxs(ports.size());

  for (int idx = 0; idx < num_ports; ++idx) {
    // find the wire associated with the indices
    string selstr = "";
    for (const auto& port_idx : port_idxs) {
      selstr += std::to_string(port_idx) + ".";
    }
    //sel_str = sel_str.substr(0, sel_str.size()-1);
    selstr.pop_back();

    // add the wire to our list
    all_wires.at(idx) = base_wire->sel(wire_prefix + selstr);

    // increment  index
    port_idxs.at(0) += 1;
    for (size_t dim = 0; dim < port_idxs.size(); ++dim) {
      if (port_idxs.at(dim) >= ports.at(dim)) {
        port_idxs.at(dim) = 0;
        if (dim + 1 < port_idxs.size()) {
          port_idxs.at(dim+1) += 1;
        }
      }
    }
  }

  return all_wires;
}


void connect_wires(CoreIR::ModuleDef *def, vector<CoreIR::Wireable*> in_wires, vector<CoreIR::Wireable*> out_wires) {
  assert(in_wires.size() == out_wires.size());

  for (size_t idx=0; idx<in_wires.size(); ++idx) {
    def->connect(in_wires.at(idx), out_wires.at(idx));
  }
}


class ContainForLoop : public IRVisitor {
  using IRVisitor::visit;
  void visit(const For *op) {
    found = true;
    varnames.push_back(op->name);
  }

public:
  bool found;
  vector<string> varnames;
  ContainForLoop() : found(false) {}
};

// identifies for loops in code statement
bool contain_for_loop(Stmt s) {
  ContainForLoop cfl;
  s.accept(&cfl);
  return cfl.found;
}

// Identifies for loop name in code statement.
//  Gives name of first for loop
string name_for_loop(Stmt s) {
  ContainForLoop cfl;
  s.accept(&cfl);
  return cfl.varnames[0];
}

// Identifies all for loop names in code statement.
vector<string> contained_for_loop_names(Stmt s) {
  ContainForLoop cfl;
  s.accept(&cfl);
  return cfl.varnames;
}


class UsesVariable : public IRVisitor {
  using IRVisitor::visit;
  void visit(const Variable *op) {
    if (op->name == varname) {
      used = true;
    }
    return;
  }

  void visit(const Call *op) {
    // only go first two variables, not loop bound checks
    if (op->name == "write_stream" && op->args.size() > 2) {
      op->args[0].accept(this);
      op->args[1].accept(this);
    } else {
      IRVisitor::visit(op);
    }
  }

public:
  bool used;
  string varname;
  UsesVariable(string varname) : used(false), varname(varname) {}
};

// identifies target variable string in code statement
bool variable_used(Stmt s, string varname) {
  UsesVariable uv(varname);
  s.accept(&uv);
  return uv.used;
}

class ROMInit : public IRVisitor {
  using IRVisitor::visit;

  bool is_const(const Expr e) {
    if (e.as<IntImm>() || e.as<UIntImm>()) {
      return true;
    } else {
      return false;
    }
  }

  void visit(const Store *op) {
    if (op->name == allocname) {
      auto value_expr = op->value;
      auto index_expr = op->index;
      //cout << "store: " << Stmt(op) << std::endl;
      internal_assert(is_const(value_expr) && is_const(index_expr));

      int index = id_const_value(index_expr);
      int value = id_const_value(value_expr);
      init_values["init"][index] = value;
    }
  }

public:
  nlohmann::json init_values;
  string allocname;
  ROMInit(string allocname) : allocname(allocname) {}
};

// returns a map with all the initialization values for a rom
nlohmann::json rom_init(Stmt s, string allocname) {
  ROMInit rom_init(allocname);
  s.accept(&rom_init);
  return rom_init.init_values;
}



class AllocationUsage : public IRVisitor {
  using IRVisitor::visit;
  void visit(const Load *op) {
    if (op->name == alloc_name) {
      num_loads++;
      load_index_exprs.emplace_back(op->index);

      if (!is_const(op->index)) {
        uses_variable_load_index = true;
      }
    }
  }

  void visit(const Store *op) {
    if (op->name == alloc_name) {
      num_stores++;
      store_index_exprs.emplace_back(op->index);

      if (!is_const(op->index)) {
        uses_variable_store_index = true;
      }
      if (!is_const(op->value)) {
        uses_variable_store_value = true;
      }

    }
  }


 public:
  bool uses_variable_load_index;
  bool uses_variable_store_index;
  bool uses_variable_store_value;
  bool load_index_equals_store_index;
  uint num_loads;
  uint num_stores;
  vector<Expr> store_index_exprs;
  vector<Expr> load_index_exprs;
  string alloc_name;

  AllocationUsage(string allocname) : uses_variable_load_index(false),
                                      uses_variable_store_index(false),
                                      uses_variable_store_value(false),
                                      load_index_equals_store_index(false),
                                      num_loads(0),
                                      num_stores(0),
                                      alloc_name(allocname) {}
};

enum AllocationType {
  NO_ALLOCATION,
  INOUT_ALLOCATION,
  ROM_ALLOCATION,
  REGS_ALLOCATION,
  SRAM_ALLOCATION,
  RMW_ALLOCATION,
  UNKNOWN_ALLOCATION
};

AllocationType identify_allocation(Stmt s, string allocname) {
  AllocationUsage au(allocname);
  s.accept(&au);

  if (au.num_stores == 0 || au.num_loads == 0) {
    return INOUT_ALLOCATION;

  } else if (!au.uses_variable_load_index &&
             !au.uses_variable_store_value) {
    //&& !au.uses_variable_store_value) {
    return NO_ALLOCATION;

  } else if (au.uses_variable_load_index &&
             //!au.uses_variable_store_index &&
             !au.uses_variable_store_value) {
    return ROM_ALLOCATION;

  } else if (au.uses_variable_load_index &&
             au.uses_variable_store_index &&
             au.load_index_equals_store_index) {
    return RMW_ALLOCATION;

  } else if (au.uses_variable_load_index &&
             au.uses_variable_store_index &&
             !au.load_index_equals_store_index) {
    return SRAM_ALLOCATION;

  } else {
    return UNKNOWN_ALLOCATION;
  }

}

bool variable_index_load(Stmt s, string allocname) {
  AllocationUsage au(allocname);
  s.accept(&au);
  return au.uses_variable_load_index;
}

bool can_use_rom(Stmt s, string allocname) {
  AllocationUsage au(allocname);
  s.accept(&au);
  return (!au.uses_variable_store_index &&
          !au.uses_variable_store_value);
}


}

CodeGen_CoreIR_Target::CodeGen_CoreIR_Target(const string &name, Target target)
  : target_name(name),
    hdrc(hdr_stream, target, CodeGen_CoreIR_C::CPlusPlusHeader),
    srcc(std::cout, target, CodeGen_CoreIR_C::CPlusPlusImplementation) { }
//srcc(src_stream, target, CodeGen_CoreIR_C::CPlusPlusImplementation) { }

  CodeGen_CoreIR_Target::CodeGen_CoreIR_C::CodeGen_CoreIR_C(std::ostream &s,
                                                            Target target,
                                                            OutputKind output_kind) :
    CodeGen_CoreIR_Base(s, target, output_kind), has_valid(target.has_feature(Target::CoreIRValid)) {
  // set up coreir generation
  bitwidth = 16;
  context = CoreIR::newContext();
  global_ns = context->getGlobal();

  // add all generators from coreirprims
  context->getNamespace("coreir");
  std::vector<string> corelib_gen_names = {"mul", "add", "sub",
                                           "and", "or", "xor", "not",
                                           "eq", "neq",
                                           "ult", "ugt", "ule", "uge",
                                           "slt", "sgt", "sle", "sge",
                                           "shl", "ashr", "lshr",
                                           "mux", "const", "wire"};

  for (auto gen_name : corelib_gen_names) {
    gens[gen_name] = "coreir." + gen_name;
    internal_assert(context->hasGenerator(gens[gen_name]))
      << "could not find " << gen_name << "\n";
  }

  // add all generators from commonlib
  CoreIRLoadLibrary_commonlib(context);
  loadHalideLib(context);
  std::vector<string> commonlib_gen_names = {"umin", "smin", "umax", "smax", "div",
                                             "counter", //"linebuffer",
                                             "muxn", "abs", "absd",
                                             "reg_array", "reshape", "transpose_reshape"
  };
  for (auto gen_name : commonlib_gen_names) {
    gens[gen_name] = "commonlib." + gen_name;
    internal_assert(context->hasGenerator(gens[gen_name]))
      << "could not find " << gen_name << "\n";
  }

  // add all generators from fplib which include floating point operators
  CoreIRLoadLibrary_float(context);
  std::vector<string> fplib_gen_names = {"fmul", "fadd", "fsub", "fdiv",
                                         "feq", //"fneq",
                                         "flt", "fgt", "fle", "fge",
  };
                                         //"fmin", "fmax",
                                         //"fmux", "fconst"};

  for (auto gen_name : fplib_gen_names) {
    // floating point library does not start with "f"
    gens[gen_name] = "float." + gen_name.substr(1);
    internal_assert(context->hasGenerator(gens[gen_name]))
      << "could not find " << gen_name << "\n";
  }
  gens["fconst"] = "coreir.const";

  // add all modules from corebit
  context->getNamespace("corebit");
  std::vector<string> corebitlib_mod_names = {"bitand", "bitor", "bitxor", "bitnot",
                                              "bitmux", "bitconst"};
  //                                              "bitlt", "bitle", "bitgt", "bitge","bitxnor"};
  for (auto mod_name : corebitlib_mod_names) {
    // these were renamed to using the corebit library
    gens[mod_name] = "corebit." + mod_name.substr(3);
    internal_assert(context->hasModule(gens[mod_name]))
      << "could not find " << mod_name << "\n";
  }


  // add all modules from memory
  context->getNamespace("memory");
  std::vector<string> memorylib_gen_names = {"ram2","rom2",
                                             "fifo",};

  for (auto gen_name : memorylib_gen_names) {
    gens[gen_name] = "memory." + gen_name;
    internal_assert(context->hasGenerator(gens[gen_name]))
      << "could not find " << gen_name << "\n";
  }

  // add all generators from lakelib which include some cgra libs
  CoreIRLoadLibrary_lakelib(context);
  std::vector<string> lakelib_gen_names = {"linebuffer", "unified_buffer",
                                           "new_unified_buffer"};

  for (auto gen_name : lakelib_gen_names) {
    gens[gen_name] = "lakelib." + gen_name;
    internal_assert(context->hasGenerator(gens[gen_name]))
      << "could not find " << gen_name << "\n";
  }

  // passthrough is now just a mantle wire
  gens["passthrough"] = "mantle.wire";
  assert(context->hasGenerator(gens["passthrough"]));

}


CodeGen_CoreIR_Target::~CodeGen_CoreIR_Target() {
  hdr_stream << "#endif\n";

  // write the header and the source streams into files
  string src_name = output_base_path + target_name + "_debug_output.cpp";
  string hdr_name = output_base_path + target_name + "_debug_output.h";
  ofstream src_file(src_name.c_str());
  ofstream hdr_file(hdr_name.c_str());
  src_file << src_stream.str() << endl;
  hdr_file << hdr_stream.str() << endl;
  src_file.close();
  hdr_file.close();
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::saveCoreIRJson(string filename) {
  std::string RED = "\033[0;31m";
  std::string RESET = "\033[0m";

  if (!saveToFile(global_ns, filename, design)) {
    cout << RED << "Could not save to json!!" << RESET << endl;
    context->die();
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::saveCoreIRDot(string filename) {
  std::string RED = "\033[0;31m";
  std::string RESET = "\033[0m";

  if (!saveToDot(design, filename)) {
    cout << RED << "Could not save to dot!!" << RESET << endl;
    context->die();
  }
}
CodeGen_CoreIR_Target::CodeGen_CoreIR_C::~CodeGen_CoreIR_C() {
  if (def != NULL && def->hasInstances()) {
    // check the completed coreir design
    design->setDef(def);
    context->checkerrors();
    design->print();

    std::string RED = "\033[0;31m";
    std::string RESET = "\033[0m";
    std::string GREEN = "\033[0;32m";

    // Save a version without any passes run
    cout << "Saving to json" << endl;
    saveCoreIRJson(output_base_path + "design_prepass.json");

    // Run generators and remove extra passthroughs for mapper.
    context->runPasses({"rungenerators","removewires"});
    saveCoreIRJson(output_base_path + "design_top.json");
    saveCoreIRDot(output_base_path + "design_top.txt");

    //context->runPasses({"rungenerators", "removewires"});
    context->runPasses({"rungenerators","flatten","removewires"});
    //context->runPasses({"verifyconnectivity --onlyinputs --noclkrst"},{"global","commonlib","memory","mantle"});
    //context->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});

    cout << "Validating json" << endl;
    design->getDef()->validate();


    // write out the json
    cout << "Saving json" << endl;
    saveCoreIRJson(output_base_path + "design_flattened.json");

    // test that these files can be read
    CoreIR::Module* m = nullptr;
    cout << "Loading json" << endl;
    if (!loadFromFile(context, output_base_path + "design_top.json", &m)) {
      cout << RED << "Could not load from json!!" << RESET << endl;
      context->die();
    }

    CoreIR::Module* mod2 = nullptr;
    if (!loadFromFile(context, output_base_path + "design_prepass.json", &mod2)) {
      cout << RED << "Could not load from json!!" << RESET << endl;
      context->die();
    }

    context->runPasses({"removewires"});
    ASSERT(m, "Could not load top: design");
    //m->print();

    cout << GREEN << "Created CoreIR design!!!" << RESET << endl;

    CoreIR::deleteContext(context);
  } else {
    if (def == NULL) {
      cout << "// null def- \n";
    } else if (!def->hasInstances()) {
      cout << "// no instances- \n";
    }
    stream << "//No target json outputted " << endl;
  }
}

namespace {
const string hls_header_includes =
  "#include <assert.h>\n"
  "#include <stdio.h>\n"
  "#include <stdlib.h>\n"
  "#include <hls_stream.h>\n"
  "#include \"Stencil.h\"\n";
}

int inst_bitwidth(int input_bitwidth) {
  // FIXME: properly create bitwidths 1, 8, 16, 32
  if (input_bitwidth == 1) {
    return 1;
  } else {
    return 16;
  }
}

void CodeGen_CoreIR_Target::init_module() {
  debug(1) << "CodeGen_CoreIR_Target::init_module\n";

  // wipe the internal streams
  hdr_stream.str("");
  hdr_stream.clear();
  src_stream.str("");
  src_stream.clear();

  // initialize the header file
  string module_name = "HALIDE_CODEGEN_COREIR_TARGET_" + target_name + "_H";
  std::transform(module_name.begin(), module_name.end(), module_name.begin(), ::toupper);
  hdr_stream << "#ifndef " << module_name << '\n';
  hdr_stream << "#define " << module_name << "\n\n";
  hdr_stream << hls_header_includes << '\n';

  // initialize the source file
  src_stream << "#include \"" << target_name << ".h\"\n\n";
  src_stream << "#include \"Linebuffer.h\"\n"
             << "#include \"halide_math.h\"\n";

}

void CodeGen_CoreIR_Target::add_kernel(Stmt s,
                                       const string &name,
                                       const vector<CoreIR_Argument> &args) {
  debug(1) << "CodeGen_CoreIR_Target::add_kernel " << name << "\n";

  hdrc.add_kernel(s, name, args);
  srcc.add_kernel(s, name, args);
}

void CodeGen_CoreIR_Target::dump() {
  std::cerr << src_stream.str() << std::endl;
}

string CodeGen_CoreIR_Target::CodeGen_CoreIR_C::print_stencil_pragma(const string &name) {
  ostringstream oss;
  internal_assert(stencils.contains(name));
  Stencil_Type stype = stencils.get(name);
  if (stype.type == Stencil_Type::StencilContainerType::Stream ||
      stype.type == Stencil_Type::StencilContainerType::AxiStream) {
    oss << "#pragma CoreIR STREAM variable=" << print_name(name) << " depth=" << stype.depth << "\n";
    if (stype.depth <= 100) {
      // use shift register implementation when the FIFO is shallow
      oss << "#pragma CoreIR RESOURCE variable=" << print_name(name) << " core=FIFO_SRL\n\n";
    }
  } else if (stype.type == Stencil_Type::StencilContainerType::Stencil) {
    oss << "#pragma CoreIR ARRAY_PARTITION variable=" << print_name(name) << ".value complete dim=0\n\n";
  } else {
    internal_error;
  }
  return oss.str();
}

uint num_bits(uint N) {
  if (N==0) { return 1; }

  uint num_shifts = 0;
  uint temp_value = N;
  while (temp_value > 0) {
    temp_value  = temp_value >> 1;
    num_shifts++;
  }
  return num_shifts;
}

// add new design
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::add_kernel(Stmt stmt,
                                                         const string &name,
                                                         const vector<CoreIR_Argument> &args) {

  //if (is_header()) {
  //  return;
  //} else {
  //  global_ns = context->getNamespace("global");
  //  HardwareInfo info;
  //  info.hasCriticalPathTarget = false;
  //  design = createCoreIRForStmt(context, info, stmt, name, args);
  //  def = design->getDef();
  //  self = def->sel("self");
  //  return;
  //}



  // Emit the function prototype
  // keep track of number of inputs/outputs to determine if file is needed
  uint num_inouts = 0;

  // Keep track of the inputs, output, and taps for this module
  std::vector<std::pair<string, CoreIR::Type*>> input_types;
  std::vector<std::pair<string, CoreIR::Type*>> in_en_types;
  std::map<string, CoreIR::Type*> tap_types;
  CoreIR::Type* output_type = context->Bit();

  stream << "void " << print_name(name) << "(\n";
  for (size_t i = 0; i < args.size(); i++) {
    string arg_name = "arg_" + std::to_string(i);

    if (args[i].is_stencil) {
      Stencil_Type stype = args[i].stencil_type;

      internal_assert(args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream ||
                      args[i].stencil_type.type == Stencil_Type::StencilContainerType::Stencil);
      stream << print_stencil_type(args[i].stencil_type) << " ";
      if (args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream) {
        stream << "&";  // hls_stream needs to be passed by reference
      }
      stream << arg_name;
      allocations.push(args[i].name, {args[i].stencil_type.elemType});
      stencils.push(args[i].name, args[i].stencil_type);

      // create an array of size in each dimension
      vector<uint> indices;
      for(const auto &range : stype.bounds) {
        assert(is_const(range.extent));
        indices.push_back(id_const_value(range.extent));
      }

      if (args[i].is_output && args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream) {
        // add as the output
        uint out_bitwidth = inst_bitwidth(stype.elemType.bits());
        if (out_bitwidth > 1) { output_type = output_type->Arr(out_bitwidth); }
        for (uint i=0; i<indices.size(); ++i) {
          //std::cout << "output with appended width=" << indices[i] << std::endl;
          output_type = output_type->Arr(indices[i]);
          //output_type = output_type->Arr(1);
        }
        hw_output_set.insert(arg_name);

      } else if (!args[i].is_output && args[i].stencil_type.type == Stencil_Type::StencilContainerType::AxiStream) {
        // add another input
        uint in_bitwidth = inst_bitwidth(stype.elemType.bits());
        CoreIR::Type* input_type = in_bitwidth > 1 ? context->BitIn()->Arr(in_bitwidth) : context->BitIn();
        for (uint i=0; i<indices.size(); ++i) {
          //input_type = input_type->Arr(indices[i]);
          input_type = input_type->Arr(1);
        }
        input_types.push_back({arg_name, input_type});
        in_en_types.push_back({arg_name, context->BitIn()});

      } else {
        // add another array of taps (configuration changes infrequently)
        uint in_bitwidth = inst_bitwidth(stype.elemType.bits());
        CoreIR::Type* tap_type = context->Bit()->Arr(in_bitwidth);
        for (uint i=0; i<indices.size(); ++i) {
          tap_type = tap_type->Arr(indices[i]);
        }
        tap_types[args[i].name] = tap_type;
      }

      num_inouts++;

    } else {
      // add another tap (single value)
      stream << print_type(args[i].scalar_type) << " " << arg_name;
      uint in_bitwidth = inst_bitwidth(args[i].scalar_type.bits());
      CoreIR::Type* tap_type = context->BitIn()->Arr(in_bitwidth);
      tap_types[arg_name] = tap_type;
    }

    if (i < args.size()-1) stream << ",\n";
  }

  // Emit prototype to coreir
  create_json = (num_inouts > 0);

  //cout << "design has " << num_inputs << " inputs with bitwidth " << to_string(bitwidth) << " " <<endl;

  // Create CoreIR design interface with input and output types.
  //  Output a valid bit if that exists in the design.
  CoreIR::Type* design_type;
  cout << "creating a design with " << input_types.size()
       << " inputs from " << args.size() << " args\n";

  if (has_valid) {
    design_type = context->Record({
      {"in", context->Record(input_types)},
      {"in_en", context->Record(in_en_types)},
      {"reset", context->BitIn()},
      {"out", output_type},
      {"valid", context->Bit()}
    });
  } else {
    design_type = context->Record({
      {"in", context->Record(input_types)},
      {"out", output_type}
    });
  }

  if (context->hasModule("global.DesignTop")) {
    return;
  }

  design = global_ns->newModuleDecl("DesignTop", design_type);
  design->print();
  def = design->newModuleDef();
  self = def->sel("self");

  for (auto input_pair : input_types) {
    string arg_name = input_pair.first;
    hw_input_set[arg_name] = self->sel("in")->sel(arg_name);
  }

  if (is_header()) {
    stream << ");\n";
  } else {
    stream << ")\n";
    open_scope();

    // add CoreIR pragma at function scope
    stream << "#pragma CoreIR DATAFLOW\n"
           << "#pragma CoreIR INLINE region\n"
           << "#pragma CoreIR INTERFACE s_axilite port=return"
           << " bundle=config\n";
    for (size_t i = 0; i < args.size(); i++) {
      string arg_name = "arg_" + std::to_string(i);
      if (args[i].is_stencil) {
        if (ends_with(args[i].name, ".stream")) {
          // stream arguments use AXI-stream interface
          stream << "#pragma CoreIR INTERFACE axis register "
                 << "port=" << arg_name << "\n";
        } else {
          // stencil arguments use AXI-lite interface
          stream << "#pragma CoreIR INTERFACE s_axilite "
                 << "port=" << arg_name
                 << " bundle=config\n";
          stream << "#pragma CoreIR ARRAY_PARTITION "
                 << "variable=" << arg_name << ".value complete dim=0\n";
        }
      } else {
        // scalar arguments use AXI-lite interface
        stream << "#pragma CoreIR INTERFACE s_axilite "
               << "port=" << arg_name << " bundle=config\n";
      }
    }
    stream << "\n";

    // create alias (references) of the arguments using the names in the IR
    do_indent();
    stream << "// alias the arguments\n";
    for (size_t i = 0; i < args.size(); i++) {
      string arg_name = "arg_" + std::to_string(i);
      do_indent();
      if (args[i].is_stencil) {
        Stencil_Type stype = args[i].stencil_type;
        stream << print_stencil_type(args[i].stencil_type) << " &"
               << print_name(args[i].name) << " = " << arg_name << ";\n";
        input_aliases[print_name(args[i].name)] = arg_name;

        // input arguments are only streams
        if (ends_with(args[i].name, ".stream")) {
          // add alias to coreir
          if (is_input(arg_name)) {
            hw_input_set[print_name(args[i].name)] = self->sel("in")->sel(arg_name);
            rename_wire(print_name(args[i].name), arg_name, Expr());
          }
          if (is_output(arg_name) ) {
            hw_output_set.insert(print_name(args[i].name));
          }

        } else {
          string taps_name = "taps" + print_name(args[i].name);
          bool has_arg = (tap_types.count(args[i].name) > 0);
          (void) has_arg;
          auto tap_type = tap_types[args[i].name];
          CoreIR::Wireable* taps_inst = def->addInstance(taps_name, gens["const_array"], {{"type",CoreIR::Const::make(context,tap_type)}});
          taps_inst->getMetaData()["tap"] = "This array of constants is expected to be changed as tap values.";

          add_wire(print_name(args[i].name), taps_inst->sel("out"));

        }


      } else {
        stream << print_type(args[i].scalar_type) << " &"
               << print_name(args[i].name) << " = " << arg_name << ";\n";

        // configurable taps are generated as constant registers
        string const_name = print_name(args[i].name);
        string tap_name = "tap" + const_name;
        int const_bitwidth = args[i].scalar_type.bits();
        int const_value = 0;
        CoreIR::Wireable* const_inst;

        if (const_bitwidth == 1) {
          CoreIR::Values mod_args = {{"value",CoreIR::Const::make(context,(bool)const_value)}};
          const_inst = def->addInstance(tap_name, gens["bitconst"], mod_args);
        } else {
          int bw = inst_bitwidth(const_bitwidth);
          CoreIR::Values gen_args = {{"width", CoreIR::Const::make(context,bw)}};
          CoreIR::Values mod_args = {{"value",CoreIR::Const::make(context,BitVector(bw,const_value))}};
          const_inst = def->addInstance(tap_name, gens["const"], gen_args, mod_args);
        }
        const_inst->getMetaData()["tap"] = "This constant is expected to be changed as a tap value.";
        add_wire(const_name, const_inst->sel("out"));

      }
    }

    stream << "\n// hw_input_set contains: ";
    for (auto x : hw_input_set) {
      stream << " " << x.first;
    }
    stream << "\n";
    stream << "\n// hw_output_set contains: ";
    for (const std::string& x : hw_output_set) {
      stream << " " << x;
    }
    stream << "\n";

    // print body
    print(stmt);

    close_scope("kernel hls_target" + print_name(name));
  }
  stream << "\n";

  for (size_t i = 0; i < args.size(); i++) {
    // Remove buffer arguments from allocation scope
    if (args[i].stencil_type.type == Stencil_Type::StencilContainerType::Stream) {
      allocations.pop(args[i].name);
      stencils.pop(args[i].name);
    }
  }
}


/////////////////////////////////////////////////////
// Functions to identify expressions and variables //
/////////////////////////////////////////////////////
int CodeGen_CoreIR_Target::CodeGen_CoreIR_C::id_const_value(const Expr e) {
  if (const IntImm* e_int = e.as<IntImm>()) {
    return e_int->value;

  } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    return e_uint->value;

  } else {
    //internal_error << "invalid constant expr\n";
    return -1;
  }
}

int get_const_bitwidth(const Expr e) {
  if (const IntImm* e_int = e.as<IntImm>()) {
    return e_int->type.bits();

  } else if (const UIntImm* e_uint = e.as<UIntImm>()) {
    return e_uint->type.bits();

  } else if (const FloatImm* e_float = e.as<FloatImm>()) {
    return e_float->type.bits();

  } else {
    internal_error << "invalid constant expr\n";
    return -1;
  }
}

bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::is_const(const Expr e) {
  if (e.as<IntImm>() || e.as<UIntImm>()) {
    return true;
  } else {
    return false;
  }
}

bool is_fconst(const Expr e) {
  if (e.as<FloatImm>()) {
    return true;
  } else {
    return false;
  }
}

bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::is_wire(string var_name) {
  bool wire_exists = hw_wire_set.count(var_name) > 0;
  return wire_exists;
}

bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::is_input(string var_name) {
	bool input_exists = hw_input_set.count(var_name) > 0;
  return input_exists;
}

bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::is_storage(string var_name) {
  bool storage_exists = hw_store_set.count(var_name) > 0;
  return storage_exists;
}

bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::is_defined(string var_name) {
  bool hardware_defined = hw_def_set.count(var_name) > 0;
  //for (auto ele : hw_def_set) {
  //  assert(ele.second);
  //}

  return hardware_defined;
}

bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::is_output(string var_name) {
  bool output_name_matches = hw_output_set.count(var_name) > 0;
  return output_name_matches;
}

std::string strip_stream(std::string input) {
  std::string output = input;
  if (ends_with(input, "_stencil_update_stream")) {
    output = input.substr(0, input.find("_stencil_update_stream"));
  } else if (ends_with(input, "_stencil_stream")) {
    output = input.substr(0, input.find("_stencil_stream"));
  }
  return output;
}

//////////////////////////////////////////////
// Functions to wire coreir things together //
//////////////////////////////////////////////
CoreIR::Wireable* index_wire(CoreIR::Wireable* in_wire, std::vector<uint> indices) {
  CoreIR::Wireable* current_wire = in_wire;
  for (int i=indices.size()-1; i >= 0; --i) {
    current_wire = current_wire->sel(indices[i]);
  }
  return current_wire;
}


// This function is used when the output is going to be connected to a useable wire
CoreIR::Wireable* CodeGen_CoreIR_Target::CodeGen_CoreIR_C::get_wire(string name, Expr e, std::vector<uint> indices) {
  //cout << "connect for " << name << "\n";
  if (is_const(e)) {
    // Create a constant.
    int const_value = id_const_value(e);
    string const_name = unique_name("const" + std::to_string(const_value) + "_" + name);
    CoreIR::Wireable* const_inst;

    uint const_bitwidth = get_const_bitwidth(e);
    if (const_bitwidth == 1) {
      const_inst = def->addInstance(const_name, gens["bitconst"], {{"value",CoreIR::Const::make(context,(bool)const_value)}});
    } else {
      int bw = inst_bitwidth(const_bitwidth);
      const_inst = def->addInstance(const_name, gens["const"], {{"width", CoreIR::Const::make(context,bw)}},
                                    {{"value",CoreIR::Const::make(context,BitVector(bw,const_value))}});
    }

    stream << "// created const: " << const_name << " with name " << name << "\n";
    return const_inst->sel("out");

  } else if (is_fconst(e)) {
    // Create a constant with floating point value
    float fconst_value = id_fconst_value(e);
    string const_name = unique_name("fconst" + std::to_string((int)fconst_value) + "_" + name);
    CoreIR::Wireable* const_inst;

    uint const_bitwidth = get_const_bitwidth(e);
    int bw = inst_bitwidth(const_bitwidth);
    const_inst = def->addInstance(const_name, gens["fconst"], {{"width", CoreIR::Const::make(context,bw)}},
                                  {{"value",CoreIR::Const::make(context,BitVector(bw,bfloat16_t(fconst_value).to_bits()))}});
    const_inst->getMetaData()["float_value"] = "Equivalent float = " + std::to_string(fconst_value);

    stream << "// created fconst: " << const_name << " with name " << name << " with float value " << fconst_value << "\n";
    return const_inst->sel("out");

  } else if (is_input(name)) {
    // Return an input wire
    CoreIR::Wireable* input_wire = hw_input_set[name];

    stream << "// " << name << " added as input " << name << endl;
    auto current_wire = index_wire(input_wire, indices);

    return current_wire;

  } else if (is_storage(name)) {
    // Return a reusable element
    auto pt_struct = hw_store_set[name];
    CoreIR::Wireable* current_wire;

    if (!pt_struct->was_written) {
      internal_assert(pt_struct->is_reg());
      current_wire = pt_struct->reg->sel("out");
    } else {
      current_wire = pt_struct->wire->sel("out");
    }

    current_wire = index_wire(current_wire, indices);
    pt_struct->was_read = true;

    return current_wire;

  } else if (is_wire(name)) {
    // Return an existing module wire.

    CoreIR::Wireable* wire = hw_wire_set[name];
    if (wire==NULL) {
      user_warning << "wire was not defined: " << name << "\n";
      stream       << "// wire was not defined: " << name << endl;
      return self->sel("in");
    }
    CoreIR::Wireable* current_wire = index_wire(wire, indices);

    return current_wire;

  } else if (is_defined(name)) {
    // hardware element defined, but not added yet
    std::shared_ptr<CoreIR_Inst_Args> inst_args = hw_def_set[name];
    assert(inst_args);
    stream << "// creating element called: " << name << endl;
    //cout << "named " << inst_args->gen <<endl;
    string inst_name = unique_name(inst_args->name);
    CoreIR::Wireable* inst = def->addInstance(inst_name, inst_args->gen, inst_args->args, inst_args->genargs);
    add_wire(name, inst->sel(inst_args->selname));

    if (inst_args->gen == gens["ram2"]) {
      add_wire(name + "_waddr", inst->sel("waddr"));
      add_wire(name + "_wdata", inst->sel("wdata"));
      add_wire(name + "_raddr", inst->sel("raddr"));

      //attach a read enable
      CoreIR::Wireable* rom_ren = def->addInstance(inst_name + "_ren", gens["bitconst"], {{"value", CoreIR::Const::make(context,true)}});
      def->connect(rom_ren->sel("out"), inst->sel("ren"));

      auto ram_wen = def->addInstance(name + "_wen", gens["bitconst"], {{"value",CoreIR::Const::make(context,false)}});
      def->connect(ram_wen->sel("out"), inst->sel("wen"));
    }

    auto ref_name = inst_args->ref_name;

    return inst->sel(inst_args->selname);

  } else {
    user_warning << "ERROR- invalid wire: " << name << "\n";
    stream << "// invalid wire: couldn't find " << name
           << "\n// hw_wire_set contains: ";
    for (auto x : hw_wire_set) {
      stream << " " << x.first;
    }
    stream << "\n";

    return self->sel("in");
  }
}

// This adds another usable wire, generally the output of a new module.
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::add_wire(string out_name, CoreIR::Wireable* in_wire, vector<uint> out_indices) {
  //cout << "add wire to " << out_name << "\n";
  if (is_storage(out_name)) {
    auto pt_struct = hw_store_set[out_name];

    if (!pt_struct->was_read && !pt_struct->was_written &&
        out_indices.empty() && !pt_struct->is_reg()) {
      // this passthrough hasn't been used, so remove it
      auto pt_wire = hw_store_set[out_name]->wire;
      internal_assert(CoreIR::Instance::classof(pt_wire));
      CoreIR::Instance* pt_inst = &(cast<CoreIR::Instance>(*pt_wire));
      def->removeInstance(pt_inst);
      hw_store_set.erase(out_name);

      hw_wire_set[out_name] = in_wire;

    } else {
      // use the found passthrough
      if (pt_struct->was_written && pt_struct->was_read) {
        // create a new passthrough, since this one is already connected
        string pt_name = "pt" + out_name + "_" + unique_name('p');
        CoreIR::Type* ptype = pt_struct->ptype;
        stream << "// created passthrough with name " << pt_name << endl;

        CoreIR::Wireable* pt = def->addInstance(pt_name, gens["passthrough"], {{"type",CoreIR::Const::make(context,ptype)}});
        // FIXME: copy over all stores (since wire might not encompass entire passthrough)
        pt_struct->wire = pt;

        pt_struct->was_read = false;
      }

      //cout << "// wiring to " << out_name << endl;
      // disconnect associated wire in reg, and connect new wire
      if (pt_struct->is_reg()) {
        stream << "// disconnecting wire for reg " << out_name << endl;
        CoreIR::Wireable* d_wire = pt_struct->reg->sel("in");
        for (int i=out_indices.size()-1; i >= 0; --i) {
          //cout << "selecting in reg add_wire: " << out_indices[i] << endl;
          d_wire = d_wire->sel(out_indices[i]);
        }
        //cout << "select in add_wire done" << endl;
        def->disconnect(d_wire);
        def->connect(in_wire, d_wire);
      }

      pt_struct->was_written = true;
      stream << "// added passthrough wire to " << out_name << endl;
      //cout << "// added passthrough wire to " << out_name << endl;

      // connect wire to passthrough
      CoreIR::Wireable* current_wire = pt_struct->wire->sel("in");
      for (int i=out_indices.size()-1; i >= 0; --i) {
        //cout << "selecting in add_wire: " << out_indices[i] << endl;
        current_wire = current_wire->sel(out_indices[i]);
      }
      def->connect(in_wire, current_wire);

    }

  } else {
    internal_assert(out_indices.empty());
    // wire is being stored for use as an input
    hw_wire_set[out_name] = in_wire;
  }
}

// This renames a wire with a new name without creating a new module.
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::rename_wire(string new_name, string in_name, Expr in_expr, vector<uint> indices) {
  //cout << "// rename wire for " << in_name << " to " << new_name << "\n";
  // if this is a definition, then just copy the pointer
  if (is_defined(in_name) && !is_wire(in_name)) {
    assert(indices.empty());
    // wire is only defined but not created yet

    hw_def_set[new_name] = hw_def_set[in_name];
    hw_def_set[new_name]->ref_name = in_name;

    assert(hw_def_set[new_name]);
    stream << "// added/modified in hw_def_set: " << new_name << " = " << in_name << "\n";
    stream << "//   for a module called: " << hw_def_set[new_name]->name << endl;
    return;

  } else if (is_const(in_expr)) {
    assert(indices.empty());
    // add hardware definition, but don't create it yet
    int const_value = id_const_value(in_expr);
    string const_name = "const" + std::to_string(const_value) + "_" + in_name;
    string gen_const;
    CoreIR::Values args, genargs;

    uint const_bitwidth = get_const_bitwidth(in_expr);
    if (const_bitwidth == 1) {
      gen_const = gens["bitconst"];
      args = {{"value",CoreIR::Const::make(context,const_value)}};
      genargs = CoreIR::Values();
    } else {
      int bw = inst_bitwidth(const_bitwidth);
      gen_const = gens["const"];
      args = {{"width", CoreIR::Const::make(context,bw)}};
      genargs = {{"value",CoreIR::Const::make(context,BitVector(bw,const_value))}};
    }

    CoreIR_Inst_Args const_args(const_name, in_name, "out", gen_const, args, genargs);
    hw_def_set[new_name] = std::make_shared<CoreIR_Inst_Args>(const_args);

    stream << "// defined const: " << const_name << " with name " << new_name << "\n";
    return;

  } else if (is_fconst(in_expr)) {
    assert(indices.empty());
    float fconst_value = id_fconst_value(in_expr);
    string const_name = unique_name("fconst" + std::to_string((int)fconst_value) + "_" + in_name);

    uint const_bitwidth = get_const_bitwidth(in_expr);
    int bw = inst_bitwidth(const_bitwidth);
    CoreIR::Values args = {{"width", CoreIR::Const::make(context,bw)}};
    CoreIR::Values genargs = {{"value",CoreIR::Const::make(context,BitVector(bw,bfloat16_t(fconst_value).to_bits()))}};

    CoreIR_Inst_Args const_args(const_name, in_name, "out", gens["fconst"], args, genargs);
    hw_def_set[new_name] = std::make_shared<CoreIR_Inst_Args>(const_args);

    stream << "// defined fconst: " << const_name << " with name " << new_name << "\n";
    return;

  } else if (is_storage(in_name) && indices.empty()) {
    // if this is simply another reference (no indexing), just add a new name
    if (is_storage(new_name)) {
      stream << "// removing another passthrough: " << new_name << " = " << in_name << endl;
      //def->connect(hw_store_set[in_name]->wire->sel("out"), hw_store_set[new_name]->wire->sel("in"));

      auto pt_wire = hw_store_set[new_name]->wire;
      internal_assert(CoreIR::Instance::classof(pt_wire));
      CoreIR::Instance* pt_inst = &(cast<CoreIR::Instance>(*pt_wire));
      def->removeInstance(pt_inst);
      hw_store_set[new_name] = hw_store_set[in_name];


      hw_store_set[in_name]->was_read = true;
    } else {
      stream << "// creating another passthrough reference: " << new_name << " = " << in_name << endl;
      hw_store_set[new_name] = hw_store_set[in_name];
    }

    if (is_output(new_name)) {
      if (hw_store_set[new_name]->is_reg()) {
        def->connect(hw_store_set[new_name]->reg->sel("out"), self->sel("out"));
      } else {
        def->connect(hw_store_set[new_name]->wire->sel("out"), self->sel("out"));
      }
      stream << "// connecting passthrough to output " << new_name << endl;
    }
    return;

  } else {
    // the wire is defined, so store the pointer with its new name
    CoreIR::Wireable* temp_wire = get_wire(in_name, in_expr, indices);
    CoreIR::Wireable* in_wire = temp_wire;

    if (in_wire!=NULL && is_output(new_name)) {
      internal_assert(indices.empty()) << "output had indices selected\n";
      stream << "// " << new_name << " added as an output from " << in_name << "\n";
      def->connect(in_wire, self->sel("out"));

    } else if (in_wire) {
      add_wire(new_name, in_wire);
      stream << "// added/modified in wire_set: " << new_name << " = " << in_name << "\n";

    } else {
      stream << "// " << in_name << " not found (prob going to fail)\n";
    }
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::record_dispatch(std::string producer_name, std::string consumer_name) {
  hw_dispatch_set[consumer_name].push_back(producer_name);
  stream << "// recording dispatch from " << producer_name
         << " to " << consumer_name << "\n";

  // connect output valid if that is the consumer, and design has valid
  if (has_valid) {
    for (auto output : hw_output_set) {
      if (strip_stream(output) == consumer_name) {
        stream << "// connecting " << producer_name << " to output valid\n";
        connect_linebuffer(consumer_name, self->sel("valid"));
        return;
      }
    }
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::record_linebuffer(std::string producer_name, CoreIR::Wireable* wire) {
  stream << "// added " << producer_name << " linebuffer to record map\n";
  lb_map[producer_name] = wire;
}


bool CodeGen_CoreIR_Target::CodeGen_CoreIR_C::connect_linebuffer(std::string consumer_name, CoreIR::Wireable* consumer_wen_wire) {
  // strip off suffix to consumer name
  std::string consumer = strip_stream(consumer_name);
  stream << "// Using lb consumer " << consumer_name
         << " (stripped " << consumer << ")\n";
  //cout << "stripped name: " << consumer << std::endl;
  std::string producer, producer_name;

  // trace back consumers looking for one that is a linebuffer
  std::string consumer_recurse = consumer;
  while (hw_dispatch_set.count(consumer_recurse) > 0) {
    consumer = consumer_recurse;
    auto producer_list = hw_dispatch_set[consumer];

    // FIXME: what about merges?
    //internal_assert(producer_list.size() == 1);

    // use the first producer (hopefully they all work equally)
    producer_name = producer_list[0];
    producer = strip_stream(producer_name);

    // connect to upstream linebuffer valid
    if (lb_map.count(producer_name) > 0 &&
        consumer_wen_wire->getConnectedWireables().size() == 0) { // ignore if wen already connected
      stream << "// connected lb valid: connecting " << producer_name << " valid to "
             << consumer_name << " wen\n";
      CoreIR::Wireable* linebuffer_wire = lb_map[producer_name];
      def->connect(linebuffer_wire->sel("valid"), consumer_wen_wire);
      return true;
    }

    stream << "// using producer " << producer << " for consumer " << consumer << "\n";
    consumer_recurse = producer;
  }

  if (consumer_wen_wire->getConnectedWireables().size() > 0) {
    // nothing to do if wen already connected
    return true;

  } else if (is_input(consumer_name)) {
    // connect to self upstream valid
    stream << "// connected to upstream valid (input enable) here for " << consumer_name << "\n";
    auto& aliased_input_name = input_aliases.at(consumer_name);
    def->connect(self->sel("in_en")->sel(aliased_input_name), consumer_wen_wire);
    return true;

    //stream << "// TODO: connect to upstream valid here\n";
    //return false;
  } else if (lb_map.count(producer_name) > 0) {
    // connect to upstream linebuffer valid
    stream << "// connecting " << producer_name << " valid to "
           << consumer_name << " wen\n";
    CoreIR::Wireable* linebuffer_wire = lb_map[producer_name];
    def->connect(linebuffer_wire->sel("valid"), consumer_wen_wire);
    return true;

  } else {
    return false;
  }

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_unaryop(Type t, Expr a, const char*  op_sym, string op_name) {
  string a_name = print_expr(a);
  string print_sym = op_sym;
  string out_var = print_assignment(t, print_sym + "(" + a_name + ")");

  // return if this variable is cached
  if (is_wire(out_var)) { return; }

  CoreIR::Wireable* a_wire = get_wire(a_name, a);
  if (a_wire != NULL) {
    string unaryop_name = op_name + a_name;
    CoreIR::Wireable* coreir_inst;

    // properly cast to generator or module
    internal_assert(gens.count(op_name) > 0) << op_name << " is not one of the names Halide recognizes\n";

    // check if it is a generator or module
    if (context->getNamespace("float")->hasGenerator(gens[op_name].substr(6))) {
      coreir_inst = def->addInstance(unaryop_name, gens[op_name],
                                     {{"exp_bits", CoreIR::Const::make(context,8)},
                                         {"frac_bits", CoreIR::Const::make(context,7)}});

    } else if (context->hasGenerator(gens[op_name])) {
      internal_assert(context->getGenerator(gens[op_name]));
      uint bw = inst_bitwidth(a.type().bits());
      coreir_inst = def->addInstance(unaryop_name, gens[op_name], {{"width", CoreIR::Const::make(context,bw)}});

    } else {
      internal_assert(context->getModule(gens[op_name]));
      coreir_inst = def->addInstance(unaryop_name, gens[op_name]);
    }

    def->connect(a_wire, coreir_inst->sel("in"));
    add_wire(out_var, coreir_inst->sel("out"));

  } else {
    // invalid operand
    out_var = "";
    print_assignment(t, print_sym + "(" + a_name + ")");
    if (a_wire == NULL) { stream << "// input 'a' was invalid!!" << endl; }
  }

  // print out operation
  if (is_input(a_name)) { stream << "// " << op_name <<"a: self.in "; } else { stream << "// " << op_name << "a: " << a_name << " "; }
  stream << "o: " << out_var << " with bitwidth:" << t.bits() << endl;
}


void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_binop(Type t, Expr a, Expr b, const char*  op_sym, string op_name) {
  string a_name = print_expr(a);
  string b_name = print_expr(b);
  string out_var = print_assignment(t, a_name + " " + op_sym + " " + b_name);

  // return if this variable is cached
  if (is_wire(out_var)) { return; }

  CoreIR::Wireable* a_wire = get_wire(a_name, a);
  CoreIR::Wireable* b_wire = get_wire(b_name, b);

  if (a_wire != NULL && b_wire != NULL) {
    internal_assert(a.type().bits() == b.type().bits()) << "function " << op_name << " with "
                                                        << a_name << "(" << a.type().bits() << "bits) and "
                                                        << b_name << "(" << b.type().bits() << "bits\n";
    uint bw = inst_bitwidth(a.type().bits());
    string binop_name = op_name + a_name + b_name + out_var;
    CoreIR::Wireable* coreir_inst;

    // properly cast to generator or module
    internal_assert(gens.count(op_name) > 0) << op_name << " is not one of the names Halide recognizes\n";
    if (context->getNamespace("float")->hasGenerator(gens[op_name].substr(6))) {
      coreir_inst = def->addInstance(binop_name, gens[op_name],
                                     {{"exp_bits", CoreIR::Const::make(context,8)},
                                         {"frac_bits", CoreIR::Const::make(context,7)}});
    } else if (context->hasGenerator(gens[op_name])) {
      coreir_inst = def->addInstance(binop_name, gens[op_name], {{"width", CoreIR::Const::make(context,bw)}});
    } else {
      coreir_inst = def->addInstance(binop_name, gens[op_name]);
    }

    def->connect(a_wire, coreir_inst->sel("in0"));
    def->connect(b_wire, coreir_inst->sel("in1"));
    add_wire(out_var, coreir_inst->sel("out"));

  } else {
    out_var = "";
    if (a_wire == NULL) { stream << "// input 'a' was invalid!!" << endl; }
    if (b_wire == NULL) { stream << "// input 'b' was invalid!!" << endl; }
  }

  if (is_input(a_name)) { stream << "// " << op_name <<"a: self.in "; } else { stream << "// " << op_name << "a: " << a_name << " "; }
  if (is_input(b_name)) { stream << "// " << op_name <<"b: self.in "; } else { stream << "// " << op_name << "b: " << b_name << " "; }
  stream << "o: " << out_var << " with obitwidth:" << t.bits() << endl;
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_ternop(Type t, Expr a, Expr b, Expr c, const char*  op_sym1, const char* op_sym2, string op_name) {
  string a_name = print_expr(a);
  string b_name = print_expr(b);
  string c_name = print_expr(c);

  string out_var = print_assignment(t, a_name + " " + op_sym1 + " " + b_name + " " + op_sym2 + " " + c_name);
  // return if this variable is cached
  if (is_wire(out_var)) { return; }

  CoreIR::Wireable* a_wire = get_wire(a_name, a);
  CoreIR::Wireable* b_wire = get_wire(b_name, b);
  CoreIR::Wireable* c_wire = get_wire(c_name, c);

  if (a_wire != NULL && b_wire != NULL && c_wire != NULL) {
    internal_assert(b.type().bits() == c.type().bits());
    uint inst_bw = inst_bitwidth(b.type().bits());
    string ternop_name = op_name + a_name + b_name + c_name;
    CoreIR::Wireable* coreir_inst;

    // properly cast to generator or module

    internal_assert(gens.count(op_name) > 0) << op_name << " is not one of the names Halide recognizes\n";
    if (context->getNamespace("float")->hasGenerator(gens[op_name].substr(6))) {
      coreir_inst = def->addInstance(ternop_name, gens[op_name],
                                     {{"exp_bits", CoreIR::Const::make(context,8)},
                                         {"frac_bits", CoreIR::Const::make(context,7)}});
    } else if (context->hasGenerator(gens[op_name])) {
      coreir_inst = def->addInstance(ternop_name, gens[op_name], {{"width", CoreIR::Const::make(context,inst_bw)}});
    } else {
      coreir_inst = def->addInstance(ternop_name, gens[op_name]);
    }

    // wiring names are different for each operator
    if (op_name.compare("bitmux")==0 || op_name.compare("mux")==0) {
      def->connect(a_wire, coreir_inst->sel("sel"));
      def->connect(b_wire, coreir_inst->sel("in1"));
      def->connect(c_wire, coreir_inst->sel("in0"));
    } else {
      def->connect(a_wire, coreir_inst->sel("in0"));
      def->connect(b_wire, coreir_inst->sel("in1"));
      def->connect(c_wire, coreir_inst->sel("in2"));
    }
    add_wire(out_var, coreir_inst->sel("out"));

  } else {
    out_var = "";

    if (a_wire == NULL) { stream << "// input 'a' was invalid!!" << endl; }
    if (b_wire == NULL) { stream << "// input 'b' was invalid!!" << endl; }
    if (c_wire == NULL) { stream << "// input 'c' was invalid!!" << endl; }
  }

  if (is_input(a_name)) { stream << "// " << op_name <<"a: self.in "; } else { stream << "// " << op_name << "a: " << a_name << " "; }
  if (is_input(b_name)) { stream << "// " << op_name <<"b: self.in "; } else { stream << "// " << op_name << "b: " << b_name << " "; }
  if (is_input(c_name)) { stream << "// " << op_name <<"c: self.in "; } else { stream << "// " << op_name << "c: " << c_name << " "; }
  stream << "o: " << out_var << endl;

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Mul *op) {
  internal_assert(op->a.type() == op->b.type());
  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f*", "fmul");
  } else {
    visit_binop(op->type, op->a, op->b, "*", "mul");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Add *op) {
  internal_assert(op->a.type() == op->b.type());
  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f+", "fadd");
  } else {
    visit_binop(op->type, op->a, op->b, "+", "add");
  }
  // check if we can instantiate a MAD instead
  /*
    if (const Mul* mul = op->a.as<Mul>()) {
    visit_ternop(op->type, mul->a, mul->b, op->b, "*", "+", "MAD");
    } else if (const Mul* mul = op->b.as<Mul>()) {
    visit_ternop(op->type, mul->a, mul->b, op->a, "*", "+", "MAD");
    } else {
    visit_binop(op->type, op->a, op->b, "+", "add");
    }
  */

}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Sub *op) {
  internal_assert(op->a.type() == op->b.type());
  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f-", "fsub");
  } else {
    visit_binop(op->type, op->a, op->b, "-", "sub");
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Div *op) {
  int shift_amt;
  if (is_const_power_of_two_integer(op->b, &shift_amt)) {
    uint param_bitwidth = op->a.type().bits();
    Expr shift_expr = UIntImm::make(UInt(param_bitwidth), shift_amt);
    if (op->a.type().is_uint()) {
      internal_assert(op->b.type().is_uint());
      visit_binop(op->type, op->a, shift_expr, ">>", "lshr");
    } else {
      internal_assert(!op->b.type().is_uint());
      visit_binop(op->type, op->a, shift_expr, ">>", "ashr");
    }
  } else {
    stream << "// divide is not fully supported" << endl;
    user_warning << "WARNING: divide is not fully supported!!!!\n";
    visit_binop(op->type, op->a, op->b, "/", "div");
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Mod *op) {
  int num_bits;
  if (is_const_power_of_two_integer(op->b, &num_bits)) {
    // equivalent to masking the bottom bits
    uint param_bitwidth = op->a.type().bits();
    uint mask = (1<<num_bits) - 1;
    Expr mask_expr = UIntImm::make(UInt(param_bitwidth), mask);
    visit_binop(op->type, op->a, mask_expr, "&", "and");

  } else if (op->type.is_int()) {
    stream << "// mod is not fully supported" << endl;
    //print_expr(lower_euclidean_mod(op->a, op->b));
  } else {
    stream << "// mod is not fully supported" << endl;
    //visit_binop(op->type, op->a, op->b, "%", "mod");
  }

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const And *op) {
  if (op->a.type().bits() == 1) {
    internal_assert(op->b.type().bits() == 1);
    visit_binop(op->type, op->a, op->b, "&&", "bitand");
  } else {
    visit_binop(op->type, op->a, op->b, "&&", "and");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Or *op) {
  if (op->a.type().bits() == 1) {
    internal_assert(op->b.type().bits() == 1);
    visit_binop(op->type, op->a, op->b, "||", "bitor");
  } else {
    visit_binop(op->type, op->a, op->b, "||", "or");
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const EQ *op) {
  internal_assert(op->a.type().bits() == op->b.type().bits());
  internal_assert(op->a.type() == op->b.type());
  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f==", "feq");
  } else {
    if (op->a.type().bits() == 1) {
      visit_binop(op->type, op->a, op->b, "~^", "bitxnor");
    } else {
      visit_binop(op->type, op->a, op->b, "==", "eq");
    }
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const NE *op) {
  internal_assert(op->a.type().bits() == op->b.type().bits());
  internal_assert(op->a.type() == op->b.type());
  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f!=", "fneq");
  } else {
    if (op->a.type().bits() == 1) {
      visit_binop(op->type, op->a, op->b, "^", "bitxor");
    } else {
      visit_binop(op->type, op->a, op->b, "!=", "neq");
    }
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const LT *op) {
  internal_assert(op->a.type() == op->b.type());

  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f<", "flt");

  } else if (op->a.type().is_uint()) {
    internal_assert(op->a.type().bits() == op->b.type().bits());
    if (op->a.type().bits() == 1) {
      Expr not_a = Not::make(op->a);
      visit_binop(op->type, not_a, op->b, "&&", "bitand");
      stream << "// created ~a * b for bitlt" << std::endl;
    } else {
      visit_binop(op->type, op->a, op->b, "<", "ult");
    }

  } else {
    internal_assert(!op->b.type().is_uint());
    internal_assert(op->a.type().bits() > 1);
    visit_binop(op->type, op->a, op->b, "s<", "slt");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const LE *op) {
  internal_assert(op->a.type() == op->b.type());

  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f<=", "fle");

  } else if (op->a.type().is_uint()) {
    internal_assert(op->a.type().bits() == op->b.type().bits());
    if (op->a.type().bits() == 1) {
      Expr not_a = Not::make(op->a);
      visit_binop(op->type, op->a, op->b, "||", "bitor");
      stream << "// created ~a + b for bitle" << std::endl;
    } else {
      visit_binop(op->type, op->a, op->b, "<=", "ule");
    }
  } else {
    internal_assert(!op->b.type().is_uint());
    internal_assert(op->a.type().bits() > 1);
    visit_binop(op->type, op->a, op->b, "s<=", "sle");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const GT *op) {
  internal_assert(op->a.type() == op->b.type());

  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f>", "fgt");

  } else if (op->a.type().is_uint()) {
    internal_assert(op->a.type().bits() == op->b.type().bits());
    if (op->a.type().bits() == 1) {
      Expr not_b = Not::make(op->b);
      visit_binop(op->type, op->a, not_b, "&&", "bitand");
      stream << "// created a * ~b for bitgt" << std::endl;
    } else {
      visit_binop(op->type, op->a, op->b, ">", "ugt");
    }
  } else {
    internal_assert(!op->b.type().is_uint());
    internal_assert(op->a.type().bits() > 1);
    visit_binop(op->type, op->a, op->b, "s>", "sgt");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const GE *op) {
  internal_assert(op->a.type() == op->b.type());

  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "f>=", "fge");

  } else if (op->a.type().is_uint()) {
    internal_assert(op->a.type().bits() == op->b.type().bits());
    if (op->a.type().bits() == 1) {
      Expr not_b = Not::make(op->b);
      visit_binop(op->type, op->a, not_b, "||", "bitor");
      stream << "// created a + ~b for bitge" << std::endl;
    } else {
      visit_binop(op->type, op->a, op->b, ">=", "uge");
    }
  } else {
    internal_assert(!op->b.type().is_uint());
    internal_assert(op->a.type().bits() > 1);
    visit_binop(op->type, op->a, op->b, "s>=", "sge");
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Max *op) {
  internal_assert(op->a.type() == op->b.type());

  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "<fmax>",  "fmax");
  } else if (op->type.is_uint()) {
    visit_binop(op->type, op->a, op->b, "<max>",  "umax");
  } else {
    visit_binop(op->type, op->a, op->b, "<smax>", "smax");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Min *op) {
  internal_assert(op->a.type() == op->b.type());

  if (op->a.type().is_float()) {
    visit_binop(op->type, op->a, op->b, "<fmin>",  "fmin");
  } else if (op->type.is_uint()) {
    visit_binop(op->type, op->a, op->b, "<min>",  "umin");
  } else {
    visit_binop(op->type, op->a, op->b, "<smin>", "smin");
  }
}
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Not *op) {
  // operator must have a one-bit/bool input
  assert(op->a.type().bits() == 1);
  visit_unaryop(op->type, op->a, "!", "bitnot");
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Select *op) {
  internal_assert(op->true_value.type() == op->false_value.type());

  if (op->true_value.type().is_float()) {
    visit_ternop(op->type, op->condition, op->true_value, op->false_value, "f?",":", "fmux");
  } else if (op->type.bits() == 1) {
    // use a special op for bitwidth 1
    visit_ternop(op->type, op->condition, op->true_value, op->false_value, "?",":", "bitmux");
  } else {
    visit_ternop(op->type, op->condition, op->true_value, op->false_value, "?",":", "mux");
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Cast *op) {
  stream << "[cast]";
  string in_var = print_expr(op->value);
  string out_var = print_assignment(op->type, "(" + print_type(op->type) + ")(" + in_var + ")");

  // casting from 1 to 16 bits
  if (op->type.bits() > 1 && op->value.type().bits() == 1) {
    stream << "// casting from 1 to 16 bits" << endl;
    Expr one_uint16 = UIntImm::make(UInt(16), 1);
    Expr zero_uint16 = UIntImm::make(UInt(16), 0);
    visit_ternop(op->type, op->value, one_uint16, zero_uint16, "?", ":", "mux");

  // casting from 16 to 1 bit
  } else if (op->type.bits() == 1 && op->value.type().bits() > 1) {
    stream << "// casting from 16 to 1 bit" << endl;
    Expr zero_uint16 = UIntImm::make(UInt(op->value.type().bits()), 0);
    visit_binop(op->type, op->value, zero_uint16, "!=", "neq");

  } else if (!is_const(in_var)) {
    // only add to list, don't duplicate constants
    rename_wire(out_var, in_var, op->value);
  }
}

// store linebuffer wires for counters
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const ProducerConsumer *op) {
  do_indent();
  if (op->is_producer) {
    stream << "// produce " << op->name << '\n';

    string target_var = strip_stream(print_name(op->name));
    stream << "//  using " << target_var << '\n';

    if (hw_dispatch_set.count(target_var) > 0) {
      // FIXME: only looking at the first linebuffer
      string lb_name = hw_dispatch_set[target_var][0];
      stream << "//    and lb " << lb_name << '\n';

      if (lb_map.count(lb_name) > 0) {
        stream << "// found the linebuffer\n";
        CoreIR::Wireable* lb_wire = lb_map.at(lb_name);

        vector<string> for_loop_names = contained_for_loop_names(op->body);
        for (string for_loop_name : for_loop_names) {
          lb_kernel_map[for_loop_name] = lb_wire;
          stream << "// adding linebuffer target for loop " << for_loop_name << "\n";
        }

      }
    }

    stream << "// emitting produce\n";
    print_stmt(op->body);

  } else { // this is a consumer
    stream << "// consume " << op->name << '\n';
    print_stmt(op->body);
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Provide *op) {
  if (ends_with(op->name, ".stencil") ||
      ends_with(op->name, ".stencil_update")) {
    // IR: buffered.stencil_update(1, 2, 3) =
    // C: buffered_stencil_update(1, 2, 3) =
    vector<string> args_indices(op->args.size());
    vector<uint> indices;
    for(size_t i = 0; i < op->args.size(); i++) {
      args_indices[i] = print_expr(op->args[i]);
      //internal_assert(is_const(op->args[i])) << "variable store used. FIXME: Demux not yet implemented\n";
      if (!is_const(op->args[i])) {
        Expr max_index = find_constant_bound(op->args[i], Direction::Upper);
        Expr min_index = find_constant_bound(op->args[i], Direction::Lower);
        user_warning << "variable store used for " << op->name << ". FIXME: Demux not yet implemented\n"
                     << op->args[i] << " ranges from " << min_index << " to " << max_index << "\n";

        indices.push_back(0);
      } else {
        indices.push_back(id_const_value(op->args[i]));
      }
    }

    internal_assert(op->values.size() == 1);
    string id_value = print_expr(op->values[0]);

    do_indent();

    stream << "[provide]" << " ";
    stream << print_name(op->name) << "(";
    string new_name = print_name(op->name);

    for(size_t i = 0; i < op->args.size(); i++) {
      stream << args_indices[i];
      if (i != op->args.size() - 1)
        {stream << ", "; }
    }
    stream << ") = " << id_value << ";\n";
    //cout << "doing a provide\n ";
    // FIXME: can we avoid clearing the entire cache? just ones where the end matches (for regs used multiple times)?
    cache.clear();

    // generate coreir: add to wire_set
    if (predicate) {
      // produce a register for accumulation
      stream << "// provide with a predicate\n";
      // FIXME: support value that is not constant
      //internal_assert(is_const(op->values[0])) << "FIXME: Only constant inits are suppported";
      int const_value;
      if (is_const(op->values[0])) {
        const_value = id_const_value(op->values[0]);
      } else {
        const_value = 0;
        stream << "// " << id_value << " is not a constant. Not yet supported for provides." << endl;
        cout << id_value << " is not a constant. Not yet supported for provides." << endl;
      }
      internal_assert(const_value == 0) << "FIXME: we only supported arrays init at 0\n";

      // hook up reset
      // FIXME: shouldn't print out again
      string cond_id = print_expr(predicate->condition);
      auto pt_struct = hw_store_set[new_name];
      if (is_storage(new_name) && !pt_struct->is_reg()) {
        // add reg_array
        CoreIR::Type* ptype = pt_struct->ptype;
        string regs_name = "regs" + new_name;
        stream << "// reg array created named " << new_name << "\n";
        //FIXME: hooking up clr or rst?
        CoreIR::Wireable* regs = def->addInstance(regs_name, gens["reg_array"], {{"type",CoreIR::Const::make(context,ptype)},
              {"has_clr", CoreIR::Const::make(context,true)}});
        pt_struct->reg = regs;

        // connect input
        string in_name = id_value;
        CoreIR::Wireable* wire = get_wire(in_name, op->values[0]);
        auto regs_wire = index_wire(regs->sel("in"), indices);
        def->connect(wire, regs_wire);
      }

      internal_assert(is_storage(new_name) && hw_store_set[new_name]->is_reg());
      auto reg_struct = hw_store_set[new_name];
      def->connect(get_wire(cond_id, predicate->condition), reg_struct->reg->sel("clr"));

      stream << "// reg rst added to: " << new_name << endl;
    } else {
      string in_name = id_value;
      CoreIR::Wireable* wire = get_wire(in_name, op->values[0]);
      //cout << "where " << new_name << " = " << in_name << " with " << indices.size() << " indices" << endl;
      add_wire(new_name, wire, indices);
    }

  } else {
    CodeGen_CoreIR_Base::visit(op);
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const For *op) {
  internal_assert(op->for_type == ForType::Serial)
    << "Can only emit serial for loops to CoreIR C\n";

  string id_min = print_expr(op->min);
  string id_extent = print_expr(op->extent);

  do_indent();
  stream << "for (int "
         << print_name(op->name)
         << " = " << id_min
         << "; "
         << print_name(op->name)
         << " < " << id_min
         << " + " << id_extent
         << "; "
         << print_name(op->name)
         << "++)\n";

  open_scope();

  // add a 'PIPELINE' pragma if it is an innermost loop
  if (!contain_for_loop(op->body)) {
    stream << "#pragma CoreIR PIPELINE II=1\n";
  }

  // pass down linebuffer
  if (lb_kernel_map.count(op->name) && contain_for_loop(op->body)) {
    string varname = name_for_loop(op->body);
    lb_kernel_map[varname] = lb_kernel_map[op->name];
    stream << "// added " << varname << " with a linebuffer\n";
  }

  // generate coreir: add counter module if variable used
  if (variable_used(op->body, op->name) || is_defined(print_name(op->name))) {
    string wirename = print_name(op->name);

    internal_assert(is_const(op->min));
    internal_assert(is_const(op->extent));
    int min_value = is_const(op->min) ? id_const_value(op->min) : 0;
    int max_value = min_value + id_const_value(op->extent) - 1;
    int inc_value = 1;
    string counter_name = unique_name("count_" + wirename);

    CoreIR::Values args = {{"width",CoreIR::Const::make(context,bitwidth)},
                           {"min",CoreIR::Const::make(context,min_value)},
                           {"max",CoreIR::Const::make(context,max_value)},
                           {"inc",CoreIR::Const::make(context,inc_value)}};

    if (!is_defined(wirename)) {
      CoreIR::Values genargs;
      CoreIR_Inst_Args const_args(wirename, wirename, "out", gens["counter"], args, genargs);
      hw_def_set[wirename] = std::make_shared<CoreIR_Inst_Args>(const_args);
    }

    stream << "// creating counter for " << wirename << " with max=" << max_value << "\n";

    CoreIR::Wireable* counter_inst = def->addInstance(counter_name, gens["counter"], args);
    add_wire(wirename, counter_inst->sel("out"));

    // connect reset wire
    if (has_valid) {
      // Hook reset to the module's reset.
      def->connect({"self", "reset"}, {counter_name, "reset"});
    } else {
      // This forces the reset to always be low.
      string reset_name = counter_name + "_reset";
      def->addInstance(reset_name, gens["bitconst"], {{"value",CoreIR::Const::make(context,false)}});
      def->connect({reset_name, "out"},{counter_name, "reset"});
    }

    // connect wen wire
    if (contain_for_loop(op->body)) {
      string varname = print_name(name_for_loop(op->body));
      hw_def_set[varname] = NULL;  // define this to ensure it's created
      op->body.accept(this);
      close_scope("for " + print_name(op->name));

      // connect inner for loop overflow to wen
      CoreIR::Select* inner_for_loop = static_cast<CoreIR::Select*>(get_wire(varname, Expr()));
      CoreIR::Wireable* inner_overflow = inner_for_loop->getParent()->sel("overflow");
      def->connect(inner_overflow, counter_inst->sel("en"));
      return;

    } else if (lb_kernel_map.count(op->name)) {
      stream << "// connected to lb valid" << "\n";
      def->connect(lb_kernel_map[op->name]->sel("valid"), counter_inst->sel("en"));
    } else {
      // connect wen wire
      string const_name = counter_name + "_wen";
      def->addInstance(const_name, gens["bitconst"], {{"value",CoreIR::Const::make(context,true)}});
      def->connect({const_name, "out"},{counter_name, "en"});
    }


  } else {
    stream << "// no counter created for " << print_name(op->name) << endl;
  }

  op->body.accept(this);
  close_scope("for " + print_name(op->name));

}

class RenameAllocation : public IRMutator {
  const string &orig_name;
  const string &new_name;

  using IRMutator::visit;

  Expr visit(const Load *op) override {
    if (op->name == orig_name ) {
      Expr index = mutate(op->index);
      return Load::make(op->type, new_name, index, op->image, op->param, op->predicate, op->alignment);
    } else {
      return IRMutator::visit(op);
    }
  }

  Stmt visit(const Store *op) override {
    if (op->name == orig_name ) {
      Expr value = mutate(op->value);
      Expr index = mutate(op->index);
      return Store::make(new_name, value, index, op->param, op->predicate, op->alignment);
    } else {
      return IRMutator::visit(op);
    }
  }

  Stmt visit(const Free *op) override {
    if (op->name == orig_name) {
      return Free::make(new_name);
    } else {
      return IRMutator::visit(op);
    }
  }

public:
  RenameAllocation(const string &o, const string &n)
    : orig_name(o), new_name(n) {}
};

// most code is copied from CodeGen_C::visit(const Allocate *)
// we want to check that the allocation size is constant, and
// add a 'CoreIR ARRAY_PARTITION' pragma to the allocation
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Allocate *op) {
  // We don't add scopes, as it messes up the dataflow directives in CoreIR compiler.
  // Instead, we rename the allocation to a unique name
  //open_scope();

  internal_assert(!op->new_expr.defined());
  internal_assert(!is_zero(op->condition));
  int32_t constant_size;
  constant_size = op->constant_allocation_size();
  if (constant_size > 0) {

  } else {
    for (size_t i = 0; i < op->extents.size(); i++) {
      //std::cout << "alloc_dim" << i << " " << op->extents[i] << "\n";
    }

    internal_error << "Size for allocation " << op->name
                   << " is not a constant.\n";
  }

  // rename allocation to avoid name conflict due to unrolling
  string alloc_name = print_name(op->name + unique_name('a'));
  /*
    if (allocations.contains(alloc_name)) {
    return;
    }
    stream << "// couldn't find allocation " << alloc_name << endl;*/
  Stmt new_body = RenameAllocation(op->name, alloc_name).mutate(op->body);

  Allocation alloc;
  alloc.type = op->type;
  allocations.push(alloc_name, alloc);

  do_indent();
  stream << print_type(op->type) << ' '
         << print_name(alloc_name)
         << "[" << constant_size << "]; [alloc]\n";

  auto alloc_type = identify_allocation(new_body, alloc_name);

  //std::cout << "found alloc#=" << alloc_type << "\n";

  // define a rom that can be created and used later
  // FIXME: better way to decide to use rom
  // FIXME: use an array of constants to load the rom
  if (alloc_type == AllocationType::ROM_ALLOCATION &&
      constant_size > 16) {
    CoreIR_Inst_Args rom_args;
    rom_args.ref_name = alloc_name;
    rom_args.name = "rom_" + alloc_name;
    rom_args.gen = gens["rom2"];
    rom_args.args = {{"width",CoreIR::Const::make(context,bitwidth)},
                     {"depth",CoreIR::Const::make(context,constant_size)}};

    // set initial values for rom
    nlohmann::json jdata = rom_init(new_body, alloc_name);
    //jdata["init"][0] = 0;
    CoreIR::Values modparams = {{"init", CoreIR::Const::make(context, jdata)}};
    rom_args.genargs = modparams;
    rom_args.selname = "rdata";

    hw_def_set[alloc_name] = std::make_shared<CoreIR_Inst_Args>(rom_args);
    stream << "// created a rom called " << rom_args.name << "\n";

  } else if (alloc_type == AllocationType::RMW_ALLOCATION) {
    CoreIR_Inst_Args rmw_args;
    rmw_args.ref_name = alloc_name;
    rmw_args.name = "rmw_" + alloc_name;
    rmw_args.gen = gens["rmw"];
    rmw_args.args = {{"width",CoreIR::Const::make(context,bitwidth)},
                     {"depth",CoreIR::Const::make(context,constant_size)}};

    // set initial values for rom
    nlohmann::json jdata;
    jdata["init"][0] = 0;
    CoreIR::Values modparams = {{"init", CoreIR::Const::make(context, jdata)}};
    rmw_args.genargs = modparams;
    rmw_args.selname = "rdata";

    hw_def_set[alloc_name] = std::make_shared<CoreIR_Inst_Args>(rmw_args);

    stream << "// created a rmw histogram called " << alloc_name << "\n";
    cout << "// created a rmw histogram called " << alloc_name << "\n";

  } else if (alloc_type == AllocationType::SRAM_ALLOCATION) {
    CoreIR_Inst_Args sram_args;
    sram_args.ref_name = alloc_name;
    sram_args.name = "sram_" + alloc_name;
    sram_args.gen = gens["ram2"];
    sram_args.args = {{"width",CoreIR::Const::make(context,bitwidth)},
                     {"depth",CoreIR::Const::make(context,constant_size)}};

    // set initial values for sram
    nlohmann::json jdata;
    jdata["init"][0] = 0;
    CoreIR::Values modparams = {{"init", CoreIR::Const::make(context, jdata)}};
    //sram_args.genargs = modparams;
    sram_args.selname = "rdata";

    hw_def_set[alloc_name] = std::make_shared<CoreIR_Inst_Args>(sram_args);

    stream << "// created an sram allocation called " << alloc_name << "\n";
    cout << "// created an sram allocation called " << alloc_name << "\n";

  }

  //  CoreIR::Type* type_input = context->Bit()->Arr(bitwidth)->Arr(constant_size);
  //  CoreIR::Wireable* wire_array = def->addInstance("array", gens["passthrough"], {{"type", CoreIR::Const::make(context,type_input)}});
  //  hw_wire_set[print_name(alloc_name)] = wire_array;

  // add a 'ARRAY_PARTITION" pragma
  //stream << "#pragma CoreIR ARRAY_PARTITION variable=" << print_name(op->name) << " complete dim=0\n\n";

  new_body.accept(this);

  // Should have been freed internally
  internal_assert(!allocations.contains(alloc_name))
    << "allocation " << alloc_name << " is not freed.\n";
  stream << "// ending this allocation" << endl;
  allocations.push(alloc_name, alloc);

  //close_scope("alloc " + print_name(op->name));

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Call *op) {
  // add bitand, bitor, bitxor, bitnot
  if (op->is_intrinsic(Call::bitwise_and)) {
    internal_assert(op->args.size() == 2);
    Expr a = op->args[0];
    Expr b = op->args[1];
    if (op->type.bits() == 1) {
      // use a special op for bitwidth 1
      visit_binop(op->type, a, b, "&&", "bitand");
    } else {
      visit_binop(op->type, a, b, "&", "and");
    }
  } else if (op->is_intrinsic(Call::bitwise_or)) {
    internal_assert(op->args.size() == 2);
    Expr a = op->args[0];
    Expr b = op->args[1];
    if (op->type.bits() == 1) {
      // use a special op for bitwidth 1
      visit_binop(op->type, a, b, "||", "bitor");
    } else {
      visit_binop(op->type, a, b, "|", "or");
    }
  } else if (op->is_intrinsic(Call::bitwise_xor)) {
    internal_assert(op->args.size() == 2);
    Expr a = op->args[0];
    Expr b = op->args[1];
    if (op->type.bits() == 1) {
      // use a special op for bitwidth 1
      visit_binop(op->type, a, b, "^", "bitxor");
    } else {
      visit_binop(op->type, a, b, "^", "xor");
    }
  } else if (op->is_intrinsic(Call::bitwise_not)) {
    internal_assert(op->args.size() == 1);
    Expr a = op->args[0];
    if (op->type.bits() == 1) {
      // use a special op for bitwidth 1
      visit_unaryop(op->type, a, "!", "bitnot");
    } else {
      visit_unaryop(op->type, a, "~", "not");
    }

  } else if (op->is_intrinsic(Call::shift_left)) {
    internal_assert(op->args.size() == 2);
    Expr a = op->args[0];
    Expr b = op->args[1];
    stream << "[shift left] ";
    visit_binop(op->type, a, b, "<<", "shl");
  } else if (op->is_intrinsic(Call::shift_right)) {
    internal_assert(op->args.size() == 2);
    Expr a = op->args[0];
    Expr b = op->args[1];
    stream << "[shift right] ";
    if (a.type().is_uint()) {
      internal_assert(b.type().is_uint());
      visit_binop(op->type, a, b, ">>", "lshr");
    } else {
      internal_assert(!b.type().is_uint());
      visit_binop(op->type, a, b, ">>", "ashr");
    }
  } else if (op->is_intrinsic(Call::abs)) {
    internal_assert(op->args.size() == 1);
    Expr a = op->args[0];
    stream << "[abs] ";
    visit_unaryop(op->type, a, "abs", "abs");
  } else if (op->is_intrinsic(Call::absd)) {
    internal_assert(op->args.size() == 2);
    Expr a = op->args[0];
    Expr b = op->args[1];
    stream << "[absd] ";
    visit_binop(op->type, a, b, "|-|", "absd");

  } else if (op->is_intrinsic(Call::reinterpret)) {
    string in_var = print_expr(op->args[0]);
    print_reinterpret(op->type, op->args[0]);

    stream << "// reinterpreting " << op->args[0] << " as " << in_var << endl;

    // generate coreir: expecting to find the expr is a constant
    rename_wire(in_var, in_var, op->args[0]);

// This intrisic was removed:
//  } else if (op->is_intrinsic(Call::address_of)) {
//    user_warning << "ignoring address_of\n";
//    stream       << "ignoring address_of" << endl;
//

  } else if (op->name == "linebuffer") {
    visit_linebuffer(op);

  } else if (op->name == "hwbuffer") {
    visit_hwbuffer(op);


  } else if (op->name == "write_stream") {
    visit_write_stream(op);

  } else if (op->name == "read_stream") {
    visit_read_stream(op);

  } else if (ends_with(op->name, ".stencil") ||
             ends_with(op->name, ".stencil_update")) {
    visit_stencil(op);


  } else if (op->name == "dispatch_stream") {
    visit_dispatch_stream(op);

  } else {
    stream << "couldn't find op named " << op->name << endl;
    cout << "couldn't find op named " << op->name << endl;
    CodeGen_CoreIR_Base::visit(op);
  }
}


void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_hwbuffer(const Call *op) {
  //IR: linebuffer(buffered.stencil_update.stream, buffered.stencil.stream, extent_0[, extent_1, ...])
  //C: linebuffer<extent_0[, extent_1, ...]>(buffered.stencil_update.stream, buffered.stencil.stream)
  std::cout << "found the hwbuffer call:\n" << Expr(op) << std::endl;

  internal_assert(op->args.size() >= 3);
  string a0 = print_expr(op->args[0]);
  string a1 = print_expr(op->args[1]);
  const Variable *stencil_var = op->args[1].as<Variable>();
  Stencil_Type stencil_type = stencils.get(stencil_var->name);
  const Variable *in_stencil_var = op->args[0].as<Variable>();
  Stencil_Type in_stencil_type = stencils.get(in_stencil_var->name);

  stream << in_stencil_var->name << "\n";

  do_indent();
  stream << "hwbuffer<";
//  for(size_t i = 2; i < op->args.size(); i++) {
//    stream << print_expr(op->args[i]);
//    if (i != op->args.size() -1)
//      stream << ", ";
//  }
  stream << ">(" << a0 << ", " << a1 << ");\n";
  id = "0"; // skip evaluation

  // generate coreir: wire up to linebuffer
  string ub_in_name = print_name(a0);
  string ub_out_name = print_name(a1);
  size_t num_dims = id_const_value(op->args[2]);
  string ub_name = "ub" + ub_in_name;

  stream << "// unified buffer " << ub_name << " created with ";

  // extract out parameter values
  int cur_idx = 3;

  CoreIR::Type* capacity_type = context->Bit();
  vector<size_t> logical_size(num_dims);
  nlohmann::json logical_json;
  stream << "logical=";
  for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
    logical_size[i] = id_const_value(op->args[cur_idx]);
    capacity_type = capacity_type->Arr(logical_size[i]);
    logical_json["capacity"][i] = logical_size[i];
    stream << logical_size[i] << " ";
  }

  // create a vector that helps linearize strides
  auto &capacity = logical_size;
  vector<int> flat_dim_strides(capacity.size());
  internal_assert(flat_dim_strides.size() == capacity.size());
  for (size_t i=0; i<capacity.size(); ++i) {
    if (i == 0) {
      flat_dim_strides.at(i) = 1;
    } else {
      flat_dim_strides.at(i) = flat_dim_strides.at(i-1) * capacity.at(i-1);
    }
  }

  nlohmann::json istream_json;
  vector<size_t> input_chunk(num_dims);
  for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
    input_chunk[i] = id_const_value(op->args[cur_idx]);
  }

  CoreIR::Type* input_ports_type = context->BitIn()->Arr(bitwidth);
  vector<size_t> input_block(num_dims);
  stream << " input=";
  for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
    input_block[i] = id_const_value(op->args[cur_idx]);
    input_ports_type = input_ports_type->Arr(input_block[i]);
    stream << input_block[i] << " ";
  }

  int num_consumers = id_const_value(op->args[cur_idx++]);
  nlohmann::json ostream_json;
  //internal_assert(num_consumers == 1);

  string output_name;
  vector<size_t> output_stencil(num_dims);
  CoreIR::Type* output_block_type;
  vector<size_t> output_block(num_dims);

  size_t num_stream_dims;
  CoreIR::Type* range_type;
  vector<size_t> access_ranges;
  CoreIR::Type* dim_ref_type;
  vector<size_t> access_dim_refs;
  CoreIR::Type* stride_type;
  vector<size_t> access_strides;

  vector<string> consumer_names;
  map<string, vector<size_t>> output_stencils;
  map<string, vector<size_t>> output_blocks;
  map<string, CoreIR::Type*>  output_block_types;

  map<string, CoreIR::Type*>  range_types;
  map<string, CoreIR::Type*>  dim_ref_types;
  map<string, CoreIR::Type*>  stride_types;
  map<string, vector<size_t>> access_ranges_map;
  map<string, vector<size_t>> access_dim_refs_map;
  map<string, vector<size_t>> access_strides_map;
  map<string, vector<size_t>> output_strides_map; // linearized version of access strides

  // Do every consumer stream
  //for (int ncons=0; ncons<num_consumers; ++ncons) {
  int ncons = 0;
  do {
    string output_name_i = op->args[cur_idx++].as<StringImm>()->value;

    //CoreIR::Type* output_stencil_type_i = context->Bit()->Arr(bitwidth);
    vector<size_t> output_stencil_i(num_dims);
    stream << " output stencil=";
    for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
      output_stencil_i[i] = id_const_value(op->args[cur_idx]);
      stream << output_stencil_i[i] << " ";
    }


    CoreIR::Type* output_block_type_i = context->Bit()->Arr(bitwidth);
    vector<size_t> output_block_i(num_dims);
    stream << " output=";
    for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
      output_block_i[i] = id_const_value(op->args[cur_idx]);
      output_block_type_i = output_block_type_i->Arr(output_block_i[i]);
      stream << output_block_i[i] << " ";
    }
    stream << "\n";

    size_t num_streaming_dims = id_const_value(op->args[cur_idx++]);

    CoreIR::Type* range_type_i = context->Bit();
    vector<size_t> access_ranges_i(num_streaming_dims);
    stream << "//   range=";
    for (size_t i = 0; i < num_streaming_dims; ++cur_idx, ++i) {
      access_ranges_i[i] = id_const_value(op->args[cur_idx]);
      range_type_i = range_type_i->Arr(access_ranges_i[i]);
      stream << access_ranges_i[i] << " ";
    }

    CoreIR::Type* dim_ref_type_i = context->Bit();
    vector<size_t> access_dim_refs_i(num_streaming_dims);
    stream << " dim_ref=";
    for (size_t i = 0; i < num_streaming_dims; ++cur_idx, ++i) {
      access_dim_refs_i[i] = id_const_value(op->args[cur_idx]);
      dim_ref_type_i = dim_ref_type_i->Arr(access_dim_refs_i[i]);
      stream << access_dim_refs_i[i] << " ";
    }

    CoreIR::Type* stride_type_i = context->Bit();
    vector<size_t> access_strides_i(num_streaming_dims);
    stream << " stride=";
    for (size_t i = 0; i < num_streaming_dims; ++cur_idx, ++i) {
      access_strides_i[i] = id_const_value(op->args[cur_idx]);
      stride_type_i = stride_type_i->Arr(access_strides_i[i]);
      stream << access_strides_i[i] << " ";
    }

    internal_assert(num_streaming_dims == access_strides_i.size());
    internal_assert(num_streaming_dims == access_dim_refs_i.size());
    internal_assert(num_streaming_dims == access_ranges_i.size());
    vector<size_t> output_strides_i(num_streaming_dims);
    stream << " fstride=";
    for (size_t i=0; i<num_streaming_dims; ++i) {
      output_strides_i.at(i) = access_strides_i.at(i) *
        flat_dim_strides.at(access_dim_refs_i.at(i));
      stream << output_strides_i[i] << " ";
    }
    stream << "\n";

    // store for the single stream version
    output_name = output_name_i;
    output_stencil = output_stencil_i;
    output_block_type = output_block_type_i;
    output_block = output_block_i;

    num_stream_dims = num_streaming_dims;
    range_type = range_type_i;
    access_ranges = access_ranges_i;
    dim_ref_type = dim_ref_type_i;
    access_dim_refs = access_dim_refs_i;
    stride_type = stride_type_i;
    access_strides = access_strides_i;

    // store for the multiple consumer stream version
    consumer_names.emplace_back(output_name_i);
    output_stencils[output_name_i] = output_stencil_i;
    output_block_types[output_name_i] = output_block_type_i;
    output_blocks[output_name_i] = output_block_i;

    range_types[output_name_i] = range_type_i;
    dim_ref_types[output_name_i] = dim_ref_type_i;
    stride_types[output_name_i] = stride_type_i;
    access_ranges_map[output_name_i] = access_ranges_i;
    access_dim_refs_map[output_name_i] = access_dim_refs_i;
    access_strides_map[output_name_i] = access_strides_i;
    output_strides_map[output_name_i] = output_strides_i;

    ncons += 1;
  }
  while (ncons < num_consumers);
  //for (int ncons=0; ncons<num_consumers; ++ncons) {

  int num_updates = id_const_value(op->args[cur_idx++]);
  stream << "num_updates=" << num_updates << std::endl;
  vector<size_t> modify_stencil(num_dims);
  vector<size_t> modify_block(num_dims);
  for (int i=0; i<0*num_updates; ++i) {
    stream << " modify stencil=";
    for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
      modify_stencil[i] = id_const_value(op->args[cur_idx]);
      stream << modify_stencil[i] << " ";
    }
    stream << std::endl;

    stream << " modify=";
    for (size_t i = 0; i < num_dims; ++cur_idx, ++i) {
      modify_block[i] = id_const_value(op->args[cur_idx]);
      stream << modify_block[i] << " ";
    }
    stream << "\n";
  }

  auto &input_ports = input_block;

  stream << "hwbuffer: " << a0 << std::endl
            << "  input_ports=" << input_ports << std::endl
            << "  output_ports=" << output_block << std::endl
            << "  capacity=" << capacity << std::endl
            << "  access: ranges=" << access_ranges << std::endl
            << "          dim_refs=" << access_dim_refs << std::endl
            << "          strides=" << access_strides << std::endl;

  CoreIR::Values aub_args = {{"input_ports", CoreIR::Const::make(context,input_ports_type)},
                            {"output_ports", CoreIR::Const::make(context,output_block_type)},
                            {"capacity", CoreIR::Const::make(context,capacity_type)},
                            {"range",CoreIR::Const::make(context,range_type)},
                            {"dim_ref",CoreIR::Const::make(context,dim_ref_type)},
                            {"stride",CoreIR::Const::make(context,stride_type)}};

  //CoreIR::Wireable* coreir_ub = def->addInstance(ub_name, gens["abstract_unified_buffer"], aub_args);
  //def->addInstance("abstract_" + ub_name, gens["abstract_unified_buffer"], aub_args);
  if (false) {
    (void) aub_args;
  }


  // calculate parameters for coreir hwbuffer generator
  int width = bitwidth;
  int depth = 1;
  for (const auto &logical_dim : logical_size) {
    depth *= logical_dim;
  }

  // always produce a double buffer, and later optimize this
  bool rate_matched = false;
  int stencil_width = 0;

  // product of ranges
  int iter_cnt = 1;
  for (const auto& range : access_ranges) {
    iter_cnt *= range;
  }
  int num_input_ports = 1;
  for (const auto &input_size : input_block) {
    num_input_ports *= input_size;
  }
  int num_output_ports = 1;
  for (const auto &output_size : output_block) {
    num_output_ports *= output_size;
  }

  size_t dimensionality = num_stream_dims;

  // set output range and stride based on access pattern
  vector<int> output_stride(std::max((int)num_stream_dims, 6));
  vector<int> output_range(std::max((int)num_stream_dims, 6));

  internal_assert(num_stream_dims <= access_strides.size());
  internal_assert(num_stream_dims <= access_dim_refs.size());
  internal_assert(num_stream_dims <= access_ranges.size());
  for (size_t i=0; i<num_stream_dims; ++i) {
    output_stride.at(i) = access_strides.at(i) *
      flat_dim_strides.at(access_dim_refs.at(i));
    output_range.at(i) = access_ranges.at(i);
  }
  stream << "// " << a0 << " flat dims=" << flat_dim_strides << std::endl
            << " output_range=" << output_range << std::endl
            << " output_stride=" << output_stride << std::endl;


  // set input range and stride
  vector<int> input_stride(std::max((int)input_chunk.size(), 6));
  vector<int> input_range(std::max((int)input_chunk.size(), 6));
  nlohmann::json input_chunk_json;
  internal_assert(input_chunk.size() == input_block.size());
  internal_assert(input_chunk.size() <= input_stride.size());
  for (size_t i = 0; i < input_chunk.size(); ++i) {
    input_chunk_json["input_chunk"][i] = input_chunk[i];
    input_stride[i] = input_block[i];
    internal_assert(input_chunk[i] % input_block[i] == 0);
    input_range[i] = input_chunk[i] / input_block[i];
  }

  // FIXME: This isn't always the case
  int num_reduction_iter = 1;
  nlohmann::json output_stencil_json;
  for (size_t i = 0; i < output_stencil.size(); ++i) {
    //internal_assert(output_stencil[i] % output_block[i] == 0)
    //  << a0 << " dim=" << i << " output stencil=" << output_stencil[i] << " + block=" << output_block[i];
    num_reduction_iter *= output_stencil[i] / output_block[i];
    output_stencil_json["output_stencil"][i] = output_stencil[i];
  }

  // input starting addresses for banking
  vector<int> input_starting_addrs(num_input_ports);
  vector<size_t> in_indexes(num_dims); // from inner to outer dim
  nlohmann::json input_starting_json;
  for (int port = 0; port < num_input_ports; ++port) {
    // calculate address
    int start_addr = 0;
    for (size_t idx = 0; idx < in_indexes.size(); ++idx) {
      int stride = idx==0 ? 1 : capacity.at(idx-1);
      //int stride = flat_dim_strides.at(idx);
      start_addr += in_indexes.at(idx) * stride;
    }

    // set input port as calculated starting address
    input_starting_addrs.at(port) = start_addr;
    input_starting_json["input_start"][port] = input_starting_addrs.at(port);

    // increment index
    in_indexes.at(0) += 1;
    for (size_t dim = 0; dim < num_dims; ++dim) {
      if (in_indexes.at(dim) >= input_block.at(dim)) {
        in_indexes.at(dim) = 0;
        if (dim + 1 < num_dims) {
          in_indexes.at(dim+1) += 1;
        }
      }
    }
  }

  // output starting adresses for banking
  vector<int> output_starting_addrs(num_output_ports);
  vector<size_t> out_indexes(num_dims); // from inner to outer dim
  nlohmann::json output_starting_json;
  for (int port = 0; port < num_output_ports; ++port) {
    // calculate address
    int start_addr = 0;
    for (size_t idx = 0; idx < out_indexes.size(); ++idx) {
      //int stride = idx==0 ? 1 : logical_size.at(idx-1);
      int stride = flat_dim_strides.at(idx);
      start_addr += out_indexes.at(idx) * stride;
    }

    // set output port as calculated starting address
    output_starting_addrs.at(port) = start_addr;
    output_starting_json["output_start"][port] = output_starting_addrs.at(port);

    // increment index
    out_indexes.at(0) += 1;
    for (size_t dim = 0; dim < num_dims; ++dim) {
      if (out_indexes.at(dim) >= output_block.at(dim)) {
        out_indexes.at(dim) = 0;
        if (dim + 1 < num_dims) {
          out_indexes.at(dim+1) += 1;
        }
      }
    }
  }

  //FIXME??
  nlohmann::json init;

  bool chain_en = false;   // always create an unchained memory
  int chain_idx = 0;


  stream << "hwbuffer: " << a0 << std::endl
            << "  width=" << width << "  depth=" << depth << "  iter_cnt=" << iter_cnt << std::endl
            << "  num_dims=" << dimensionality << "  rate_matched=" << rate_matched << "  stencil_width=" << stencil_width << std::endl
            << "  num_ports:  in=" << num_input_ports << " out=" << num_output_ports << std::endl
            << "  output:  stride0=" << output_stride.at(0) << " range0=" << output_range.at(0) << std::endl
            << "           stride1=" << output_stride.at(1) << " range1=" << output_range.at(1) << std::endl
            << "  output_start_addr=" << output_starting_addrs << std::endl
            << "  chain_en=" << chain_en << "  chain_idx=" << chain_idx << std::endl;


  CoreIR::Values ub_args = {{"width",                CoreIR::Const::make(context, width)},
                            {"depth",                CoreIR::Const::make(context, depth)},
                            {"rate_matched",         CoreIR::Const::make(context, rate_matched)},
                            {"stencil_width",        CoreIR::Const::make(context, stencil_width)},
                            {"iter_cnt",             CoreIR::Const::make(context, iter_cnt)},
                            {"num_input_ports",      CoreIR::Const::make(context, num_input_ports)},
                            {"num_output_ports",     CoreIR::Const::make(context, num_output_ports)},
                            {"dimensionality",       CoreIR::Const::make(context, dimensionality)},
                            {"stride_0",             CoreIR::Const::make(context, output_stride.at(0))},
                            {"range_0",              CoreIR::Const::make(context, output_range.at(0))},
                            {"stride_1",             CoreIR::Const::make(context, output_stride.at(1))},
                            {"range_1",              CoreIR::Const::make(context, output_range.at(1))},
                            {"stride_2",             CoreIR::Const::make(context, output_stride.at(2))},
                            {"range_2",              CoreIR::Const::make(context, output_range.at(2))},
                            {"stride_3",             CoreIR::Const::make(context, output_stride.at(3))},
                            {"range_3",              CoreIR::Const::make(context, output_range.at(3))},
                            {"stride_4",             CoreIR::Const::make(context, output_stride.at(4))},
                            {"range_4",              CoreIR::Const::make(context, output_range.at(4))},
                            {"stride_5",             CoreIR::Const::make(context, output_stride.at(5))},
                            {"range_5",              CoreIR::Const::make(context, output_range.at(5))},
                            {"input_stride_0",       CoreIR::Const::make(context, 1)},
                            {"input_range_0",        CoreIR::Const::make(context, 64)},
                            {"input_stride_1",       CoreIR::Const::make(context, 64)},
                            {"input_range_1",        CoreIR::Const::make(context, 64)},
                            //{"input_stride_0",       CoreIR::Const::make(context, input_stride.at(0))},
                            //{"input_range_0",        CoreIR::Const::make(context, input_range.at(0))},
                            //{"input_stride_1",       CoreIR::Const::make(context, input_stride.at(1))},
                            //{"input_range_1",        CoreIR::Const::make(context, input_range.at(1))},
                            {"input_stride_2",       CoreIR::Const::make(context, input_stride.at(2))},
                            {"input_range_2",        CoreIR::Const::make(context, input_range.at(2))},
                            {"input_stride_3",       CoreIR::Const::make(context, input_stride.at(3))},
                            {"input_range_3",        CoreIR::Const::make(context, input_range.at(3))},
                            {"input_stride_4",       CoreIR::Const::make(context, input_stride.at(4))},
                            {"input_range_4",        CoreIR::Const::make(context, input_range.at(4))},
                            {"input_stride_5",       CoreIR::Const::make(context, input_stride.at(5))},
                            {"input_range_5",        CoreIR::Const::make(context, input_range.at(5))},
                            {"chain_en",             CoreIR::Const::make(context, chain_en)},
                            {"chain_idx",            CoreIR::Const::make(context, chain_idx)},
                            {"input_starting_addrs", CoreIR::Const::make(context, input_starting_json)},
                            {"output_starting_addrs",CoreIR::Const::make(context, output_starting_json)},
                            {"logical_size",         CoreIR::Const::make(context, logical_json)},
                            {"init",                 CoreIR::Const::make(context, init)},
                            {"num_reduction_iter",   CoreIR::Const::make(context, num_reduction_iter)},
                            {"input_chunk",          CoreIR::Const::make(context, input_chunk_json)},
                            {"output_stencil",       CoreIR::Const::make(context, output_stencil_json)},
                            {"num_stencil_acc_dim",  CoreIR::Const::make(context, 0)}, // FIXME
  };

  cout << "making the ubuffer" << endl;

  //CoreIR::Wireable* old_coreir_ub = def->addInstance("old_" + ub_name, gens["unified_buffer"], ub_args);
  //(void) old_coreir_ub;

  //string new_ub_name = "new_" + ub_name;
  string new_ub_name = ub_name;
  //nlohmann::json istream_json;
  istream_json["input"]["input_stride"] = input_stride;
  istream_json["input"]["input_range"] = input_range;
  istream_json["input"]["input_starting_addrs"] = input_starting_addrs;
  istream_json["input"]["input_chunk"] = input_chunk;
  istream_json["input"]["input_block"] = input_block;
  //istream_json["input"]["num_input_ports"] = num_input_ports; // unneeded, just use the number of starting addresses

  //nlohmann::json ostream_json;
  //ostream_json[output_name]["output_stride"] = access_strides;
  //ostream_json[output_name]["output_range"] = access_ranges;
  //ostream_json[output_name]["output_starting_addrs"] = output_starting_addrs;
  //ostream_json[output_name]["output_stencil"] = output_stencil;
  //ostream_json[output_name]["output_block"] = output_block;
  //ostream_json[output_name]["num_stencil_acc_dim"] = 0;
  //ostream_json[output_name]["stencil_width"] = {1};                   // default: used only after hw mapping
  //ostream_json[output_name]["iter_cnt"] = 1;                          // remove: this is the product of all ranges
  //ostream_json[output_name]["num_loops"] = dimensionality;            // remove: aka dimensionality, this can be inferred perhaps from the length of each ostream?
  //ostream_json[output_name]["num_output_ports"] = num_output_ports;   // remove: this is the product of output block dims

  cout << "now doing the new ubuffer creation" << endl;
  for (auto consumer_name : consumer_names) {
    ostream_json[consumer_name]["output_stride"] = output_strides_map[consumer_name];
    ostream_json[consumer_name]["output_range"] = access_ranges_map[consumer_name];
    ostream_json[consumer_name]["output_starting_addrs"] = output_starting_addrs; //**
    ostream_json[consumer_name]["output_stencil"] = output_stencils[consumer_name];
    ostream_json[consumer_name]["output_block"] = output_blocks[consumer_name];
    ostream_json[consumer_name]["num_stencil_acc_dim"] = 0;
    ostream_json[consumer_name]["stencil_width"] = vector<size_t>(access_ranges_map[consumer_name].size(), 0);// default: used only after hw mapping
  }
  stream << ostream_json << std::endl;

  CoreIR::Values ub_arg2 = {{"width",                CoreIR::Const::make(context, width)},
                            {"logical_size",         CoreIR::Const::make(context, logical_json)},
                            {"ostreams",             CoreIR::Const::make(context, ostream_json)},
                            {"istreams",             CoreIR::Const::make(context, istream_json)},
                            {"chain_en",             CoreIR::Const::make(context, chain_en)},
                            {"chain_idx",            CoreIR::Const::make(context, chain_idx)},
                            {"init",                 CoreIR::Const::make(context, init)}
  };

  CoreIR::Wireable* coreir_ub = def->addInstance(new_ub_name, gens["new_unified_buffer"], ub_arg2);

  CoreIR::Values input_reshape_args =
    {{"input_type", CoreIR::Const::make(context, input_ports_type)},
     {"output_type", CoreIR::Const::make(context, context->Flip(context->BitIn()->Arr(bitwidth)->Arr(num_input_ports)))}};
  CoreIR::Wireable* input_reshape = def->addInstance(ub_name + "_in_reshape",
                                                     gens["reshape"],
                                                     input_reshape_args);

  CoreIR::Values output_reshape_args =
    {{"input_type", CoreIR::Const::make(context, context->Flip(context->Bit()->Arr(bitwidth)->Arr(num_output_ports)))},
     {"output_type", CoreIR::Const::make(context, output_block_type)}};
  CoreIR::Wireable* output_reshape = def->addInstance(ub_name + "_out_reshape",
                                                      gens["reshape"],
                                                      output_reshape_args);

  // go through the reshapes for the consumers
  map<string, CoreIR::Wireable*> output_reshapes;
  map<string, int> map_num_oports;
  for (auto consumer_name : consumer_names) {
    auto output_block = output_blocks.at(consumer_name);
    int num_output_ports = 1;
    for (const auto &output_size : output_block) {
      num_output_ports *= output_size;
    }
    map_num_oports[consumer_name] = num_output_ports;
    auto output_block_type = output_block_types.at(consumer_name);

    CoreIR::Values output_reshape_args =
      {{"input_type", CoreIR::Const::make(context, context->Flip(context->Bit()->Arr(bitwidth)->Arr(num_output_ports)))},
       {"output_type", CoreIR::Const::make(context, output_block_type)}};
    CoreIR::Wireable* output_reshape = def->addInstance(ub_name + "_for_" + consumer_name + "_out_reshape",
                                                        gens["reshape"],
                                                        output_reshape_args);
    output_reshapes[consumer_name] = output_reshape;
  }

  bool simulation_compatible = true;
  bool new_ubuffer = true;
  if (new_ubuffer) {
    auto coreir_ub_inputs = get_wires(coreir_ub, {(size_t)num_input_ports}, "datain_input_");
    auto coreir_ub_outputs = get_wires(coreir_ub, {(size_t)num_output_ports}, "dataout_"+output_name+"_");
    auto coreir_reshape_for_inputs = get_wires(input_reshape->sel("out"), {(size_t)num_input_ports});
    auto coreir_reshape_for_outputs = get_wires(output_reshape->sel("in"), {(size_t)num_output_ports});

    connect_wires(def, coreir_ub_inputs, coreir_reshape_for_inputs);
    connect_wires(def, coreir_ub_outputs, coreir_reshape_for_outputs);

    for (auto consumer_name : consumer_names) {
      auto num_output_ports = map_num_oports.at(consumer_name);
      auto cons_ub_outputs = get_wires(coreir_ub, {(size_t)num_output_ports}, "dataout_"+consumer_name+"_");
      auto output_reshape = output_reshapes.at(consumer_name);
      auto coreir_reshape_for_outputs = get_wires(output_reshape->sel("in"), {(size_t)num_output_ports});
      connect_wires(def, cons_ub_outputs, coreir_reshape_for_outputs);
      add_wire(ub_out_name + "." + consumer_name, output_reshape->sel("out"));
    }

  } else if (simulation_compatible) {
    auto coreir_ub_inputs = get_wires(coreir_ub, {(size_t)num_input_ports}, "datain");
    auto coreir_ub_outputs = get_wires(coreir_ub, {(size_t)num_output_ports}, "dataout");
    auto coreir_reshape_for_inputs = get_wires(input_reshape->sel("out"), {(size_t)num_input_ports});
    auto coreir_reshape_for_outputs = get_wires(output_reshape->sel("in"), {(size_t)num_output_ports});
    connect_wires(def, coreir_ub_inputs, coreir_reshape_for_inputs);
    connect_wires(def, coreir_ub_outputs, coreir_reshape_for_outputs);

  } else {
    def->connect(input_reshape->sel("out"), coreir_ub->sel("datain"));
    def->connect(coreir_ub->sel("dataout"), output_reshape->sel("in"));

  }

  stream << "created that unified buffer! " << ub_name << std::endl;

  // add linebuffer to coreir
  //uint num_dims = op->args.size() - 2;


  bool connected_wen = false;
  if (has_valid) {
    if (coreir_ub == NULL) {
      internal_assert(false) << "NULL LINEBUFFER before recording\n";
    }
    record_linebuffer(ub_out_name, coreir_ub);
    connected_wen = connect_linebuffer(ub_in_name, coreir_ub->sel("wen"));
    def->connect({"self", "reset"}, {ub_name, "reset"});

  } else {
    def->addInstance(ub_name + "_reset", gens["bitconst"], {{"value", CoreIR::Const::make(context,false)}});
    def->connect({ub_name + "_reset", "out"}, {ub_name, "reset"});
  }

  def->addInstance(ub_name + "_flush", gens["bitconst"], {{"value", CoreIR::Const::make(context,false)}});
  def->connect({ub_name + "_flush", "out"}, {ub_name, "flush"});

  CoreIR::Wireable* ub_in_wire = get_wire(ub_in_name, op->args[0]);

  def->connect(ub_in_wire, input_reshape->sel("in"));
  add_wire(ub_out_name, output_reshape->sel("out"));

  if (!connected_wen) {
    CoreIR::Wireable* ub_wen = def->addInstance(ub_name+"_wen", gens["bitconst"], {{"value",CoreIR::Const::make(context,true)}});
    def->connect(ub_wen->sel("out"), coreir_ub->sel("wen"));
  }

  CoreIR::Wireable* ub_ren = def->addInstance(ub_name+"_ren", gens["bitconst"], {{"value",CoreIR::Const::make(context,true)}});
  def->connect(ub_ren->sel("out"), coreir_ub->sel("ren"));

}


void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_linebuffer(const Call *op) {
  //IR: linebuffer(buffered.stencil_update.stream, buffered.stencil.stream, extent_0[, extent_1, ...])
  //C: linebuffer<extent_0[, extent_1, ...]>(buffered.stencil_update.stream, buffered.stencil.stream)
  internal_assert(op->args.size() >= 3);
  string a0 = print_expr(op->args[0]);
  string a1 = print_expr(op->args[1]);
  const Variable *stencil_var = op->args[1].as<Variable>();
  Stencil_Type stencil_type = stencils.get(stencil_var->name);
  const Variable *in_stencil_var = op->args[0].as<Variable>();
  Stencil_Type in_stencil_type = stencils.get(in_stencil_var->name);

  stream << in_stencil_var->name << "\n";

  do_indent();
  stream << "linebuffer<";
  for(size_t i = 2; i < op->args.size(); i++) {
    stream << print_expr(op->args[i]);
    if (i != op->args.size() -1)
      stream << ", ";
  }
  stream << ">(" << a0 << ", " << a1 << ");\n";
  id = "0"; // skip evaluation

  // generate coreir: wire up to linebuffer
  string lb_in_name = print_name(a0);
  string lb_out_name = print_name(a1);

  // add linebuffer to coreir
  uint num_dims = op->args.size() - 2;
  string lb_name = "lb" + lb_in_name;

  bool connected_wen = false;
  // FIXME: use proper bitwidth
  CoreIR::Type* input_type = context->BitIn()->Arr(bitwidth);
  CoreIR::Type* output_type = context->Bit()->Arr(bitwidth);
  CoreIR::Type* image_type = context->Bit()->Arr(bitwidth);

  stream << "// linebuffer " << lb_name << " created with ";

  stream << "input=";
  uint input_dims [num_dims];
  for (uint i=0; i<num_dims; ++i) {
    input_dims[i] = id_const_value(in_stencil_type.bounds[i].extent);
    input_type = input_type->Arr(input_dims[i]);
    stream << input_dims[i] << " ";
  }

  stream << " output=";
  uint output_dims [num_dims];
  for (uint i=0; i<num_dims; ++i) {
    //for (int i=num_dims-1; i>=0; --i) {
    output_dims[i] = id_const_value(stencil_type.bounds[i].extent);
    output_type = output_type->Arr(output_dims[i]);
    stream << output_dims[i] << " ";
  }

  stream << " image=";
  uint image_dims [num_dims];
  for (uint i=0; i<num_dims; ++i) {
    image_dims[i] = id_const_value(id_const_value(op->args[i+2]));
    image_type = image_type->Arr(image_dims[i]);
    stream << image_dims[i] << " ";
  }
  stream << "\n";

  CoreIR::Values lb_args = {{"input_type", CoreIR::Const::make(context,input_type)},
                            {"output_type", CoreIR::Const::make(context,output_type)},
                            {"image_type", CoreIR::Const::make(context,image_type)},
                            {"has_valid",CoreIR::Const::make(context,has_valid)}};

  CoreIR::Wireable* coreir_lb = def->addInstance(lb_name, gens["linebuffer"], lb_args);
  if (has_valid) {
    if (coreir_lb == NULL) {
      internal_assert(false) << "NULL LINEBUFFER before recording\n";
    }
    record_linebuffer(lb_out_name, coreir_lb);
    connected_wen = connect_linebuffer(lb_in_name, coreir_lb->sel("wen"));

    def->connect({"self", "reset"}, {lb_name, "reset"});
  } else {
    def->addInstance(lb_name + "_reset", gens["bitconst"], {{"value", CoreIR::Const::make(context,false)}});
    def->connect({lb_name + "_reset", "out"}, {lb_name, "reset"});
  }

  // connect linebuffer
  //CoreIR::Module* bc_gen = static_cast<CoreIR::Module*>(gens["bitconst"]);
  CoreIR::Wireable* lb_in_wire = get_wire(lb_in_name, op->args[0]);

  def->connect(lb_in_wire, coreir_lb->sel("in"));
  add_wire(lb_out_name, coreir_lb->sel("out"));
  if (!connected_wen) {
    CoreIR::Wireable* lb_wen = def->addInstance(lb_name+"_wen", gens["bitconst"], {{"value",CoreIR::Const::make(context,true)}});
    def->connect(lb_wen->sel("out"), coreir_lb->sel("wen"));
  }
  def->addInstance(lb_name + "_flush", gens["bitconst"], {{"value", CoreIR::Const::make(context,false)}});
  def->connect({lb_name + "_flush", "out"}, {lb_name, "flush"});


  //hw_wire_set[lb_out_name] = coreir_lb->sel("out");

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_write_stream(const Call *op) {
  string printed_stream_name;
  string input_name;
  if (op->args.size() == 2) {
    // normal case
    // IR: write_stream(buffered.stencil_update.stream, buffered.stencil_update)
    // C: buffered_stencil_update_stream.write(buffered_stencil_update);
    string a0 = print_expr(op->args[0]);
    string a1 = print_expr(op->args[1]);
    do_indent();

    stream << a0 << ".write(" << a1 << ");\n";
    id = "0"; // skip evaluation
    printed_stream_name = a0;
    input_name = a1;
  } else {
    // write stream call for the dag output kernel
    // IR: write_stream(output.stencil.stream, output.stencil, loop_var_1, loop_max_1, ...)
    // C:  AxiPackedStencil<uint8_t, 1, 1, 1> _output_stencil_packed = _output_stencil;
    //     if (_loop_var_1 == loop_max_1 && ...)
    //       _output_stencil_packed.last = 1;
    //     else
    //       _output_stencil_packed.last = 0;
    //     _output_stencil_stream.write(_output_stencil_packed);
    internal_assert(op->args.size() > 2 && op->args.size() % 2 == 0);
    const Variable *stream_var = op->args[0].as<Variable>();
    const Variable *stencil_var = op->args[1].as<Variable>();
    internal_assert(stream_var && stencil_var);
    string stream_name = stream_var->name;
    string stencil_name = stencil_var->name;
    string packed_stencil_name = stencil_name + "_packed";

    internal_assert(stencils.contains(stencil_name));
    Stencil_Type stencil_type = stencils.get(stencil_name);
    internal_assert(stencil_type.type == Stencil_Type::StencilContainerType::Stencil);

    // emit code declaring the packed stencil
    do_indent();
    stream << "AxiPacked" << print_stencil_type(stencil_type) << " "
           << print_name(packed_stencil_name) << " = "
           << print_name(stencil_name) << ";\n";

    // emit code asserting TLAST
    vector<string> loop_vars, loop_maxes;
    for (size_t i = 2; i < op->args.size(); i += 2) {
      loop_vars.push_back(print_expr(op->args[i]));
      loop_maxes.push_back(print_expr(op->args[i+1]));
    }
    do_indent();
    stream << "if (";
    for (size_t i = 0; i < loop_vars.size(); i++) {
      stream << loop_vars[i] << " == " << loop_maxes[i];
      if (i < loop_vars.size() - 1)
        stream << " && ";
    }
    stream << ") {\n";
    do_indent();
    stream << ' ' << print_name(packed_stencil_name) << ".last = 1;\n";
    do_indent();
    stream << "} else {\n";
    do_indent();
    stream << ' ' << print_name(packed_stencil_name) << ".last = 0;\n";
    do_indent();
    stream << "}\n";

    // emit code writing stream
    do_indent();
    stream << print_name(stream_name) << ".write("
           << print_name(packed_stencil_name) << ");\n";
    id = "0"; // skip evaluation
    printed_stream_name = print_name(stream_name);
    input_name = print_name(stencil_name);
  }

  // generate coreir: wire with indices maybe
  if (predicate) {
    // FIXME: implement predicate
    stream << "// writing stream with a predicate\n";
  }

  rename_wire(printed_stream_name, input_name, op->args[1]);
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_read_stream(const Call *op) {
  internal_assert(op->args.size() == 2 || op->args.size() == 3);
  string a1 = print_expr(op->args[1]);

  const Variable *stream_name_var = op->args[0].as<Variable>();
  internal_assert(stream_name_var);
  string stream_name = stream_name_var->name;
  string stream_print_name = print_name(stream_name);
  if (op->args.size() == 3) {
    // stream name is maggled with the consumer name
    const StringImm *consumer_imm = op->args[2].as<StringImm>();
    internal_assert(consumer_imm);
    stream_name += ".to." + consumer_imm->value;
    stream_print_name += "." + consumer_imm->value;
  }
  do_indent();
  stream << a1 << " = " << print_name(stream_name) << ".read();\n";
  id = "0"; // skip evaluation

  // generate coreir: add as coreir input
  rename_wire(a1, stream_print_name, op->args[0]);

  if (predicate) {
    stream << "// reading stream with a predicate\n";
    // hook up the enable
    //CoreIR::Wireable* stencil_wire = get_wire(stream_print_name, op->args[0]);
    //CoreIR::Wireable* lb_wire = stencil_wire->getTopParent();
    // FIXME: assert that this is a linebuffer (rgb)
    // FIXME: shouldn't print out again
    string cond_id = print_expr(predicate->condition);
    //def->disconnect(lb_wire->sel("wen"));
    //def->connect(lb_wire->sel("wen"), get_wire(cond_id, predicate->condition));
  }

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_dispatch_stream(const Call *op) {
  //cout << "doing a dispatch\n";
  // emits the calling arguments in comment
  vector<string> args(op->args.size());
  for(size_t i = 0; i < op->args.size(); i++)
    args[i] = print_expr(op->args[i]);

  do_indent();
  stream << "// dispatch_stream(";
  for(size_t i = 0; i < args.size(); i++) {
    stream << args[i];

    if (i != args.size() - 1)
      stream << ", ";
  }
  stream << ");\n";
  // syntax:
  //   dispatch_stream(stream_name, num_of_dimensions,
  //                   stencil_size_dim_0, stencil_step_dim_0, store_extent_dim_0,
  //                   [stencil_size_dim_1, stencil_step_dim_1, store_extent_dim_1, ...]
  //                   num_of_consumers,
  //                   consumer_0_name, fifo_0_depth,
  //                   consumer_0_offset_dim_0, consumer_0_extent_dim_0,
  //                   [consumer_0_offset_dim_1, consumer_0_extent_dim_1, ...]
  //                   [consumer_1_name, ...])

  // recover the structed data from op->args
  internal_assert(op->args.size() >= 2);
  const Variable *stream_name_var = op->args[0].as<Variable>();
  internal_assert(stream_name_var);
  string stream_name = stream_name_var->name;
  size_t num_of_dimensions = *as_const_int(op->args[1]);
  vector<int> stencil_steps(num_of_dimensions);
  vector<int> store_extents(num_of_dimensions);

  stream << "// unpacking dispatch\n";
  int idx = 2; // the next param
  internal_assert(op->args.size() >= num_of_dimensions*2 + 2);
  for (size_t i = 0; i < num_of_dimensions; i++) {
    //stencil_sizes[i] = *as_const_int(op->args[i*3 + 2]);
    stencil_steps[i] = *as_const_int(op->args[idx++]);
    store_extents[i] = *as_const_int(op->args[idx++]);
  }

  internal_assert(op->args.size() >= num_of_dimensions*2 + 3);
  size_t num_of_consumers = *as_const_int(op->args[idx++]);
  vector<string> consumer_names(num_of_consumers);
  vector<int> consumer_fifo_depth(num_of_consumers);
  vector<vector<int> > consumer_stencils(num_of_consumers);
  vector<vector<int> > consumer_offsets(num_of_consumers);
  vector<vector<int> > consumer_extents(num_of_consumers);

  stream << "// going through consumers\n";

  internal_assert(op->args.size() >= 2 + num_of_dimensions*2 + 1 + num_of_consumers*(2 + 3*num_of_dimensions));
  //internal_assert(op->args.size() >= num_of_demensions*3 + 3 + num_of_consumers*(2 + 2*num_of_demensions));
  for (const auto &arg : args) {
    stream << arg << ", ";
  }
  stream << std::endl;

  for (size_t i = 0; i < num_of_consumers; i++) {
    const StringImm *string_imm = op->args[idx++].as<StringImm>();
    internal_assert(string_imm);
    consumer_names[i] = string_imm->value;
    const IntImm *int_imm = op->args[idx++].as<IntImm>();
    internal_assert(int_imm);
    consumer_fifo_depth[i] = int_imm->value;
    vector<int> stencilsizes(num_of_dimensions);
    vector<int> offsets(num_of_dimensions);
    vector<int > extents(num_of_dimensions);
    for (size_t j = 0; j < num_of_dimensions; j++) {
      stencilsizes[j] = *as_const_int(op->args[idx++]);
      offsets[j] = *as_const_int(op->args[idx++]);
      extents[j] = *as_const_int(op->args[idx++]);
    }
    consumer_stencils[i] = stencilsizes;
    consumer_offsets[i] = offsets;
    consumer_extents[i] = extents;
  }

  // emits declarations of streams for each consumer
  internal_assert(stencils.contains(stream_name));
  Stencil_Type stream_type = stencils.get(stream_name);

  stream << "// doing dispatch opt\n";
  // Optimization. if there is only one consumer and its fifo depth is zero
  // , use C++ reference for the consumer stream
  if (num_of_consumers == 1 && consumer_fifo_depth[0] == 0) {
    string consumer_stream_name = stream_name + ".to." + consumer_names[0];
    do_indent();
    stream << print_stencil_type(stream_type) << " &"
           << print_name(consumer_stream_name) << " = "
           << print_name(stream_name) << ";\n";
    id = "0"; // skip evaluation

    // generate coreir: update coreir structure
    string stream_in_name = print_name(stream_name);
    string stream_out_name = print_name(consumer_stream_name);
    string next_lb_name = print_name(consumer_names[0]);

    stream << "// connecting " << stream_in_name << " to " << next_lb_name << "\n";
    //cout << "doing optimization of dispatch\n";
    record_dispatch(stream_in_name, next_lb_name);
    rename_wire(stream_out_name, stream_in_name, op->args[0]);
    //cout << "finished optimization of dispatch\n";
    //def->connect({stream_in_name,"valid"}, {next_lb_name, "wen"});
    return;
  }

  for (size_t i = 0; i < num_of_consumers; i++) {
    string consumer_stream_name = stream_name + ".to." + consumer_names[i];
    Stencil_Type consumer_stream_type = stream_type;
    consumer_stream_type.depth = std::max(consumer_fifo_depth[i], 1); // HLS tool doesn't support zero-depth FIFO yet
    do_indent();
    stream << print_stencil_type(consumer_stream_type) << ' '
           << print_name(consumer_stream_name) << ";\n";
    // pragma
    stencils.push(consumer_stream_name, consumer_stream_type);
    stream << print_stencil_pragma(consumer_stream_name);
    stencils.pop(consumer_stream_name);

    // record dispatch for coreir
    string stream_in_name = print_name(stream_name);
    string next_lb_name = print_name(consumer_names[i]);
    record_dispatch(stream_in_name, next_lb_name);
  }

  // emits for a loop for each dimensions (larger dimension number, outer the loop)
  for (int i = num_of_dimensions - 1; i >= 0; i--) {
    string dim_name = "_dim_" + std::to_string(i);
    do_indent();
    // HLS C: for(int dim = 0; dim <= store_extent - stencil.size; dim += stencil.step)
    stream << "for (int " << dim_name <<" = 0; "
           << dim_name << " <= " << store_extents[i] - consumer_stencils[0][i] << "; "
           << dim_name << " += " << stencil_steps[i] << ")\n";

  }


  open_scope();
  // pragma
  stream << "#pragma HLS PIPELINE\n";
  // read stencil from stream
  Stencil_Type stencil_type = stream_type;
  stencil_type.type = Stencil_Type::StencilContainerType::Stencil;
  string stencil_name = "tmp_stencil";
  do_indent();
  stream << "Packed" << print_stencil_type(stencil_type) << ' '
         << print_name(stencil_name) << " = "
         << print_name(stream_name) << ".read();\n";

  // dispatch the stencil to each consumer stream
  for (size_t i = 0; i < num_of_consumers; i++) {
    string consumer_stream_name = stream_name + ".to." + consumer_names[i];
    // emits the predicate for dispatching stencils
    // HLS C: if(dim_0 >= consumer_offset_0 && dim_0 <= consumer_offset_0 + consumer_extent_0 - stencil_size_0
    //           [&& dim_1 >= consumer_offset_1 && dim_1 <= consumer_offset_1 + consumer_extent_1 - stencil_size_1...])
    do_indent();
    stream << "if (";
    for (size_t j = 0; j < num_of_dimensions; j++) {
      string dim_name = "_dim_" + std::to_string(j);
      stream << dim_name << " >= " << consumer_offsets[i][j] << " && "
             << dim_name << " <= " << consumer_offsets[i][j] + consumer_extents[i][j] - consumer_stencils[i][j];
      if (j != num_of_dimensions - 1)
        stream << " && ";
    }
    stream << ")\n";

    // emits the write call in the if body
    open_scope();
    do_indent();
    stream << print_name(consumer_stream_name) << ".write("
           << print_name(stencil_name) << ");\n";
    close_scope("");

    // generate coreir
    string stream_in_name = print_name(stream_name);
    string stream_out_name = print_name(consumer_stream_name);
    rename_wire(stream_out_name, stream_in_name, op->args[0]);
    //def->connect({stream_in_name,"valid"}, {print_name(consumer_names[i]), "wen"});

  }

  close_scope("");
  id = "0"; // skip evaluation

}


void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit_stencil(const Call *op) {
  ostringstream rhs;
  // IR: out.stencil_update(0, 0, 0)
  // C: out_stencil_update(0, 0, 0)
  vector<string> args_indices(op->args.size());
  vector<uint> stencil_indices;
  bool constant_indices = true;
  for(size_t i = 0; i < op->args.size(); i++) {
    args_indices[i] = print_expr(op->args[i]);
    constant_indices = constant_indices && is_const(op->args[i]);
    if (constant_indices) {
      stencil_indices.push_back(id_const_value(op->args[i]));
    }
  }

  rhs << print_name(op->name) << "(";
  string stencil_print_name = print_name(op->name);
  for(size_t i = 0; i < op->args.size(); i++) {
    rhs << args_indices[i];
    //stencil_print_name += "_" + args_indices[i];
    if (i != op->args.size() - 1)
      rhs << ", ";
  }
  rhs << ")";
  rhs << "[stencil]";

  string out_var = print_assignment(op->type, rhs.str());
  if (is_wire(out_var)) { return; }

  // generate coreir

  if (constant_indices) {// && (is_wire(stencil_print_name) || is_defined(stencil_print_name))) {
    //cout << "// added to set: " << out_var << " using stencil+idx\n";
    rename_wire(out_var, stencil_print_name, op, stencil_indices);
    stream << "// added to set: " << out_var << " using stencil+idx\n";

  } else if (is_wire(stencil_print_name) || is_defined(stencil_print_name) || is_storage(stencil_print_name)) {
    stream << "// trying to hook up " << print_name(op->name) << endl;
    //cout << "trying to hook up " << print_name(op->name) << endl;
    /*
      if (hw_wire_set.count(stencil_print_name)) {
      hw_wire_set[out_var] = hw_wire_set[stencil_print_name];
      stream << "// added to wire_set: " << out_var << " using stencil+idx\n";
      } else if (hw_wire_set.count(print_name(op->name)) > 0) {
      //stream << "// trying to hook up " << print_name(op->name) << endl;
      */

    //CoreIR::Wireable* stencil_wire = hw_wire_set[print_name(op->name)]; // one example wire
    CoreIR::Wireable* stencil_wire = get_wire(print_name(op->name), op);
    CoreIR::Wireable* orig_stencil_wire = stencil_wire;
    vector<std::pair<CoreIR::Wireable*, CoreIR::Wireable*>> stencil_mux_pairs;

    // FIXME: use proper bitwidth
    CoreIR::Type* ptype;
    if (op->type.bits() == 1) {
      ptype = context->Bit();
    } else {
      ptype = context->Bit()->Arr(bitwidth);
    }

    string pt_name = "pt" + out_var + "_" + unique_name('p');
    stream << "// created passthrough with name " << pt_name << endl;

    CoreIR::Wireable* pt = def->addInstance(pt_name, gens["passthrough"], {{"type",CoreIR::Const::make(context,ptype)}});
    stencil_mux_pairs.push_back(std::make_pair(stencil_wire, pt->sel("in")));

    // for every dim in the stencil, keep track of correct index and create muxes
    for (size_t i = op->args.size(); i-- > 0 ;) { // count down from args-1 to 0
      vector<std::pair<CoreIR::Wireable*, CoreIR::Wireable*>> new_pairs;

      CoreIR::Type* wire_type = stencil_wire->getType();
      uint array_len = wire_type->getKind() == CoreIR::Type::TypeKind::TK_Array ?
        //FIXME: CoreIR::isa<CoreIR::ArrayType>(wire_type) ?
        static_cast<CoreIR::ArrayType*>(wire_type)->getLen() : 0;
      // cast<ArrayType>

      // OR_ : dyn_cast<ArrayType>

      if (is_const(op->args[i])) {
        // constant index

        //uint index = stoi(args_indices[i]);
        uint index = id_const_value(op->args[i]);
        //cout << "type is " << wire_type->getKind() << " and has length " << static_cast<CoreIR::ArrayType*>(wire_type)->getLen() << endl;

        if (index < array_len) {
          stream << "// using constant index " << std::to_string(index) << endl;
          stencil_wire = stencil_wire->sel(index);

          // keep track of corresponding stencil and mux inputs
          for (auto sm_pair : stencil_mux_pairs) {
            CoreIR::Wireable* stencil_i = sm_pair.first;
            CoreIR::Wireable* mux_i = sm_pair.second;
            new_pairs.push_back(std::make_pair(stencil_i->sel(index), mux_i));
          }
          stencil_mux_pairs = new_pairs;

        } else {
          stream << "// ERROR- couldn't find selectStr " << std::to_string(index) << endl;
          //cout << "couldn't find selectStr " << to_string(index) << endl;

          stencil_mux_pairs.clear();
          stencil_mux_pairs.push_back(std::make_pair(orig_stencil_wire, pt->sel("in")));
          break;
        }

      } else {
        // non-constant, variable index
        // create muxes
        uint num_muxes = stencil_mux_pairs.size();
        //cout << "  variable index creating " << num_muxes << " mux(es)" << endl;

        stream << "// variable index creating " << num_muxes << " mux(es)" << endl;

        // create mux for every input from previous layer
        for (uint j = 0; j < num_muxes; j++) {
          CoreIR::Wireable* stencil_i = stencil_mux_pairs[j].first;
          CoreIR::Wireable* mux_i = stencil_mux_pairs[j].second;

          string mux_name = unique_name(print_name(op->name) + std::to_string(i) +
                                        "_mux" + std::to_string(array_len) + "_" + std::to_string(j));

          // FIXME: use proper bitwidth
          CoreIR::Values sliceArgs = {{"width", CoreIR::Const::make(context,bitwidth)},
                                      {"lo", CoreIR::Const::make(context,0)},
                                      {"hi", CoreIR::Const::make(context,num_bits(array_len-1))}};
          CoreIR::Wireable* slice_inst = def->addInstance("selslice" + mux_name, "coreir.slice", sliceArgs);


          CoreIR::Wireable* mux_inst = def->addInstance(mux_name, gens["muxn"],
                                                        {{"width",CoreIR::Const::make(context,bitwidth)},{"N",CoreIR::Const::make(context,array_len)}});

          def->connect(mux_inst->sel("out"), mux_i);
          stream << "// created mux called " << mux_name << endl;

          // wire up select
          def->connect(get_wire(args_indices[i], op->args[i]), slice_inst->sel("in"));
          def->connect(slice_inst->sel("out"), mux_inst->sel("in")->sel("sel"));

          // add each corresponding stencil and mux input to list
          for (uint mux_i = 0; mux_i < array_len; ++mux_i) {
            CoreIR::Wireable* stencil_new = stencil_i->sel(mux_i);
            CoreIR::Wireable* mux_new = mux_inst->sel("in")->sel("data")->sel(mux_i);
            new_pairs.push_back(std::make_pair(stencil_new, mux_new));
          } // for every mux input
        } // for every mux

          //cout << "all muxes created" << endl;
        stencil_wire = stencil_wire->sel(0);
        stencil_mux_pairs = new_pairs;
      }
    } // for every dimension in stencil

      // connect the passthrough output
    add_wire(out_var, pt->sel("out"));

    // wire up stencil to generated muxes
    for (auto sm_pair : stencil_mux_pairs) {
      CoreIR::Wireable* stencil_i = sm_pair.first;
      CoreIR::Wireable* mux_i = sm_pair.second;
      def->connect(stencil_i, mux_i);
    }


    stream << "// added to wire_set: " << out_var << " using stencil\n";
    //cout << "// added to wire_set: " << out_var << " using stencil\n";
  } else {
    stream       << "// " << stencil_print_name << " not found so it's not going to work" << endl;
    user_warning << "// " << stencil_print_name << " not found so it's not going to work\n";
  }

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Realize *op) {
  // This realized stream variable can be reused. Stream constructed as hierarchical array.
  if (ends_with(op->name, ".stream") ||
      ends_with(op->name, ".stencil") ||
      ends_with(op->name, ".stencil_update")) {
    // create a stream type
    internal_assert(op->types.size() == 1);
    allocations.push(op->name, {op->types[0]});
    Stencil_Type stream_type;
    if (ends_with(op->name, ".stream")) {
      stream_type = Stencil_Type({Stencil_Type::StencilContainerType::Stream, op->types[0], op->bounds, 1});
    } else {
      stream_type = Stencil_Type({Stencil_Type::StencilContainerType::Stencil, op->types[0], op->bounds, 1});
    }
    stencils.push(op->name, stream_type);

    // emits the declaration for the stream
    do_indent();
    stream << "[realize] ";
    stream << print_stencil_type(stream_type) << ' ' << print_name(op->name) << ";\n";
    stream << print_stencil_pragma(op->name);

    // generate coreir: add passthrough using vector of widths
    vector<uint> indices;
    for(const auto &range : stream_type.bounds) {
      assert(is_const(range.extent));
      indices.push_back(id_const_value(range.extent));
    }

    // FIXME: use proper bitwidth
    CoreIR::Type* ptype;
    if (stream_type.elemType.bits() == 1) {
      ptype = context->Bit();
    } else {
      ptype = context->Bit()->Arr(bitwidth);
    }

    // create type for passthrough
    for (uint i=0; i<indices.size(); ++i) {
      ptype = ptype->Arr(indices[i]);
    }

    if (indices.size() > 4) {
      internal_error << "provide stencil is too large=" << indices.size()
                     << ". Sizes up to 4 are supported.\n";
    }

    // create coreir instance
    string pt_name = "pt" + print_name(op->name)+ "_" + unique_name('p');
    stream << "// created a passthrough for " << pt_name << endl;
    CoreIR::Wireable* pt = def->addInstance(pt_name, gens["passthrough"], {{"type",CoreIR::Const::make(context,ptype)}});

    // store passthrough in storage in case variable reused multiple times
    hw_store_set[print_name(op->name)] = std::make_shared<Storage_Def>(Storage_Def(ptype, pt));
    stream << "// created storage called " << op->name << endl;

    // traverse down
    op->body.accept(this);

    // We didn't generate free stmt inside for stream/stencil type
    allocations.pop(op->name);
    stencils.pop(op->name);
  } else {
    CodeGen_C::visit(op);
  }
}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const IfThenElse *op) {
  string cond_id = print_expr(op->condition);

  do_indent();
  stream << "if (" << cond_id << ")\n";
  open_scope();

  if (!op) {
    internal_error << "there is already a predicate set\n";
    // FIXME: handle nested branches
  }
  predicate = op;
  op->then_case.accept(this);
  predicate = NULL;

  close_scope("if " + cond_id);

  if (op->else_case.defined()) {
    // FIXME: handle else case
    user_error << "we don't do else cases\n";
    do_indent();
    stream << "else\n";
    open_scope();
    op->else_case.accept(this);
    close_scope("if " + cond_id + " else");
  }

}

void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Store *op) {
  Type t = op->value.type();

  bool type_cast_needed =
    t.is_handle() ||
    !allocations.contains(op->name) ||
    allocations.get(op->name).type != t;

  string id_index = print_expr(op->index);
  string id_value = print_expr(op->value);
  string name = print_name(op->name);
  do_indent();

  if (type_cast_needed) {
    stream << "((const "
           << print_type(t)
           << " *)"
           << name
           << ")";
  } else {
    stream << name;
  }
  // FIXME: handle case that id_index is not a constant value
//  internal_assert(is_const(op->index)) << "index is not a const (unsupported)";
//  int index = id_const_value(op->index);
//  internal_assert(index >= 0) << "should be non-negative";
//  uint index_unsigned = (unsigned int) index;

  stream << "["
         << id_index
         << "] = "
         << id_value
         << ";  [store]\n";

  // generate coreir

  //rename_wire(out_var, id_value, op->value, {index_unsigned});

  if (is_const(op->index)) {
    string out_var = name + "_" + id_index;
    rename_wire(out_var, id_value, op->value);
  } else if (is_defined(name) && hw_def_set[name]->gen==gens["ram2"]) {
    stream << "// " << name << " connected by ram\n";
    get_wire(name, op->name); // creates the sram
    def->disconnect(get_wire(name + "_wdata", Expr()));
    def->disconnect(get_wire(name + "_waddr", Expr()));
    def->connect(get_wire(name + "_wdata", Expr()), get_wire(id_value, op->value));
    def->connect(get_wire(name + "_waddr", Expr()), get_wire(id_index, op->index));
  }

}

// Load from an array. A variable load leads to a complicated mux
//   with complicated wiring so variables not computed.
void CodeGen_CoreIR_Target::CodeGen_CoreIR_C::visit(const Load *op) {
  Type t = op->type;
  bool type_cast_needed =
    !allocations.contains(op->name) ||
    allocations.get(op->name).type != t;

  string id_index = print_expr(op->index);
  string name = print_name(op->name);
  ostringstream rhs;
  if (type_cast_needed) {
    rhs << "(("
        << print_type(op->type)
        << " *)"
        << name
        << ")";
  } else {
    rhs << name;
  }
  rhs << "["
      << id_index
      << "][load]";

  string out_var = print_assignment(op->type, rhs.str());

  // generate coreir
  if (is_const(op->index)) {
    string in_var = name + "_" + id_index;
    rename_wire(out_var, in_var, Expr());

  } else if (is_defined(name) && hw_def_set[name]->gen==gens["rom2"]) {
    stream << "loading from rom " << name << " with gen " << hw_def_set[name]->gen << std::endl;

    std::shared_ptr<CoreIR_Inst_Args> inst_args = hw_def_set[name];
    string inst_name = unique_name(inst_args->name);
    CoreIR::Wireable* inst = def->addInstance(inst_name, inst_args->gen, inst_args->args, inst_args->genargs);
    add_wire(out_var, inst->sel(inst_args->selname));

    // attach the read address
    CoreIR::Wireable* raddr_wire = get_wire(id_index, op->index);
    def->connect(raddr_wire, inst->sel("raddr"));
    //attach a read enable
    CoreIR::Wireable* rom_ren = def->addInstance(inst_name + "_ren", gens["bitconst"], {{"value", CoreIR::Const::make(context,true)}});
    def->connect(rom_ren->sel("out"), inst->sel("ren"));

  } else if (is_defined(name) && hw_def_set[name]->gen==gens["ram2"]) {
    stream << "loading from sram " << name << std::endl;

    add_wire(out_var, get_wire(name, Expr()));

    // attach the read address
    CoreIR::Wireable* raddr_wire = get_wire(id_index, op->index);
    auto ram_raddr = get_wire(name+"_raddr", Expr());
    def->disconnect(ram_raddr);
    def->connect(raddr_wire, ram_raddr);


  } else { // variable load: use a mux for where data originates
    std::vector<const Variable*> dep_vars = find_dep_vars(op->index);
    stream << "// vars for " << name << ": ";
    //cout << "// vars for " << name << ": ";
    for (auto v : dep_vars) {
      stream << v->name << ", ";
      //cout << v->name << ", ";
    }
    stream << endl;
    //cout << endl;

    // Calculate variable expression values and wire up mux instead of calculating expr in hw.
    std::vector<VarValues> pts;
    for (const Variable* v : dep_vars) {
      string id_var = print_name(v->name);

      internal_assert(is_defined(id_var)) << " didn't work for " << id_var << "\n";

      std::shared_ptr<CoreIR_Inst_Args> counter_args = hw_def_set[id_var];
      uint counter_max = counter_args->args.at("max")->get<int>();
      uint counter_min = counter_args->args.at("min")->get<int>();
      uint counter_inc = counter_args->args.at("inc")->get<int>();
      internal_assert(counter_min == 0);
      internal_assert(counter_inc == 1);

      stream << "// found counter named " << id_var << " with max " << counter_max << endl;

      std::vector<VarValues> old_pts = pts;
      pts.clear();
      if (old_pts.empty()) {
        // add all points from counter
        for (uint count=0; count<=counter_max; ++count) {
          VarValues vvv;
          vvv[id_var] = count;
          stream << "//     pushed var,count: " << id_var << "," << count << endl;
          pts.push_back(vvv);
        }

        std::vector<int> indices = eval_expr_with_vars(op->index, pts);

        stream << "// found " << indices.size() << " indices for pts size " << pts.size()
               <<endl << "// ";
        for (auto idx : indices) {
          stream << idx << ", ";
        }
        stream << endl;

        uint mux_size = indices.size();
        string mux_name = unique_name(name + "_mux" + std::to_string(mux_size));

        // FIXME: use proper bitwidth
        CoreIR::Values sliceArgs = {{"width", CoreIR::Const::make(context,bitwidth)},
                                    {"lo", CoreIR::Const::make(context,0)},
                                    {"hi", CoreIR::Const::make(context,num_bits(mux_size-1))}};
        CoreIR::Wireable* slice_inst = def->addInstance("selslice" + mux_name, "coreir.slice", sliceArgs);


        CoreIR::Wireable* mux_inst = def->addInstance(mux_name, gens["muxn"],
                                                      {{"width",CoreIR::Const::make(context,bitwidth)},{"N",CoreIR::Const::make(context,mux_size)}});

        def->connect(get_wire(id_var, Expr()), slice_inst->sel("in"));
        def->connect(mux_inst->sel("in")->sel("sel"), slice_inst->sel("out"));
        for (uint i=0; i<mux_size; ++i) {
          CoreIR::Wireable* wire_in = get_wire(name + "_" + std::to_string(indices[i]), Expr());
          internal_assert(wire_in) << "Allocation " << name << " was not saved yet for index " << indices[i] << "\n";
          def->connect(wire_in, mux_inst->sel("in")->sel("data")->sel(i));
        }
        add_wire(out_var, mux_inst->sel("out"));


      } else {
        for (uint count=0; count<counter_max; ++count) {
          //FIXME: implement this
          internal_error << "Multiple variable indexing into loads isn't implemented yet!\n";
        }
      }
    }

  }
}

// analysis passes structured in a similar fashion to those in Simplify.cpp const_int_bounds( )
std::vector<const Variable*> CodeGen_CoreIR_Target::CodeGen_CoreIR_C::find_dep_vars(Expr e) {
  std::vector<const Variable*> vars;

  if (is_const(e)) {
    stream << "encountering const\n";
    // no variable to add
  } else if (const Max* max = e.as<Max>()) {
    auto vec_a = find_dep_vars(max->a);
    auto vec_b = find_dep_vars(max->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Min* min = e.as<Min>()) {
    auto vec_a = find_dep_vars(min->a);
    auto vec_b = find_dep_vars(min->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Select* sel = e.as<Select>()) {
    auto vec_true = find_dep_vars(sel->true_value);
    auto vec_fals = find_dep_vars(sel->false_value);
    auto vec_cond = find_dep_vars(sel->condition);
    vars.insert( vars.end(), vec_true.begin(), vec_true.end() );
    vars.insert( vars.end(), vec_fals.begin(), vec_fals.end() );
    vars.insert( vars.end(), vec_cond.begin(), vec_cond.end() );
  } else if (const Add* add = e.as<Add>()) {
    stream << "encountering add" << "\n";
    auto vec_a = find_dep_vars(add->a);
    auto vec_b = find_dep_vars(add->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Sub* sub = e.as<Sub>()) {
    auto vec_a = find_dep_vars(sub->a);
    auto vec_b = find_dep_vars(sub->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Mul* mul = e.as<Mul>()) {
    auto vec_a = find_dep_vars(mul->a);
    auto vec_b = find_dep_vars(mul->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Mod* mod = e.as<Mod>()) {
    auto vec_a = find_dep_vars(mod->a);
    auto vec_b = find_dep_vars(mod->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Div* div = e.as<Div>()) {
    auto vec_a = find_dep_vars(div->a);
    auto vec_b = find_dep_vars(div->b);
    vars.insert( vars.end(), vec_a.begin(), vec_a.end() );
    vars.insert( vars.end(), vec_b.begin(), vec_b.end() );
  } else if (const Cast* cast = e.as<Cast>()) {
    auto vec_v = find_dep_vars(cast->value);
    vars.insert( vars.end(), vec_v.begin(), vec_v.end() );
  } else if (const Call* call = e.as<Call>()) {
    if (call->is_intrinsic(Call::shift_right)) {

    }

  } else if (const Variable *v = e.as<Variable>()) {
    stream << "encountering var " << v->name << " " << "\n";
    if (v->reduction_domain.defined()) {
      stream << "it's a rdom " << v->reduction_domain.domain()[0].var << "\n";
    }
    if (v->param.defined()) {
      stream << "it's a param " << v->param.name() << "\n";
    }

    if (v->image.defined()) {
      stream << "it's a image " << v->image.name() << "\n";
    }

    vars.push_back(v);

  } else {
    internal_error << "function " << e << " not supported in simplification...\n";
    stream << "// function not supported...";
  }

  // all functions return variable "vars"
  return vars;
}

std::vector<int> CodeGen_CoreIR_Target::CodeGen_CoreIR_C::eval_expr_with_vars(Expr e, std::vector<VarValues> pts) {
  std::vector<int> indices (pts.size());

  if (is_const(e)) {
    int value = id_const_value(e);
    std::fill( indices.begin(), indices.end(), value );
  } else if (const Variable *v = e.as<Variable>()) {
    stream << "// evaling for var " << print_name(v->name) << endl;
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = pts[i][print_name(v->name)];
      stream << "// pushing " << indices[i] << " for index " << i << endl;
    }
  } else if (const Max* max = e.as<Max>()) {
    auto vec_a = eval_expr_with_vars(max->a, pts);
    auto vec_b = eval_expr_with_vars(max->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = std::max(vec_a[i], vec_b[i]);
    }
  } else if (const Min* min = e.as<Min>()) {
    auto vec_a = eval_expr_with_vars(min->a, pts);
    auto vec_b = eval_expr_with_vars(min->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = std::min(vec_a[i], vec_b[i]);
    }
  } else if (const Select* sel = e.as<Select>()) {
    auto vec_true = eval_expr_with_vars(sel->true_value, pts);
    auto vec_fals = eval_expr_with_vars(sel->false_value, pts);
    auto vec_cond = eval_expr_with_vars(sel->condition, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = vec_cond[i] ? vec_true[i] : vec_fals[i];
    }
  } else if (const Add* add = e.as<Add>()) {
    auto vec_a = eval_expr_with_vars(add->a, pts);
    auto vec_b = eval_expr_with_vars(add->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = vec_a[i] + vec_b[i];
    }
  } else if (const Sub* sub = e.as<Sub>()) {
    auto vec_a = eval_expr_with_vars(sub->a, pts);
    auto vec_b = eval_expr_with_vars(sub->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = vec_a[i] - vec_b[i];
    }
  } else if (const Mul* mul = e.as<Mul>()) {
    auto vec_a = eval_expr_with_vars(mul->a, pts);
    auto vec_b = eval_expr_with_vars(mul->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = vec_a[i] * vec_b[i];
    }
  } else if (const Mod* mod = e.as<Mod>()) {
    auto vec_a = eval_expr_with_vars(mod->a, pts);
    auto vec_b = eval_expr_with_vars(mod->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = vec_a[i] % vec_b[i];
    }
  } else if (const Div* div = e.as<Div>()) {
    auto vec_a = eval_expr_with_vars(div->a, pts);
    auto vec_b = eval_expr_with_vars(div->b, pts);
    for (uint i=0; i<indices.size(); ++i) {
      indices[i] = vec_a[i] / vec_b[i];
    }
  } else {
    internal_error << "function not supported...";
    stream << "// function not supported...";
  }

  // all cases return variable "indices"
  return indices;
}

}
}

