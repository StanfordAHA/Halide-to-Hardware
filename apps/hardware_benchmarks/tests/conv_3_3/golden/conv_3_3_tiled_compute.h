#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.stencil(hw_input_global_wrapper_s0_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(hw_input_global_wrapper_s0_x, hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) hw_input_stencil.extract<0, 15>();

  return _hw_input_stencil_1;
}

//store is: conv.stencil(conv_s0_x, conv_s0_y) = (uint16)0
hw_uint<16> hcompute_conv_stencil() {
  uint16_t _284 = (uint16_t)(0);
  return _284;
}

//store is: conv.stencil(conv_s1_x, conv_s1_y) = ((hw_input_global_wrapper.stencil(conv_s1_x, conv_s1_y)*(uint16)11) + (conv.stencil(conv_s1_x, conv_s1_y) + ((hw_input_global_wrapper.stencil((conv_s1_x + 1), conv_s1_y)*(uint16)14) + ((hw_input_global_wrapper.stencil((conv_s1_x + 2), conv_s1_y)*(uint16)17) + ((hw_input_global_wrapper.stencil(conv_s1_x, (conv_s1_y + 1))*(uint16)12) + ((hw_input_global_wrapper.stencil((conv_s1_x + 1), (conv_s1_y + 1))*(uint16)255) + ((hw_input_global_wrapper.stencil((conv_s1_x + 2), (conv_s1_y + 1))*(uint16)18) + ((hw_input_global_wrapper.stencil(conv_s1_x, (conv_s1_y + 2))*(uint16)13) + ((hw_input_global_wrapper.stencil((conv_s1_x + 2), (conv_s1_y + 2))*(uint16)19) + (hw_input_global_wrapper.stencil((conv_s1_x + 1), (conv_s1_y + 2))*(uint16)16))))))))))
hw_uint<16> hcompute_conv_stencil_1(hw_uint<16>& conv_stencil, hw_uint<144>& hw_input_global_wrapper_stencil) {
  uint16_t _conv_stencil_1 = (uint16_t) conv_stencil.extract<0, 15>();

  uint16_t _hw_input_global_wrapper_stencil_1 = (uint16_t) hw_input_global_wrapper_stencil.extract<0, 15>();
  uint16_t _hw_input_global_wrapper_stencil_2 = (uint16_t) hw_input_global_wrapper_stencil.extract<16, 31>();
  uint16_t _hw_input_global_wrapper_stencil_3 = (uint16_t) hw_input_global_wrapper_stencil.extract<32, 47>();
  uint16_t _hw_input_global_wrapper_stencil_4 = (uint16_t) hw_input_global_wrapper_stencil.extract<48, 63>();
  uint16_t _hw_input_global_wrapper_stencil_5 = (uint16_t) hw_input_global_wrapper_stencil.extract<64, 79>();
  uint16_t _hw_input_global_wrapper_stencil_6 = (uint16_t) hw_input_global_wrapper_stencil.extract<80, 95>();
  uint16_t _hw_input_global_wrapper_stencil_7 = (uint16_t) hw_input_global_wrapper_stencil.extract<96, 111>();
  uint16_t _hw_input_global_wrapper_stencil_8 = (uint16_t) hw_input_global_wrapper_stencil.extract<112, 127>();
  uint16_t _hw_input_global_wrapper_stencil_9 = (uint16_t) hw_input_global_wrapper_stencil.extract<128, 143>();

  uint16_t _287 = (uint16_t)(11);
  uint16_t _288 = _hw_input_global_wrapper_stencil_1 * _287;
  uint16_t _289 = (uint16_t)(14);
  uint16_t _290 = _hw_input_global_wrapper_stencil_2 * _289;
  uint16_t _291 = (uint16_t)(17);
  uint16_t _292 = _hw_input_global_wrapper_stencil_3 * _291;
  uint16_t _293 = (uint16_t)(12);
  uint16_t _294 = _hw_input_global_wrapper_stencil_4 * _293;
  uint16_t _295 = (uint16_t)(255);
  uint16_t _296 = _hw_input_global_wrapper_stencil_5 * _295;
  uint16_t _297 = (uint16_t)(18);
  uint16_t _298 = _hw_input_global_wrapper_stencil_6 * _297;
  uint16_t _299 = (uint16_t)(13);
  uint16_t _300 = _hw_input_global_wrapper_stencil_7 * _299;
  uint16_t _301 = (uint16_t)(19);
  uint16_t _302 = _hw_input_global_wrapper_stencil_8 * _301;
  uint16_t _303 = (uint16_t)(16);
  uint16_t _304 = _hw_input_global_wrapper_stencil_9 * _303;
  uint16_t _305 = _302 + _304;
  uint16_t _306 = _300 + _305;
  uint16_t _307 = _298 + _306;
  uint16_t _308 = _296 + _307;
  uint16_t _309 = _294 + _308;
  uint16_t _310 = _292 + _309;
  uint16_t _311 = _290 + _310;
  uint16_t _312 = _conv_stencil_1 + _311;
  uint16_t _313 = _288 + _312;
  return _313;
}

//store is: hw_output.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi) = conv.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& conv_stencil) {
  uint16_t _conv_stencil_2 = (uint16_t) conv_stencil.extract<0, 15>();

  return _conv_stencil_2;
}

