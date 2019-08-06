#include "coreir_sim_plugins.h"

using namespace std;
using namespace CoreIR;

vector<int> get_dims(Type* type) {
  vector<int> lengths;
  uint bitwidth = 1;
  Type* cType = type;
  while(!cType->isBaseType()) {
    if (auto aType = dyn_cast<ArrayType>(cType)) {
      
      uint length = aType->getLen();
          
      cType = aType->getElemType();
      if (cType->isBaseType()) {
        bitwidth = length;
      } else {
        lengths.insert(lengths.begin(), length);
        //lengths.push_back(length);
      }
    }
  }

  lengths.insert(lengths.begin(), bitwidth);
  return lengths;
}



void UnifiedBuffer::initialize(vdisc vd, SimulatorState& simState) {
  auto wd = simState.getCircuitGraph().getNode(vd);
  Wireable* w = wd.getWire();

  assert(isInstance(w));

  // extract parameters
  Instance* inst = toInstance(w);
  width = inst->getModuleRef()->getGenArgs().at("width")->get<int>();
  auto outputRangeType = inst->getModuleRef()->getGenArgs().at("output_range_type")->get<Type*>();
  auto outputStrideType = inst->getModuleRef()->getGenArgs().at("output_stride_type")->get<Type*>();
  auto outputStartType = inst->getModuleRef()->getGenArgs().at("output_start_type")->get<Type*>();

  auto inputRangeType = inst->getModuleRef()->getGenArgs().at("input_range_type")->get<Type*>();
  auto inputStrideType = inst->getModuleRef()->getGenArgs().at("input_stride_type")->get<Type*>();
  auto inputStartType = inst->getModuleRef()->getGenArgs().at("input_start_type")->get<Type*>();

  auto output_range = get_dims(outputRangeType);
  auto output_stride = get_dims(outputStrideType);
  auto output_start = get_dims(outputStartType);
            
  write_iterator = UnifiedBufferAddressGenerator(
    get_dims(inputRangeType), get_dims(inputRangeType), get_dims(inputStartType), width);
  read_iterator = UnifiedBufferAddressGenerator(
    get_dims(outputRangeType), get_dims(outputStrideType), get_dims(outputStartType), width);
      
      
  // deduce capacity from other output iterator
  capacity = 1;
  size_t dimension = output_range.size();

  for (size_t i = 0; i < dimension; ++i) {
    capacity += (output_range.at(i) - 1) * max(output_stride.at(i), 0);
  }

  int max_start = 0;
  for (const auto &start_value : output_start) {
    max_start = max(max_start, start_value);
  }
  capacity += max_start;

  // initialize data and select
  data = vector< vector<int> >(2, vector<int>(capacity, 0));
  select = 0;

}

void UnifiedBuffer::switch_check() {
  if (write_iterator.isDone() && read_iterator.isDone()) {
    select = select ^ 1;
    read_iterator.restart();
    write_iterator.restart();
  }
}
    

void UnifiedBuffer::exeSequential(vdisc vd, SimulatorState& simState) {
  auto wd = simState.getCircuitGraph().getNode(vd);

  simState.updateInputs(vd);

  assert(isInstance(wd.getWire()));
      
  Instance* inst = toInstance(wd.getWire());

  auto inSels = getInputSelects(inst);

  Select* arg1 = toSelect(CoreIR::findSelect("reset", inSels));
  Select* arg2 = toSelect(CoreIR::findSelect("datain", inSels));
  assert(arg1 != nullptr);
  BitVector resetVal = simState.getBitVec(arg1);
  //auto in_data = simState.getBitVec(arg2);

  assert((!write_iterator.isDone()) && "No more write allowed.\n");
  write_iterator.updateIterator(resetVal);
      
  auto write_addr_array = write_iterator.getAddresses();
  //assert((write_addr_array.size() == in_data.size()) && "Input data width not equals to port width.\n");
  for (size_t i = 0; i < write_addr_array.size(); i++) {
    int write_addr = write_addr_array[i];
    auto in_data = simState.getBitVec(arg2->sel(i));
    data[0x1 ^ select][write_addr] = in_data.to_type<int>();
  }

  switch_check(); 
}

void UnifiedBuffer::exeCombinational(vdisc vd, SimulatorState& simState) {
  auto wd = simState.getCircuitGraph().getNode(vd);
      
  Instance* inst = toInstance(wd.getWire());

  assert((!read_iterator.isDone()) && "No more read allowed.\n");
  read_iterator.updateIterator(BitVector(1, 0));
      
  vector<int> out_data;
  for (auto read_addr : read_iterator.getAddresses()) {
    out_data.push_back(data[select][read_addr]);
  }

  switch_check();

  for (size_t i=0; i<out_data.size(); ++i) {
    simState.setValue(toSelect(inst->sel("dataout")->sel(i)), BitVector(width, out_data.at(i)));
  }
}


void UnifiedBufferAddressGenerator::restart() {
  is_done = false;
  for (auto &iter : iter_list) {
    iter = 0;
  }
}

void UnifiedBufferAddressGenerator::updateIterator(BitVector resetVal) {
  if (resetVal == BitVector(1, 1)) {
    restart();
    return;
  }

  // logic to update the internal iterator
  for (size_t dim = 0; dim < dimension; dim++) {
    iter_list[dim] += 1;

    //return to zero for the previous dim if we enter the next dim
    if (dim > 0)
      iter_list[dim - 1] = 0;

    //not the last dimension
    if (dim < dimension - 1) {
      if (iter_list[dim] < output_range[dim]) {
        is_done = false;
        break;
      }
    } else {
      if (iter_list[dim] == output_range[dim]){
        is_done = true;
        break;
      }
    }
  }
}

int UnifiedBufferAddressGenerator::calcBaseAddress() {
  std::cout << "dims=" << dimension << std::endl;
  int addr_offset = 0;
  assert(iter_list.size() <= dimension);
  assert(output_stride.size() <= dimension);
  for (size_t i = 0; i < dimension; i++) {
    addr_offset += iter_list.at(i) * output_stride.at(i);
  }
  return addr_offset;
}

vector<int> UnifiedBufferAddressGenerator::getAddresses() {
  vector<int> addresses = vector<int>(output_start.size());
  auto baseAddr = calcBaseAddress();

  for (size_t i=0; i<addresses.size(); ++i) {
    addresses.at(i) = baseAddr + output_start.at(i);
  }

  return addresses;
}


void UnifiedBufferAddressGenerator::init_parameters(int _width, vector<int> _output_range, vector<int> _output_stride, vector<int> _output_start) {
  width = _width;
  output_range = _output_range;
  output_stride = _output_stride;
  output_start = _output_start;
      
  // start values all need to decrement 1
  for (auto& start_num : output_start) {
    start_num -= 1;
  }

  // deduce from other parameters
  assert(output_range.size() == output_stride.size());
  dimension = output_range.size();
  std::cout << "buffer has " << dimension << " dims\n";
  port = output_start.size();
  total_iter = 1;

  for (size_t i = 0; i < dimension; ++i) {
    total_iter *= output_range.at(i) / output_stride.at(i);
  }

  // initialize parameters
  iter_list = vector<int>(dimension);
  assert(iter_list.size() == dimension);
  restart();
}
                        

void UnifiedBufferAddressGenerator::initialize(vdisc vd, SimulatorState& simState) {
  auto wd = simState.getCircuitGraph().getNode(vd);
  Wireable* w = wd.getWire();

  assert(isInstance(w));

  // extract parameters
  Instance* inst = toInstance(w);
  auto myWidth = inst->getModuleRef()->getGenArgs().at("width")->get<int>();
  auto rangeType = inst->getModuleRef()->getGenArgs().at("range_type")->get<Type*>();
  auto strideType = inst->getModuleRef()->getGenArgs().at("stride_type")->get<Type*>();
  auto startType = inst->getModuleRef()->getGenArgs().at("start_type")->get<Type*>();

  init_parameters(myWidth, get_dims(rangeType), get_dims(strideType), get_dims(startType));
}

void UnifiedBufferAddressGenerator::exeSequential(vdisc vd, SimulatorState& simState) {
  auto wd = simState.getCircuitGraph().getNode(vd);

  simState.updateInputs(vd);

  assert(isInstance(wd.getWire()));
      
  Instance* inst = toInstance(wd.getWire());

  auto inSels = getInputSelects(inst);

  Select* arg1 = toSelect(CoreIR::findSelect("reset", inSels));
  assert(arg1 != nullptr);
  BitVector resetVal = simState.getBitVec(arg1);

  updateIterator(resetVal);
}

void UnifiedBufferAddressGenerator::exeCombinational(vdisc vd, SimulatorState& simState) {
  std::cout << "execomb..\n";
  auto wd = simState.getCircuitGraph().getNode(vd);
      
  Instance* inst = toInstance(wd.getWire());

  int addr_offset = calcBaseAddress();
  simState.setValue(toSelect(inst->sel("out")), BitVector(width, addr_offset));
}
