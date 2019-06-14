#include <cstdio>

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("handcrafted_ub_matvec_mult",
                                            {
                                              {"cpu",
                                                  [&]() {  }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(2, 256);
  processor.output = Buffer<uint8_t>(4, 256);
  auto input = processor.input;
  auto output = processor.output;
  
  processor.process_command(argc, argv);

  for (int y = 0; y < input.height(); y++) {
    output(0, y) = input(0, y) * 1 + input(1, y) * 3;
    output(1, y) = input(0, y) * 2 + input(1, y) * 4;
    output(2, y) = input(0, y) * 3 + input(1, y) * 6;
    output(3, y) = input(0, y) * 5 + input(1, y) * 2;
  }

  save_image(output, "bin/output_cpu.png");
  
}
