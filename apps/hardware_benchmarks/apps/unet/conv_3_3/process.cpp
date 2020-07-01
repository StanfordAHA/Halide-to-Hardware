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
  auto input_copy_stencil = processor.inputs["input.png"];
  for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
    for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
      for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
        input_copy_stencil(z, x, y) = z + x + y;
        //input_copy_stencil(z, x, y) = 1;
  } } }

  std::cout << "input has dims: " << processor.inputs["input.png"].dim(0).extent() << "x"
            << processor.inputs["input.png"].dim(1).extent() << "x"
            << processor.inputs["input.png"].dim(2).extent() << "\n";

  
  processor.inputs["kernel.png"] = Buffer<uint8_t>(z, w, k_x, k_y);
  auto kernel_copy_stencil = processor.inputs["kernel.png"];
  for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
    for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
      for (int w = 0; w < kernel_copy_stencil.dim(1).extent(); w++) {
        for (int z = 0; z < kernel_copy_stencil.dim(0).extent(); z++) {
          kernel_copy_stencil(z, w, x, y) = z + w + x + y;
          //kernel_copy_stencil(z, w, x, y) = 1;
   } } } }
  
  std::cout << "kernel has dims: " << processor.inputs["kernel.png"].dim(0).extent() << "x"
            << processor.inputs["kernel.png"].dim(1).extent() << "x"
            << processor.inputs["kernel.png"].dim(2).extent() << "x"
            << processor.inputs["kernel.png"].dim(3).extent() << "\n";
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
