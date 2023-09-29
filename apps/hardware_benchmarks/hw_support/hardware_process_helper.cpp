#include <cstdio>
#include <iostream>

#include "HalideBuffer.h"
#include "halide_image_io.h"
#include "halide_benchmark.h"

#include "hardware_process_helper.h"
#include "hardware_image_helpers.h"

using namespace std;

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
    first_call = false;
  }
  hardware_set += "}";
  return hardware_set;
}

bool function_defined(std::string key, std::map<std::string, std::function<void()>> map) {
  return map.count(key) > 0;
}

template <class TI, class TO>
int OneInOneOut_ProcessController<TI,TO>::make_image_def(std::vector<std::string> args) {
  if (args.size() != 0 && args.size() != 1) {
    std::cout << "Usage:\n"
              << "  ./process image [image_type]\n";
    return 1;
  }

  if (args.size() == 0) {
    create_image(&input);
  } else {
    create_image(&input, static_cast<ImageType>(stoi(args[0])));
  }
  std::cout << "Generated input image\n";
  
  if (input.channels() > 4) {
    save_image(input, "bin/input.mat");
  } else {
    save_image(input, "bin/input.png");
  }

  std::cout << "Generated and saved input image\n";
  return 0;
}

template <class TI, class TO>
int OneInOneOut_ProcessController<TI,TO>::make_run_def(std::vector<std::string> args) {
  // Check hardware name used exists in run_calls
  bool hw_name_defined = args.size() > 0 ? function_defined(args[0], run_calls) : false;

  // Input image: create or load
  if (args.size() == 1 && hw_name_defined) {
    create_image(&input);
  } else if (args.size() == 2 && hw_name_defined) {
    if (!inputs_preset) {
      input = load_and_convert_image(args[1]);
    }
    //input(0, 0) = 22.f;
    //input(1, 0) = 1.0f;
    //input(2, 0) = 3.17188f;
    //input(3, 0) = 3.17187f;
    //input(4, 0) = 3.15625f;
    //input(5, 0) = 3.14063f;
    //input(6, 0) = 3.14062f;


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

  //std::string extension = output.channels() > 4 ? "mat" : "png";
  std::string extension = getenv("EXT");
  std::cout << "using extension " << extension << std::endl;
  std::string output_filename = "bin/output_" + hardware_name + "." + extension;
  convert_and_save_image(output, output_filename);
  cout << "First pixel of output..." << endl;
  cout << (int) output(0, 0) << endl;

  std::cout << "Ran " << design_name << " on " << hardware_name << "\n";
  return 0;

}

template <class TI, class TO>
int OneInOneOut_ProcessController<TI,TO>::make_compare_def(std::vector<std::string> args) {
  Buffer<TO> output_comparison(output);

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
  bool equal_images = compare_images<TO>(output, output_comparison, tolerance);

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

template <class TI, class TO>
int OneInOneOut_ProcessController<TI,TO>::make_test_def(std::vector<std::string> args) {
  std::cout << "make test: Not yet defined" << std::endl;
  return 1;
}

template <class TI, class TO>
int OneInOneOut_ProcessController<TI,TO>::make_eval_def(std::vector<std::string> args) {
  // Check hardware name used exists in run_calls
  bool hw_name_defined = args.size() > 0 ? function_defined(args[0], run_calls) : false;

  // Input image: create or load
  if (args.size() == 1 && hw_name_defined) {
    if (!inputs_preset) {
      create_image(&input);
    }
  } else if (args.size() == 2 && hw_name_defined) {
    if (!inputs_preset) {
      input = load_and_convert_image(args[1]);
    }
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
  //  benchmark(num_samples, num_iterations)
  //  Take minimum of after sample runs. Average across iterations.
  double min_t_manual = benchmark(30, 30, [&]() {
      run_call();
    });
  printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

  return 0;
}

// template <class TI, class TO>
// int OneInOneOut_ProcessController<TI,TO>::make_image_def(std::vector<std::string> args) {
//   return OneInOneOut_ProcessController<TI>::make_image_def(args);
// }
// 
// template <class TI, class TO>
// int OneInOneOut_ProcessController<TI,TO>::make_run_def(std::vector<std::string> args) {
//   return OneInOneOut_ProcessController<TO>::make_run_def(args);
// }
// 
// 
// template <class TI, class TO>
// int OneInOneOut_ProcessController<TI,TO>::make_compare_def(std::vector<std::string> args) {
//   return OneInOneOut_ProcessController<TO>::make_compare_def(args);
// }
// 
// template <class TI, class TO>
// int OneInOneOut_ProcessController<TI,TO>::make_eval_def(std::vector<std::string> args) {
//   return OneInOneOut_ProcessController<TO>::make_eval_def(args);
// }

template <class TI, class TO>
int ManyInOneOut_ProcessController<TI,TO>::make_image_def(std::vector<std::string> args) {
  if (args.size() != 0 && args.size() != 1) {
    std::cout << "Usage:\n"
              << "  ./process image [image_type]\n";
    return 1;
  }

  for (auto filename : input_filenames) {
    if (args.size() == 0) {
      create_image(&inputs[filename]);
    } else {
      create_image(&inputs[filename], static_cast<ImageType>(stoi(args[0])));
    }
    std::cout << "Generated input image\n";
    
    save_image(inputs[filename], "bin/" + filename);
    
    std::cout << "Generated and saved input image " + filename + "\n";
  }

  return 0;
}

template <class TI, class TO>
int ManyInOneOut_ProcessController<TI,TO>::make_run_def(std::vector<std::string> args) {
  // Check hardware name used exists in run_calls
  bool hw_name_defined = args.size() > 0 ? function_defined(args[0], run_calls) : false;

  // Input image: create or load
  if ((args.size() == 1 || args.size() == 2) && hw_name_defined) {
    // Note that we ignore a single which probably is "input.png"
    // If inputs are already set, we don't need to read file names.
    // This may occur if input values are generated or N-D data.
    if (!inputs_preset) {
      for (auto filename : input_filenames) {
        inputs[filename] = load_and_convert_image(filename);
      }
    }

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

  //std::string extension = output.channels() > 4 ? "mat" : "png";
  std::string extension = getenv("EXT");
  std::cout << "using extension " << extension << std::endl;

  std::string output_filename = "bin/output_" + hardware_name + "." + extension;
  convert_and_save_image(output, output_filename);
  cout << "First pixel of output..." << endl;
  cout << (int) output(0, 0) << endl;

  std::cout << "Ran " << design_name << " on " << hardware_name << "\n";
  return 0;

}

template <class TI, class TO>
int ManyInOneOut_ProcessController<TI,TO>::make_compare_def(std::vector<std::string> args) {
  Buffer<TO> output_comparison(output);

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
  bool equal_images = compare_images<TO>(output, output_comparison, tolerance);

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

template <class TI, class TO>
int ManyInOneOut_ProcessController<TI,TO>::make_test_def(std::vector<std::string> args) {
  std::cout << "make test: Not yet defined" << std::endl;
  return 1;
}

template <class TI, class TO>
int ManyInOneOut_ProcessController<TI,TO>::make_eval_def(std::vector<std::string> args) {
  // Check hardware name used exists in run_calls
  bool hw_name_defined = args.size() > 0 ? function_defined(args[0], run_calls) : false;

  // Input image: create or load
  if ((args.size() == 1 || args.size() == 2) && hw_name_defined) {
    // Note that we ignore a single which probably is "input.png"
    if (!inputs_preset) {
      for (auto filename : input_filenames) {
        inputs[filename] = load_and_convert_image(filename);
      }
    }

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
  double min_t_manual = benchmark(100, 100, [&]() {
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
template int ManyInOneOut_ProcessController<uint16_t>::make_image_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint16_t>::make_run_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint16_t>::make_compare_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint16_t>::make_test_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint16_t>::make_eval_def(std::vector<std::string> args);

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
template int ManyInOneOut_ProcessController<int16_t>::make_image_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<int16_t>::make_run_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<int16_t>::make_compare_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<int16_t>::make_test_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<int16_t>::make_eval_def(std::vector<std::string> args);

template int ProcessController<uint8_t>::process_command(int argc, char **argv);
template int ProcessController<uint8_t>::make_image_def(std::vector<std::string> args);
template int ProcessController<uint8_t>::make_run_def(std::vector<std::string> args);
template int ProcessController<uint8_t>::make_compare_def(std::vector<std::string> args);
template int ProcessController<uint8_t>::make_test_def(std::vector<std::string> args);
template int ProcessController<uint8_t>::make_eval_def(std::vector<std::string> args);
template void ProcessController<uint8_t>::print_usage();
template int OneInOneOut_ProcessController<uint8_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint8_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint8_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint8_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint8_t>::make_eval_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t>::make_image_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t>::make_run_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t>::make_compare_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t>::make_test_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t>::make_eval_def(std::vector<std::string> args);

template int ProcessController<int8_t>::process_command(int argc, char **argv);
template int ProcessController<int8_t>::make_image_def(std::vector<std::string> args);
template int ProcessController<int8_t>::make_run_def(std::vector<std::string> args);
template int ProcessController<int8_t>::make_compare_def(std::vector<std::string> args);
template int ProcessController<int8_t>::make_test_def(std::vector<std::string> args);
template int ProcessController<int8_t>::make_eval_def(std::vector<std::string> args);
template void ProcessController<int8_t>::print_usage();
template int OneInOneOut_ProcessController<int8_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int8_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int8_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int8_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int8_t>::make_eval_def(std::vector<std::string> args);

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

template int ProcessController<float>::process_command(int argc, char **argv);
template int ProcessController<float>::make_image_def(std::vector<std::string> args);
template int ProcessController<float>::make_run_def(std::vector<std::string> args);
template int ProcessController<float>::make_compare_def(std::vector<std::string> args);
template int ProcessController<float>::make_test_def(std::vector<std::string> args);
template int ProcessController<float>::make_eval_def(std::vector<std::string> args);
template void ProcessController<float>::print_usage();
template int OneInOneOut_ProcessController<float>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<float>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<float>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<float>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<float>::make_eval_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<float>::make_image_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<float>::make_run_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<float>::make_compare_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<float>::make_test_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<float>::make_eval_def(std::vector<std::string> args);

template int ProcessController<uint32_t>::process_command(int argc, char **argv);
template int ProcessController<uint32_t>::make_image_def(std::vector<std::string> args);
template int ProcessController<uint32_t>::make_run_def(std::vector<std::string> args);
template int ProcessController<uint32_t>::make_compare_def(std::vector<std::string> args);
template int ProcessController<uint32_t>::make_test_def(std::vector<std::string> args);
template int ProcessController<uint32_t>::make_eval_def(std::vector<std::string> args);
template void ProcessController<uint32_t>::print_usage();
template int OneInOneOut_ProcessController<uint32_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint32_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint32_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint32_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint32_t>::make_eval_def(std::vector<std::string> args);

template int ProcessController<int32_t>::process_command(int argc, char **argv);
template int ProcessController<int32_t>::make_image_def(std::vector<std::string> args);
template int ProcessController<int32_t>::make_run_def(std::vector<std::string> args);
template int ProcessController<int32_t>::make_compare_def(std::vector<std::string> args);
template int ProcessController<int32_t>::make_test_def(std::vector<std::string> args);
template int ProcessController<int32_t>::make_eval_def(std::vector<std::string> args);
template void ProcessController<int32_t>::print_usage();
template int OneInOneOut_ProcessController<int32_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int32_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int32_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int32_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<int32_t>::make_eval_def(std::vector<std::string> args);

template int OneInOneOut_ProcessController<uint16_t, uint8_t>::make_image_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t, uint8_t>::make_run_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t, uint8_t>::make_compare_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t, uint8_t>::make_test_def(std::vector<std::string> args);
template int OneInOneOut_ProcessController<uint16_t, uint8_t>::make_eval_def(std::vector<std::string> args);

template int ManyInOneOut_ProcessController<uint8_t, float>::make_image_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t, float>::make_run_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t, float>::make_compare_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t, float>::make_test_def(std::vector<std::string> args);
template int ManyInOneOut_ProcessController<uint8_t, float>::make_eval_def(std::vector<std::string> args);
