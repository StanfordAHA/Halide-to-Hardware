#include <cstdio>

#include "conv_3_3.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"
#include "clockwork_testscript.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
  int x = 16;
  int y = 16;
  int k_x = 3;
  int k_y = 3;
  int z = 2;
  int w = 4;

  ManyInOneOut_ProcessController<uint8_t> processor("unet_conv_3_3", {"input.png", "kernel.png"},
                                            {
                                              {"cpu",
                                                  [&]() { conv_3_3(processor.inputs["input.png"], processor.inputs["kernel.png"], processor.output); }
                                              },
                                              {"rewrite",
                                                  [&]() { run_coreir_rewrite_on_interpreter<>("bin/design_top.json", "bin/ubuffers.json",
                                                                                       processor.inputs["input.png"], processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json",
                                                                                       processor.inputs["input.png"], processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"clockwork",
                                                  [&]() { run_clockwork_program<>(processor.inputs["input.png"], processor.inputs["kernel.png"], processor.output); }
                                                  //[&]() { run_clockwork_program<>(processor.inputs["input.png"], processor.output); }
                                              }
                                            });
  
  processor.inputs["input.png"] = Buffer<uint8_t>(z, x, y);
  processor.inputs["kernel.png"] = Buffer<uint8_t>(z, k_x, k_y, w);
  processor.inputs_preset = true;
  processor.output = Buffer<uint8_t>(x - k_x + 1, y - k_y + 1, w);

  return processor.process_command(argc, argv);
  
  //std::vector<Buffer<uint8_t>*> inputs;
  //std::vector<Buffer<uint8_t>*> outputs;
  //Buffer<uint8_t> image(x, y, z);
  //Buffer<uint8_t> kernel(k_x, k_y, z, w);
  //Buffer<uint8_t> output(x - k_x + 1, y - k_y + 1, w);
  //inputs.push_back(&image);
  //inputs.push_back(&kernel);
  //outputs.push_back(&output);
  //
  //General_ProcessController<uint8_t> processor("conv_3_3",
  //                                          {
  //                                            {"cpu",
  //                                                [&]() { conv_3_3(*inputs[0], *inputs[1], *outputs[0]); }
  //                                            },
  //                                            {"coreir",
  //                                                [&]() { run_coreir_on_interpreter<>("bin/design_top.json", *inputs[0], *outputs[0],
  //                                                                                    "self.in_arg_0_0_0", "self.out_0_0"); }
  //                                            }
  //
  //                                          }, &inputs, &outputs);
 
  processor.process_command(argc, argv);
  
}
