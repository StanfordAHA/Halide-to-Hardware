#include <cstdio>
#include <map>

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

template <class T>
class OneInOneOut_ProcessController : public ProcessController<T> {
 public:
 OneInOneOut_ProcessController(std::string app_name, std::map<std::string, std::function<void()>> ops) :
  ProcessController<T>(app_name), run_calls(ops), design_name(app_name) { }

  // overridden methods
  virtual int make_image_def(std::vector<std::string> args);
  virtual int make_run_def(std::vector<std::string> args);
  virtual int make_compare_def(std::vector<std::string> args);
  virtual int make_test_def(std::vector<std::string> args);
  virtual int make_eval_def(std::vector<std::string> args);

  // buffers
  Halide::Runtime::Buffer<T> input;
  Halide::Runtime::Buffer<T> output;
  std::map<std::string, std::function<void()>> run_calls;

  // names
  std::string design_name;

};
