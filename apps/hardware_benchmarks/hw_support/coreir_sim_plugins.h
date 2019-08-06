#include <iostream>

#include "coreir.h"
#include "coreir/simulator/interpreter.h"

// Define address generator simulation class
class UnifiedBufferAddressGenerator : public CoreIR::SimulatorPlugin {
private:
  int width;
  std::vector<int> iter_list;

  std::vector<int> output_range;
  std::vector<int> output_stride;
  std::vector<int> output_start;
    
  size_t dimension;
  size_t port;
  int total_iter;
  bool is_done;

public:
  UnifiedBufferAddressGenerator() {
    is_done = false;
  }
    
  UnifiedBufferAddressGenerator(std::vector<int> range, std::vector<int> stride,
                                std::vector<int> start, int myWidth) :
    is_done(false) {
    init_parameters(myWidth, range, stride, start);
  }

  bool isDone() { return is_done; }
  void restart();
  void updateIterator(BitVector resetVal);
  int calcBaseAddress();
  std::vector<int> getAddresses();
  void init_parameters(int _width, std::vector<int> _output_range,
                       std::vector<int> _output_stride, std::vector<int> _output_start);

  void initialize(CoreIR::vdisc vd, CoreIR::SimulatorState& simState) override;
  void exeSequential(CoreIR::vdisc vd, CoreIR::SimulatorState& simState) override;
  void exeCombinational(CoreIR::vdisc vd, CoreIR::SimulatorState& simState) override;
};

class UnifiedBuffer : public CoreIR::SimulatorPlugin {
private:
  int capacity;
  int select;
  int width;

  UnifiedBufferAddressGenerator write_iterator;
  UnifiedBufferAddressGenerator read_iterator;
  std::vector< std::vector<int> > data;

public:
  void switch_check();
  void initialize(CoreIR::vdisc vd, CoreIR::SimulatorState& simState) override;
  void exeSequential(CoreIR::vdisc vd, CoreIR::SimulatorState& simState) override;
  void exeCombinational(CoreIR::vdisc vd, CoreIR::SimulatorState& simState) override;
};
