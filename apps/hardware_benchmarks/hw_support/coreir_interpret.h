#include "HalideBuffer.h"

template<typename T>
void run_coreir_on_interpreter(std::string coreir_design,
                               Halide::Runtime::Buffer<T> input,
                               Halide::Runtime::Buffer<T> output,
                               std::string input_name,
                               std::string output_name);


