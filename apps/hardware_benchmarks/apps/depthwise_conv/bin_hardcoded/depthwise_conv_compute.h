#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: input_host_global_wrapper.stencil(0, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(0, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_1 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_1;
}

//store is: input_host_global_wrapper.stencil(1, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(1, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil_1(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_2 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_2;
}

//store is: input_host_global_wrapper.stencil(2, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(2, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil_2(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_3 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_3;
}

//store is: input_host_global_wrapper.stencil(3, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(3, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil_3(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_4 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_4;
}

//store is: input_host_global_wrapper.stencil(4, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(4, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil_4(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_5 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_5;
}

//store is: input_host_global_wrapper.stencil(5, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(5, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil_5(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_6 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_6;
}

//store is: input_host_global_wrapper.stencil(6, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(6, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_stencil_6(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_7 = (uint16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_7;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(0, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(0, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_1 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_1;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(1, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(1, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil_1(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_2 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_2;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(2, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(2, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil_2(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_3 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_3;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(3, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(3, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil_3(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_4 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_4;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(4, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(4, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil_4(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_5 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_5;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(5, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(5, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil_5(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_6 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_6;
}

//store is: input_host_global_wrapper_global_wrapper.stencil(6, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(6, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_input_host_global_wrapper_global_wrapper_stencil_6(hw_uint<16>& input_host_global_wrapper_stencil) {
  uint16_t _input_host_global_wrapper_stencil_7 = (uint16_t) (input_host_global_wrapper_stencil.extract<0, 15>());

  return _input_host_global_wrapper_stencil_7;
}

//store is: depthwise_conv.stencil(0, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil() {
  uint16_t _400 = (uint16_t)(0);
  return _400;
}

//store is: depthwise_conv.stencil(1, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil_1() {
  uint16_t _403 = (uint16_t)(0);
  return _403;
}

//store is: depthwise_conv.stencil(2, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil_2() {
  uint16_t _406 = (uint16_t)(0);
  return _406;
}

//store is: depthwise_conv.stencil(3, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil_3() {
  uint16_t _409 = (uint16_t)(0);
  return _409;
}

//store is: depthwise_conv.stencil(4, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil_4() {
  uint16_t _412 = (uint16_t)(0);
  return _412;
}

//store is: depthwise_conv.stencil(5, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil_5() {
  uint16_t _415 = (uint16_t)(0);
  return _415;
}

//store is: depthwise_conv.stencil(6, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
hw_uint<16> hcompute_depthwise_conv_stencil_6() {
  uint16_t _418 = (uint16_t)(0);
  return _418;
}

//store is: depthwise_conv.stencil(0, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(0, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(0, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(0, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(0, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_7(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_1 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_1 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_2 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_3 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_4 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_5 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_6 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_7 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_8 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_9 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _421 = (uint16_t)(24);
  uint16_t _422 = _input_host_global_wrapper_global_wrapper_stencil_1 * _421;
  uint16_t _423 = (uint16_t)(30);
  uint16_t _424 = _input_host_global_wrapper_global_wrapper_stencil_2 * _423;
  uint16_t _425 = _input_host_global_wrapper_global_wrapper_stencil_3 * _421;
  uint16_t _426 = _input_host_global_wrapper_global_wrapper_stencil_4 * _423;
  uint16_t _427 = (uint16_t)(37);
  uint16_t _428 = _input_host_global_wrapper_global_wrapper_stencil_5 * _427;
  uint16_t _429 = _input_host_global_wrapper_global_wrapper_stencil_6 * _423;
  uint16_t _430 = _input_host_global_wrapper_global_wrapper_stencil_7 * _421;
  uint16_t _431 = _input_host_global_wrapper_global_wrapper_stencil_8 * _421;
  uint16_t _432 = _input_host_global_wrapper_global_wrapper_stencil_9 * _423;
  uint16_t _433 = _431 + _432;
  uint16_t _434 = _430 + _433;
  uint16_t _435 = _429 + _434;
  uint16_t _436 = _428 + _435;
  uint16_t _437 = _426 + _436;
  uint16_t _438 = _425 + _437;
  uint16_t _439 = _424 + _438;
  uint16_t _440 = _depthwise_conv_stencil_1 + _439;
  uint16_t _441 = _422 + _440;
  return _441;
}

//store is: depthwise_conv.stencil(1, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(1, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(1, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(1, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(1, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_8(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_2 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_10 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_11 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_12 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_13 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_14 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_15 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_16 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_17 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_18 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _498 = (uint16_t)(24);
  uint16_t _499 = _input_host_global_wrapper_global_wrapper_stencil_10 * _498;
  uint16_t _500 = (uint16_t)(30);
  uint16_t _501 = _input_host_global_wrapper_global_wrapper_stencil_11 * _500;
  uint16_t _502 = _input_host_global_wrapper_global_wrapper_stencil_12 * _498;
  uint16_t _503 = _input_host_global_wrapper_global_wrapper_stencil_13 * _500;
  uint16_t _504 = (uint16_t)(37);
  uint16_t _505 = _input_host_global_wrapper_global_wrapper_stencil_14 * _504;
  uint16_t _506 = _input_host_global_wrapper_global_wrapper_stencil_15 * _500;
  uint16_t _507 = _input_host_global_wrapper_global_wrapper_stencil_16 * _498;
  uint16_t _508 = _input_host_global_wrapper_global_wrapper_stencil_17 * _498;
  uint16_t _509 = _input_host_global_wrapper_global_wrapper_stencil_18 * _500;
  uint16_t _510 = _508 + _509;
  uint16_t _511 = _507 + _510;
  uint16_t _512 = _506 + _511;
  uint16_t _513 = _505 + _512;
  uint16_t _514 = _503 + _513;
  uint16_t _515 = _502 + _514;
  uint16_t _516 = _501 + _515;
  uint16_t _517 = _depthwise_conv_stencil_2 + _516;
  uint16_t _518 = _499 + _517;
  return _518;
}

//store is: depthwise_conv.stencil(2, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(2, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(2, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(2, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(2, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_9(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_3 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_19 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_20 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_21 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_22 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_23 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_24 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_25 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_26 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_27 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _575 = (uint16_t)(24);
  uint16_t _576 = _input_host_global_wrapper_global_wrapper_stencil_19 * _575;
  uint16_t _577 = (uint16_t)(30);
  uint16_t _578 = _input_host_global_wrapper_global_wrapper_stencil_20 * _577;
  uint16_t _579 = _input_host_global_wrapper_global_wrapper_stencil_21 * _575;
  uint16_t _580 = _input_host_global_wrapper_global_wrapper_stencil_22 * _577;
  uint16_t _581 = (uint16_t)(37);
  uint16_t _582 = _input_host_global_wrapper_global_wrapper_stencil_23 * _581;
  uint16_t _583 = _input_host_global_wrapper_global_wrapper_stencil_24 * _577;
  uint16_t _584 = _input_host_global_wrapper_global_wrapper_stencil_25 * _575;
  uint16_t _585 = _input_host_global_wrapper_global_wrapper_stencil_26 * _575;
  uint16_t _586 = _input_host_global_wrapper_global_wrapper_stencil_27 * _577;
  uint16_t _587 = _585 + _586;
  uint16_t _588 = _584 + _587;
  uint16_t _589 = _583 + _588;
  uint16_t _590 = _582 + _589;
  uint16_t _591 = _580 + _590;
  uint16_t _592 = _579 + _591;
  uint16_t _593 = _578 + _592;
  uint16_t _594 = _depthwise_conv_stencil_3 + _593;
  uint16_t _595 = _576 + _594;
  return _595;
}

//store is: depthwise_conv.stencil(3, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(3, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(3, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(3, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(3, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_10(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_4 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_28 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_29 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_30 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_31 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_32 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_33 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_34 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_35 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_36 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _652 = (uint16_t)(24);
  uint16_t _653 = _input_host_global_wrapper_global_wrapper_stencil_28 * _652;
  uint16_t _654 = (uint16_t)(30);
  uint16_t _655 = _input_host_global_wrapper_global_wrapper_stencil_29 * _654;
  uint16_t _656 = _input_host_global_wrapper_global_wrapper_stencil_30 * _652;
  uint16_t _657 = _input_host_global_wrapper_global_wrapper_stencil_31 * _654;
  uint16_t _658 = (uint16_t)(37);
  uint16_t _659 = _input_host_global_wrapper_global_wrapper_stencil_32 * _658;
  uint16_t _660 = _input_host_global_wrapper_global_wrapper_stencil_33 * _654;
  uint16_t _661 = _input_host_global_wrapper_global_wrapper_stencil_34 * _652;
  uint16_t _662 = _input_host_global_wrapper_global_wrapper_stencil_35 * _652;
  uint16_t _663 = _input_host_global_wrapper_global_wrapper_stencil_36 * _654;
  uint16_t _664 = _662 + _663;
  uint16_t _665 = _661 + _664;
  uint16_t _666 = _660 + _665;
  uint16_t _667 = _659 + _666;
  uint16_t _668 = _657 + _667;
  uint16_t _669 = _656 + _668;
  uint16_t _670 = _655 + _669;
  uint16_t _671 = _depthwise_conv_stencil_4 + _670;
  uint16_t _672 = _653 + _671;
  return _672;
}

//store is: depthwise_conv.stencil(4, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(4, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(4, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(4, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(4, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_11(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_5 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_37 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_38 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_39 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_40 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_41 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_42 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_43 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_44 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_45 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _729 = (uint16_t)(24);
  uint16_t _730 = _input_host_global_wrapper_global_wrapper_stencil_37 * _729;
  uint16_t _731 = (uint16_t)(30);
  uint16_t _732 = _input_host_global_wrapper_global_wrapper_stencil_38 * _731;
  uint16_t _733 = _input_host_global_wrapper_global_wrapper_stencil_39 * _729;
  uint16_t _734 = _input_host_global_wrapper_global_wrapper_stencil_40 * _731;
  uint16_t _735 = (uint16_t)(37);
  uint16_t _736 = _input_host_global_wrapper_global_wrapper_stencil_41 * _735;
  uint16_t _737 = _input_host_global_wrapper_global_wrapper_stencil_42 * _731;
  uint16_t _738 = _input_host_global_wrapper_global_wrapper_stencil_43 * _729;
  uint16_t _739 = _input_host_global_wrapper_global_wrapper_stencil_44 * _729;
  uint16_t _740 = _input_host_global_wrapper_global_wrapper_stencil_45 * _731;
  uint16_t _741 = _739 + _740;
  uint16_t _742 = _738 + _741;
  uint16_t _743 = _737 + _742;
  uint16_t _744 = _736 + _743;
  uint16_t _745 = _734 + _744;
  uint16_t _746 = _733 + _745;
  uint16_t _747 = _732 + _746;
  uint16_t _748 = _depthwise_conv_stencil_5 + _747;
  uint16_t _749 = _730 + _748;
  return _749;
}

//store is: depthwise_conv.stencil(5, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(5, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(5, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(5, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(5, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_12(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_6 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_46 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_47 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_48 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_49 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_50 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_51 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_52 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_53 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_54 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _806 = (uint16_t)(24);
  uint16_t _807 = _input_host_global_wrapper_global_wrapper_stencil_46 * _806;
  uint16_t _808 = (uint16_t)(30);
  uint16_t _809 = _input_host_global_wrapper_global_wrapper_stencil_47 * _808;
  uint16_t _810 = _input_host_global_wrapper_global_wrapper_stencil_48 * _806;
  uint16_t _811 = _input_host_global_wrapper_global_wrapper_stencil_49 * _808;
  uint16_t _812 = (uint16_t)(37);
  uint16_t _813 = _input_host_global_wrapper_global_wrapper_stencil_50 * _812;
  uint16_t _814 = _input_host_global_wrapper_global_wrapper_stencil_51 * _808;
  uint16_t _815 = _input_host_global_wrapper_global_wrapper_stencil_52 * _806;
  uint16_t _816 = _input_host_global_wrapper_global_wrapper_stencil_53 * _806;
  uint16_t _817 = _input_host_global_wrapper_global_wrapper_stencil_54 * _808;
  uint16_t _818 = _816 + _817;
  uint16_t _819 = _815 + _818;
  uint16_t _820 = _814 + _819;
  uint16_t _821 = _813 + _820;
  uint16_t _822 = _811 + _821;
  uint16_t _823 = _810 + _822;
  uint16_t _824 = _809 + _823;
  uint16_t _825 = _depthwise_conv_stencil_6 + _824;
  uint16_t _826 = _807 + _825;
  return _826;
}

//store is: depthwise_conv.stencil(6, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(6, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(6, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(6, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(6, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_depthwise_conv_stencil_13(hw_uint<16>& depthwise_conv_stencil, hw_uint<144>& input_host_global_wrapper_global_wrapper_stencil) {
  uint16_t _depthwise_conv_stencil_7 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  uint16_t _input_host_global_wrapper_global_wrapper_stencil_55 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_56 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_57 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_58 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_59 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_60 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_61 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_62 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _input_host_global_wrapper_global_wrapper_stencil_63 = (uint16_t) (input_host_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _883 = (uint16_t)(24);
  uint16_t _884 = _input_host_global_wrapper_global_wrapper_stencil_55 * _883;
  uint16_t _885 = (uint16_t)(30);
  uint16_t _886 = _input_host_global_wrapper_global_wrapper_stencil_56 * _885;
  uint16_t _887 = _input_host_global_wrapper_global_wrapper_stencil_57 * _883;
  uint16_t _888 = _input_host_global_wrapper_global_wrapper_stencil_58 * _885;
  uint16_t _889 = (uint16_t)(37);
  uint16_t _890 = _input_host_global_wrapper_global_wrapper_stencil_59 * _889;
  uint16_t _891 = _input_host_global_wrapper_global_wrapper_stencil_60 * _885;
  uint16_t _892 = _input_host_global_wrapper_global_wrapper_stencil_61 * _883;
  uint16_t _893 = _input_host_global_wrapper_global_wrapper_stencil_62 * _883;
  uint16_t _894 = _input_host_global_wrapper_global_wrapper_stencil_63 * _885;
  uint16_t _895 = _893 + _894;
  uint16_t _896 = _892 + _895;
  uint16_t _897 = _891 + _896;
  uint16_t _898 = _890 + _897;
  uint16_t _899 = _888 + _898;
  uint16_t _900 = _887 + _899;
  uint16_t _901 = _886 + _900;
  uint16_t _902 = _depthwise_conv_stencil_7 + _901;
  uint16_t _903 = _884 + _902;
  return _903;
}

//store is: hw_output.stencil(0, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(0, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_8 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_8;
}

//store is: hw_output.stencil(1, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(1, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil_1(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_9 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_9;
}

//store is: hw_output.stencil(2, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(2, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil_2(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_10 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_10;
}

//store is: hw_output.stencil(3, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(3, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil_3(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_11 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_11;
}

//store is: hw_output.stencil(4, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(4, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil_4(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_12 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_12;
}

//store is: hw_output.stencil(5, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(5, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil_5(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_13 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_13;
}

//store is: hw_output.stencil(6, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(6, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
hw_uint<16> hcompute_hw_output_stencil_6(hw_uint<16>& depthwise_conv_stencil) {
  uint16_t _depthwise_conv_stencil_14 = (uint16_t) (depthwise_conv_stencil.extract<0, 15>());

  return _depthwise_conv_stencil_14;
}

//store is: hw_output_global_wrapper.glb.stencil(0, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(0, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_1 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_1;
}

//store is: hw_output_global_wrapper.glb.stencil(1, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(1, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_1(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_2 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_2;
}

//store is: hw_output_global_wrapper.glb.stencil(2, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(2, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_2(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_3 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_3;
}

//store is: hw_output_global_wrapper.glb.stencil(3, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(3, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_3(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_4 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_4;
}

//store is: hw_output_global_wrapper.glb.stencil(4, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(4, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_4(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_5 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_5;
}

//store is: hw_output_global_wrapper.glb.stencil(5, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(5, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_5(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_6 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_6;
}

//store is: hw_output_global_wrapper.glb.stencil(6, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(6, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_6(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_7 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_7;
}

