#include <cstdio>

#include "harris.h"
#include "coreir/libs/commonlib.h"
#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"
#include "clockwork_testscript.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;
using namespace CoreIR;
using namespace std;

int main(int argc, char **argv) {

  OneInOneOut_ProcessController<uint8_t> processor("harris",
                                            {
                                              {"cpu",
                                                  [&]() { harris(processor.input, processor.output); }
                                              },
                                              {"rewrite",
                                                  [&]() { run_coreir_rewrite_on_interpreter<>("bin/design_top.json", "bin/ubuffers.json", processor.input, processor.output,
                                                                                      "self.in_arg_1_0_0", "self.out_0_0"); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_1_0_0", "self.out_0_0"); }
                                              },
                                              {"clockwork",
                                                  [&]() { run_clockwork_program<>(processor.input, processor.output); }
                                              }
                                            });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(58, 58);

  processor.process_command(argc, argv);

}

