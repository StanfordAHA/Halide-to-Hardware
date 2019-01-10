#include "HalideBuffer.h"

void run_coreir_on_interpreter(std::string coreir_design,
                               Halide::Runtime::Buffer<uint16_t> input,
                               Halide::Runtime::Buffer<uint16_t> output,
                               std::string input_name,
                               std::string output_name);
