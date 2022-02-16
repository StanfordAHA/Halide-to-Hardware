#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"
#include "coreir/libs/float_DW.h"
#include "lakelib.h"
#include "coreir/passes/transform/rungenerators.h"
#include "cgralib.h"

using namespace std;
using namespace CoreIR;

string sanatize_name(string name) {
  string out = name;
  for (size_t i=0; i<name.length(); ++i) {
    if (name[i] == '-') {
      out[i] = 'n';
    } else if (name[i] == '.') {
      out[i] = '_';
    }
  }
  return out;
}

namespace MapperPasses {
class TreeReduction : public ModulePass {
  
  vector<Instance*> targetSubgraphs;

  public:
    static std::string ID;
    TreeReduction() : ModulePass(ID,"Finds associative operators joined together and replaces with tree implementation",false) {}

    bool runOnModule(Module* m) override;

    //Print function will output when in verbose mode.
    void print() override;

    //These are our class's custom APIs. We will be able to use this in other passes
    vector<Wireable*> collectInputs(Instance* head);
    vector<pair<Wireable*,Wireable*>> collectInputConnections(Instance* head);
    vector<Instance*> collectInsts(Instance* head);
    bool isAssocSubgraph(Instance* i);
    Instance* getSelectedInst(Instance* i, string sel);
    string getOpName(Instance* i);
    int getTotalSubgraphs();
  
    Context* c;
    Namespace* commonlib;
};

}

std::string MapperPasses::TreeReduction::ID = "treereduction";

Wireable* getConnectedWireable(Wireable* headInst, string out_sel) {
  auto local_conx = headInst->getLocalConnections();
  string out_name = out_sel;
  //std::cout << "looking for " << out_name << endl;
  for (auto wire_pair : local_conx) {
    //std::cout << "  " << wire_pair.first->toString() << " connected to " << wire_pair.second->toString() << endl;
    if (out_name == wire_pair.first->toString()) {
      //std::cout << "  " << wire_pair.first->toString() << " connected to " << wire_pair.second->toString() << endl;
      return wire_pair.second;
    } else if (out_name == wire_pair.second->toString()) {
      //std::cout << "  " << wire_pair.first->toString() << " connected to " << wire_pair.second->toString() << endl;
      return wire_pair.first;
    }
  }
  std::cout << "didn't find it" << endl;
  return NULL;
}

// return a vector of the input wireables for a reduction tree ending at given instance 
vector<Wireable*> MapperPasses::TreeReduction::collectInputs(Instance* head) {
  vector<Wireable*> inputs;
  string opName = getOpName(head);

  //std::cout << "collecting inputs for " << opName << " named " << head->toString() << endl;
  // check in0 branch
  Wireable* in0_wire = getConnectedWireable(head, head->getInstname() + ".in0");
  //std::cout << "wire=" << in0_wire->toString() << " parent=" << in0_wire->getTopParent()->toString() << std::endl;
  
  if (in0_wire->getTopParent()->toString() == "self"){
    inputs.push_back(in0_wire);
    
  } else {
    Instance* in0_inst = getSelectedInst(head, "in0");
    //std::cout << "selected inst for " << head->getInstname() << " named " << in0_inst->getInstname() << endl;
    if (opName == getOpName(in0_inst)) {
      vector<Wireable*> in0_inputs = collectInputs(in0_inst);
      inputs.insert(inputs.end(), in0_inputs.begin(), in0_inputs.end());
    } else {
      inputs.push_back(head->sel("in0"));
    }
  }
  //std::cout << "done with in0 " << opName << endl;

  // check in1 branch
  Wireable* in1_wire = getConnectedWireable(head, head->getInstname() + ".in1");
  //std::cout << "wire=" << in1_wire->toString() << " parent=" << in1_wire->getTopParent()->toString() << std::endl;
  
  if (in1_wire->getTopParent()->toString() == "self"){
    inputs.push_back(in1_wire);
    
  } else {
    Instance* in1_inst = getSelectedInst(head, "in1");
    if (opName == getOpName(in1_inst)) {
      vector<Wireable*> in1_inputs = collectInputs(in1_inst);
      inputs.insert(inputs.end(), in1_inputs.begin(), in1_inputs.end());
    } else {
      inputs.push_back(head->sel("in1"));
    }
  }

  return inputs;
}

// return a map of the input wireable connections for a reduction tree ending at given instance
//  First in the pair is the input to use. Second is the connection endpoint that should be removed.
vector<pair<Wireable*, Wireable*>> MapperPasses::TreeReduction::collectInputConnections(Instance* head) {
  vector<pair<Wireable*, Wireable*>> inputs;
  string opName = getOpName(head);

  //std::cout << "collecting inputs for " << opName << " named " << head->toString() << endl;
  // check in0 branch
  Wireable* in0_wire = getConnectedWireable(head, head->getInstname() + ".in0");
  //std::cout << "wire=" << in0_wire->toString() << " parent=" << in0_wire->getTopParent()->toString() << std::endl;
  
  if (in0_wire->getTopParent()->toString() == "self"){
    inputs.push_back(make_pair(head->sel("in0"), in0_wire));
    
  } else {
    Instance* in0_inst = getSelectedInst(head, "in0");
    //std::cout << "selected inst for " << head->getInstname() << " named " << in0_inst->getInstname() << endl;
    if (opName == getOpName(in0_inst)) {
      vector<pair<Wireable*, Wireable*>> in0_inputs = collectInputConnections(in0_inst);
      inputs.insert(inputs.end(), in0_inputs.begin(), in0_inputs.end());
    } else {
      inputs.push_back(make_pair(head->sel("in0"), in0_wire));
    }
  }
  //std::cout << "done with in0 " << opName << endl;

  // check in1 branch
  Wireable* in1_wire = getConnectedWireable(head, head->getInstname() + ".in1");
  //std::cout << "wire=" << in1_wire->toString() << " parent=" << in1_wire->getTopParent()->toString() << std::endl;
  
  if (in1_wire->getTopParent()->toString() == "self"){
    inputs.push_back(make_pair(head->sel("in1"), in1_wire));
    
  } else {
    Instance* in1_inst = getSelectedInst(head, "in1");
    //std::cout << "selected inst for " << head->getInstname() << " named " << in1_inst->getInstname() << endl;
    if (opName == getOpName(in1_inst)) {
      vector<pair<Wireable*, Wireable*>> in1_inputs = collectInputConnections(in1_inst);
      inputs.insert(inputs.end(), in1_inputs.begin(), in1_inputs.end());
    } else {
      inputs.push_back(make_pair(head->sel("in1"), in1_wire));
    }
  }
  //std::cout << "done with in1 " << opName << endl;

  return inputs;
}

// return a vector of the instances in a reduction tree ending at given instance
vector<Instance*> MapperPasses::TreeReduction::collectInsts(Instance* head) {
  vector<Instance*> insts;
  string opName = getOpName(head);
  insts.push_back(head);

  // check in0 branch
  Wireable* in0_wire = getConnectedWireable(head, head->getInstname() + ".in0");
  //std::cout << "wire=" << in0_wire->toString() << " parent=" << in0_wire->getTopParent()->toString() << std::endl;
  
  if (in0_wire->getTopParent()->toString() == "self"){
    //std::cout << "***** inst includes self" << endl;
    //insts.insert(insts.end(), in0_insts.begin(), in0_wire);
    
  } else {
    Instance* in0_inst = getSelectedInst(head, "in0");
    if (opName == getOpName(in0_inst)) {
      vector<Instance*> in0_insts = collectInsts(in0_inst);
      insts.insert(insts.end(), in0_insts.begin(), in0_insts.end());
    }
  }

  // check in1 branch
  Wireable* in1_wire = getConnectedWireable(head, head->getInstname() + ".in1");
  //std::cout << "wire=" << in1_wire->toString() << " parent=" << in1_wire->getTopParent()->toString() << std::endl;
  
  if (in1_wire->getTopParent()->toString() == "self"){
    //std::cout << "***** inst includes self" << endl;
    //insts.insert(insts.end(), in1_insts.begin(), in1_wire);
  } else {
    Instance* in1_inst = getSelectedInst(head, "in1");
    if (opName == getOpName(in1_inst)) {
      vector<Instance*> in1_insts = collectInsts(in1_inst);
      insts.insert(insts.end(), in1_insts.begin(), in1_insts.end());
    }
  }

  return insts;
}


bool MapperPasses::TreeReduction::runOnModule(Module* m) {
  //Context* c = this->getContext();
  
  // early out if module is undefined
  if (!m->hasDef()) return false;
  ModuleDef* def = m->getDef();
  //std::cout << "Got def" << endl;
  
  //Define our vector of instances to replace
  vector<Instance*> treeHeads;
  unordered_set<std::string> operators = {"coreir.add", "coreir.mul", "corebit.and", "corebit.or", 
                                          "commonlib.smin", "commonlib.smax", "commonlib.umin", "commonlib.umax"};
  //std::cout << "Running tree reduction!" << endl;

  //Loop through all the instances 
  for (auto instmap : def->getInstances()) {
    Instance* inst = instmap.second;
    std::string opName = getOpName(inst);
    //std::cout << "checking " << opName << endl;

    if (operators.count(opName)>0 && isAssocSubgraph(inst)) {
      Instance* in0Inst = getSelectedInst(inst, "in0");
      Instance* in1Inst = getSelectedInst(inst, "in1");
      if ((in0Inst != NULL && getOpName(in0Inst) == opName) ||
          (in1Inst != NULL && getOpName(in1Inst) == opName)) {
        //cout << "found inst to replace" << endl;
        treeHeads.push_back(inst);
      }
    }
  }

  //std::cout << "time to plant a tree" << endl;
  //Loop through all instances and replace with a tree implementation
  //Namespace* commonlib = CoreIRLoadLibrary_commonlib(c);
  Generator* opN = commonlib->getGenerator("opn");
  Generator* bitopN = commonlib->getGenerator("bitopn");
  for (auto headInst : treeHeads) {

    //std::cout << "starting at head" << endl;
    // find boundary interface for reduction tree
    vector<pair<Wireable*, Wireable*>> inputConxs = collectInputConnections(headInst);
    //std::cout << "inputs collected" << endl;
    auto insts = collectInsts(headInst);
    //std::cout << "insts collected" << endl;
    
    std::cout << headInst->toString() << " has " << inputConxs.size() << " inputs:" << endl;
    for (auto inst_pair : inputConxs) {
      std::cout << " " << inst_pair.first->toString();
    }
    std::cout << endl;

    // create tree version
    string opName = getOpName(headInst);
    auto arg_op = Const::make(c, opName);

    auto arg_N = Const::make(c,inputConxs.size());

    Type* out_type = headInst->sel("out")->getType();
    Instance* tree;
    if (out_type->getKind() == Type::TypeKind::TK_Array) {
      auto arg_width = Const::make(c,static_cast<ArrayType*>(out_type)->getLen());

      string tree_name = headInst->getInstname() + "_tree";

      tree = def->addInstance(tree_name, opN,
                                        {{"width",arg_width},{"N",arg_N},{"operator",arg_op}}
        );
      
    } else {
      // this is a bit operator
      string tree_name = headInst->getInstname() + "_tree";

      tree = def->addInstance(tree_name, bitopN,
                                        {{"N",arg_N},{"operator",arg_op}}
        );
    }
    
    targetSubgraphs.push_back(tree);

    Wireable* pt_out_wire = getConnectedWireable(headInst, headInst->getInstname() + ".out");
    headInst->disconnect();
    //Wireable* pt_out_wire = pt_out->sel("in");
    
    vector<Wireable*> pt_inputs;
    for (auto input_pair : inputConxs) {
      auto input = input_pair.first;
      // add port that is connected to the input
      Wireable* pt_in_wire = getConnectedWireable(input, input->toString());
      pt_inputs.push_back(pt_in_wire);
      // disconnect the exisiting wire
      input->getContainer()->disconnect(input, input_pair.second);
      //std::cout << "disconnected " << input->toString() << " from " << input_pair.second << endl;
    }

    // remove old instances
    cout << " " << headInst->toString() << " had the insts:" << endl;
    for (auto inst : insts) {
      cout << " " << inst->toString();
      def->removeInstance(inst);
    }
    cout << endl;

    // wire up new tree version
    //std::cout << "connecting output" << endl;
    def->connect(pt_out_wire, tree->sel("out"));
    //std::cout << "connecting " << pt_inputs.size() << " inputs" << endl;
    for (uint j=0; j<pt_inputs.size(); ++j) {
      Wireable* pt_in = pt_inputs[j];
      //std::cout << "connecting " << pt_in->toString() << " to tree " << std::to_string(j) << std::endl;
      //std::cout << "connecting " << " to tree " << std::to_string(j) << std::endl;
      def->connect(pt_in, tree->sel("in")->sel(j));
    }
  }

  //Add this current module to the user datastructure we defined before
  if (getTotalSubgraphs() > 0 ) {
    return true;
  } else {
    return false;
  }
}

// return string for operator used in given instance
string MapperPasses::TreeReduction::getOpName(Instance* i) {
  //std::string opName = i->getInstantiableRef()->getName();
  //std::string opName = i->getInstname();
  if (i->getModuleRef()->hasDef()) {
    std::string opName = i->getModuleRef()->getDef()->getName();
    return opName;
  } else {
    return i->getModuleRef()->getRefName();
  }
}

// return the instance that is connected to given instance select
Instance* MapperPasses::TreeReduction::getSelectedInst(Instance* i, string sel) {
  if (!i->canSel(sel)) {
    return NULL;
  }

  Select* connectedSel = NULL;
  LocalConnections conxs = i->getLocalConnections();
  for (auto conx : conxs) {
    Select* thisWire = static_cast<Select*>(conx.first);

    if (thisWire->getSelStr() == sel) {
      //cout << conx.first->toString() << " connected to " << conx.second->toString() << endl;
      if (conx.second->getKind() != Wireable::WireableKind::WK_Select) { continue; }
      connectedSel = static_cast<Select*>(conx.second);
    }
  }
  if (connectedSel == NULL) { std::cout << "couldn't find sel" << endl; return NULL; }

  Wireable* parentWire = connectedSel->getParent();
  //std::cout << "sel=" << connectedSel->toString() << " parent=" << parentWire->toString() << std::endl;
  //std::cout << "parent is a " << parentWire->getKind() << endl;
  if (parentWire->getKind() == Wireable::WireableKind::WK_Select) { parentWire = parentWire->getTopParent(); }
  if (parentWire->getKind() != Wireable::WireableKind::WK_Instance) { /*std::cout << "wire not inst" << endl;*/ return NULL; }

  Instance* parentInst = static_cast<Instance*>(parentWire);
  return parentInst;
}

// identify if this is the top of a series of the same operator
bool MapperPasses::TreeReduction::isAssocSubgraph(Instance* i) {
  Instance* parentInst = getSelectedInst(i, "out");
  if (parentInst == NULL) { return true; }

  std::string opName = getOpName(i);
  std::string parentOpName = getOpName(parentInst);

  //cout << "  " << opName << "  connected to " << parentOpName << endl;

  if (parentOpName == opName) {
    return false;
  } else {
    return true;
  }
}

int MapperPasses::TreeReduction::getTotalSubgraphs() {
  return targetSubgraphs.size();
}

void MapperPasses::TreeReduction::print() {
  cout << "Replaced " << getTotalSubgraphs() << " subgraphs into reduction trees" << endl;
}

void coreir_tree_reduction(std::string coreir_design_filename,
                           std::string output_design_filename) {
  
  // New context for coreir test
  Context* context = newContext();
  Namespace* g = context->getGlobal();

  auto commonlib = CoreIRLoadLibrary_commonlib(context);
  CoreIRLoadLibrary_lakelib(context);
  CoreIRLoadLibrary_float(context);
  CoreIRLoadLibrary_float_DW(context);
  CoreIRLoadLibrary_cgralib(context);

  Module* top;
  if (!loadFromFile(context, coreir_design_filename, &top)) {
    cout << "Could not load " << coreir_design_filename
         << " from json!!" << endl;
    context->die();
  }

  auto pass = new MapperPasses::TreeReduction();
  pass->c = context;
  pass->commonlib = commonlib;
  //std::cout << "pass created" << endl;
  
  std::cout << "design top exists=" << context->hasTop() << endl;
  //Module* design = g->getModule("DesignTop");
  Module* design = top;
  if (design != nullptr) {
    bool modified = pass->runOnModule(design);
  } else {
    std::map<std::string, Module*> moduleList = context->getNamespace("global")->getModules();
    bool modified = false;
    for (auto nameAndModule : moduleList) {
      bool module_modified = pass->runOnModule(nameAndModule.second);
      modified = modified || module_modified;
    }
  }

  pass->print();
  saveToFile(context, output_design_filename, NULL);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    cout << "Usage:\n"
         << "  coreir_tree_reduction [design].json [design_new].json"
         << endl;
    return 1;
  }

  coreir_tree_reduction(argv[1], argv[2]);
  return 0;
}
