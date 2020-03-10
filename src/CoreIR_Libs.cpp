#include "CoreIR_Libs.h"

#include "IR.h"

using namespace std;
using namespace CoreIR;

namespace Halide {
  namespace Internal {

    void coreir_builder_set_context(CoreIR::Context* context) {
      active_ctx = context;
    }

    void coreir_builder_set_def(CoreIR::ModuleDef* def) {
      active_def = def;
    }

    Instance* build_counter(CoreIR::ModuleDef* def,
        const int width,
        const int min_val,
        const int max_val,
        const int inc_val) {
      auto c = def->getContext();
      CoreIR::Values args = {{"width",CoreIR::Const::make(c, width)},
        {"min",CoreIR::Const::make(c, min_val)},
        {"max",CoreIR::Const::make(c, max_val)},
        {"inc",CoreIR::Const::make(c, inc_val)}};

      string lname = "counter_" + def->getContext()->getUnique();
      CoreIR::Instance* counter_inst = def->addInstance(lname, "commonlib.counter", args);
      return counter_inst;
    }

    CoreIR::Wireable* sub(Wireable* a, Wireable* b) {
      internal_assert(a->getType() == b->getType());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto inst = active_def->addInstance("sub" + active_ctx->getUnique(), "coreir.sub", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), b);
      return inst->sel("out");
    }

    CoreIR::Wireable* add(Wireable* a, Wireable* b) {
      internal_assert(a->getType() == b->getType());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto inst = active_def->addInstance("add" + active_ctx->getUnique(), "coreir.add", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), b);
      return inst->sel("out");
    }

    CoreIR::Wireable* mul(Wireable* a, const int c) {
      internal_assert(a->getType() != active_ctx->Bit());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto ct = mkConst(active_def, "const" + active_ctx->getUnique(), width, c);
      auto inst = active_def->addInstance("mul" + active_ctx->getUnique(), "coreir.mul", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), ct->sel("out"));
      return inst->sel("out");
    }

    CoreIR::Wireable* smod(Wireable* a, const int c) {
      internal_assert(a->getType() != active_ctx->Bit());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto ct = mkConst(active_def, "const" + active_ctx->getUnique(), width, c);
      auto inst = active_def->addInstance("smod" + active_ctx->getUnique(), "coreir.smod", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), ct->sel("out"));
      return inst->sel("out");
    }

    CoreIR::Wireable* udiv(Wireable* a, const int c) {
      internal_assert(a->getType() != active_ctx->Bit());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto ct = mkConst(active_def, "const" + active_ctx->getUnique(), width, c);
      auto inst = active_def->addInstance("udiv" + active_ctx->getUnique(), "coreir.udiv", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), ct->sel("out"));
      return inst->sel("out");
    }

    CoreIR::Wireable* add(Wireable* a, const int c) {
      internal_assert(a->getType() != active_ctx->Bit());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto ct = mkConst(active_def, "const" + active_ctx->getUnique(), width, c);
      auto inst = active_def->addInstance("add" + active_ctx->getUnique(), "coreir.add", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), ct->sel("out"));
      return inst->sel("out");
    }

    CoreIR::Wireable* eq(Wireable* a, const int c) {
      internal_assert(a->getType() == active_ctx->Bit());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto ct = mkConst(active_def, "const" + active_ctx->getUnique(), width, c);
      auto inst = active_def->addInstance("add" + active_ctx->getUnique(), "coreir.add", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), ct->sel("out"));
      return inst->sel("out");
    }

    CoreIR::Wireable* geq(Wireable* a, const int c) {
      //internal_assert(a->getType() == active_ctx->Bit());

      auto def = active_def;
      internal_assert(arrayDims(a).size() == 1);
      int width = arrayDims(a).at(0);
      auto ct = mkConst(active_def, "const" + active_ctx->getUnique(), width, c);
      auto inst = active_def->addInstance("geq" + active_ctx->getUnique(), "coreir.uge", {{"width", COREMK(active_ctx, width)}});
      def->connect(inst->sel("in0"), a);
      def->connect(inst->sel("in1"), ct->sel("out"));
      return inst->sel("out");
    }

CoreIR::Instance* mkConst(CoreIR::ModuleDef* def, const std::string& name, const int width, const int val) {
  return def->addInstance(name, "coreir.const", {{"width", COREMK(def->getContext(), width)}}, {{"value", COREMK(def->getContext(), BitVector(width, val))}});
}

CoreIR::Wireable* andVals(CoreIR::ModuleDef* def, CoreIR::Wireable* a, CoreIR::Wireable* b) {
  //auto c = def->getContext();
  auto ad = def->addInstance("and_all_" + def->getContext()->getUnique(), "corebit.and");
  def->connect(ad->sel("in0"), a);
  def->connect(ad->sel("in1"), b);

  return ad->sel("out");
}

CoreIR::Wireable* andList(CoreIR::ModuleDef* def, const std::vector<CoreIR::Wireable*>& vals) {
  CoreIR::Wireable* val = nullptr;
  if (vals.size() == 0) {
    return def->addInstance("and_all_" + def->getContext()->getUnique(), "corebit.const", {{"value", COREMK(def->getContext(), true)}})->sel("out");
  }

  if (vals.size() == 1) {
    return vals[0];
  }

  val = vals[0];
  for (int i = 1; i < ((int) vals.size()); i++) {
    val = andVals(def, val, vals[i]);
  }
  return val;
}

void loadHalideLib(CoreIR::Context* context) {
  auto hns = context->newNamespace("halidehw");

  {
    CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("passthrough", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto t = args.at("type")->get<CoreIR::Type*>();
        return c->Record({
            {"in", t->getFlipped()},
            {"out", t}
            });
        });
    auto srGen = hns->newGeneratorDecl("passthrough", srTg, srParams);
    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto self = def->sel("self");
        def->connect(self->sel("in"), self->sel("out"));
        });
  }
  {
    CoreIR::Params srParams{{"width", context->Int()}};
    CoreIR::TypeGen* srTg = hns->newTypeGen("cast", srParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto width = args.at("width")->get<int>();
        return c->Record({
            {"in", c->BitIn()->Arr(width)},
            {"out", c->Bit()->Arr(width)}
            });
        });
    auto srGen = hns->newGeneratorDecl("cast", srTg, srParams);
    srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto self = def->sel("self");
        def->connect(self->sel("in"), self->sel("out"));
        });
  }

  {
  CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}, {"delay", context->Int()}};
  CoreIR::TypeGen* srTg = hns->newTypeGen("stream_trimmer", srParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      auto t = args.at("type")->get<CoreIR::Type*>();
      return c->Record({
          {"clk", c->Named("coreir.clkIn")},
          {"in", t->getFlipped()},
          {"en", c->BitIn()},
          {"out", t},
          {"valid", c->Bit()}
          });
      });
  auto srGen = hns->newGeneratorDecl("stream_trimmer", srTg, srParams);
  srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      auto self = def->sel("self");
      int delay = args.at("delay")->get<int>();
      auto tp = args.at("type")->get<CoreIR::Type*>();
      auto sr = def->addInstance("sr", "halidehw.shift_register", {{"type", COREMK(c, tp)}, {"delay", COREMK(c, delay)}});
      //auto sr = def->addInstance("sr", "halidehw.shift_register", {{"type", COREMK(c, tp)}, {"delay", COREMK(c, 9*2 + 2)}});
      
      def->connect(sr->sel("in_data"), self->sel("in"));
      def->connect(sr->sel("in_en"), self->sel("en"));
      def->connect(sr->sel("valid"), self->sel("valid"));
      def->connect(sr->sel("out_data"), self->sel("out"));

      //def->connect(self->sel("in"), self->sel("out"));
      //def->connect(self->sel("en"), self->sel("valid"));
      });
  }

  CoreIR::Params srParams{{"type", CoreIR::CoreIRType::make(context)}, {"delay", context->Int()}};
  CoreIR::TypeGen* srTg = hns->newTypeGen("shift_register", srParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      auto t = args.at("type")->get<CoreIR::Type*>();
      return c->Record({
          {"clk", c->Named("coreir.clkIn")},
          {"in_data", t->getFlipped()},
          {"in_en", c->BitIn()},
          {"out_data", t},
          {"valid", c->Bit()}
          });
      });
  auto srGen = hns->newGeneratorDecl("shift_register", srTg, srParams);
  srGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      auto self = def->sel("self");
      CoreIR::Type* type = args.at("type")->get<CoreIR::Type*>();
      int delay = args.at("delay")->get<int>();

      Wireable* inData = self->sel("in_data");
      Wireable* inEn = self->sel("in_en");

      for (int i = 0; i < delay; i++) {
        auto delayReg = def->addInstance("delay_reg_" + std::to_string(i), "commonlib.reg_array", {{"type", COREMK(c, type)}});
        def->connect(delayReg->sel("clk"), self->sel("clk"));
        def->connect(delayReg->sel("in"), inData);

        inData = delayReg->sel("out");
        auto validReg = def->addInstance("valid_reg_" + std::to_string(i), "corebit.reg");
        def->connect(validReg->sel("in"), inEn);
        def->connect(validReg->sel("clk"), self->sel("clk"));
        inEn = validReg->sel("out");
      }

      def->connect(self->sel("out_data"), inData);
      def->connect(self->sel("valid"), inEn);
      });
  
  CoreIR::Params widthParams{{"width", context->Int()}};
  CoreIR::Params romParams{{"width", context->Int()}, {"depth", context->Int()}, {"nports", context->Int()}};
  CoreIR::Params widthDimParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}};
  CoreIR::Params widthDimParams3{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}, {"nchannels", context->Int()}};
  CoreIR::Params dynamicStencilReadParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}};
  CoreIR::Params stencilReadParams{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()},
    {"r", context->Int()}, {"c", context->Int()}};
  CoreIR::Params stencilReadParams3{{"width", context->Int()}, {"nrows", context->Int()}, {"ncols", context->Int()}, {"nchannels", context->Int()},
    {"r", context->Int()}, {"c", context->Int()}, {"b", context->Int()}};

  CoreIR::TypeGen* romTg = hns->newTypeGen("ROM", romParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      int addrWidth = 16;
      int dataWidth = args.at("width")->get<int>();
      auto nports = args.at("nports")->get<int>();
      return c->Record({{"reset", c->BitIn()}, {"clk", c->Named("coreir.clkIn")}, {"raddr", c->BitIn()->Arr(addrWidth)->Arr(nports)}, {"rdata", c->Bit()->Arr(dataWidth)->Arr(nports)},
          {"ren", c->BitIn()->Arr(nports)}});
      });
  auto romGen = hns->newGeneratorDecl("ROM", romTg, romParams);
  auto romGenModParamFun = [](CoreIR::Context* c, CoreIR::Values genargs) {
    CoreIR::Params modparams;
    CoreIR::Values defaultargs;
    modparams["init"] = CoreIR::JsonType::make(c);
    return std::pair<CoreIR::Params, CoreIR::Values>(modparams, defaultargs);
  };
  romGen->setModParamsGen(romGenModParamFun);

  romGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      auto self = def->sel("self");
      int width = args.at("width")->get<int>();
      int depth = args.at("depth")->get<int>();
      int nports = args.at("nports")->get<int>();

      auto vals = def->getModule()->getArg("init");
      for (int i = 0; i < nports; i++) {
      auto romBank = def->addInstance("rom_bank_" + std::to_string(i), "memory.rom2",
          {{"width", COREMK(c, width)}, {"depth", COREMK(c, depth)}},
          {{"init", vals}});
      def->connect(romBank->sel("raddr"), self->sel("raddr")->sel(i));
      def->connect(romBank->sel("rdata"), self->sel("rdata")->sel(i));
      def->connect(romBank->sel("ren"), self->sel("ren")->sel(i));
      def->connect(romBank->sel("clk"), self->sel("clk"));
      }
      });

  
  CoreIR::TypeGen* tg = hns->newTypeGen("rd_stream", widthDimParams,
      [](CoreIR::Context* c, CoreIR::Values args) {
      auto nr = args.at("nrows")->get<int>();
      auto nc = args.at("ncols")->get<int>();
      auto w = args.at("width")->get<int>();
      return c->Record({{"in", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}});
      });
  auto rdStreamGen = hns->newGeneratorDecl("rd_stream", tg, widthDimParams);
  rdStreamGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
      def->connect("self.in", "self.out");
      });

  {
    CoreIR::TypeGen* tg = hns->newTypeGen("rd_stream_3", widthDimParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto rdStreamGen = hns->newGeneratorDecl("rd_stream_3", tg, widthDimParams3);
    rdStreamGen->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        def->connect("self.in", "self.out");
        });
  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("write_stream", widthDimParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"stream", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}, {"stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)}});
        });
    auto wrStream = hns->newGeneratorDecl("write_stream", ws, widthDimParams);

    wrStream->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        def->connect("self.stream", "self.stencil");
        });

  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("write_stream_3", widthDimParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"stream", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}, {"stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto wrStream = hns->newGeneratorDecl("write_stream_3", ws, widthDimParams3);

    wrStream->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        def->connect("self.stream", "self.stencil");
        });

  }
  
  {
    CoreIR::TypeGen* ws = hns->newTypeGen("init_stencil", widthDimParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}});
        });
    auto initStencil = hns->newGeneratorDecl("init_stencil", ws, widthDimParams);

    initStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto m = def->getModule();
        auto r = m->getGenArgs().at("nrows")->get<int>();
        auto cls = m->getGenArgs().at("ncols")->get<int>();

        cout << "r = " << r << endl;
        cout << "cls = " << cls << endl;
        auto w = m->getGenArgs().at("width")->get<int>();
        for (int i = 0; i < r; i++) {
        for (int j = 0; j < cls; j++) {

          auto cs = def->addInstance("init_const_" + std::to_string(i) + "_" + std::to_string(j), "coreir.const", {{"width", CoreIR::Const::make(c, w)}}, {{"value", CoreIR::Const::make(c, BitVector(w, 0))}});
          def->connect(def->sel("self")->sel("out")->sel(j)->sel(i), cs->sel("out"));
        }
        }
        });
  }


  {
    CoreIR::TypeGen* ws = hns->newTypeGen("init_stencil_3", widthDimParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto initStencil = hns->newGeneratorDecl("init_stencil_3", ws, widthDimParams3);

    initStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto m = def->getModule();
        auto r = m->getGenArgs().at("nrows")->get<int>();
        auto cls = m->getGenArgs().at("ncols")->get<int>();
        auto chans = m->getGenArgs().at("nchannels")->get<int>();
        cout << "row = " << r << endl;
        cout << "col = " << cls << endl;
        cout << "chn = " << chans << endl;
        auto w = m->getGenArgs().at("width")->get<int>();
        for (int i = 0; i < r; i++) {
        for (int j = 0; j < cls; j++) {
        for (int b = 0; b < chans; b++) {
          auto cs = def->addInstance("init_const_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(b),
              "coreir.const",
              {{"width", CoreIR::Const::make(c, w)}}, {{"value", CoreIR::Const::make(c, BitVector(w, 0))}});
          def->connect(def->sel("self")->sel("out")->sel(b)->sel(j)->sel(i), cs->sel("out"));
          }
        }
        }
        });
  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("dynamic_stencil_read", dynamicStencilReadParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto width = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(width)->Arr(nr)->Arr(nc)}, {"r", c->BitIn()->Arr(16)}, {"c", c->BitIn()->Arr(16)}, {"out", c->Bit()->Arr(width)}});
        });

    auto readStencil = hns->newGeneratorDecl("dynamic_stencil_read", ws, dynamicStencilReadParams);
    readStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {

        //def->connect(def->sel("self")->sel("in")->sel(col)->sel(row), def->sel("self")->sel("out"));
        //def->connect(def->sel("self")->sel("in")->sel(0)->sel(row), def->sel("self")->sel("out"));
        });
  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("stencil_read", stencilReadParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto width = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(width)->Arr(nr)->Arr(nc)}, {"out", c->Bit()->Arr(width)}});
        });
    auto readStencil = hns->newGeneratorDecl("stencil_read", ws, stencilReadParams);
    readStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto row = args.at("r")->get<int>();
        auto col = args.at("c")->get<int>();

        def->connect(def->sel("self")->sel("in")->sel(col)->sel(row), def->sel("self")->sel("out"));
        //def->connect(def->sel("self")->sel("in")->sel(0)->sel(row), def->sel("self")->sel("out"));
        });
  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("stencil_read_3", stencilReadParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto width = args.at("width")->get<int>();
        return c->Record({{"in", c->BitIn()->Arr(width)->Arr(nr)->Arr(nc)->Arr(nb)}, {"out", c->Bit()->Arr(width)}});
        });
    auto readStencil = hns->newGeneratorDecl("stencil_read_3", ws, stencilReadParams3);
    readStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto row = args.at("r")->get<int>();
        auto col = args.at("c")->get<int>();
        auto chan = args.at("b")->get<int>();

        def->connect(def->sel("self")->sel("in")->sel(chan)->sel(col)->sel(row), def->sel("self")->sel("out"));
        //def->connect(def->sel("self")->sel("in")->sel(0)->sel(row), def->sel("self")->sel("out"));
        });
  }

  {
    CoreIR::TypeGen* ws = hns->newTypeGen("create_stencil", stencilReadParams,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"new_val", c->BitIn()->Arr(w)}, {"in_stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)}});
        });
    auto createStencil = hns->newGeneratorDecl("create_stencil", ws, stencilReadParams);
    createStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto nRows = args.at("nrows")->get<int>();
        auto nCols = args.at("ncols")->get<int>();
        //auto width = args.at("width")->get<int>();
        auto newR = args.at("r")->get<int>();

        cout << "Got r from create stencil = " << newR << endl;
        auto newC = args.at("c")->get<int>();
        cout << "Got C " << endl;

        auto self = def->sel("self");
        for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
          if ((i == newR) && (j == newC)) {
            //def->connect(def->sel("self")->sel("new_val"), self->sel("out")->sel(i)->sel(j));
            def->connect(def->sel("self")->sel("new_val"), self->sel("out")->sel(j)->sel(i));
          } else {
            //def->connect(self->sel("in_stencil")->sel(i)->sel(j), self->sel("out")->sel(i)->sel(j));
            // TODO: Fix this mismatch?
            //def->connect(self->sel("in_stencil")->sel(i)->sel(j), self->sel("out")->sel(j)->sel(i));
            def->connect(self->sel("in_stencil")->sel(j)->sel(i), self->sel("out")->sel(j)->sel(i));
          }
        }
        }
        });
  }
  
  {
    CoreIR::TypeGen* ws = hns->newTypeGen("create_stencil_3", stencilReadParams3,
        [](CoreIR::Context* c, CoreIR::Values args) {
        auto nr = args.at("nrows")->get<int>();
        auto nc = args.at("ncols")->get<int>();
        auto nb = args.at("nchannels")->get<int>();
        auto w = args.at("width")->get<int>();
        return c->Record({{"new_val", c->BitIn()->Arr(w)}, {"in_stencil", c->BitIn()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}, {"out", c->Bit()->Arr(w)->Arr(nr)->Arr(nc)->Arr(nb)}});
        });
    auto createStencil = hns->newGeneratorDecl("create_stencil_3", ws, stencilReadParams3);
    createStencil->setGeneratorDefFromFun([](CoreIR::Context* c, CoreIR::Values args, CoreIR::ModuleDef* def) {
        auto nRows = args.at("nrows")->get<int>();
        auto nCols = args.at("ncols")->get<int>();
        auto nChans = args.at("nchannels")->get<int>();
        
        auto newR = args.at("r")->get<int>();
        auto newC = args.at("c")->get<int>();
        auto newB = args.at("b")->get<int>();

        auto self = def->sel("self");
        for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
        for (int b = 0; b < nChans; b++) {
          if ((i == newR) && (j == newC) && (b == newB)) {
            def->connect(def->sel("self")->sel("new_val"), self->sel("out")->sel(b)->sel(j)->sel(i));
          } else {
            def->connect(self->sel("in_stencil")->sel(b)->sel(j)->sel(i), self->sel("out")->sel(b)->sel(j)->sel(i));
          }
          }
        }
        }
        });
  }
}

CoreIR::Values getGenArgs(Wireable* p) {
  internal_assert(isa<Instance>(p));
  auto inst = toInstance(p);
  internal_assert(inst->getModuleRef()->isGenerated());
  return inst->getModuleRef()->getGenArgs();
}

vector<int> arrayDims(CoreIR::Type* tp) {
  internal_assert(isa<CoreIR::ArrayType>(tp));
  auto arrTp = sc<CoreIR::ArrayType>(tp);
  if (!isa<CoreIR::ArrayType>(arrTp->getElemType())) {
    vector<int> dims;
    dims.push_back(arrTp->getLen());
    return dims;
  } else {
    auto tps = arrayDims(arrTp->getElemType());
    tps.push_back(arrTp->getLen());
    return tps;
  }
}

std::vector<int> arrayDims(CoreIR::Wireable* w) {
  return arrayDims(w->getType());
}

std::string coreStr(const Wireable* w) {
  return CoreIR::toString(*w);
}

std::string coreStr(const CoreIR::Type* w) {
  return CoreIR::toString(*w);
}


  }
}
