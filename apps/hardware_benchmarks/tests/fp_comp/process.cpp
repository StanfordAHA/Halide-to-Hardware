#include <cstdio>

#include "fp_comp.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("fp_comp",
                                          {
                                            {"cpu",
                                                [&]() { fp_comp(processor.input, processor.output); }
                                            },
                                            {"coreir",
                                                [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                    "self.in_arg_0_0_0", "self.out_0_0",
                                                                                    /*has_float_input=*/true, /*has_float_output=*/false); }
                                            }
                                          });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(64, 64);
  
  processor.process_command(argc, argv);

  for (size_t i=0; i<10; ++i) {
    std::cout << "input(" << i << ",0) = " << +processor.input(i,0)
              << "   /pi = " << +processor.output(i,0) << std::endl;
  }

  
}
