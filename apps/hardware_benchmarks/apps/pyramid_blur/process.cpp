#include <cstdio>

#include "cascade.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"
#include "clockwork_testscript.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

const int inImgSize = 64;
//const int outImgSize = inImgSize - 2;
const int outImgSize = inImgSize - 4;

typedef uint8_t Pixel;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<Pixel> processor("cascade",
                                            {
                                              {"cpu",
                                                  [&]() { cascade(processor.input, processor.output); }
                                              },
                                              {"rewrite",
                                                  [&]() { run_coreir_rewrite_on_interpreter<>("bin/design_top.json", "bin/ubuffers.json", processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_0_0_0", "self.out_0_0"); }
                                              },
                                              {"clockwork",
                                                  [&]() { run_clockwork_program<>(processor.input, processor.output); }
                                              }

                                            });

  processor.input = Buffer<Pixel>(inImgSize, inImgSize);
  processor.output = Buffer<Pixel>(outImgSize, outImgSize);

  return processor.process_command(argc, argv);

}
