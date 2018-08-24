#include <cstdio>
#include <iostream>

#include "HalideBuffer.h"
#include "halide_image_io.h"
#include "halide_benchmark.h"

#include "hardware_process_helper.h"
#include "hardware_image_helpers.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int ProcessController::process_command(int argc, char **argv) {
  if (argc < 2) {
    print_usage();
    return 1;
  }

  std::vector<std::string> full_args(argv, argv + argc);
  std::string command = full_args[1];
  std::vector<std::string> args(argv + 2, argv + argc);
  
  if (command == "image") {
    return make_image_def(args);
    
  } else if (command == "run") {
    return make_run_def(args);
    
  } else if (command == "compare") {
    return make_compare_def(args);
    
  } else if (command == "test") {
    return make_test_def(args);
    
  } else if (command == "eval") {
    return make_eval_def(args);
    
  } else {
    print_usage();
    return 1;
  }
}

int ProcessController::make_image_def(std::vector<std::string> args) {
  std::cout << "make image not defined\n";
  return 1;
}

int ProcessController::make_run_def(std::vector<std::string> args) {
  std::cout << "make run not defined\n";
  return 1;
}

int ProcessController::make_compare_def(std::vector<std::string> args) {
  std::cout << "make compare not defined\n";
  return 1;
}

int ProcessController::make_test_def(std::vector<std::string> args) {
  std::cout << "make test not defined\n";
  return 1;
}

int ProcessController::make_eval_def(std::vector<std::string> args) {
  std::cout << "make eval not defined\n";
  return 1;
}

void ProcessController::print_usage() {
  std::cout << "Usage:\n"
            << "  make image\n"
            << "  make run\n"
            << "  make compare\n"
            << "  make test\n"
            << "  make eval\n";
}

int OneInOneOut_ProcessController::make_image_def(std::vector<std::string> args) {
  if (args.size() != 0) {
    std::cout << "Usage:\n"
              << "  make image\n";
    return 1;
  }

  create_image(&input);
  save_image(input, "bin/input.png");

  std::cout << "Generated and saved input image\n";
  return 0;
}

int OneInOneOut_ProcessController::make_run_def(std::vector<std::string> args) {
  // Input image: create or load
  if (args.size() == 0) {
    create_image(&input);
  } else if (args.size() == 1) {
    input = load_and_convert_image(args[0]);
  } else {
    std::cout << "Usage:\n"
              << "  make run-" << hardware_name << " {input.png}\n"
              << "  Note: input.png is optional\n";
    return 1;
  }

  // run on input image
  run_call();
  convert_and_save_image(output, "bin/output.png");
    
  std::cout << "Ran " << design_name << "\n";
  return 0;

}

int OneInOneOut_ProcessController::make_compare_def(std::vector<std::string> args) {
  Buffer<uint16_t> output_comparison(output);
    
  // Input image: create or load images
  if (args.size() == 2) {
    output = load_and_convert_image(args[0]);
    output_comparison = load_and_convert_image(args[1]);
  } else {
    printf("Usage:\n");
    printf("  make compare output0.png output1.png\n");
    return 1;
  }

  // compare images
  bool equal_images = compare_images(output, output_comparison);

  if (equal_images) {
    printf("Images are equivalent!\n");
    return 0;
  } else {
    printf("Images are different...\n");
    return 1;
  }

}

int OneInOneOut_ProcessController::make_test_def(std::vector<std::string> args) {
  std::cout << "make test: Not yet defined" << std::endl;
  return 1;
}

int OneInOneOut_ProcessController::make_eval_def(std::vector<std::string> args) {
  // Input image: create or load
  if (args.size() == 0) {
    create_image(&input);
  } else if (args.size() == 1) {
    input = load_and_convert_image(args[0]);
  } else {
    printf("Usage:\n");
    printf("  make eval {input.png}\n");
    printf("  Note: input.png is optional");
    return 1;
  }

  // Timing code
  double min_t_manual = benchmark(10, 10, [&]() {
      run_call();
    });
  printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

  return 0;
}

