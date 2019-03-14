#include <cstdio>

#include "fp_conv_3_3.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint16_t> processor("fp_conv_3_3",
                                            {
                                              {"cpu",
                                                  [&]() { fp_conv_3_3(processor.input, processor.output); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              }

                                            });

  processor.input = Buffer<uint16_t>(64, 64);
  processor.output = Buffer<uint16_t>(62, 62);
  
  processor.process_command(argc, argv);


  // Buffer<uint8_t> old_input;
  // Buffer<uint8_t> new_input(64, 64);
  // 
  // old_input = load_and_convert_image("input.png");
  // for (int i = 0; i < processor.input.width(); ++i) {
  //   for (int j = 0; j < processor.input.height(); ++j) {
  //     new_input(i, j) = i+j;
  //   }
  // }
  // convert_and_save_image(new_input, "new_input.png");
  
}
