#include <cstdio>

#include "conv_2_1.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;
using namespace std;


#include "coreir_interpret.cpp"
using namespace CoreIR;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint16_t> processor("conv_2_1",
                                          {
                                            {"cpu",
                                                [&]() { conv_2_1(processor.input, processor.output); }
                                            },
                                            {"coreir",
                                                [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                    "self.in_arg_0_0_0", "self.out_0_0"); }
                                            }
                                          });

  processor.input = Buffer<uint16_t>(64, 64);
  processor.output = Buffer<uint16_t>(64, 63);
  
  processor.process_command(argc, argv);
  /*
  {
    string coreir_design = "bin/design_top.json";
    Halide::Runtime::Buffer<uint16_t> input = processor.input;
    Halide::Runtime::Buffer<uint16_t> output = Buffer<uint16_t>(63, 63);
    string input_name = "self.in_arg_0_0_0";
    string output_name = "self.out_0_0";

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

    // This sets each input for the coreir simulator before testing.
    auto self_conxs = m->getDef()->sel("self")->getLocalConnections();
    set<string> visited_connections;
    bool uses_valid = false;
  
    for (auto wireable_pair : self_conxs) {
      cout << wireable_pair.first->toString() << " is connected to "
           << wireable_pair.second->toString() << endl;

      string port_name = wireable_pair.first->toString();
      Type* port_type = wireable_pair.first->getType();

      // only process each connection once
      if (visited_connections.count(port_name) > 0) {
        continue;
      }
      visited_connections.insert(port_name);

      // identify the valid signal
      if (port_name == "self.valid") {
        cout << "image is using output valid" << endl;
        uses_valid = true;
      }

      if ("self.clk" == port_name) {
        state.setClock(port_name, 0, 1);
      
        cout << "reset clock " << port_name << endl;
      
      } else if (port_type->isOutput()) {
        if (port_name.find("[")) {
          string port_name_wo_index = port_name.substr(0, port_name.find("["));
          state.setValue(port_name_wo_index, BitVector(1));

          cout << "reset indexed port " << port_name_wo_index << " with size 1" << endl;
        
        } else {
          auto port_output = static_cast<BitType*>(port_type);
          uint type_bitwidth = port_output->getSize();
          state.setValue(port_name, BitVector(type_bitwidth));
      
          cout << "reset " << port_name << " with size " << type_bitwidth << endl;

        }
      }
    }


    //  state.setValue(input_name, BitVector(16));
    //  state.setValue("self.reset", BitVector(1));
    //  state.setClock("self.clk", 0, 1);
    cout << "starting coreir simulation" << endl;  
    state.resetCircuit();

    //state.setClock("self.clk", 0, 1);

    ImageWriter<uint16_t> coreir_img_writer(output);

    for (int y = 0; y < input.height(); y++) {
      for (int x = 0; x < input.width(); x++) {
        for (int c = 0; c < input.channels(); c++) {
          // set input value
          state.setValue(input_name, BitVector(16, input(x,y,c)));

          // propogate to all wires
          state.exeCombinational();
          
          // read output wire
          if (uses_valid) {
            bool valid_value = state.getBitVec("self.valid").to_type<bool>();

            if (valid_value) {
              coreir_img_writer.write(state.getBitVec(output_name).to_type<uint16_t>());
              uint16_t val0 = state.getBitVec("add_296_300_301.in0").to_type<uint16_t>();
              uint16_t val1 = state.getBitVec("add_296_300_301.in1").to_type<uint16_t>();
              uint16_t val2 = state.getBitVec("add_296_300_301.out").to_type<uint16_t>();
              cout << "y=" << y << "x=" << x << std::hex << ":  val0=" << val0 << " val1=" << val1
                   << " sum=" << val2 << std::dec << endl;

            }
          } else {
            output(x,y,c) = state.getBitVec(output_name).to_type<uint16_t>();

            uint16_t val0 = state.getBitVec("add_303_307_308.in0").to_type<uint16_t>();
            uint16_t val1 = state.getBitVec("add_303_307_308.in1").to_type<uint16_t>();
            uint16_t val2 = state.getBitVec("add_303_307_308.out").to_type<uint16_t>();
            cout << "y=" << y << "x=" << x << std::hex << ":  val0=" << val0 << " val1=" << val1
                 << " sum=" << val2 << std::dec << endl;

          }
        
          // give another rising edge (execute seq)
          state.exeSequential();

        }
      }
    }
    coreir_img_writer.print_coords();

    deleteContext(c);
    printf("finished running CoreIR code\n");


  }*/
}
