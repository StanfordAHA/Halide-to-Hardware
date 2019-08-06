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
      } else if (length > 0) {
        lengths.insert(lengths.begin(), length);
        //lengths.push_back(length);
      }
    }
  }

  lengths.insert(lengths.begin(), bitwidth);
  return lengths;
}

UnifiedBufferAddressGenerator::UnifiedBufferAddressGenerator() {
  is_done = false;
}

UnifiedBuffer::UnifiedBuffer() : has_switched_once(false) { }


UnifiedBufferAddressGenerator::UnifiedBufferAddressGenerator(std::vector<int> range, std::vector<int> stride,
                                                             std::vector<int> start, int myWidth) :
  is_done(false) {
  init_parameters(myWidth, range, stride, start);
}

UnifiedBufferAddressGenerator::~UnifiedBufferAddressGenerator() {}
UnifiedBuffer::~UnifiedBuffer() {}



void UnifiedBuffer::initialize(vdisc vd, SimulatorState& simState) {
  has_switched_once = false;
  auto wd = simState.getCircuitGraph().getNode(vd);
  Wireable* w = wd.getWire();

  assert(isInstance(w));

  // extract parameters
  Instance* inst = toInstance(w);
  width = inst->getModuleRef()->getGenArgs().at("width")->get<int>();
  auto output_range_0 = inst->getModuleRef()->getGenArgs().at("range_0")->get<int>();
  auto output_range_1 = inst->getModuleRef()->getGenArgs().at("range_1")->get<int>();
  auto output_range_2 = inst->getModuleRef()->getGenArgs().at("range_2")->get<int>();
  auto output_range_3 = inst->getModuleRef()->getGenArgs().at("range_3")->get<int>();
  auto output_range_4 = inst->getModuleRef()->getGenArgs().at("range_4")->get<int>();
  auto output_range_5 = inst->getModuleRef()->getGenArgs().at("range_5")->get<int>();
  vector<int> outputRangeVec = {output_range_0, output_range_1, output_range_2, output_range_3, output_range_4, output_range_5};
    
  auto output_stride_0 = inst->getModuleRef()->getGenArgs().at("stride_0")->get<int>();
  auto output_stride_1 = inst->getModuleRef()->getGenArgs().at("stride_1")->get<int>();
  auto output_stride_2 = inst->getModuleRef()->getGenArgs().at("stride_2")->get<int>();
  auto output_stride_3 = inst->getModuleRef()->getGenArgs().at("stride_3")->get<int>();
  auto output_stride_4 = inst->getModuleRef()->getGenArgs().at("stride_4")->get<int>();
  auto output_stride_5 = inst->getModuleRef()->getGenArgs().at("stride_5")->get<int>();
  vector<int> outputStrideVec = {output_stride_0, output_stride_1, output_stride_2, output_stride_3, output_stride_4, output_stride_5};
  //auto outputStrideType = Bit()->Arr(output_stride_0)->Arr(output_stride_1)->Arr(output_stride_2)->Arr(output_stride_3)->Arr(output_stride_4)->Arr(output_stride_5);

  auto input_range_0 = inst->getModuleRef()->getGenArgs().at("input_range_0")->get<int>();
  auto input_range_1 = inst->getModuleRef()->getGenArgs().at("input_range_1")->get<int>();
  auto input_range_2 = inst->getModuleRef()->getGenArgs().at("input_range_2")->get<int>();
  auto input_range_3 = inst->getModuleRef()->getGenArgs().at("input_range_3")->get<int>();
  auto input_range_4 = inst->getModuleRef()->getGenArgs().at("input_range_4")->get<int>();
  auto input_range_5 = inst->getModuleRef()->getGenArgs().at("input_range_5")->get<int>();
  vector<int> inputRangeVec = {input_range_0, input_range_1, input_range_2, input_range_3, input_range_4, input_range_5};
  //auto inputRangeType = c->Bit()->Arr(input_range_0)->Arr(input_range_1)->Arr(input_range_2)->Arr(input_range_3)->Arr(input_range_4)->Arr(input_range_5);

  auto input_stride_0 = inst->getModuleRef()->getGenArgs().at("input_stride_0")->get<int>();
  auto input_stride_1 = inst->getModuleRef()->getGenArgs().at("input_stride_1")->get<int>();
  auto input_stride_2 = inst->getModuleRef()->getGenArgs().at("input_stride_2")->get<int>();
  auto input_stride_3 = inst->getModuleRef()->getGenArgs().at("input_stride_3")->get<int>();
  auto input_stride_4 = inst->getModuleRef()->getGenArgs().at("input_stride_4")->get<int>();
  auto input_stride_5 = inst->getModuleRef()->getGenArgs().at("input_stride_5")->get<int>();
  vector<int> inputStrideVec = {input_stride_0, input_stride_1, input_stride_2, input_stride_3, input_stride_4, input_stride_5};
  //auto inputStrideType = Bit()->Arr(input_stride_0)->Arr(input_stride_1)->Arr(input_stride_2)->Arr(input_stride_3)->Arr(input_stride_4)->Arr(input_stride_5);

  auto outputStartJson = inst->getModuleRef()->getGenArgs().at("output_starting_addrs")->get<Json>();
  std::cout << "output json=" << outputStartJson << endl;
  vector<int> outputStartVec;
  for (auto const& start_value : outputStartJson["output_start"]) {
    outputStartVec.push_back(start_value);
    std::cout << "out_start=" << start_value << endl;
  }
  
  auto inputStartJson = inst->getModuleRef()->getGenArgs().at("input_starting_addrs")->get<Json>();
  std::cout << "input json=" << inputStartJson << endl;
  vector<int> inputStartVec;
  for (auto const& start_value : inputStartJson["input_start"]) {
    inputStartVec.push_back(start_value);
    std::cout << "in_start=" << start_value << endl;
  }

  //auto outputStrideType = inst->getModuleRef()->getGenArgs().at("stride_type")->get<Type*>();
  //auto outputStartType = inst->getModuleRef()->getGenArgs().at("output_start_type")->get<Type*>();
  //auto inputRangeType = inst->getModuleRef()->getGenArgs().at("input_range_type")->get<Type*>();
  //auto inputStrideType = inst->getModuleRef()->getGenArgs().at("input_stride_type")->get<Type*>();
  //auto inputStartJson = inst->getModuleRef()->getGenArgs().at("input_start_type")->get<JsonType*>();

  //auto output_range = get_dims(outputRangeType);
  //auto output_stride = get_dims(outputStrideType);
  //auto output_start = get_dims(outputStartType);
            
  write_iterator = UnifiedBufferAddressGenerator(
    inputRangeVec, inputRangeVec, inputStartVec, width);
  read_iterator = UnifiedBufferAddressGenerator(
    outputRangeVec, outputStrideVec, outputStartVec, width);
      

  capacity = inst->getModuleRef()->getGenArgs().at("depth")->get<int>();
  // deduce capacity from other output iterator
  //capacity = 1;
  //size_t dimension = output_range.size();
  //
  //for (size_t i = 0; i < dimension; ++i) {
  //  capacity += (output_range.at(i) - 1) * max(output_stride.at(i), 0);
  //}

  //int max_start = 0;
  //for (const auto &start_value : output_start) {
  //  max_start = max(max_start, start_value);
  //}
  //capacity += max_start;

  // initialize data and select
  data = vector< vector<int> >(2, vector<int>(capacity, 0));
  select = 0;

}

void UnifiedBuffer::switch_check() {
  if (write_iterator.isDone() && read_iterator.isDone()) {
    select = select ^ 1;
    read_iterator.restart();
    write_iterator.restart();
    has_switched_once = true;
  }
}
    

void UnifiedBuffer::exeSequential(vdisc vd, SimulatorState& simState) {

  auto wd = simState.getCircuitGraph().getNode(vd);

  simState.updateInputs(vd);

  assert(isInstance(wd.getWire()));
      
  Instance* inst = toInstance(wd.getWire());

  auto inSels = getInputSelects(inst);

  Select* arg1 = toSelect(CoreIR::findSelect("reset", inSels));
  assert(arg1 != nullptr);
  BitVector resetVal = simState.getBitVec(arg1);
  //auto in_data = simState.getBitVec(arg2);

  assert((!write_iterator.isDone()) && "No more write allowed.\n");
  write_iterator.updateIterator(resetVal);
      
  auto write_addr_array = write_iterator.getAddresses();
  std::cout << "ub write with reset=" << resetVal << " and num_addrs=" << write_addr_array.size();
  //assert((write_addr_array.size() == in_data.size()) && "Input data width not equals to port width.\n");
  for (size_t i = 0; i < write_addr_array.size(); i++) {
    int write_addr = write_addr_array[i];

    Select* arg_data = toSelect(CoreIR::findSelect("datain" + to_string(i), inSels));
    auto in_data = simState.getBitVec(arg_data);
    
    data[0x1 ^ select][write_addr] = in_data.to_type<int>();
    std::cout << " wrote data[" << write_addr << "]= " << in_data.to_type<int>();
  }
  std::cout << std::endl;
  
  switch_check(); 
}

void UnifiedBuffer::exeCombinational(vdisc vd, SimulatorState& simState) {
  //std::cout << "ub read\n";
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
    //simState.setValue(toSelect(inst->sel("dataout"+to_string(i))), BitVector(width, out_data.at(i)));
    simState.setValue(toSelect(inst->sel("dataout"+to_string(i))), BitVector(width, out_data.at(i)));
  }

  bool is_valid_data = !read_iterator.isDone() && has_switched_once;
  simState.setValue(toSelect(inst->sel("valid")), BitVector(1, has_switched_once));
  //std::cout << "finishing this read\n";
}

void UnifiedBufferAddressGenerator::restart() {
  is_done = false;
  for (auto &iter : iter_list) {
    iter = 0;
  }
}

void UnifiedBufferAddressGenerator::updateIterator(BitVector resetVal) {
  if (resetVal == BitVector(1, 1)) {
    std::cout << "restarting\n";
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
  //std::cout << "dims=" << dimension << std::endl;
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
    assert(addresses.at(i) >=0);
  }

  return addresses;
}


void UnifiedBufferAddressGenerator::init_parameters(int _width, vector<int> _output_range, vector<int> _output_stride, vector<int> _output_start) {
  width = _width;
  output_range = _output_range;
  output_stride = _output_stride;
  output_start = _output_start;
      
  // deduce from other parameters
  assert(output_range.size() == output_stride.size());
  dimension = output_range.size();
  std::cout << "buffer has " << dimension << " dims\n";
  port = output_start.size();
  total_iter = 1;

  for (size_t i = 0; i < dimension; ++i) {
    total_iter *= output_range.at(i);// / output_stride.at(i);
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


