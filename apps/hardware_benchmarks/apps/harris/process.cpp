#include <cstdio>

#include "harris.h"
#include "coreir/libs/commonlib.h"
#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;
using namespace CoreIR;
using namespace std;
/*
template<typename T>
void run_harris_on_interpreter(string coreir_design,
                               Halide::Runtime::Buffer<T> input,
                               Halide::Runtime::Buffer<T> output,
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

  if (!saveToFile(g, "bin/design_simulated.json", m)) {
    cout << "Could not save to json!!" << endl;
    c->die();
  }
  cout << "generated simulated coreir design" << endl;

  // sets initial values for all inputs/outputs/clock
  bool uses_valid = reset_coreir_circuit(state, m);

  cout << "starting coreir simulation" << endl;
  state.resetCircuit();

  ImageWriter<T> coreir_img_writer(output);

  for (int y = 0; y < input.height(); y++) {
    for (int x = 0; x < input.width(); x++) {
      for (int c = 0; c < input.channels(); c++) {
        // set input value
        //state.setValue(input_name, BitVector(16, input(x,y,c) & 0xff));
        state.setValue(input_name, BitVector(16, input(x,y,c)));

        // propogate to all wires
        state.exeCombinational();

        // read output wire
        if (uses_valid) {
          bool valid_value = state.getBitVec("self.valid").to_type<bool>();

          if (valid_value) {
            T output_value = state.getBitVec(output_name).to_type<T>();
            coreir_img_writer.write(output_value);
            //std::cout << "y=" << y << ",x=" << x << " " << hex << "in=" << (input(x,y,c) & 0xff) << " out=" << output_value << dec << endl;
          }
        } else {
          T output_value = state.getBitVec(output_name).to_type<T>();
          output(x,y,c) = output_value;
          //std::cout << "y=" << y << ",x=" << x << " " << hex << "in=" << (input(x,y,c) & 0xff) << " out=" << output_value << dec << endl;
        }

        // give another rising edge (execute seq)
        state.exeSequential();

      //}
    //}
  //}
  //coreir_img_writer.print_coords();

  //deleteContext(c);
  //printf("finished running CoreIR code\n");

//}

void run_harris_on_interpreter(string coreir_design,
                               Halide::Runtime::Buffer<uint8_t> input,
                               Halide::Runtime::Buffer<uint8_t> output,
                               string input_name,
                               string output_name);
*/
int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("harris",
                                            {
                                              {"cpu",
                                                  [&]() { harris(processor.input, processor.output); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_rewrite_on_interpreter<>("bin/design_top.json", "bin/ubuffers.json", processor.input, processor.output,
                                                                                      "self.in_arg_1_0_0", "self.out_0_0"); }
                                                  //[&]() { run_harris_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      //"self.in_arg_1_0_0", "self.out_0_0"); }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(58, 58);

  processor.process_command(argc, argv);

}

