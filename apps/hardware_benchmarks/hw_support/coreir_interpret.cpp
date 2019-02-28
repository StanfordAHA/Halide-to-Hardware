#include "coreir/passes/transform/rungenerators.h"

#include "coreir_interpret.h"

using namespace std;
using namespace CoreIR;

template <typename elem_t>
void ImageWriter<elem_t>::write(elem_t data) {
  if (current_x < width &&
      current_y < height &&
      current_z < channels) {

    assert(current_x < width &&
           current_y < height &&
           current_z < channels);
    image(current_x, current_y, current_z) = data;

    // increment coords
    current_x++;
    if (current_x == width) {
      current_y++;
      current_x = 0;
    }
    if (current_y == height) {
      current_z++;
      current_y = 0;
    }
  }
}

template <typename elem_t>
elem_t ImageWriter<elem_t>::read(uint x, uint y, uint z) {
  return image(x,y,z);
}

template <typename elem_t>
void ImageWriter<elem_t>::save_image(std::string image_name) {
  convert_and_save_image(image, image_name);
}

template <typename elem_t>
void ImageWriter<elem_t>::print_coords() {
  std::cout << "x=" << current_x
            << ",y=" << current_y
            << ",z=" << current_z << std::endl;
}

// This sets each input for the coreir simulator before testing.
// Returns if a wire for output valid is found.
bool reset_coreir_circuit(SimulatorState &state, Module *m) {

  auto self_conxs = m->getDef()->sel("self")->getLocalConnections();
  set<string> visited_connections;
  bool uses_valid = false;
  
  for (auto wireable_pair : self_conxs) {
    //cout << wireable_pair.first->toString() << " is connected to " << wireable_pair.second->toString() << endl;

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
      if (port_name.find("[") != string::npos) {
        string port_name_wo_index = port_name.substr(0, port_name.find("["));
        state.setValue(port_name_wo_index, BitVector(1));

        cout << "reset " << port_name << " as indexed port "
             << port_name_wo_index << " with size 1" << endl;
        
      } else {
        auto port_output = static_cast<BitType*>(port_type);
        uint type_bitwidth = port_output->getSize();
        state.setValue(port_name, BitVector(type_bitwidth));
      
        cout << "reset " << port_name << " with size " << type_bitwidth << endl;

      }
    }
  }
  return uses_valid;

}

bool circuit_uses_valid(Module *m) {
  bool uses_valid = false;
  auto self_conxs = m->getDef()->sel("self")->getLocalConnections();
  for (auto wireable_pair : self_conxs) {
    string port_name = wireable_pair.first->toString();
    if (port_name == "self.valid") {
      uses_valid = true;
      return uses_valid;
    }
  }

  // no valid found
  return uses_valid;
}

template<typename T>
void run_coreir_on_interpreter(string coreir_design,
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

      }
    }
  }
  coreir_img_writer.print_coords();

  deleteContext(c);
  printf("finished running CoreIR code\n");

}

// declare which types will be used with template function
template void run_coreir_on_interpreter<uint16_t>(std::string coreir_design,
                                                  Halide::Runtime::Buffer<uint16_t> input,
                                                  Halide::Runtime::Buffer<uint16_t> output,
                                                  std::string input_name,
                                                  std::string output_name);

template void run_coreir_on_interpreter<int16_t>(std::string coreir_design,
                                                 Halide::Runtime::Buffer<int16_t> input,
                                                 Halide::Runtime::Buffer<int16_t> output,
                                                 std::string input_name,
                                                 std::string output_name);

template void run_coreir_on_interpreter<uint8_t>(std::string coreir_design,
                                                 Halide::Runtime::Buffer<uint8_t> input,
                                                 Halide::Runtime::Buffer<uint8_t> output,
                                                 std::string input_name,
                                                 std::string output_name);

template void run_coreir_on_interpreter<int8_t>(std::string coreir_design,
                                                Halide::Runtime::Buffer<int8_t> input,
                                                Halide::Runtime::Buffer<int8_t> output,
                                                std::string input_name,
                                                std::string output_name);

template void run_coreir_on_interpreter<bool>(std::string coreir_design,
                                              Halide::Runtime::Buffer<bool> input,
                                              Halide::Runtime::Buffer<bool> output,
                                              std::string input_name,
                                              std::string output_name);

