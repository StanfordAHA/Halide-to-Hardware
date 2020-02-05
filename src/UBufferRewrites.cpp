#include "UBufferRewrites.h"
#include "ubuf_coreirsim.h"
#include "lakelib.h"

#include "RemoveTrivialForLoops.h"
#include "UnrollLoops.h"
#include "CoreIR_Libs.h"

#include "coreir.h"
#include "coreir/libs/commonlib.h"

using namespace CoreIR;
using namespace std;

namespace Halide {
  namespace Internal {

  class MemoryConstraints {
    public:
      HWBuffer ubuf;
      string name;
      map<string, const Provide*> write_ports;
      map<string, const Call*> read_ports;

      // This is an ordinal schedule, it does not
      // describe cycle accurate timing of inputs and
      // outputs
      map<string, StmtSchedule> schedules;

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
      int next_level;
      map<string, vector<const Provide*> > provides;
      map<string, vector<const Call*> > calls;
      map<const Provide*, StmtSchedule> write_scheds;
      map<const Call*, StmtSchedule> read_scheds;

      FuncOpCollector() : activeVars({{"", Expr(0), Expr(1)}}), next_level(1) {}

      map<string, MemoryConstraints> hwbuffers() const {
        map<string, MemoryConstraints> bufs;
        set<string> buffer_names = domain<string, vector<const Call*> >(calls);
        for (auto n : domain(provides)) {
          buffer_names.insert(n);
        }

        // Need to create port names for each read / write port

        for (auto b : buffer_names) {
          map<string, StmtSchedule> schedules;
          int rd_num = 0;
          map<string, const Call*> reads;
          for (auto rd : calls) {
            if (rd.first == b) {
              for (auto rdOp : rd.second) {
                reads["read_port_" + to_string(rd_num)] = rdOp;
                schedules["read_port_" + to_string(rd_num)] = map_find(rdOp, read_scheds);
                rd_num++;
              }
            }
          }
          int wr_num = 0;
          map<string, const Provide*> writes;
          for (auto rd : provides) {
            if (rd.first == b) {
              for (auto rdOp : rd.second) {
                writes["write_port_" + to_string(wr_num)] = rdOp;
                schedules["write_port_" + to_string(wr_num)] = map_find(rdOp, write_scheds);
                wr_num++;
              }
            }
          }
          bufs[b] = {{}, b, writes, reads, schedules};
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
        IRGraphVisitor::visit(p);

        inc_level();
        map_insert(provides, p->name, p);
        write_scheds[p] = activeVars;
      }

      void visit(const Call* p) override {
        if (p->call_type == Call::CallType::Halide) {
          inc_level();
          map_insert(calls, p->name, p);
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

  void synthesize_ubuffer(CoreIR::ModuleDef* def, MemoryConstraints& buffer) {
    cout << "ubuffer parameter" << buffer.ubuf ;

    // use this helper IRVisitor to extract the range stride information
    Scope<Expr> temp;    //create a empty scope
    IdentifyAddressing id_addr(buffer.ubuf.func, temp, buffer.ubuf.stride_map);
    buffer.ubuf.output_access_pattern.accept(&id_addr);

    std::cout << "Read range extracted: ";
    std::for_each(id_addr.ranges.begin(),
                id_addr.ranges.end(),
                [] (const int c) {std::cout << c << " ";}
            );
    std::cout << endl;


    if (buffer.read_ports.size() == 0) {
      // Buffer is write only, so it has no effect
      return;
    }
    auto self = def->sel("self");

    bool use_lake_lib = true;
    if (use_lake_lib) {

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

        string wp = begin(buffer.write_ports)->first;

        coreir_builder_set_context(def->getContext());
        coreir_builder_set_def(def);

        auto wen = self->sel(wp + "_en");
        auto w_data = self->sel(wp);
        //TODO:figure out where does the datawidth saved
        int width = 16;
        int depth = std::accumulate(buffer.ubuf.ldims.begin(), buffer.ubuf.ldims.end(), 1,
                [](int product, LogicalDimSize b){
                return product * id_const_value(b.logical_size);
                });
        int iter_cnt = std::accumulate(id_addr.ranges.begin(), id_addr.ranges.end(), 1,
                [](int a, int b){return a * b;});
        auto context = def->getContext();

        //initial the unified buffer parameter
        Json logical_size;
        Json in_chunk;
        Json out_stencil;
        for (size_t i = 0; i < buffer.ubuf.ldims.size(); i ++){
            logical_size["capacity"][i] = id_const_value(buffer.ubuf.ldims[i].logical_size);
            in_chunk["input_chunk"][i] = id_const_value(buffer.ubuf.dims[i].input_chunk);
            out_stencil["output_stencil"][i] = id_const_value(buffer.ubuf.dims[i].output_stencil);
        }

        Json out_start;
        size_t num_dim = buffer.ubuf.dims.size();
        vector<size_t> output_block(num_dim);
        for (size_t i = 0; i < num_dim; ++i) {
            output_block[i] = id_const_value(buffer.ubuf.dims[i].output_block);
        }
        //save in this json
        size_t num_read_port = buffer.read_ports.size();
        vector<int> output_start_addrs(num_read_port);
        vector<size_t> out_indexes(num_dim);

        for (size_t port = 0; port < num_read_port; port ++) {
            int start_port = 0;
            for (size_t idx = 0; idx < num_dim; idx ++) {
                start_port += out_indexes[idx] * id_const_value(buffer.ubuf.ldims[idx].logical_size_flatten);
            }
            output_start_addrs[port] = start_port;
            out_start["output_start"][port] = start_port;

            //update the iterator, using the state machine style to flatten the loop
            out_indexes[0] += 1;
            for (size_t dim = 0; dim < num_dim; dim ++) {
                if (out_indexes[dim] >= output_block[dim]) {
                    out_indexes[dim] = 0;
                    if (dim + 1 < num_dim) {
                        out_indexes[dim + 1] += 1;
                    }
                }
            }
        }

        cout << "Start creating instance" << endl;

        auto ubuf_coreir = def->addInstance("ubuf_"+context->getUnique(),
                "lakelib.unified_buffer",
                {{"width", Const::make(context, width)},
                {"num_input_ports", Const::make(context, buffer.write_ports.size())},
                {"num_output_ports", Const::make(context, buffer.read_ports.size())},
                {"stencil_width", Const::make(context, 0)},
                {"depth", Const::make(context, depth)},
                {"chain_idx", Const::make(context, 0)},
                {"rate_matched", Const::make(context, false)},
                {"chain_en", Const::make(context, false)},
                {"dimensionality", Const::make(context, id_addr.ranges.size())},
                {"iter_cnt", Const::make(context, iter_cnt)},
                {"logical_size", Const::make(context, logical_size)},
                {"input_chunk", Const::make(context, in_chunk)},
                {"output_stencil", Const::make(context, out_stencil)},
                {"num_stencil_acc_dim", Const::make(context, 0)},
                {"input_range_0",  Const::make(context, id_const_value(buffer.ubuf.ldims[0].logical_size))},
                {"input_stride_0", Const::make(context, id_const_value(buffer.ubuf.ldims[0].logical_size_flatten))},
                {"input_range_1",  Const::make(context, id_const_value(buffer.ubuf.ldims[1].logical_size))},
                {"input_stride_1", Const::make(context, id_const_value(buffer.ubuf.ldims[1].logical_size_flatten))},
                {"range_0",  Const::make(context, id_addr.ranges[0])},
                {"stride_0", Const::make(context, id_addr.strides_in_dim[0]
                        * id_const_value(buffer.ubuf.ldims[id_addr.dim_refs[0]].logical_size_flatten))},
                {"range_1",  Const::make(context, id_addr.ranges[1])},
                {"stride_1", Const::make(context, id_addr.strides_in_dim[1]
                        * id_const_value(buffer.ubuf.ldims[id_addr.dim_refs[1]].logical_size_flatten))},
                {"output_starting_addrs", Const::make(context, out_start)}
        });

        //wiring the write port
        def->connect(ubuf_coreir->sel("datain0"), w_data);
        def->connect(ubuf_coreir->sel("wen"), wen);

        //TODO: Not sure if this is robust, hacky solution for ren
        auto const_true = def->addInstance("ubuf_bitconst_"+context->getUnique(), "corebit.const", {{"value", COREMK(context, true)}});
        def->connect(ubuf_coreir->sel("ren"), const_true->sel("out"));

        //wiring the read port, TODO: counting method seems hacky
        int port_cnt = 0;
        for (auto const & rp : buffer.read_ports){
            auto read_data = self->sel(rp.first);
            def->connect(ubuf_coreir->sel("dataout" + std::to_string(port_cnt)), read_data);
            def->connect(ubuf_coreir->sel("valid"), self->sel(rp.first + "_valid"));
            port_cnt ++;
        }

        //wire the peripherial
        def->connect(ubuf_coreir->sel("reset"), self->sel("reset"));
        cout << "finish wiring!" << endl;

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
        f.second.schedule().is_accelerator_input()) {
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
      vector<pair<string, CoreIR::Type*> > ubuffer_fields{{"clk", context->Named("coreir.clkIn")}, {"reset", context->BitIn()}};
      cout << "\t\tReads..." << endl;
      for (auto rd : buf.read_ports) {
        cout << "\t\t\t" << rd.first << " : " << buf.port_schedule(rd.first) << " " << buf.port_address_stream(rd.first) << endl;
        ubuffer_fields.push_back({rd.first + "_valid", context->Bit()});
        ubuffer_fields.push_back({rd.first, context->Bit()->Arr(16)});
      }
      cout << "\t\tWrites..." << endl;
      for (auto rd : buf.write_ports) {
        cout << "\t\t\t" << rd.first << " : " << buf.port_schedule(rd.first) << buf.port_address_stream(rd.first) << endl;
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
  if (!saveToFile(ns, "ubuffers.json")) {
    cout << "Could not save ubuffers" << endl;
    context->die();
  }
  deleteContext(context);

  return {};

}

  }
}
