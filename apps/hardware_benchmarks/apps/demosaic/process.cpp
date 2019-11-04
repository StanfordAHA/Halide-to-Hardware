
#include <cstdio>

#include "demosaic.h"

#include "hardware_process_helper.h"
#include "coreir_interpret.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

//#include <cstdio>
//#include <chrono>

////#include "harris.h"

//#include "halide_benchmark.h"
//#include "HalideBuffer.h"

//using namespace Halide::Tools;
//using namespace Halide::Runtime;

int main(int argc, char **argv) {
  OneInOneOut_ProcessController<uint8_t> processor("cascade",
                                            {
                                              {"cpu",
                                                  [&]() { demosaic(processor.input, processor.output); }
                                              },
                                              {"coreir",
                                                  [&]() { run_coreir_on_interpreter<>("bin/design_top.json", processor.input, processor.output,
                                                                                      "self.in_arg_3_0_0", "self.out_2_0_0"); }
                                                                                      //"self.in_arg_0_0_0", "self.out_0_0"); }
                                              }

                                            });

  processor.input = Buffer<uint8_t>(64, 64);
  processor.output = Buffer<uint8_t>(60, 60);
  
  processor.process_command(argc, argv);
  
  //Buffer<int16_t> input(70, 70);

  //for (int y = 0; y < input.height(); y++) {
    //for (int x = 0; x < input.width(); x++) {
      //input(x, y) = rand();
    //}
  //}

  //Buffer<int16_t> output(64, 64);

  //harris(input, output);

  //// Timing code
  //double min_t_manual = benchmark(10, 10, [&]() {
      //harris(input, output);
    //});
  //printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

  //return 0;
}
