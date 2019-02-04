#include <cstdio>
#include <iostream>

#include "HalideBuffer.h"
#include "halide_image_io.h"
#include "halide_benchmark.h"

#include "hardware_process_helper.h"
#include "hardware_image_helpers.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

template <class T>
int ProcessController<T>::process_command(int argc, char **argv) {
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

template <class T>
int ProcessController<T>::make_image_def(std::vector<std::string> args) {
  std::cout << "make image not defined\n";
  return 1;
}

template <class T>
int ProcessController<T>::make_run_def(std::vector<std::string> args) {
  std::cout << "make run not defined\n";
  return 1;
}

template <class T>
int ProcessController<T>::make_compare_def(std::vector<std::string> args) {
  std::cout << "make compare not defined\n";
  return 1;
}

template <class T>
int ProcessController<T>::make_test_def(std::vector<std::string> args) {
  std::cout << "make test not defined\n";
  return 1;
}

template <class T>
int ProcessController<T>::make_eval_def(std::vector<std::string> args) {
  std::cout << "make eval not defined\n";
  return 1;
}

template <class T>
void ProcessController<T>::print_usage() {
  std::cout << "Usage:\n"
            << "  make image\n"
            << "  make run\n"
            << "  make compare\n"
            << "  make test\n"
            << "  make eval\n";
}

std::string enumerate_keys(std::map<std::string, std::function<void()>> run_calls) {
  std::string hardware_set = "{";
  bool first_call = true;
  for (auto hw_func_pair : run_calls) {
    if (!first_call) {
      hardware_set += ",";
    }
    hardware_set += hw_func_pair.first;
  }
  hardware_set += "}";
  return hardware_set;
}

bool function_defined(std::string key, std::map<std::string, std::function<void()>> map) {
  return map.count(key) > 0;
}

template <class T>
int OneInOneOut_ProcessController<T>::make_image_def(std::vector<std::string> args) {
  if (args.size() != 0) {
    std::cout << "Usage:\n"
              << "  ./process image\n";
    return 1;
  }

  create_image(&input);
  save_image(input, "bin/input.png");

  std::cout << "Generated and saved input image\n";
  return 0;
}

template <class T>
int OneInOneOut_ProcessController<T>::make_run_def(std::vector<std::string> args) {
  // Check hardware name used exists in run_calls
  bool hw_name_defined = args.size() > 0 ? function_defined(args[0], run_calls) : false;
  
  // Input image: create or load
  if (args.size() == 1 && hw_name_defined) {
    create_image(&input);
  } else if (args.size() == 2 && hw_name_defined) {
    input = load_and_convert_image(args[1]);
  } else {
    std::string hardware_set = enumerate_keys(run_calls);
    std::cout << "Usage:\n"
              << "  ./process run " << hardware_set << " [input.png]\n"
              << "  Note: input.png is optional\n";
    return 1;
  }

  // run on input image
  std::string hardware_name = args[0];
  std::function<void()> run_call = run_calls.at(hardware_name);
  run_call();
  std::string output_filename = "bin/output_" + hardware_name + ".png";
  convert_and_save_image(output, output_filename);
    
  std::cout << "Ran " << design_name << " on " << hardware_name << "\n";
  return 0;

}

template <class T>
int OneInOneOut_ProcessController<T>::make_compare_def(std::vector<std::string> args) {
  Buffer<T> output_comparison(output);
    
  // Input image: create or load images
  if (args.size() == 2) {
    output = load_and_convert_image(args[0]);
    output_comparison = load_and_convert_image(args[1]);
  } else {
    printf("Usage:\n");
    printf("  ./process compare output0.png output1.png\n");
    return 1;
  }

  // compare images
  bool equal_images = compare_images<T>(output, output_comparison);

  std::string GREEN = "\033[32m";
  std::string RED = "\033[31m";
  std::string RESET = "\033[0m";
  if (equal_images) {
    std::cout << GREEN << "Images are equivalent!" << RESET << std::endl;
    return 0;
  } else {
    std::cout << RED << "Images are different..." << RESET << std::endl;
    return 1;
  }

}

template <class T>
int OneInOneOut_ProcessController<T>::make_test_def(std::vector<std::string> args) {
  std::cout << "make test: Not yet defined" << std::endl;
  return 1;
}

template <class T>
int OneInOneOut_ProcessController<T>::make_eval_def(std::vector<std::string> args) {
  // Check hardware name used exists in run_calls
  bool hw_name_defined = args.size() > 0 ? function_defined(args[0], run_calls) : false;
    
  // Input image: create or load
  if (args.size() == 1 && hw_name_defined) {
    create_image(&input);
  } else if (args.size() == 2 && hw_name_defined) {
    input = load_and_convert_image(args[1]);
  } else {
    std::string hardware_set = enumerate_keys(run_calls);
    std::cout << "Usage:\n"
              << "  ./process eval " << hardware_set << " [input.png]\n"
              << "  Note: input.png is optional\n";
    return 1;
  }

  std::string hardware_name = args[0];
  std::function<void()> run_call = run_calls.at(hardware_name);
  // Timing code
  double min_t_manual = benchmark(10, 10, [&]() {
      run_call();
    });
  printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

  return 0;
}



template int ProcessController<uint16_t>::process_command(int argc, char **argv);
template int ProcessController<uint16_t>::make_image_def(std::vector<std::string> args);
template int ProcessController<uint16_t>::make_run_def(std::vector<std::string> args);
template int ProcessController<uint16_t>::make_compare_def(std::vector<std::string> args);
template int ProcessController<uint16_t>::make_test_def(std::vector<std::string> args);
template int ProcessController<uint16_t>::make_eval_def(std::vector<std::string> args);
template void ProcessController<uint16_t>::print_usage();
template int OneInOneOut_ProcessController<uint16_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t>::make_eval_def(std::vector<std::string> args);

template int ProcessController<int16_t>::process_command(int argc, char **argv);
template int ProcessController<int16_t>::make_image_def(std::vector<std::string> args);
template int ProcessController<int16_t>::make_run_def(std::vector<std::string> args);
template int ProcessController<int16_t>::make_compare_def(std::vector<std::string> args);
template int ProcessController<int16_t>::make_test_def(std::vector<std::string> args);
template int ProcessController<int16_t>::make_eval_def(std::vector<std::string> args);
template void ProcessController<int16_t>::print_usage();
template int OneInOneOut_ProcessController<int16_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int16_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int16_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int16_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int16_t>::make_eval_def(std::vector<std::string> args);

template int ProcessController<bool>::process_command(int argc, char **argv);
template int ProcessController<bool>::make_image_def(std::vector<std::string> args);
template int ProcessController<bool>::make_run_def(std::vector<std::string> args);
template int ProcessController<bool>::make_compare_def(std::vector<std::string> args);
template int ProcessController<bool>::make_test_def(std::vector<std::string> args);
template int ProcessController<bool>::make_eval_def(std::vector<std::string> args);
template void ProcessController<bool>::print_usage();
template int OneInOneOut_ProcessController<bool>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<bool>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<bool>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<bool>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<bool>::make_eval_def(std::vector<std::string> args);
