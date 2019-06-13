#include "coreir/libs/commonlib.h"
#include "coreir/libs/float.h"
#include "coreir/passes/transform/rungenerators.h"

using namespace std;
using namespace CoreIR;

void run_coreir_on_interpreter(std::string coreir_design_filename,
                               std::string output_dot_filename) {
  // New context for coreir test
  Context* context = newContext();
  Namespace* g = context->getGlobal();

  CoreIRLoadLibrary_commonlib(context);
  CoreIRLoadLibrary_float(context);
  if (!loadFromFile(context, coreir_design)) {
    cout << "Could not load " << coreir_design
         << " from json!!" << endl;
    context->die();
  }

  context->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});

  Module* design = g->getModule("DesignTop");
  assert(design != nullptr);

  if (!saveToDot(design, output_dot_filename)) {
    cout << "Could not save to dot!!" << endl;
    context->die();
  }


}
