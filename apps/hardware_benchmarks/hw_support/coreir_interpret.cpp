#include "coreir.h"
#include "coreir/passes/transform/rungenerators.h"
#include "coreir/simulator/interpreter.h"
#include "coreir/libs/commonlib.h"

#include "coreir_interpret.h"

using namespace std;
using namespace CoreIR;

void run_coreir_on_interpreter(string coreir_design,
                               Halide::Runtime::Buffer<uint16_t> input,
                               Halide::Runtime::Buffer<uint16_t> output,
                               string input_name,
                               string output_name) {
  // New context for coreir test
  Context* c = newContext();
  Namespace* g = c->getGlobal();

  CoreIRLoadLibrary_commonlib(c);
  if (!loadFromFile(c, coreir_design)) {
    cout << "Could not load " << coreir_design
         << " from json!!" << endl;
    c->die();
  }

  c->runPasses({"rungenerators", "flattentypes", "flatten", "wireclocks-coreir"});


  Module* m = g->getModule("DesignTop");
  assert(m != nullptr);
  SimulatorState state(m);

  state.setValue(input_name, BitVector(16));
  state.resetCircuit();
  //state.setClock("self.clk", 0, 1);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      for (int c = 0; c < input.channels(); c++) {
        // set input value
        state.setValue(input_name, BitVector(16, input(x,y,c)));

        // propogate to all wires
        state.exeCombinational();

        // read output wire
        output(x,y,c) = state.getBitVec(output_name).to_type<uint16_t>();

        // give another rising edge (execute seq)
        state.exeSequential();

      }
    }
  }

  deleteContext(c);
  printf("finished running CoreIR code\n");

}
