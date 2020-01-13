#include "UBufferRewrites.h"

#include "RemoveTrivialForLoops.h"
#include "UnrollLoops.h"

using namespace CoreIR;
using namespace std;

namespace Halide {
  namespace Internal {

    template<typename D, typename R>
      set<D> domain(const std::map<D, R>& m) {
        set<D> d;
        for (auto e : m) {
          d.insert(e.first);
        }
        return d;
      }

  class RealizeFinder : public IRGraphVisitor {
    public:

      using IRGraphVisitor::visit;

      const Realize* r;
      string target;

      RealizeFinder(const std::string& target_) : r(nullptr), target(target_) {}

      void visit(const Realize* rl) override {
        cout << "Searching realize: " << rl->name << " for " << target << endl;
        if (rl->name == target) {
          r = rl;
        } else {
          rl->body->accept(this);
        }
      }
  };

class VarSpec {
  public:
    std::string name;
    Expr min;
    Expr extent;

    bool is_const() const {
      return name == "";
    }

    int const_value() const {
      internal_assert(is_const());
      return id_const_value(min);
    }
};

bool operator==(const VarSpec& a, const VarSpec& b) {
  if (a.is_const() != b.is_const()) {
    return false;
  }

  if (a.is_const()) {
    return a.const_value() == b.const_value();
  } else {
    return a.name == b.name;
  }
}

typedef std::vector<VarSpec> StmtSchedule;

std::ostream& operator<<(std::ostream& out, const VarSpec& e) {
  if (e.name != "") {
    out << e.name << " : [" << e.min << " " << simplify(e.min + e.extent - 1) << "]";
  } else {
    internal_assert(is_const(e.min));
    internal_assert(is_one(e.extent));
    out << e.min;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const StmtSchedule& s) {
  for (auto e : s ) {
    out << e << ", ";
  }
  return out;
}

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
    if (buffer.read_ports.size() == 0) {
      // Buffer is write only, so it has no effect
      return;
    }

    if (buffer.read_loop_levels().size() == 1) {
      if (buffer.write_loop_levels().size() == 1) {
        if (buffer.read_loop_levels()[0] == buffer.write_loop_levels()[0]) {
          set<string> ports = buffer.port_names();
          vector<string> port_list(begin(ports), end(ports));
          sort(begin(port_list), end(port_list), [buffer](const std::string& pta, const std::string& ptb) {
              return buffer.lt(buffer.schedule(pta).back(), buffer.schedule(ptb).back());
          });

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
          //internal_assert(false) << "Two separate read / write levels for: " << buffer.name << "\n";
        }
      }
    }

    //internal_assert(false) << "Cannot classify buffer: " << buffer.name << "\n";
  }

  void synthesize_hwbuffers(const Stmt& stmt, const std::map<std::string, Function>& env) {
    auto pre_simple = simplify(stmt);
    cout << "Pre simplification..." << endl;
    cout << pre_simple << endl;
    Stmt simple = simplify(remove_trivial_for_loops(simplify(unroll_loops(pre_simple))));
    cout << "Simplified code for stmt:" << endl;
    cout << simple << endl;

    FuncOpCollector mic;
    simple.accept(&mic);

    cout << "### New buffers in constant fold ROMs" << endl;
    for (auto b : mic.provides) {
      cout << "\t" << b.second.size() << " Provides to: " << b.first << endl;
      cout << "\t\tis rom " << mic.is_rom(b.first) << endl;
      if (contains_key(b.first, mic.calls)) {
        cout << "\t" << map_find(b.first, mic.calls).size() << " calls to: " << b.first << endl;
      }
    }

    ROMLoadFolder folder;
    folder.mic = mic;
    Stmt replaced = folder.mutate(simple);
    
    cout << "After ROM simplification..." << endl;
    cout << replaced << endl;
   
    {
      CoreIR::Context* context = newContext();
      auto ns = context->getNamespace("global");

      RecordType* mtp = context->Record({{"clk", context->Named("coreir.clkIn")}, {"rst", context->BitIn()}});
      auto m = ns->newModuleDecl("m", mtp);
      auto mDef = m->newModuleDef();

      RealizeFinder rFinder("hw_output");
      replaced->accept(&rFinder);
      internal_assert(rFinder.r != nullptr);

      FuncOpCollector mic;
      replaced.accept(&mic);

      cout << "--- Hardware buffers" << endl;
      for (auto bufInfo : mic.hwbuffers()) {
        MemoryConstraints buf = bufInfo.second;
        cout << "\tFound buffer: " << buf.name << endl;
        vector<pair<string, CoreIR::Type*> > ubuffer_fields{{"clk", context->Named("coreir.clkIn")}, {"rst", context->BitIn()}};
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
        CoreIR::Module* ubuffer = ns->newModuleDecl("unified_buffer_" + buf.name, utp);
        auto def = ubuffer->newModuleDef();
        synthesize_ubuffer(def, buf);
        ubuffer->setDef(def);

        cout << "Unified buffer..." << endl;
        ubuffer->print();

        mDef->addInstance("ubuffer_" + buf.name, "global.unified_buffer_" + buf.name);
      }

      m->setDef(mDef);
      cout << "Output module" << endl;
      m->print();

      deleteContext(context);
    }
    internal_assert(false) << "Stopping so dillon can view\n";
  }

  }
}
