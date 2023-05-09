#ifndef HW_SUPPORT_HARDWARE_PROCESS_HELPER_H
#define HW_SUPPORT_HARDWARE_PROCESS_HELPER_H

#include <cstdio>
#include <iostream>
#include <functional>
#include <map>
#include <string>

#include "HalideBuffer.h"

template <class T>
class ProcessController {
 public:
  ProcessController(std::string app_name) :
    design_name(app_name) {
  }
  
  int process_command(int argc, char **argv);
  
  virtual int make_image_def(std::vector<std::string> args);
  virtual int make_run_def(std::vector<std::string> args);
  virtual int make_compare_def(std::vector<std::string> args);
  virtual int make_test_def(std::vector<std::string> args);
  virtual int make_eval_def(std::vector<std::string> args);

  virtual void print_usage();
  
  // names
  std::string hardware_name;
  std::string design_name;
  
};

// template <class TI, class TO>
// class OneInOneOut_ProcessController : public ProcessController<TI> {
//  public:
//  OneInOneOut_ProcessController(std::string app_name, std::map<std::string, std::function<void()>> ops) :
//    ProcessController<TI>(app_name), inputs_preset(false), run_calls(ops), design_name(app_name) { }
//  OneInOneOut_ProcessController(std::string app_name) :
//    ProcessController<TI>(app_name), inputs_preset(false), design_name(app_name) { }
// 
//   // overridden methods
//   virtual int make_image_def(std::vector<std::string> args);
//   virtual int make_run_def(std::vector<std::string> args);
//   virtual int make_compare_def(std::vector<std::string> args);
//   virtual int make_test_def(std::vector<std::string> args);
//   virtual int make_eval_def(std::vector<std::string> args);
// 
//   // buffers
//   Halide::Runtime::Buffer<TI> input;
//   Halide::Runtime::Buffer<TO> output;
//   bool inputs_preset;
//   std::map<std::string, std::function<void()>> run_calls;
// 
//   // names
//   std::string design_name;
// 
// };

template <class TI, class TO = TI>
class OneInOneOut_ProcessController : public ProcessController<TO> {
 public:
 OneInOneOut_ProcessController(std::string app_name, std::map<std::string, std::function<void()>> ops) :
   ProcessController<TO>(app_name), inputs_preset(false), run_calls(ops), design_name(app_name) { }
 OneInOneOut_ProcessController(std::string app_name) :
   ProcessController<TO>(app_name), inputs_preset(false), design_name(app_name) { }

  // overridden methods
  virtual int make_image_def(std::vector<std::string> args);
  virtual int make_run_def(std::vector<std::string> args);
  virtual int make_compare_def(std::vector<std::string> args);
  virtual int make_test_def(std::vector<std::string> args);
  virtual int make_eval_def(std::vector<std::string> args);

  // buffers
  Halide::Runtime::Buffer<TI> input;
  Halide::Runtime::Buffer<TO> output;
  bool inputs_preset;
  std::map<std::string, std::function<void()>> run_calls;
  
  int tolerance = 0;

  // names
  std::string design_name;

};

template <class TI, class TO = TI>
class ManyInOneOut_ProcessController : public ProcessController<TO> {
 public:
  ManyInOneOut_ProcessController(std::string app_name, std::vector<std::string> filenames,
                                std::map<std::string, std::function<void()>> ops) :
    ProcessController<TO>(app_name), input_filenames(filenames), inputs_preset(false), design_name(app_name) {
    for (auto filename : filenames) {
      inputs[filename] = Halide::Runtime::Buffer<TI>();
    }
    run_calls = ops;
  }
  ManyInOneOut_ProcessController(std::string app_name, std::vector<std::string> filenames) :
    ProcessController<TO>(app_name), input_filenames(filenames), inputs_preset(false), design_name(app_name) {
    for (auto filename : filenames) {
      inputs[filename] = Halide::Runtime::Buffer<TI>();
    }
  }

  // overridden methods
  virtual int make_image_def(std::vector<std::string> args);
  virtual int make_run_def(std::vector<std::string> args);
  virtual int make_compare_def(std::vector<std::string> args);
  virtual int make_test_def(std::vector<std::string> args);
  virtual int make_eval_def(std::vector<std::string> args);

  int tolerance = 0;

  // buffers
  std::vector<std::string> input_filenames;
  std::map<std::string, Halide::Runtime::Buffer<TI>> inputs;
  Halide::Runtime::Buffer<TO> output;
  bool inputs_preset;
  std::map<std::string, std::function<void()>> run_calls;

  // names
  std::string design_name;

};

#endif
