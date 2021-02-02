#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.stencil(hw_input_global_wrapper_s0_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(hw_input_global_wrapper_s0_x, hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) hw_input_stencil.extract<0, 15>();

  return _hw_input_stencil_1;
}

//store is: conv1.stencil(conv1_s0_x, conv1_s0_y) = (uint16)0
hw_uint<16> hcompute_conv1_stencil() {
  uint16_t _284 = (uint16_t)(0);
  return _284;
}

//store is: conv1.stencil(conv1_s1_x, conv1_s1_y) = (hw_input_global_wrapper.stencil(conv1_s1_x, conv1_s1_y) + (conv1.stencil(conv1_s1_x, conv1_s1_y) + ((hw_input_global_wrapper.stencil(conv1_s1_x, (conv1_s1_y + 1))*(uint16)2) + (hw_input_global_wrapper.stencil(conv1_s1_x, (conv1_s1_y + 2)) + ((hw_input_global_wrapper.stencil((conv1_s1_x + 1), conv1_s1_y)*(uint16)2) + ((hw_input_global_wrapper.stencil((conv1_s1_x + 1), (conv1_s1_y + 1))*(uint16)4) + ((hw_input_global_wrapper.stencil((conv1_s1_x + 1), (conv1_s1_y + 2))*(uint16)2) + (hw_input_global_wrapper.stencil((conv1_s1_x + 2), conv1_s1_y) + (hw_input_global_wrapper.stencil((conv1_s1_x + 2), (conv1_s1_y + 2)) + (hw_input_global_wrapper.stencil((conv1_s1_x + 2), (conv1_s1_y + 1))*(uint16)2))))))))))
hw_uint<16> hcompute_conv1_stencil_1(hw_uint<16>& conv1_stencil, hw_uint<144>& hw_input_global_wrapper_stencil) {
  uint16_t _conv1_stencil_1 = (uint16_t) conv1_stencil.extract<0, 15>();

  uint16_t _hw_input_global_wrapper_stencil_1 = (uint16_t) hw_input_global_wrapper_stencil.extract<0, 15>();
  uint16_t _hw_input_global_wrapper_stencil_2 = (uint16_t) hw_input_global_wrapper_stencil.extract<16, 31>();
  uint16_t _hw_input_global_wrapper_stencil_3 = (uint16_t) hw_input_global_wrapper_stencil.extract<32, 47>();
  uint16_t _hw_input_global_wrapper_stencil_4 = (uint16_t) hw_input_global_wrapper_stencil.extract<48, 63>();
  uint16_t _hw_input_global_wrapper_stencil_5 = (uint16_t) hw_input_global_wrapper_stencil.extract<64, 79>();
  uint16_t _hw_input_global_wrapper_stencil_6 = (uint16_t) hw_input_global_wrapper_stencil.extract<80, 95>();
  uint16_t _hw_input_global_wrapper_stencil_7 = (uint16_t) hw_input_global_wrapper_stencil.extract<96, 111>();
  uint16_t _hw_input_global_wrapper_stencil_8 = (uint16_t) hw_input_global_wrapper_stencil.extract<112, 127>();
  uint16_t _hw_input_global_wrapper_stencil_9 = (uint16_t) hw_input_global_wrapper_stencil.extract<128, 143>();

  uint16_t _287 = (uint16_t)(2);
  uint16_t _288 = _hw_input_global_wrapper_stencil_2 * _287;
  uint16_t _289 = _hw_input_global_wrapper_stencil_4 * _287;
  uint16_t _290 = (uint16_t)(4);
  uint16_t _291 = _hw_input_global_wrapper_stencil_5 * _290;
  uint16_t _292 = _hw_input_global_wrapper_stencil_6 * _287;
  uint16_t _293 = _hw_input_global_wrapper_stencil_9 * _287;
  uint16_t _294 = _hw_input_global_wrapper_stencil_8 + _293;
  uint16_t _295 = _hw_input_global_wrapper_stencil_7 + _294;
  uint16_t _296 = _292 + _295;
  uint16_t _297 = _291 + _296;
  uint16_t _298 = _289 + _297;
  uint16_t _299 = _hw_input_global_wrapper_stencil_3 + _298;
  uint16_t _300 = _288 + _299;
  uint16_t _301 = _conv1_stencil_1 + _300;
  uint16_t _302 = _hw_input_global_wrapper_stencil_1 + _301;
  return _302;
}

//store is: conv2.stencil(conv2_s0_x, conv2_s0_y) = (uint16)0
hw_uint<16> hcompute_conv2_stencil() {
  uint16_t _349 = (uint16_t)(0);
  return _349;
}

//store is: conv2.stencil(conv2_s1_x, conv2_s1_y) = (conv1.stencil(conv2_s1_x, conv2_s1_y) + (conv2.stencil(conv2_s1_x, conv2_s1_y) + ((conv1.stencil((conv2_s1_x + 1), conv2_s1_y)*(uint16)2) + (conv1.stencil((conv2_s1_x + 2), conv2_s1_y) + ((conv1.stencil(conv2_s1_x, (conv2_s1_y + 1))*(uint16)2) + ((conv1.stencil((conv2_s1_x + 1), (conv2_s1_y + 1))*(uint16)4) + ((conv1.stencil((conv2_s1_x + 2), (conv2_s1_y + 1))*(uint16)2) + (conv1.stencil(conv2_s1_x, (conv2_s1_y + 2)) + (conv1.stencil((conv2_s1_x + 2), (conv2_s1_y + 2)) + (conv1.stencil((conv2_s1_x + 1), (conv2_s1_y + 2))*(uint16)2))))))))))
hw_uint<16> hcompute_conv2_stencil_1(hw_uint<144>& conv1_stencil, hw_uint<16>& conv2_stencil) {
  uint16_t _conv1_stencil_10 = (uint16_t) conv1_stencil.extract<0, 15>();
  uint16_t _conv1_stencil_2 = (uint16_t) conv1_stencil.extract<16, 31>();
  uint16_t _conv1_stencil_3 = (uint16_t) conv1_stencil.extract<32, 47>();
  uint16_t _conv1_stencil_4 = (uint16_t) conv1_stencil.extract<48, 63>();
  uint16_t _conv1_stencil_5 = (uint16_t) conv1_stencil.extract<64, 79>();
  uint16_t _conv1_stencil_6 = (uint16_t) conv1_stencil.extract<80, 95>();
  uint16_t _conv1_stencil_7 = (uint16_t) conv1_stencil.extract<96, 111>();
  uint16_t _conv1_stencil_8 = (uint16_t) conv1_stencil.extract<112, 127>();
  uint16_t _conv1_stencil_9 = (uint16_t) conv1_stencil.extract<128, 143>();

  uint16_t _conv2_stencil_1 = (uint16_t) conv2_stencil.extract<0, 15>();

  uint16_t _352 = (uint16_t)(2);
  uint16_t _353 = _conv1_stencil_3 * _352;
  uint16_t _354 = _conv1_stencil_5 * _352;
  uint16_t _355 = (uint16_t)(4);
  uint16_t _356 = _conv1_stencil_6 * _355;
  uint16_t _357 = _conv1_stencil_7 * _352;
  uint16_t _358 = _conv1_stencil_10 * _352;
  uint16_t _359 = _conv1_stencil_9 + _358;
  uint16_t _360 = _conv1_stencil_8 + _359;
  uint16_t _361 = _357 + _360;
  uint16_t _362 = _356 + _361;
  uint16_t _363 = _354 + _362;
  uint16_t _364 = _conv1_stencil_4 + _363;
  uint16_t _365 = _353 + _364;
  uint16_t _366 = _conv2_stencil_1 + _365;
  uint16_t _367 = _conv1_stencil_2 + _366;
  return _367;
}

//store is: hw_output.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi) = conv2.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& conv2_stencil) {
  uint16_t _conv2_stencil_2 = (uint16_t) conv2_stencil.extract<0, 15>();

  return _conv2_stencil_2;
}

