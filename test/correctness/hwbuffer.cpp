#include "Halide.h"
#include "test/common/check_call_graphs.h"

#include <stdio.h>
#include <map>

#include "hwbuffer.h"

namespace {

using std::map;
using std::string;
using std::to_string;
using std::cout;
using std::endl;

using namespace Halide;
using namespace Halide::Internal;

void h_assert(bool condition, const char* msg="") {
    if (!condition) {
        printf("FAIL: %s\n", msg);
        abort();
    }
}
void h_assert(bool condition, const string msg="") {
    if (!condition) {
        std::cout << "FAIL: " << msg << std::endl;
        abort();
    }
}

void check_param(const string paramname, Expr param1, Expr param2) {
  std::ostringstream debug_stream;
  debug_stream << paramname << " not correct: " << param1 << " vs ref=" << param2;
  h_assert(param1.defined() && param2.defined(), debug_stream.str());
  h_assert(is_one(simplify(param1 == param2)), debug_stream.str());
}
void check_param(const string paramname, int param1, int param2) {
  std::ostringstream debug_stream;
  debug_stream << paramname << " not correct: " << param1 << " vs ref=" << param2;
  h_assert(param1 == param2, debug_stream.str());
}

std::vector<HWXcel> lower_to_hwbuffer(const vector<Function> &output_funcs, const string &pipeline_name, const Target &t,
                                      const vector<Argument> &args) {

    std::vector<std::string> namespaces;
    std::string simple_pipeline_name = extract_namespaces(pipeline_name, namespaces);

    Module result_module(simple_pipeline_name, t);

    // Compute an environment
    map<string, Function> env;
    for (Function f : output_funcs) {
        populate_environment(f, env);
    }

    vector<Function> outputs;
    std::tie(outputs, env) = deep_copy(output_funcs, env);
    bool any_strict_float = strictify_float(env, t);
    result_module.set_any_strict_float(any_strict_float);
    for (Function f: outputs) {
        Func(f).compute_root().store_root();
    }

    // Finalize all the LoopLevels
    for (auto &iter : env) {
        iter.second.lock_loop_levels();
    }
    env = wrap_func_calls(env);

    vector<string> order;
    vector<vector<string>> fused_groups;
    std::tie(order, fused_groups) = realization_order(outputs, env);
    simplify_specializations(env);

    debug(1) << "Creating initial loop nests...\n";
    bool any_memoized = false;
    Stmt s = schedule_functions(outputs, fused_groups, env, t, any_memoized);

    if (any_memoized) {
        s = inject_memoization(s, env, pipeline_name, outputs);
    }

    s = inject_tracing(s, pipeline_name, env, outputs, t);
    s = add_parameter_checks(s, t);

    // Compute the maximum and minimum possible value of each
    // function. Used in later bounds inference passes.
    FuncValueBounds func_bounds = compute_function_value_bounds(order, env);
    s = add_image_checks(s, outputs, t, order, env, func_bounds);

    // This pass injects nested definitions of variable names, so we
    // can't simplify statements from here until we fix them up. (We
    // can still simplify Exprs).
    vector<BoundsInference_Stage> inlined_stages;
    s = bounds_inference(s, outputs, order, fused_groups, env, func_bounds, inlined_stages, t);

    for (auto stage : inlined_stages) {
      std::cout << "found stage: " << stage.name << std::endl;
      for (auto map_entry : stage.bounds) {
        std::cout << "  bounds for " << map_entry.first.first << " " << map_entry.second << std::endl;
      }
    }
    
    s = remove_extern_loops(s);
    s = allocation_bounds_inference(s, env, func_bounds);
    debug(2) << "Lowering after allocation bounds inference:\n" << s << '\n';
    //std::cout << "Lowering after allocation bounds inference:\n" << s << '\n';

    if (!t.has_feature(Target::HLS)) {
      s = sliding_window(s, env);
      std::cout << "sliding some windows\n";
    }
    
    std::cout << "extracting hw buffers\n";
    vector<HWXcel> xcels;
    if (t.has_feature(Target::CoreIR)) {
      xcels = extract_hw_accelerators(s, env, inlined_stages);
      for (auto hwbuffer : xcels.at(0).hwbuffers) {
        std::cout << hwbuffer.first << " is lower w/ inline=" << hwbuffer.second.is_inlined << std::endl;
      }
    }

    /*
    //std::cout << "Performing sliding window optimization...\n" << s << '\n';
    //std::cout << "Lowering after sliding window:\n" << s << '\n';
    
    s = remove_undef(s);
    s = uniquify_variable_names(s);

    Stmt s_ub;
    if (t.has_feature(Target::CoreIR) || t.has_feature(Target::HLS)) {
      // passes specific to HLS backend
      debug(1) << "Performing HLS target optimization..\n";
      //std::cout << "Performing HLS target optimization..." << s << '\n';

      for (const HWXcel &xcel : xcels) {
        s = insert_hwbuffers(s, xcel);
        std::cout << "inserted hwbuffers:\n" << s_ub << "\n";
      }

    }
    */

    return xcels;
}


int check_hwbuffer_params(HWBuffer hwbuffer, HWBuffer ref) {
  h_assert(hwbuffer.name == ref.name, "wrong name for hwbuffer: " + hwbuffer.name + " vs ref=" + ref.name);

  // check store, compute, and streaming loops
  h_assert(hwbuffer.store_level == ref.store_level,
           hwbuffer.name + " has the wrong store level: " + hwbuffer.store_level + " vs ref=" + ref.store_level);
  h_assert(hwbuffer.compute_level == ref.compute_level,
           hwbuffer.name + " has the wrong compute level: " + hwbuffer.compute_level + " vs ref=" + ref.compute_level);

  h_assert(hwbuffer.streaming_loops.size() == ref.streaming_loops.size(), hwbuffer.name + " has a differing number of streaming loops");
  for (size_t i=0; i<hwbuffer.streaming_loops.size(); ++i) {
    h_assert(hwbuffer.streaming_loops.at(i) == ref.streaming_loops.at(i),
             hwbuffer.name + " has the wrong streaming loop"  + to_string(i) + " name: " + hwbuffer.streaming_loops.at(i));
  }

  // check logical, stencil, chunk, and block sizes
  h_assert(hwbuffer.dims.size() == ref.dims.size(), "doesn't have correct num of dims");
  for (size_t i=0; i<ref.dims.size(); ++i) {
    check_param(hwbuffer.name + " logical size dim" + to_string(i), hwbuffer.ldims.at(i).logical_size, ref.ldims.at(i).logical_size);
  }
  for (size_t i=0; i<ref.dims.size(); ++i) {
    check_param(hwbuffer.name + " output stencil dim" + to_string(i), hwbuffer.dims.at(i).output_stencil, ref.dims.at(i).output_stencil);
  }
  for (size_t i=0; i<ref.dims.size(); ++i) {
    check_param(hwbuffer.name + " output block dim" + to_string(i), hwbuffer.dims.at(i).output_block, ref.dims.at(i).output_block);
  }
  for (size_t i=0; i<ref.dims.size(); ++i) {
    check_param(hwbuffer.name + " input chunk dim" + to_string(i), hwbuffer.dims.at(i).input_chunk, ref.dims.at(i).input_chunk);
  }
  for (size_t i=0; i<ref.dims.size(); ++i) {
    check_param(hwbuffer.name + " input block dim" + to_string(i), hwbuffer.dims.at(i).input_block, ref.dims.at(i).input_block);
  }

  // check linear address loops
  std::cout << "linear address loop length is " << hwbuffer.linear_addr.size() << std::endl;
  check_param(hwbuffer.name + " num of linear addr loops", hwbuffer.linear_addr.size(), ref.linear_addr.size());
  for (size_t i=0; i<ref.linear_addr.size(); ++i) {
    check_param(hwbuffer.name + " range loop" + to_string(i), hwbuffer.linear_addr.at(i).range, ref.linear_addr.at(i).range);
  }
  for (size_t i=0; i<ref.linear_addr.size(); ++i) {
    check_param(hwbuffer.name + " stride loop" + to_string(i), hwbuffer.linear_addr.at(i).stride, ref.linear_addr.at(i).stride);
  }
  for (size_t i=0; i<ref.linear_addr.size(); ++i) {
    check_param(hwbuffer.name + " dim_ref loop" + to_string(i), hwbuffer.linear_addr.at(i).dim_ref, ref.linear_addr.at(i).dim_ref);
  }

  // Note: the number of consumer streams includes the update
  //check_param(hwbuffer.name + " number of consumer streams", hwbuffer.ostreams.size(), ref.ostreams.size());
  
  for (const auto& ostream_pair : ref.ostreams) {
    h_assert(hwbuffer.ostreams.count(ostream_pair.first) > 0, hwbuffer.name + " has an ostream the hwbuffer does not have: " + ostream_pair.first);
    auto& ref_odims = ostream_pair.second.odims;
    auto& hwbuffer_odims = hwbuffer.ostreams.at(ostream_pair.first).odims;
  
    check_param("num of ostream dims", hwbuffer_odims.size(), ref_odims.size());
    for (size_t i=0; i<ref.dims.size(); ++i) {
      //std::cout << "output stencil looks like: " << hwbuffer_odims.at(i).output_stencil << " and " << ref_odims.at(i).output_stencil << std::endl;
      check_param(hwbuffer.name + " output stencil dim" + to_string(i), hwbuffer_odims.at(i).output_stencil, ref_odims.at(i).output_stencil);
    }
    for (size_t i=0; i<ref.dims.size(); ++i) {
      //std::cout << "output block looks like: " << hwbuffer_odims.at(i).output_block << " and " << ref_odims.at(i).output_block << std::endl;
      check_param(hwbuffer.name + " output block dim" + to_string(i), hwbuffer_odims.at(i).output_block, ref_odims.at(i).output_block);
    }
  }

  return 0;
}

int conv_hwbuffer_test(int ksize, int imgsize) {
    std::string suffix = "_" + to_string(ksize) + "_" + to_string(imgsize);
    Func kernel("kernel"+suffix), conv("conv"+suffix);
    Func hw_input("hw_input"+suffix), hw_output("hw_output"+suffix), output("output"+suffix);
    Var x("x"), y("y");
    Var xi("xi"), yi("yi"), xo("xo"), yo("yo");

    RDom r(0, ksize, 0, ksize);
    kernel(x, y) = 5*x + y;
    hw_input(x, y) = x + y;

    conv(x, y)  += kernel(r.x, r.y) * hw_input(x + r.x, y + r.y);
    hw_output(x, y) = conv(x, y);
    output(x, y) = hw_output(x, y);

    //// Schedule ////
    output.bound(x, 0, imgsize);
    output.bound(y, 0, imgsize);
    hw_output.compute_root();
          
    hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
      .hw_accelerate(xi, xo);

    //hw_input.compute_at(conv, x).store_at(hw_output, xo);
    hw_input.compute_at(hw_output, xi).store_at(hw_output, xo);

    conv.update()
      .unroll(r.x, ksize)
      .unroll(r.y, ksize);

    conv.linebuffer();
    conv.store_at(hw_output, xo)
      .compute_at(hw_output, xi);

    hw_input.stream_to_accelerator();
    kernel.compute_at(hw_output, xo);

    //// Run through compiler and find hardware buffer
    auto hwxcels = lower_to_hwbuffer({output.function()}, "conv_test",
                                     Target().with_feature(Target::CoreIR),
                                     {output.infer_arguments()});
    h_assert(hwxcels.size() == 1, "Incorrect number of xcels found");
    auto xcel = hwxcels.at(0);
    h_assert(xcel.hwbuffers.size() == 4, "Incorrect number of hwbuffers found");
    h_assert(xcel.hwbuffers.count("hw_input" + suffix) == 1, "Can't find hwbuffer named hw_input");
    auto input_hwbuffer = xcel.hwbuffers.at("hw_input" + suffix);
    std::cout << "done with hwbuffer creation\n";

    //// Create ref buffer and check the hardware buffers
    int ref_logsize = imgsize + ksize - 1;
    auto dims = create_hwbuffer_sizes({ref_logsize, ref_logsize},
                                      {ksize, ksize}, {ksize, ksize},
                                      {1, 1}, {1, 1});
    auto addrs = create_linear_addr({imgsize, imgsize},
                                    {1, 1}, {0, 1});
    vector<string> loops;
    vector<string> loopvars = {".xo", ".s0.y.yi", ".s0.x.xi"};
    for (auto loopvar : loopvars) {
      loops.emplace_back("hw_output" + suffix + loopvar);
    }
    HWBuffer ref_hwbuffer = HWBuffer("hw_input" + suffix, dims, addrs,
                                     loops, 0, 2,
                                     false, true,
                                     "", "conv"+suffix);
    std::cout << input_hwbuffer << std::endl;
    int output_value = check_hwbuffer_params(input_hwbuffer, ref_hwbuffer);


    return output_value;
}

int general_pipeline_hwbuffer_test(vector<int> ksizes, int imgsize, int tilesize) {
    std::string suffix = "_";
    for (auto ksize : ksizes) {
      suffix += to_string(ksize) + "_";
    }
    suffix += to_string(imgsize) + "_" + to_string(tilesize);

    size_t num_conv = ksizes.size();
    Func kernel[num_conv];
    Func conv[num_conv];
    RDom r[num_conv];
    for (size_t i=0; i<num_conv; ++i) {
      std::string ii = to_string(i);
      kernel[i] = Func("kernel"+ii+suffix);
      conv[i] = Func("conv"+ii+suffix);
      r[i] = RDom(0, ksizes.at(i), 0, ksizes.at(i));
    }
    Func hw_input("hw_input"+suffix), hw_output("hw_output"+suffix), output("output"+suffix);
    Var x("x"), y("y");
    Var xi("xi"), yi("yi"), xo("xo"), yo("yo");

    hw_input(x, y) = x + y;
    
    for (size_t i=0; i<num_conv; ++i) {
      kernel[i](x, y) = Expr(7*i) + 5*x + y;

      if (i > 0) {
        conv[i](x, y) += conv[i-1](x+r[i].x, y+r[i].y) * kernel[i](r[i].x, r[i].y);
      } else {
        conv[i](x, y) += hw_input(x+r[i].x, y+r[i].y) * kernel[i](r[i].x, r[i].y);
      }
    }

    hw_output(x, y) = conv[num_conv-1](x, y);
    output(x, y) = hw_output(x, y);

    //// Schedule ////
    output.bound(x, 0, imgsize);
    output.bound(y, 0, imgsize);
    hw_output.compute_root();
          
    hw_output.tile(x,y, xo,yo, xi,yi, tilesize, tilesize)
      .hw_accelerate(xi, xo);

    //hw_input.store_at(hw_output, xo).compute_at(conv[0], x);
    hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);
    hw_output.bound(x, 0, imgsize);
    hw_output.bound(y, 0, imgsize);

    for (uint i=0; i < num_conv; ++i) {
      conv[i].store_at(hw_output, xo).compute_at(hw_output, xi);
      kernel[i].compute_at(hw_output, xo);
      conv[i].update().unroll(r[i].x).unroll(r[i].y);
		}

    hw_input.stream_to_accelerator();


    //// Run through compiler and find hardware buffer
    auto hwxcels = lower_to_hwbuffer({output.function()}, "convchain_test",
                                     Target().with_feature(Target::CoreIR),
                                     {output.infer_arguments()});

    h_assert(hwxcels.size() == 1, "Incorrect number of xcels found");
    auto xcel = hwxcels.at(0);
    h_assert(xcel.hwbuffers.size() == 2 + 2*num_conv, "Incorrect number of hwbuffers found");
    h_assert(xcel.hwbuffers.count("hw_input" + suffix) == 1, "Can't find hwbuffer named hw_input");
    std::cout << "done with hwbuffer creation of convchain" << suffix << "\n";
      
    //// Create ref buffer and check the hardware buffers
    vector<string> buffer_names = vector<string>(num_conv);
    for (size_t i=0; i<num_conv; ++i) {
      string hwbuffer_name = i==0 ? "hw_input" + suffix : "conv" + to_string(i-1) + suffix;
      buffer_names[i] = hwbuffer_name;
    }
    
    for (size_t i=0; i<num_conv; ++i) {
      string hwbuffer_name = buffer_names.at(i);
      string producer_name = i==0 ? "" : buffer_names.at(i-1);
      string consumer_name = i==num_conv-1 ? "conv"+std::to_string(i)+suffix : buffer_names.at(i+1);
      h_assert(xcel.hwbuffers.count(hwbuffer_name) == 1, "Can't find hwbuffer named " + hwbuffer_name);
      auto hwbuffer = xcel.hwbuffers.at(hwbuffer_name);
      
      int ref_logsize = tilesize;
      for (size_t j=i; j<num_conv; ++j) {
        ref_logsize += ksizes.at(j) - 1;
      }
      int ksize = ksizes.at(i);
      auto dims = create_hwbuffer_sizes({ref_logsize, ref_logsize},
                                        {ksize, ksize}, {ksize, ksize},
                                        {1, 1}, {1, 1});
      int range = ref_logsize - (ksizes.at(i) - 1); // range does not include the last conv size
      auto addrs = create_linear_addr({range, range},
                                      {1, 1}, {0, 1});
      vector<string> loops;
      vector<string> loopvars = {".xo", ".s0.y.yi", ".s0.x.xi"};
      for (auto loopvar : loopvars) {
        if (i == 0) {
          loops.emplace_back("hw_output" + suffix + loopvar.substr(0));
        } else {
          loops.emplace_back("hw_output" + suffix + loopvar);
        }
      }
      int store_index = 0; //i==0 ? 0 : 0;
      int compute_index = 2;

      HWBuffer ref_hwbuffer = HWBuffer(hwbuffer_name, dims, addrs,
                                       loops, store_index, compute_index,
                                       false, false,
                                       producer_name, consumer_name);
      int output_value = check_hwbuffer_params(hwbuffer, ref_hwbuffer);
      if (output_value != 0) { return output_value; }
    }
    
    return 0;
}

int tiled_pipeline_hwbuffer_test(vector<int> ksizes, int imgsize, int tilesize) {
  return general_pipeline_hwbuffer_test(ksizes, imgsize, tilesize);
}

int pipeline_hwbuffer_test(vector<int> ksizes, int imgsize) {
  return general_pipeline_hwbuffer_test(ksizes, imgsize, imgsize);
}

int forked_pipeline_hwbuffer_test(int initk, vector<int> ksizes, int lastk, int imgsize) {
    // Create a unique suffix
    std::string suffix = "_";
    suffix += to_string(initk) + "_";
    for (auto ksize : ksizes) {
      suffix += to_string(ksize) + "_";
    }
    suffix += to_string(lastk) + "_";
    suffix += to_string(imgsize);

    /// Algorithm ///
    size_t num_conv = ksizes.size();
    Func kernel[num_conv];
    Func conv[num_conv];
    RDom r[num_conv];
    for (size_t i=0; i<num_conv; ++i) {
      std::string ii = to_string(i);
      kernel[i] = Func("kernel"+ii+suffix);
      conv[i] = Func("conv"+ii+suffix);
      r[i] = RDom(0, ksizes.at(i), 0, ksizes.at(i));
    }
    Func hw_input("hw_input"+suffix), hw_output("hw_output"+suffix), output("output"+suffix);
    Func conv_init("conv_init"+suffix), conv_last("conv_last"+suffix);
    Func k_init("k_init"+suffix), k_last("k_last"+suffix);
    RDom r_init(0, initk, 0, initk), r_last(0, lastk, 0, lastk);

    Var x("x"), y("y");
    Var xi("xi"), yi("yi"), xo("xo"), yo("yo");

    hw_input(x, y) = x + y;

    k_init(x, y) = 3*x + y;
    conv_last(x, y) = 0;
    
    conv_init(x, y) = 0;
    conv_init(x, y) += hw_input(x+r_init.x, y+r_init.y) * k_init(r_init.x, r_init.y);
    
    for (size_t i=0; i<num_conv; ++i) {
      kernel[i](x, y) = Expr(7*i) + 5*x + y;
      conv[i](x, y) += conv_init(x+r[i].x, y+r[i].y) * kernel[i](r[i].x, r[i].y);
      conv_last(x, y) += cast<int32_t>(conv[i](x + r_last.x, y + r_last.y) * Expr(i));
    }

    //k_last(x, y) = 3*y + x;
    //hw_output(x, y) = conv_last(x+r_last.x, y+r_last.y) * k_last(r_last.x, r_last.y);
    hw_output(x, y) = conv_last(x, y);
    output(x, y) = hw_output(x, y);

    //// Schedule ////
    output.bound(x, 0, imgsize);
    output.bound(y, 0, imgsize);
    hw_output.compute_root();
          
    hw_output.tile(x,y, xo,yo, xi,yi, imgsize, imgsize)
      .hw_accelerate(xi, xo);

    for (uint i=0; i < num_conv; ++i) {
      conv[i].store_at(hw_output, xo).compute_at(hw_output, xi);
      kernel[i].compute_at(hw_output, xo);
      conv[i].update().unroll(r[i].x).unroll(r[i].y);
		}

    k_init.compute_at(hw_output, xo);
    conv_init.store_at(hw_output, xo).compute_at(hw_output, xi);
    conv_init.update().unroll(r_init.x).unroll(r_init.y);

    conv_last.store_at(hw_output, xo).compute_at(hw_output, xi);
    
    hw_input.store_at(hw_output, xo).compute_at(conv_init, x);
    hw_input.stream_to_accelerator();

    //// Run through compiler and find hardware buffer
    auto hwxcels = lower_to_hwbuffer({output.function()}, "convdiverge_test",
                                     Target().with_feature(Target::CoreIR),
                                     {output.infer_arguments()});

    h_assert(hwxcels.size() == 1, "Incorrect number of xcels found");
    auto xcel = hwxcels.at(0);
    
    // number of convs: hw_input, k_init,conv_init, 2*(k_conv, conv_mid), conv_last,hw_output
    size_t num_hwbuffers = 1 + 2 + 2*num_conv + 2;
    h_assert(xcel.hwbuffers.size() == num_hwbuffers, "Incorrect number of hwbuffers found: " + 
             to_string(xcel.hwbuffers.size()) + " vs " + to_string(num_hwbuffers));
    h_assert(xcel.hwbuffers.count("hw_input" + suffix) == 1, "Can't find hwbuffer named hw_input");
    std::cout << "done with hwbuffer creation of forked" << suffix << "\n";
      
    //// Create ref buffer and check the hardware buffers
    vector<string> buffer_names = vector<string>(num_conv + 2);
    for (size_t i=0; i<num_conv+2; ++i) {
      string hwbuffer_name =
        i==0 ? "hw_input" + suffix :
        i==1 ? "conv_init" + suffix :
        "conv" + to_string(i-2) + suffix;
      buffer_names[i] = hwbuffer_name;
    }

    for (size_t i=0; i<num_conv+2; ++i) {
      string hwbuffer_name = buffer_names.at(i);
      string producer_name =
        i==0 ? "" :
        i==1 ? buffer_names.at(0) :
        buffer_names.at(1);
      string consumer_name =
        i==0 ? buffer_names.at(1) :
        i==1 ? buffer_names.at(2) :
        "conv_last"+suffix;
      
      h_assert(xcel.hwbuffers.count(hwbuffer_name) == 1, "Can't find hwbuffer named " + hwbuffer_name);
      auto hwbuffer = xcel.hwbuffers.at(hwbuffer_name);

      int max_conv_size = 0;
      for (size_t j=0; j<num_conv; ++j) {
        max_conv_size = std::max(ksizes.at(j), max_conv_size);
      }
      int ref_logsize =
        i==0 ? imgsize + (lastk-1) + (max_conv_size-1) + (initk-1) :
        i==1 ? imgsize + (lastk-1) + (max_conv_size-1) :
        imgsize + (lastk-1);

      int ksize =
        i==0 ? initk :
        i==1 ? max_conv_size :
        lastk;
      auto dims = create_hwbuffer_sizes({ref_logsize, ref_logsize},
                                        {ksize, ksize}, {ksize, ksize},
                                        {1, 1}, {1, 1});
      auto addrs = create_linear_addr({ref_logsize, ref_logsize},
                                      {1, 1}, {0, 1});
      vector<string> loops;
      vector<string> loopvars = {".xo", ".s0.y.yi", ".s0.x.xi"};
      for (auto loopvar : loopvars) {
        if (i == 0) {
          loops.emplace_back("hw_output" + suffix + loopvar.substr(0));
        } else {
          loops.emplace_back("hw_output" + suffix + loopvar);
        }
      }
      int store_index = 0; //i==0 ? 0 : 0;
      int compute_index = 2;

      HWBuffer ref_hwbuffer = HWBuffer(hwbuffer_name, dims, addrs,
                                       loops, store_index, compute_index,
                                       false, false,
                                       producer_name, consumer_name);
      int output_value = check_hwbuffer_params(hwbuffer, ref_hwbuffer);
      if (output_value != 0) { return output_value; }
    }
    
    return 0;
}

int doublebuffer_pipeline_hwbuffer_test(vector<int> ksizes, int imgsize, int tilesize) {
    std::string suffix = "_db_";
    for (auto ksize : ksizes) {
      suffix += to_string(ksize) + "_";
    }
    suffix += to_string(imgsize) + "_" + to_string(tilesize);

    size_t num_conv = ksizes.size();
    Func kernel[num_conv];
    Func conv[num_conv];
    RDom r[num_conv];
    for (size_t i=0; i<num_conv; ++i) {
      std::string ii = to_string(i);
      kernel[i] = Func("kernel"+ii+suffix);
      conv[i] = Func("conv"+ii+suffix);
      r[i] = RDom(0, ksizes.at(i), 0, ksizes.at(i));
    }
    Func hw_input("hw_input"+suffix), hw_output("hw_output"+suffix), output("output"+suffix);
    Var x("x"), y("y");
    Var xi("xi"), yi("yi"), xo("xo"), yo("yo");

    hw_input(x, y) = x + y;
    
    for (size_t i=0; i<num_conv; ++i) {
      kernel[i](x, y) = Expr(7*i) + 5*x + y;

      if (i > 0) {
        conv[i](x, y) += conv[i-1](x+r[i].x, y+r[i].y) * kernel[i](r[i].x, r[i].y);
      } else {
        conv[i](x, y) += hw_input(x+r[i].x, y+r[i].y) * kernel[i](r[i].x, r[i].y);
      }
    }

    hw_output(x, y) = conv[num_conv-1](x, y);
    output(x, y) = hw_output(x, y);

    //// Schedule ////
    output.bound(x, 0, imgsize);
    output.bound(y, 0, imgsize);
    hw_output.compute_root();
          
    hw_output.tile(x,y, xo,yo, xi,yi, tilesize, tilesize)
      .hw_accelerate(xi, xo);

    //hw_input.store_at(hw_output, xo).compute_at(conv[0], x);
    hw_input.store_at(hw_output, xo).compute_at(hw_output, xi);
    hw_output.bound(x, 0, imgsize);
    hw_output.bound(y, 0, imgsize);

    for (uint i=0; i < num_conv; ++i) {
      conv[i].store_at(hw_output, xo).compute_at(hw_output, xo);
      kernel[i].compute_at(hw_output, xo);
      conv[i].update().unroll(r[i].x).unroll(r[i].y);
		}

    hw_input.stream_to_accelerator();


    //// Run through compiler and find hardware buffer
    auto hwxcels = lower_to_hwbuffer({output.function()}, "doublebuffer_test",
                                     Target().with_feature(Target::CoreIR),
                                     {output.infer_arguments()});

    h_assert(hwxcels.size() == 1, "Incorrect number of xcels found");
    auto xcel = hwxcels.at(0);
    h_assert(xcel.hwbuffers.size() == 2 + 2*num_conv, "Incorrect number of hwbuffers found");
    h_assert(xcel.hwbuffers.count("hw_input" + suffix) == 1, "Can't find hwbuffer named hw_input");
    std::cout << "done with hwbuffer creation of doublebuffer" << suffix << "\n";
      
    //// Create ref buffer and check the hardware buffers
    vector<string> buffer_names = vector<string>(num_conv);
    for (size_t i=0; i<num_conv; ++i) {
      string hwbuffer_name = i==0 ? "hw_input" + suffix : "conv" + to_string(i-1) + suffix;
      buffer_names[i] = hwbuffer_name;
    }
    
    for (size_t i=0; i<num_conv; ++i) {
      string hwbuffer_name = buffer_names.at(i);
      string producer_name = i==0 ? "" : buffer_names.at(i-1);
      string consumer_name = i==num_conv-1 ? "conv"+std::to_string(i)+suffix : buffer_names.at(i+1);
      h_assert(xcel.hwbuffers.count(hwbuffer_name) == 1, "Can't find hwbuffer named " + hwbuffer_name);
      auto hwbuffer = xcel.hwbuffers.at(hwbuffer_name);
      
      int ref_logsize = tilesize;
      for (size_t j=i; j<num_conv; ++j) {
        ref_logsize += ksizes.at(j) - 1;
      }
      int ksize = ksizes.at(i);
      auto dims = create_hwbuffer_sizes({ref_logsize, ref_logsize},
                                        {ksize, ksize}, {ksize, ksize},
                                        {1, 1}, {1, 1});
      int range = ref_logsize - (ksizes.at(i) - 1); // range does not include the last conv size
      auto addrs = create_linear_addr({range, range},
                                      {1, 1}, {0, 1});
      vector<string> loops;
      vector<string> loopvars = {".xo", ".s0.y.yi", ".s0.x.xi"};
      for (auto loopvar : loopvars) {
        if (i == 0) {
          loops.emplace_back("hw_output" + suffix + loopvar.substr(0));
        } else {
          loops.emplace_back("hw_output" + suffix + loopvar);
        }
      }
      int store_index = 0; //i==0 ? 0 : 0;
      int compute_index = 2;

      HWBuffer ref_hwbuffer = HWBuffer(hwbuffer_name, dims, addrs,
                                       loops, store_index, compute_index,
                                       false, false,
                                       producer_name, consumer_name);
      int output_value = check_hwbuffer_params(hwbuffer, ref_hwbuffer);
      if (output_value != 0) { return output_value; }
    }
    
    return 0;
}

}  // namespace

int main(int argc, char **argv) {

    printf("Running conv hwbuffer tests\n");
    printf("    checking hwbuffers...\n");

    //if (conv_hwbuffer_test(1, 64) != 0) { return -1; }
    //if (conv_hwbuffer_test(2, 64) != 0) { return -1; }
    if (conv_hwbuffer_test(3, 64) != 0) { return -1; }
    //if (conv_hwbuffer_test(5, 64) != 0) { return -1; }
    //if (conv_hwbuffer_test(3, 16) != 0) { return -1; }
    //if (conv_hwbuffer_test(3, 32) != 0) { return -1; }
    //if (conv_hwbuffer_test(3, 19) != 0) { return -1; }

    printf("Running conv chain hwbuffer tests\n");
    printf("    checking hwbuffers...\n");

    //if (pipeline_hwbuffer_test({1, 1}, 64) != 0) { return -1; }
    //if (pipeline_hwbuffer_test({7, 5, 2}, 64) != 0) { return -1; }
    if (pipeline_hwbuffer_test({5, 3}, 64) != 0) { return -1; }
    //if (pipeline_hwbuffer_test({1, 4}, 64) != 0) { return -1; }
    //if (pipeline_hwbuffer_test({3, 3, 3}, 64) != 0) { return -1; }

    printf("Running tiled conv chain hwbuffer tests\n");
    printf("    checking hwbuffers...\n");
    
    //if (tiled_pipeline_hwbuffer_test({3}, 64, 32) != 0) { return -1; }
    //if (tiled_pipeline_hwbuffer_test({7, 3, 5, 2}, 64, 16) != 0) { return -1; }

    printf("Running forked conv hwbuffer tests\n");
    printf("    checking hwbuffers...\n");

    //if (forked_pipeline_hwbuffer_test(3, {1, 1}, 3, 64) != 0) { return -1; }
    //if (forked_pipeline_hwbuffer_test(3, {3, 3}, 3, 64) != 0) { return -1; }
    //if (forked_pipeline_hwbuffer_test(5, {4, 3}, 2, 64) != 0) { return -1; }

    printf("Running compute level hwbuffer tests\n");
    printf("    checking hwbuffers...\n");
    if (tiled_pipeline_hwbuffer_test({3, 4}, 64, 32) != 0) { return -1; }
    // double buffer
    // something in between

    printf("Running loop reordering hwbuffer tests\n");
    printf("    checking hwbuffers...\n");
    // no reorder
    // reordering with loops unrolled
    // something in between
    
    printf("Running sampling hwbuffer tests\n");
    printf("    checking hwbuffers...\n");
    // downsample
    // upsample
    // up and down sampling

    printf("Running multi-pixel hwbuffer tests\n");
    printf("    checking hwbuffers...\n");
    // 1 pixel/cycle
    // 2 pixels/cycle
    
    printf("Running rolled hwbuffer tests\n");
    printf("    checking hwbuffers...\n");
    // input block equal to input chunk
    // row of stencil at a time (3 cycles)
    // pixel of stencil at a time (9 cycles)
    
    printf("Success!\n");
    return 0;
}
