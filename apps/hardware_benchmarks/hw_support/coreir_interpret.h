#include "HalideBuffer.h"

#include "coreir.h"
#include "coreir/simulator/interpreter.h"


template <typename elem_t>
class ImageWriter {
public:
  ImageWriter(Halide::Runtime::Buffer<elem_t> &output) :
    width(output.width()), height(output.height()), channels(output.channels()),
    image(output),
    current_x(0), current_y(0), current_z(0) { }

  void write(elem_t data);
  elem_t read(uint x, uint y, uint z);
  void save_image(std::string image_name);
  void print_coords();

  uint getHeight() const { return height; }
  uint getWidth() const { return width; }
  uint getChannels() const { return channels; }

private:
  const uint width, height, channels;
  Halide::Runtime::Buffer<elem_t> image;
  uint current_x, current_y, current_z;
};


bool reset_coreir_circuit(CoreIR::SimulatorState &state, CoreIR::Module *m);

template<typename T>
void run_coreir_on_interpreter(std::string coreir_design,
                               Halide::Runtime::Buffer<T> input,
                               Halide::Runtime::Buffer<T> output,
                               std::string input_name,
                               std::string output_name,
                               bool has_float_input=false,
                               bool has_float_output=false);


