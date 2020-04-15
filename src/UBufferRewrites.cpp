#include "UBufferRewrites.h"
#include "ubuf_coreirsim.h"
#include "lakelib.h"

#include "IRMutator.h"
#include "RemoveTrivialForLoops.h"
#include "Simplify.h"
#include "UnrollLoops.h"

#include "HWBuffer.h"
#include "CoreIR_Libs.h"

#include "coreir.h"
#include "coreir/libs/commonlib.h"
using namespace CoreIR;
using namespace std;

namespace Halide {
  namespace Internal {

  enum class IO_TYPE{READ, WRITE};

  class MemoryConstraints {
    public:
      HWBuffer ubuf;
      string name;
      map<string, const Provide*> write_ports;
      map<string, const Call*> read_ports;
      map<string, vector<string> > read_stream_bundle;

      //starting address
      map<string, vector<int>> read_ports_offset;
      map<string, vector<int>> write_ports_offset;

      // This is an ordinal schedule, it does not
      // describe cycle accurate timing of inputs and
      // outputs
      map<string, StmtSchedule> schedules;


      static bool cmp(const pair<string, const Call* > pp1, const pair<string,const Call* > pp2) {
          auto p1 = pp1.second->args;
          auto p2 = pp2.second->args;
          cout << p1.size() << ", " << p2.size() <<endl;
          cout << pp1.first << ", " << pp2.first << endl;
          internal_assert(p1.size() == p2.size()) << "two compared vector has different size!\n";
          int sum = 0;
          for (size_t i = 0; i < p1.size(); i ++) {
              int diff = id_const_value(simplify(p1[i] - p2[i]));
              sum += diff;
          }
          return sum <= 0;
      }

      //Use a more generic way to deduce output starting address
      map<string, vector<int> > get_port_offset(map<string, const Call*> ports) {

          map<string, vector<int> > ports_offset;

          vector<pair<string, const Call*> > vec(ports.begin(), ports.end());
          sort(vec.begin(), vec.end(), cmp);

          vector<Expr> baseArgs = vec.begin()->second->args;
          cout <<"BASE Arg:" << baseArgs << endl;
          for (auto pt : ports) {
              vector<Expr> args = pt.second->args;
              auto call_name = pt.second->name;
              std::cout << "read ports: "  << pt.first <<"\nname:" << call_name << args << endl;
              vector<int> offsets;

              for (size_t i = 0; i < baseArgs.size(); i++) {
                cout << "simplify offset :" << (simplify(args[i] - baseArgs[i])) <<id_const_value(simplify(args[i] - baseArgs[i])) << endl;
                map_insert(ports_offset, pt.first, id_const_value(simplify(args[i] - baseArgs[i])));

              }

          }
          return ports_offset;
      }


      bool is_write(const std::string& pt) const {
        return contains_key(pt, write_ports);
      }

      bool is_read(const std::string& pt) const {
        return contains_key(pt, read_ports);
      }

      set<string> port_names() const {
        auto names = domain(write_ports);
        for (auto n : domain(read_ports)) {
          names.insert(n);
        }
        return names;
      }

      StmtSchedule schedule(const std::string& port_name) const {
        return map_find(port_name, schedules);
      }

      StmtSchedule loop_schedule(const std::string& port_name) const {
        auto s = map_find(port_name, schedules);
        StmtSchedule ls;
        for (auto r : s) {
          if (!r.is_const()) {
            ls.push_back(r);
          }
        }
        return ls;
      }

      vector<StmtSchedule> write_loop_levels() const {
        vector<StmtSchedule> ls;
        for (auto r : write_ports) {
          auto s = loop_schedule(r.first);
          if (!elem(s, ls)) {
            ls.push_back(s);
          }
        }
        return ls;
      }

      vector<StmtSchedule> read_loop_levels() const {
        vector<StmtSchedule> ls;
        for (auto r : read_ports) {
          auto s = loop_schedule(r.first);
          if (!elem(s, ls)) {
            ls.push_back(s);
          }
        }
        return ls;
      }

      vector<StmtSchedule> write_levels() const {
        vector<StmtSchedule> ls;
        for (auto r : write_ports) {
          auto s = schedule(r.first);
          if (!elem(s, ls)) {
            ls.push_back(s);
          }
        }
        return ls;
      }
      vector<StmtSchedule> read_levels() const {
        vector<StmtSchedule> ls;
        for (auto r : read_ports) {
          auto s = schedule(r.first);
          if (!elem(s, ls)) {
            ls.push_back(s);
          }
        }
        return ls;
      }

      bool lt(const VarSpec& a, const VarSpec& b) const {
        internal_assert(a.is_const() && b.is_const());
        return a.const_value() < b.const_value();
      }

      std::vector<Expr> port_address_stream(const std::string& str) const {
        vector<Expr> addrs;
        if (contains_key(str, write_ports)) {
          for (auto arg : map_find(str, write_ports)->args) {
            addrs.push_back(arg);
          }
        } else {
          internal_assert(contains_key(str, read_ports));
          for (auto arg : map_find(str, read_ports)->args) {
            addrs.push_back(arg);
          }
        }
        return addrs;
      }

      StmtSchedule port_schedule(const std::string& str) const {
        internal_assert(contains_key(str, schedules));
        return map_find(str, schedules);
      }
  };

  class FuncOpCollector : public IRGraphVisitor {
    public:
      using IRGraphVisitor::visit;

      vector<VarSpec> activeVars;
      string provide_name;
      int next_level;
      map<string, vector<const Provide*> > provides;
      map<string, vector<const Call*> > calls;
      map<const Provide*, StmtSchedule> write_scheds;
      map<const Call*, StmtSchedule> read_scheds;
      map<const Call*, string> read_consumer;

      FuncOpCollector() : activeVars({{"", Expr(0), Expr(1)}}), next_level(1) {}

      map<string, MemoryConstraints> hwbuffers() {
        map<string, MemoryConstraints> bufs;
        set<string> buffer_names = domain<string, vector<const Call*> >(calls);
        for (auto n : domain(provides)) {
          buffer_names.insert(n);
        }

        // Need to create port names for each read / write port

        for (auto b : buffer_names) {
          map<string, StmtSchedule> schedules;
          int wr_num = 0;
          map<string, const Provide*> writes;
          for (auto rd : provides) {
            if (rd.first == b) {

              //first remove the const initialization
              //FIXME: a better way is rewrite IR
              size_t offset = 0;
              for (auto rdOp : rd.second) {
                if (rdOp->values.size() == 1)
                    if(id_const_value(rdOp->values.front()) != -1) {
                        //initial a const value also remove one out port
                        auto & call_list = calls.at(b);
                        call_list.erase(call_list.begin());
                        rd.second.erase(rd.second.begin() + offset);
                        break;
                    }
                offset ++;
              }

              for (auto rdOp : rd.second) {
                writes["write_port_" + to_string(wr_num)] = rdOp;
                schedules["write_port_" + to_string(wr_num)] = map_find(rdOp, write_scheds);
                wr_num++;
              }
            }
          }
          int rd_num = 0;
          map<string, const Call*> reads;
          map<string, vector<string>> reads_stream_bundle;
          for (auto rd : calls) {
            if (rd.first == b) {
              for (auto rdOp : rd.second) {
                string pt_name = "read_port_" + to_string(rd_num);
                reads[pt_name] = rdOp;
                schedules[pt_name] = map_find(rdOp, read_scheds);
                map_insert(reads_stream_bundle, map_find(rdOp, read_consumer), pt_name);
                rd_num++;
              }
            }
          }
          bufs[b] = {{}, b, writes, reads, reads_stream_bundle, {}, {}, schedules};
          cout << "Add buffer constraints for " << b << endl;
        }
        return bufs;
      }

      Expr last_provide_to(const std::string& name, const vector<Expr>& args) const {
        vector<const Provide*> ps = map_find(name, provides);
        CoreIR::reverse(ps);
        for (auto p : ps) {
          bool args_match = true;
          internal_assert(p->args.size() == args.size());
          for (size_t i = 0; i < p->args.size(); i++) {
            auto pa = p->args.at(i);
            auto a = args.at(i);
            if (!is_const(pa) || !is_const(a)) {
              args_match = false;
              break;
            }
            if (id_const_value(pa) != id_const_value(a)) {
              args_match = false;
              break;
            }
          }

          if (args_match) {
            internal_assert(p->values.size() == 1);
            return p->values.at(0);
          }
        }

        internal_assert(false) << "Could not find provide to location: " << name << "\n";
          //<< args << end;
        return args[0];
      }

      void visit(const For* f) override {
        activeVars.push_back({f->name, f->min, f->extent});
        push_level();
        f->body.accept(this);
        pop_level();
        activeVars.pop_back();
      }

      void inc_level() {
        auto back = activeVars.back();
        internal_assert(is_const(back.min)) << back.min << " is not const\n";
        activeVars[activeVars.size() - 1] = {"", simplify(back.min + 1), 1};
      }

      void push_level() {
        activeVars.push_back({"", Expr(0), Expr(1)});
        next_level = 1;
      }

      void pop_level() {
        activeVars.pop_back();
        next_level = 1;
      }
      void visit(const Provide* p) override {
        //track the string name
        provide_name = p->name;
        //cout << "provide name: " << p->name << p->values << endl;
        cout << Stmt(p) << endl;

        IRGraphVisitor::visit(p);

        inc_level();

        //FIXME: hacky solution for local accumulation
        if (provides.find(p->name) != provides.end()) {
            for (auto pd: provides.at(p->name)) {
                //cout << "check provide: " << Stmt(pd) << endl;
                bool is_same = true;
                for (size_t i = 0; i < std::min(pd->args.size(), p->args.size()); i ++) {
                  auto target_expr = pd->args[i];
                  auto cur_expr = p->args[i];
                  is_same &= (id_const_value(simplify(target_expr -cur_expr)) == 0);
                }
                if (is_same) {
                    //cout << "Update the same location" << endl;
                    return;
                }
            }
        }

        map_insert(provides, p->name, p);
        write_scheds[p] = activeVars;
      }

      void visit(const Call* p) override {
        if (p->call_type == Call::CallType::Halide) {
          inc_level();
          //cout << provide_name << "\tInsert consumer to map: " << p->name << "\n " << p->args << endl;
          map_insert(calls, p->name, p);
          read_consumer[p] = provide_name;
          read_scheds[p] = activeVars;
        }

        IRGraphVisitor::visit(p);
      }

      bool is_rom(const std::string& func) const {
        if (!contains_key(func, provides)) {
          // If we have no information about this function assume it is
          // not going to be stored in a rom
          return false;
        }

        auto ps = map_find(func, provides);
        for (auto p : ps) {
          for (auto v : p->values) {
            if (!is_const(v)) {
              return false;
            }
          }
          for (auto a : p->args) {
            if (!is_const(a)) {
              return false;
            }
          }
        }
        return true;
      }
  };

  class ROMLoadFolder : public IRMutator {
    public:

      FuncOpCollector mic;

      using IRMutator::visit;

      Stmt visit(const AssertStmt* ld) override {
        return Evaluate::make(Expr(0));
      }

      Expr visit(const Call* ld) override {

        Expr newLd = IRMutator::visit(ld);

        if (ld->call_type != Call::CallType::Halide) {
          return newLd;
        }
        if (mic.is_rom(ld->name)) {
          cout << "found call to rom: " << ld->name << endl;
          //internal_assert(false) << ld->name << " is rom!\n";
          bool all_const_addr = true;
          for (auto a : ld->args) {
            if (!is_const(a)) {
              cout << "\targument: " << a << " is not const" << endl;
              all_const_addr = false;
              break;
            }
          }

          if (all_const_addr) {
            return mic.last_provide_to(ld->name, ld->args);
          }
        }

        return newLd;
      }

  };
/*
  class PortMap {

  };
*/
  class ShiftReg {
      public:
          int port_number;
          vector<int> size;
          //vector<int> depth;
          int stencil_valid_depth;

          vector<int> stride;
          vector<int> stride_ref_dim;
          vector<int> range;

          //map from depth to name
          //vector<tuple<string, int>> port_map;
          map<int, string> port_map;
          int num_child_ports;

          map<string, ShiftReg> child;

          //use for generate coreir save this pointer
          CoreIR::ModuleDef* def;

          ShiftReg(){port_number = 0;}
          ShiftReg(CoreIR::ModuleDef* def_in):port_number(0), def(def_in){}

          ShiftReg(CoreIR::ModuleDef* ubuf_def,
                  vector<int> range_in, vector<int> stride_in, vector<int> stride_ref_dim_in,
                  vector<tuple<string, int>> port_map_in, map<string, ShiftReg> HWTree, int addr_dim, int ref_dim) {
              def = ubuf_def;
              size = vector<int>(addr_dim, 0);
              range = range_in;
              stride = stride_in;
              stride_ref_dim = stride_ref_dim_in;
              size[stride_ref_dim[ref_dim]] = stride[stride_ref_dim[ref_dim]];
              for (const auto &port_depth_tuple : port_map_in) {
                  //use map to sort
                  port_map.insert(make_pair(get<1>(port_depth_tuple), get<0>(port_depth_tuple)));
              }
              for (auto it : port_map) {
                  string key = it.second;
                  child.insert(std::make_pair(key, HWTree[key]));
              }
              port_number = port_map.size();
              stencil_valid_depth = 0;
              for(auto it: port_map) {
                  int depth = it.first;
                  stencil_valid_depth = std::max(stencil_valid_depth, depth);
              }
          }

          void recursive_update_range(int add_depth, size_t loop_dim) {
              if (port_number == 0)
                  return;
              cout << "rewrite range for reg" << endl;
              range[loop_dim] += add_depth;
              for (auto & it : child) {
                  it.second.recursive_update_range(add_depth, loop_dim);
              }
          }

          void recursive_wire_valid(Wireable* stencil_valid, string rp) {
              if (port_number == 0){
                  def->connect(stencil_valid, def->sel("self")->sel(rp+"_valid"));
                  return;
              }
              else {
                  for (auto & it : child) {
                      it.second.recursive_wire_valid(stencil_valid, it.first);
                  }
              }
          }

          Values generate_ubuf_args(string port_name, int stencil_valid_depth, int width, int flatten_size) {
              auto context = def->getContext();
              json out_start;
              out_start["output_start"][0] = 0;
              json logical_size;
              logical_size["capacity"][0] = flatten_size;
              json input_chunk;
              json output_stencil;
              input_chunk["input_chunk"][0] = flatten_size;
              output_stencil["output_stencil"][0] = flatten_size;
              Values args =
                      {{"width", Const::make(context, width)},
                      {"stencil_width", Const::make(context, stencil_valid_depth)},
                      {"depth", Const::make(context, flatten_size)},
                      {"chain_idx", Const::make(context, 0)},
                      {"rate_matched", Const::make(context, false)},
                      {"chain_en", Const::make(context, false)},
                      {"dimensionality", Const::make(context, 1)},
                      {"input_stride_0", Const::make(context, 1)},
                      {"input_range_0", Const::make(context, flatten_size)},
                      {"stride_0", Const::make(context, 1)},
                      {"range_0", Const::make(context, flatten_size)},
                      {"iter_cnt", Const::make(context, flatten_size)},
                      {"logical_size", Const::make(context, logical_size)},
                      {"input_chunk", Const::make(context, input_chunk)},
                      {"output_stencil", Const::make(context, output_stencil)},
                      {"output_starting_addrs", Const::make(context, out_start)},
                      {"num_stencil_acc_dim", Const::make(context, 1)}};
              return args;
          }

          Wireable* generate_coreir(pair<Wireable*, Wireable*> input_pair, string rp,  vector<int> flatten_vec) {
              /* The shift register always have only one input and depth number of output
               * The first port will simply pass through, and the other node will go through
               * different modules, either a row buffer or a shift register,
               * when its read port number = 0, it will be wire to the true output port
               *
               * input_pair.first: data path
               * input_pair.second: control path
               *
               * return: The stencil valid signal for current level
               */
              if (port_number == 0) {
                  //you reach the leaf node, connect to the port
                  auto read_data = def->sel("self")->sel(rp);
                  def->connect(input_pair.first, read_data);
                  //connect the valid later
                  return nullptr;
              }

              int width = 16;
              auto context = def->getContext();
              //add the module of shift reg
              int flatten_size = std::inner_product(size.begin(), size.end(), flatten_vec.begin(), 0);
              cout << "Flatten size = " << flatten_size << endl;
              Wireable* last_data_valid = input_pair.second;
              Wireable* last_data = input_pair.first;
              if (flatten_size == 1) {
                  //TODO: use port map to modify the depth
                  //possible bug: cold only support dilation = 1 convolution kernel
                  //add register
                  //add delay use a flag to decide whether use the stencil valid from tile
                  for (auto it: port_map) {
                      string port_name = it.second;
                      auto child_node = child[port_name];
                      if (port_name == rp) {
                          child_node.generate_coreir(input_pair, rp, flatten_vec);
                          continue;
                      }
                      cout << "visit port: " << port_name << endl;
                      //auto d0 = def->addInstance("d_reg" + context->getUnique(),"corebit.reg",
                      //       {{"width",Const::make(context,width)}});
                      //FIXME: possible bug, should use depth of the fifo
                      auto d0 = def->addInstance("d_reg" + context->getUnique(),"mantle.reg",{{"width",Const::make(context,16)},{"has_en",Const::make(context,false)}});
                      def->connect(d0->sel("in"), last_data);
                      last_data = d0->sel("out");

                      //use a dummy data valid since not use in register
                      child_node.generate_coreir(make_pair(last_data, last_data_valid), port_name, flatten_vec);


                  }
                  return nullptr;
              }
              else {
                  //add row buffer
                  for (auto it: port_map) {
                      string port_name = it.second;
                      auto child_node = child[port_name];
                      if (port_name == rp) {
                          cout << "visit port: " << port_name << endl;
                          child_node.generate_coreir(input_pair, rp, flatten_vec);
                      }
                      else{
                          cout << "visit port: " << port_name << endl;
                          bool last_port_flag = (port_name == port_map.rbegin()->second);

                          int stencil_valid_depth = 0;
                          if (last_port_flag)
                              stencil_valid_depth = child_node.stencil_valid_depth;

                          //FIXME: possible bug, should use depth of the fifo
                          auto args = generate_ubuf_args(port_name, stencil_valid_depth, width, flatten_size);
                          //add ubuffer and wiring
                          auto row_buffer_coreir = def->addInstance("row_buf_"+context->getUnique(),
                                  "lakelib.unified_buffer", args);
                          def->connect(last_data_valid, row_buffer_coreir->sel("wen"));
                          def->connect(last_data, row_buffer_coreir->sel("datain0"));

                          //update the last data and valid
                          last_data = row_buffer_coreir->sel("dataout0");
                          last_data_valid = row_buffer_coreir->sel("valid");

                          //generate coreIR for next level, ignore the inner level stencil valid
                          child_node.generate_coreir(make_pair(last_data, last_data_valid), port_name, flatten_vec);

                          //wire the reset and ren
                          //wire the peripherial
                          def->connect(row_buffer_coreir->sel("reset"), def->sel("self")->sel("reset"));
                          def->connect(row_buffer_coreir->sel("flush"), def->sel("self")->sel("flush"));

                          //TODO:use the same const
                          auto const_true = def->addInstance("ubuf_bitconst_"+context->getUnique(), "corebit.const", {{"value", COREMK(context, true)}});
                          def->connect(row_buffer_coreir->sel("ren"), const_true->sel("out"));

                          //check if the ret is NULL
                          if (last_port_flag) {
                              Wireable* next_level_stencil_valid = row_buffer_coreir->sel("valid");
                              return next_level_stencil_valid;
                          }
                      }
                  }
              }

              cout << "Should not reach this place" << endl;
              return nullptr;
          }

  };

  /*ostream& operator<<(ostream& os, const vector<int> & vec) {
      os << "[";
      for (const auto & val: vec) {
          os << val << " ";
      }
      os << "] ";
      return os;
  }*/

  ostream& operator<<(ostream& os, const ShiftReg& sr) {
      os << " Shift Register: size= " << sr.size << ", with " << sr.port_number << " port \n";;
      os << "range: " << sr.range << "\nstride: " << sr.stride << std::endl;
      for_each(sr.port_map.begin(), sr.port_map.end(), [&os](std::pair<int, string> port_info){
              os << "Port name = " << port_info.second  << ", Depth = " << port_info.first << endl;
              });
      for (const auto &[k, v] : sr.child)
              os << "child map: [" << k << "] = \n(" << v << ") " << std::endl;
      return os;
  }


  class RecursiveBuffer {
      public:
          map<string, vector<int>> start_addr;
          //save the source link and the distance
          map<string, std::tuple<string, int>> merge_addr;
          vector<int> stride;
          vector<int> stride_ref_dim;
          vector<int> range;
          size_t loop_dim;
          size_t addr_dim;
          map<string, ShiftReg> HWTree;
          CoreIR::ModuleDef * def;

    //RecursiveBuffer(const IdentifyAddressing id_addr, HWBuffer ubuf, CoreIR::ModuleDef * ubuf_def,
    RecursiveBuffer(const vector<AccessDimSize>& id_addr, HWBuffer ubuf, CoreIR::ModuleDef * ubuf_def,
                  const map<string, vector<int>> read_ports_offset) {

              //initialize a bunch of parameter
              def = ubuf_def;
              addr_dim = ubuf.ldims.size();
              loop_dim = id_addr.size();
              for (auto rg: id_addr)
                range.push_back(to_int(rg.range));
              for (auto st: id_addr)
                stride.push_back(to_int(st.stride));
              for (auto ref_dim: id_addr)
                stride_ref_dim.push_back(to_int(ref_dim.dim_ref));
              for (const auto it: read_ports_offset) {
                  start_addr[it.first] = it.second;
                  merge_addr[it.first] = std::make_tuple(it.first, 0);
                  HWTree[it.first] = ShiftReg(def);
              }

          }

          void reset_merge_addr() {
              merge_addr.clear();
              for (auto it: start_addr)
                  merge_addr[it.first] = std::make_tuple(it.first, 0);
          }

          bool find_overlap() {
              for (const auto it : merge_addr) {
                  if (it.first != std::get<0>(it.second)) {
                      return true;
                  }
              }
              return false;
          }

          //recursive update range of buffer if we add shift register
          //TODO: may not work for irregular starting address, aka non block case
          void recursive_update_range(map<string, int> depth_map, size_t loop_dim) {
              cout << "rewrite range" << endl;
              int range_add = depth_map.begin()->second;
              for (auto it: depth_map) {
                  //FIXME: enable this check when parameter extraction is correct
                  //internal_assert(range_add == it.second) << "Update range dimension is not same in port: " <<
                      //it.first <<" from loop dim: " << loop_dim;
              }
              range[loop_dim] += range_add;
              for (auto & it : HWTree) {
                  cout << "Port name: " << it.first << std::endl;
                  it.second.recursive_update_range(range_add, loop_dim);
              }
          }

          //recursive find the source port, union find source
          std::pair<string, int> find_source_and_depth(string port_name) {
              string source_name = std::get<0>(merge_addr[port_name]);
              int source_depth = std::get<1>(merge_addr[port_name]);
              if (std::get<0>(merge_addr[source_name]) == source_name) {
                  return make_pair(source_name, source_depth);
              }
              else{
                  auto ret = find_source_and_depth(source_name);
                  return make_pair(ret.first, ret.second + source_depth);
              }
          }

          void port_reduction(int threshold) {
              //make sure the threshold is less than the max of range
              for (size_t i = 0; i < loop_dim; i ++) {
                  int c = 1;
                  while (c <= threshold) {
                      for (const auto it: start_addr) {
                          auto temp = it.second;
                          temp[stride_ref_dim[i]] -= c * stride[i];

                          for (const auto overlap_port : start_addr) {
                              if (std::equal(temp.begin(), temp.end(), overlap_port.second.begin())) {
                                  auto & merge_port = merge_addr[overlap_port.first];
                                  if (std::get<0>(merge_port) == overlap_port.first){
                                      //this port is not merged under this loop dimension
                                      std::get<0>(merge_port) = it.first;
                                      std::get<1>(merge_port) = c;
                                  }
                                  else
                                      break;
                              }
                          }
                      }
                    c++;
                  }

                    for (const auto &[k, v] : merge_addr)
                        cout << "\nMERGED ADDR: [" << k << "] = \n(" << get<0>(v)<< ", " << get<1>(v) << ") " << std::endl;
                  if (find_overlap()) {

                      //create a new map
                      map<string, vector<int>> new_start_addr;
                      map<string, vector<tuple<string, int> > > merge_addr_gather;
                      map<string, ShiftReg> newTree;
                      for (auto & it: merge_addr) {
                          if (it.first == std::get<0>(it.second)) {
                              //find the source port
                              new_start_addr.insert(make_pair(it.first,  start_addr[it.first]));
                              merge_addr_gather[it.first].push_back(it.second);
                          }
                          else {
                              //if it's not the source port, update the merge port to be the source node
                              auto ret = find_source_and_depth(it.first);
                              std::get<0>(it.second) = ret.first;
                              std::get<1>(it.second) = ret.second;
                          }
                      }

                      cout << "After First Pass processing of the merge address" << endl;
                      for (const auto &[k, v] : merge_addr)
                        cout << "\nMERGED ADDR: [" << k << "] = \n(" << get<0>(v)<< ", " << get<1>(v) << ") " << std::endl;
                      for (auto it : merge_addr){
                          if (it.first != std::get<0>(it.second)) {
                              //all the source should be put in front of the merged node
                              internal_assert(merge_addr_gather.count(get<0>(it.second))) << "Source node " <<
                                get<0>(it.second) << "is not add to the merge list";
                              merge_addr_gather[get<0>(it.second)].push_back(make_tuple(it.first, get<1>(it.second)));
                          }
                      }

                      for (const auto &[k, v] : merge_addr_gather){
                        cout << "\nMERGED ADDR UNION: [" << k << "] = \n" ;
                        for (auto vv: v)
                            cout << "( " << get<0>(vv)<< ", " << get<1>(vv) << ") " << std::endl;
                      }

                      //save the stencil valid depth
                      map<string, int> depth_map;
                      for (auto it: new_start_addr) {
                          //create the shift reg structure for all the new output port
                          string port_name = it.first;
                          newTree[port_name] = ShiftReg(def, range, stride, stride_ref_dim, merge_addr_gather[port_name], HWTree, addr_dim, stride_ref_dim[i]);
                          depth_map[port_name] = newTree[port_name].stencil_valid_depth;
                      }


                      //mutate the old structure
                      start_addr = new_start_addr;
                      HWTree = newTree;
                      recursive_update_range(depth_map, i);
                      //update merge addr
                      reset_merge_addr();
                  }
              }

          }

          bool flatten_ubuf_to_wire(vector<int> l_size, vector<int> l_size_flatten) {
              for (size_t dim = 0; dim < loop_dim; dim ++) {
                  if(range[dim] != l_size[dim] || stride_ref_dim[dim] != (int)dim) {
                      return false;
                  }
              }
              return true;
          }

          void generate_coreir(MemoryConstraints & buffer) {

              auto self = def->sel("self");

              //wiring the input
              string wp = begin(buffer.write_ports)->first;
              auto wen = self->sel(wp + "_en");
              auto w_data = self->sel(wp);

              //prepare for the output wiring, map for port name to the wire data and valid
              map<string, pair<Wireable*, Wireable*>> port2wire;

              //initialize the flatten vector, the dot product doing the linearization
              vector<int> flatten_dim_vector;
              vector<int> dim_vector;
              cout << buffer.ubuf << endl;
              for (size_t idx =0 ; idx < loop_dim; idx ++) {
                  auto ref_dim = stride_ref_dim[idx];
                  cout << "logical size flatten: " << id_const_value(buffer.ubuf.ldims[ref_dim].logical_size_flatten) << endl;
                  flatten_dim_vector.push_back(id_const_value(buffer.ubuf.ldims[ref_dim].logical_size_flatten));
                  dim_vector.push_back(id_const_value(buffer.ubuf.ldims[ref_dim].logical_size));
              }

              bool flatten_buffer = flatten_ubuf_to_wire(dim_vector, flatten_dim_vector);
              cout << "flatten_buffer = " << flatten_buffer <<endl;
              if (!flatten_buffer) {
                  //TODO:figure out where does the datawidth saved
                  int width = 16;
                  int depth = std::accumulate(buffer.ubuf.ldims.begin(), buffer.ubuf.ldims.end(), 1,
                          [](int product, LogicalDimSize b){
                          return product * id_const_value(b.logical_size);
                          });
                  int iter_cnt = std::accumulate(range.begin(), range.end(), 1,
                          [](int a, int b){return a * b;});
                  auto context = def->getContext();

                  //initial the unified buffer parameter
                  Json logical_size;
                  Json in_chunk;
                  Json out_stencil;
                  Json out_start;
                  for (size_t i = 0; i < buffer.ubuf.ldims.size(); i ++){
                      logical_size["capacity"][i] = id_const_value(buffer.ubuf.ldims[i].logical_size);
                      in_chunk["input_chunk"][i] = id_const_value(buffer.ubuf.dims[i].input_chunk);
                      out_stencil["output_stencil"][i] = id_const_value(buffer.ubuf.dims[i].output_stencil);
                  }

                  //TODO: make the flatten to be another pass
                  size_t read_port_cnt = 0;
                  for (const auto rp_offset: start_addr) {
                      const vector<int> offset_vec = rp_offset.second;
                      int start_addr = std::inner_product(offset_vec.begin(), offset_vec.end(), flatten_dim_vector.begin(), 0);
                      cout << "flatten starting address: " << start_addr << "=" << offset_vec << "x"<< flatten_dim_vector<< endl;
                      //output_start_addrs[read_port_cnt] = start_addr;
                      out_start["output_start"][read_port_cnt] = start_addr;
                      read_port_cnt ++;

                  }


                  cout << "Start creating coreIR instance" << endl;

                  Values args =
                          {{"width", Const::make(context, width)},
                          {"num_input_ports", Const::make(context, buffer.write_ports.size())},
                          {"num_output_ports", Const::make(context, buffer.read_ports.size())},
                          {"stencil_width", Const::make(context, 0)},
                          {"depth", Const::make(context, depth)},
                          {"chain_idx", Const::make(context, 0)},
                          {"rate_matched", Const::make(context, false)},
                          {"chain_en", Const::make(context, false)},
                          {"dimensionality", Const::make(context, range.size())},
                          {"iter_cnt", Const::make(context, iter_cnt)},
                          {"logical_size", Const::make(context, logical_size)},
                          {"input_chunk", Const::make(context, in_chunk)},
                          {"output_stencil", Const::make(context, out_stencil)},
                          {"num_stencil_acc_dim", Const::make(context, 0)},
                          {"output_starting_addrs", Const::make(context, out_start)}};

                  for (size_t dim = 0; dim < addr_dim; dim ++) {
                      args["input_range_" + to_string(dim)] =  Const::make(context, id_const_value(buffer.ubuf.ldims[dim].logical_size));
                      args["input_stride_" + to_string(dim)] = Const::make(context, id_const_value(buffer.ubuf.ldims[dim].logical_size_flatten));
                  }

                  for (size_t dim = 0; dim < loop_dim ; dim ++) {
                      args["range_" + to_string(dim) ] = Const::make(context, range[dim]);
                      args["stride_" + to_string(dim) ] = Const::make(context, stride[dim]
                                  * id_const_value(buffer.ubuf.ldims[stride_ref_dim[dim]].logical_size_flatten));
                  }

                  auto ubuf_coreir = def->addInstance("ubuf_"+context->getUnique(),
                          "lakelib.unified_buffer", args);

                  //wiring the write port
                  def->connect(ubuf_coreir->sel("datain0"), w_data);
                  def->connect(ubuf_coreir->sel("wen"), wen);

                  //update wen
                  wen = ubuf_coreir->sel("valid");

                  //push the output wire to the port map for 2nd wiring pass;
                  int port_cnt = 0;
                  for (auto rp : start_addr){
                      port2wire[rp.first] = make_pair(ubuf_coreir->sel("dataout" + std::to_string(port_cnt)),
                              ubuf_coreir->sel("valid"));
                  }

                  //wire the peripherial
                  def->connect(ubuf_coreir->sel("reset"), self->sel("reset"));
                  def->connect(ubuf_coreir->sel("flush"), self->sel("flush"));

                  //TODO: Not sure if this is robust, hacky solution for ren
                  auto const_true = def->addInstance("ubuf_bitconst_"+context->getUnique(), "corebit.const", {{"value", COREMK(context, true)}});
                  def->connect(ubuf_coreir->sel("ren"), const_true->sel("out"));

              }
              //get all the port need to be wire for the coreIR mmodule
              else {
                  for (auto rp : start_addr)
                      port2wire[rp.first] = make_pair(w_data, wen);
              }
              cout << "Wiring the output port!" << std::endl;

              //wiring the output and output valid
              for (auto const & rp : start_addr){
                  //Wireable* read_data = port2wire[rp.first];
                  //Wireable* read_valid = self->sel(rp.first + "_valid");
                  cout << "wiring port: " << rp.first << endl;
                  Wireable* valid_wire = HWTree[rp.first].generate_coreir(port2wire[rp.first], rp.first, flatten_dim_vector);
                  if (valid_wire) {
                      //TODO: heruistic: all the output share the same valid, may be not true for the future
                      //need a second pass to wire all the stencil valid signal
                      //in current memory tile ther is only one valid port
                      HWTree[rp.first].recursive_wire_valid(valid_wire, rp.first);
                  }
                  else {
                      def->connect(wen, self->sel(rp.first+"_valid"));
                  }
            }

              cout << "finish wiring!" << endl;

          }

  };

  ostream& operator<<(ostream& os, const RecursiveBuffer& rb) {
      os << "start addr info: \n";
      os << "range: " << rb.range << "\nstride: " << rb.stride << std::endl;
      for_each(rb.start_addr.begin(), rb.start_addr.end(), [&os](std::pair<string, vector<int>> it) {
              os << "port name = " << it.first << ", start pos = " << it.second << std::endl;
              });
      for (const auto &[k, v] : rb.HWTree)
              os << "child map: [" << k << "] = (" << v << ") " << std::endl;
      return os;
  }


  void synthesize_ubuffer(CoreIR::ModuleDef* def, MemoryConstraints& buffer) {
    cout << "ubuffer parameter" << buffer.ubuf ;

    if (buffer.read_ports.size() == 0) {
      // Buffer is write only, so it has no effect
      return;
    }
    auto self = def->sel("self");

    bool use_lake_lib = true;
    if (use_lake_lib) {
        cout << buffer.read_loop_levels()[0] << "\t"<< buffer.write_loop_levels()[0] << endl;

        if (buffer.read_loop_levels()[0] == buffer.write_loop_levels()[0]) {
          set<string> ports = buffer.port_names();
          vector<string> port_list(begin(ports), end(ports));
          sort(begin(port_list), end(port_list), [buffer](const std::string& pta, const std::string& ptb) {
              return buffer.lt(buffer.schedule(pta).back(), buffer.schedule(ptb).back());
          });

          cout << "Buffer: " << buffer.name << " is all wires!" << endl;
          string last_pt = port_list[0];
          internal_assert(buffer.is_write(last_pt)) << "Buffer: " << buffer.name << " is read before it is written?\n";

          for (size_t i = 1; i < port_list.size(); i++) {
            string next_pt = port_list[i];

            if (buffer.is_write(next_pt)) {
              auto last_cntrl = def->sel("self." + last_pt + "_" + (buffer.is_write(last_pt) ? "en" : "valid"));
              auto last_data = def->sel("self." + last_pt);

              auto next_cntrl = def->sel("self." + next_pt + "_" + (buffer.is_write(next_pt) ? "en" : "valid"));
              auto next_data = def->sel("self." + next_pt);

              def->connect(last_cntrl, next_cntrl);
              def->connect(last_data, next_data);
            }

            last_pt = next_pt;
          }
          return;
        }
        internal_assert(buffer.write_ports.size() == 1);
        std::cout << "Using lake rewrite rules." << std::endl;


        coreir_builder_set_context(def->getContext());
        coreir_builder_set_def(def);


        //internal_assert(buffer.ubuf.ostreams.size() == 1) << "\n" << "Multiple stream analysis not implemented!\n";
        //Loop over multiple stream
        for (auto out_stream: buffer.ubuf.ostreams) {
            string name = out_stream.first;
            //TODO: solve this self update rewrite
            if (name == buffer.name)
                continue;
            map<string, const Call*> read_ports_in_stream;
            for (auto pt_name: buffer.read_stream_bundle.at(name)) {
                read_ports_in_stream.insert(make_pair(pt_name, buffer.read_ports.at(pt_name)) );
            }
            auto read_ports_offset = buffer.get_port_offset(read_ports_in_stream);
            size_t num_dim = buffer.ubuf.ldims.size();
            std::cout << "Extract the offset of read port:\n" ;

            internal_assert(read_ports_offset.begin()->second.size() == num_dim);

            //print out the offset
            std::for_each(read_ports_offset.begin(),
                    read_ports_offset.end(),
                    [] (const pair<std::string, vector<int>> c) {
                        std::cout << c.first << " has offset = [";
                        for (const int val: c.second) {
                            std::cout << val << " ";
                        }
                        std::cout << "]" << std::endl;
                    }
                );
            std::cout << endl;

            //get the access pattern
            vector<AccessDimSize> id_addr;
            id_addr = out_stream.second.linear_access;
            std::cout << "Read range extracted: \n";
            std::for_each(id_addr.begin(),
                    id_addr.end(),
                    [] (const AccessDimSize c) {std::cout << to_int(c.range) << "\t" << to_int(c.stride) <<"\t" << to_int(c.dim_ref) << "\n";}
                );
            std::cout << endl;

            //TODO: add a pass to merge the stream into one

            //create the data structure for rewrite
            RecursiveBuffer port_opt(id_addr, buffer.ubuf, def, read_ports_offset);
            cout << "Before Rewrite: " << port_opt;
            //port_opt.generate_coreir(buffer);
            port_opt.port_reduction(2);
            cout << "After Rewrite: " << port_opt;
            port_opt.generate_coreir(buffer);
        }

        return;

    }

    else {
    if (buffer.read_loop_levels().size() == 1) {
      if (buffer.write_loop_levels().size() == 1) {
        if (buffer.read_loop_levels()[0] == buffer.write_loop_levels()[0]) {
          set<string> ports = buffer.port_names();
          vector<string> port_list(begin(ports), end(ports));
          sort(begin(port_list), end(port_list), [buffer](const std::string& pta, const std::string& ptb) {
              return buffer.lt(buffer.schedule(pta).back(), buffer.schedule(ptb).back());
          });

          cout << "Buffer: " << buffer.name << " is all wires!" << endl;
          string last_pt = port_list[0];
          internal_assert(buffer.is_write(last_pt)) << "Buffer: " << buffer.name << " is read before it is written?\n";

          for (size_t i = 1; i < port_list.size(); i++) {
            string next_pt = port_list[i];

            if (buffer.is_write(next_pt)) {
              auto last_cntrl = def->sel("self." + last_pt + "_" + (buffer.is_write(last_pt) ? "en" : "valid"));
              auto last_data = def->sel("self." + last_pt);

              auto next_cntrl = def->sel("self." + next_pt + "_" + (buffer.is_write(next_pt) ? "en" : "valid"));
              auto next_data = def->sel("self." + next_pt);

              def->connect(last_cntrl, next_cntrl);
              def->connect(last_data, next_data);
            }

            last_pt = next_pt;
          }
          return;
        } else {
          internal_assert(buffer.write_ports.size() == 1);

          string wp = begin(buffer.write_ports)->first;

          coreir_builder_set_context(def->getContext());
          coreir_builder_set_def(def);

          auto wen = self->sel(wp + "_en");
          // TODO: Populate counter with real values
          Instance* en_cnt_last = build_counter(def, 16, 0, 16, 1);
          def->connect(en_cnt_last->sel("en"), self->sel(wp + "_en"));
          def->connect(en_cnt_last->sel("reset"), self->sel("reset"));

          auto row_cnt = udiv(en_cnt_last->sel("out"), 4);
          auto col_cnt = sub(en_cnt_last->sel("out"), mul(row_cnt, 4));

          auto context = def->getContext();

          Wireable* inside_out_row = geq(row_cnt, 2);
          Wireable* inside_out_col = geq(col_cnt, 2);
          Wireable* started = andList(def,
              {geq(en_cnt_last->sel("out"), 1),
              inside_out_row,
              inside_out_col,
              wen});

          // TODO: Set to real max value
          int min_row_offset = 10000;
          int min_col_offset = 10000;
          int max_row_offset = -1;
          int max_col_offset = -1;
          vector<Expr> baseArgs = map_find(string("read_port_0"), buffer.read_ports)->args;
          for (auto rp : buffer.read_ports) {
            vector<Expr> args = rp.second->args;
            vector<int> offsets;
            internal_assert(args.size() == 2);
            for (size_t i = 0; i < baseArgs.size(); i++) {
              offsets.push_back(id_const_value(simplify(args[i] - baseArgs[i])));
            }

            if (offsets[1] < min_row_offset) {
              min_row_offset = offsets[1];
            }
            if (offsets[0] < min_col_offset) {
              min_col_offset = offsets[0];
            }


            if (offsets[1] > max_row_offset) {
              max_row_offset = offsets[1];
            }

            if (offsets[0] > max_col_offset) {
              max_col_offset = offsets[0];
            }
          }

          max_row_offset += -min_row_offset;
          max_col_offset += -min_col_offset;

          internal_assert(max_col_offset >= 0);
          internal_assert(max_row_offset >= 0);

          vector<Wireable*> rowDelays{self->sel(wp)};
          vector<Wireable*> rowDelayValids{wen};
          for (int i = 1; i < max_row_offset + 1; i++) {
            auto lastD = rowDelays[i - 1];
            auto lastEn = rowDelayValids[i - 1];
            // Create delays
            auto r0Delay =
              def->addInstance("r" + to_string(i) + "_delay" + context->getUnique(),
                  "memory.rowbuffer",
                  {{"width", COREMK(context, 16)}, {"depth", COREMK(context, 4)}});
            def->connect(r0Delay->sel("wdata"), lastD);
            def->connect(r0Delay->sel("wen"), lastEn);
            def->connect(r0Delay->sel("flush"), self->sel("reset"));

            rowDelays.push_back(r0Delay->sel("rdata"));
            rowDelayValids.push_back(r0Delay->sel("valid"));
          }

          vector<vector<Wireable*> > colDelays;
          for (size_t i = 0; i < rowDelays.size(); i++) {
            cout << "\ti = " << i << endl;
            auto d = rowDelays.at(i);
            auto dValid = rowDelayValids.at(i);
            Wireable* lastReg = d;
            Wireable* lastEn = dValid;

            internal_assert(dValid != nullptr);
            vector<Wireable*> cds{d};

            for (int c = 1; c < max_col_offset + 1; c++) {
              auto d0 = def->addInstance("d0" + context->getUnique(),"mantle.reg",{{"width",Const::make(context,16)},{"has_en",Const::make(context,true)}});
              def->connect(d0->sel("in"), lastReg);
              def->connect(d0->sel("en"), lastEn);

              auto delayedEn = def->addInstance("delayed_en" + context->getUnique(),"corebit.reg");
              def->connect(dValid, delayedEn->sel("in"));

              lastReg = d0->sel("out");
              lastEn = delayedEn->sel("out");

              cds.push_back(d0->sel("out"));
            }
            colDelays.push_back(cds);
          }

          for (auto rp : buffer.read_ports) {
            vector<Expr> args = rp.second->args;
            vector<int> offsets;
            internal_assert(args.size() == 2);
            for (size_t i = 0; i < baseArgs.size(); i++) {
              offsets.push_back(id_const_value(simplify(args[i] - baseArgs[i])));
            }

            int rowOffset = max_row_offset - offsets[1] + min_row_offset;
            int colOffset = max_col_offset - offsets[0] + min_col_offset;

            internal_assert(rowOffset >= 0);
            internal_assert(colOffset >= 0);
            internal_assert(rowOffset < (int) colDelays.size());

            def->connect(started, self->sel(rp.first + "_valid"));
            auto read_data = self->sel(rp.first);
            internal_assert(rowOffset < (int) colDelays.size());
            def->connect(colDelays.at(rowOffset).at(colOffset), read_data);
          }

          return;
        }
      }
    }
    }

    // Dummy definition
    internal_assert(buffer.write_ports.size() > 0);

    string wp = begin(buffer.write_ports)->first;
    auto write_en = self->sel(wp + "_en");
    auto write_data = self->sel(wp);
    for (auto rp : buffer.read_ports) {
      auto read_valid = self->sel(rp.first + "_valid");
      auto read_data = self->sel(rp.first);

      def->connect(write_en, read_valid);
      def->connect(write_data, read_data);
    }

    //internal_assert(false) << "Cannot classify buffer: " << buffer.name << "\n";
  }

map<string, CoreIR::Module*>
synthesize_hwbuffers(const Stmt& stmt, const std::map<std::string, Function>& env, std::vector<HWXcel>& xcels) {
  Stmt simple = simplify(remove_trivial_for_loops(simplify(unroll_loops(simplify(stmt)))));

  cout << "Doing rewrites for" << endl;
  cout << simple << endl;

  FuncOpCollector mic;
  simple.accept(&mic);
  auto buffers = mic.hwbuffers();

  CoreIR::Context* context = newContext();
  CoreIRLoadLibrary_commonlib(context);
  CoreIRLoadLibrary_lakelib(context);
  /*std::vector<string> lakelib_gen_names = {"linebuffer", "unified_buffer",
                                                 "new_unified_buffer"};

    for (auto gen_name : lakelib_gen_names) {
        string str = "lakelib." + gen_name;
        internal_assert(context->hasGenerator(str))
            << "could not find " << str << "\n";
    }*/
  auto ns = context->getNamespace("global");

  for (auto f : env) {
    if (f.second.schedule().is_accelerated() ||
        f.second.schedule().is_accelerator_input() ||
        (f.second.schedule().is_hw_kernel() && !f.second.schedule().compute_level().is_inlined()) ) {
        //f.second.schedule().is_accelerator_input() ||
        //f.second.schedule().is_hw_kernel()) {

      cout << "Buffer for " << f.first << endl;
      internal_assert(contains_key(f.first, buffers)) << f.first << " was not found in memory analysis\n";
      MemoryConstraints buf = map_find(f.first, buffers);
      // Add hwbuffer field to memory constraints wrapper
      for (auto xcel : xcels) {
        for (auto buffer : xcel.hwbuffers) {
          if (buffer.first == buf.name) {
            buf.ubuf = buffer.second;
          }
        }
      }
      vector<pair<string, CoreIR::Type*> > ubuffer_fields{{"clk", context->Named("coreir.clkIn")}, {"reset", context->BitIn()}, {"flush", context->BitIn()}};
      cout << "\t\tReads..." << endl;
      for (auto rd : buf.read_ports) {
        cout << "\t\t\t" << rd.first << " : " << buf.port_schedule(rd.first) << " || " << buf.port_address_stream(rd.first) << endl;
        ubuffer_fields.push_back({rd.first + "_valid", context->Bit()});
        ubuffer_fields.push_back({rd.first, context->Bit()->Arr(16)});
      }
      cout << "\t\tWrites..." << endl;
      for (auto rd : buf.write_ports) {
        cout << "\t\t\t" << rd.first << " : " << buf.port_schedule(rd.first) << " || " << buf.port_address_stream(rd.first) << endl;
        ubuffer_fields.push_back({rd.first + "_en", context->BitIn()});
        ubuffer_fields.push_back({rd.first, context->BitIn()->Arr(16)});
      }

      RecordType* utp = context->Record(ubuffer_fields);
      auto ub = ns->newModuleDecl(f.first + "_ubuffer", utp);
      auto def = ub->newModuleDef();
      synthesize_ubuffer(def, buf);
      ub->setDef(def);
    } else {
      cout << f.first << " is not accelerated" << endl;
      //internal_assert(!f.second.schedule().is_hw_kernel());
    }
  }

  // Save ubuffers and finish
  if (!saveToFile(ns, "bin/ubuffers.json")) {
    cout << "Could not save ubuffers" << endl;
    context->die();
  }
  deleteContext(context);

  return {};

}

  }
}
