#include <cstdio>

#include "HalideBuffer.h"

class ProcessController {
 public:
  ProcessController(std::string hw_name, std::string name) :
    hardware_name(hw_name), design_name(name) {
    
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

class OneInOneOut_ProcessController : public ProcessController {
 public:
 OneInOneOut_ProcessController(std::string hw_name, std::string name, std::function<void()> op) :
  ProcessController(hw_name, name), run_call(op) { }

  // overridden methods
  virtual int make_image_def(std::vector<std::string> args);
  virtual int make_run_def(std::vector<std::string> args);
  virtual int make_compare_def(std::vector<std::string> args);
  virtual int make_test_def(std::vector<std::string> args);
  virtual int make_eval_def(std::vector<std::string> args);

  // buffers
  Halide::Runtime::Buffer<uint16_t> input;
  Halide::Runtime::Buffer<uint16_t> output;
  std::function<void()> run_call;
};
