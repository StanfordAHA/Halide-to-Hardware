#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.stencil(hw_input_global_wrapper_s0_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(hw_input_global_wrapper_s0_x, hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) hw_input_stencil.extract<0, 15>();

  return _hw_input_stencil_1;
}

//store is: blur_unnormalized.stencil(blur_unnormalized_s0_x, blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil() {
  uint16_t _274 = (uint16_t)(0);
  return _274;
}

//store is: blur_unnormalized.stencil(blur_unnormalized_s1_x, blur_unnormalized_s1_y) = ((hw_input_global_wrapper.stencil(blur_unnormalized_s1_x, blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(blur_unnormalized_s1_x, blur_unnormalized_s1_y) + ((hw_input_global_wrapper.stencil((blur_unnormalized_s1_x + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper.stencil((blur_unnormalized_s1_x + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper.stencil(blur_unnormalized_s1_x, (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper.stencil((blur_unnormalized_s1_x + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper.stencil((blur_unnormalized_s1_x + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper.stencil(blur_unnormalized_s1_x, (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper.stencil((blur_unnormalized_s1_x + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper.stencil((blur_unnormalized_s1_x + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_1(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_1 = (uint16_t) blur_unnormalized_stencil.extract<0, 15>();

  uint16_t _hw_input_global_wrapper_stencil_1 = (uint16_t) hw_input_global_wrapper_stencil.extract<0, 15>();
  uint16_t _hw_input_global_wrapper_stencil_2 = (uint16_t) hw_input_global_wrapper_stencil.extract<16, 31>();
  uint16_t _hw_input_global_wrapper_stencil_3 = (uint16_t) hw_input_global_wrapper_stencil.extract<32, 47>();
  uint16_t _hw_input_global_wrapper_stencil_4 = (uint16_t) hw_input_global_wrapper_stencil.extract<48, 63>();
  uint16_t _hw_input_global_wrapper_stencil_5 = (uint16_t) hw_input_global_wrapper_stencil.extract<64, 79>();
  uint16_t _hw_input_global_wrapper_stencil_6 = (uint16_t) hw_input_global_wrapper_stencil.extract<80, 95>();
  uint16_t _hw_input_global_wrapper_stencil_7 = (uint16_t) hw_input_global_wrapper_stencil.extract<96, 111>();
  uint16_t _hw_input_global_wrapper_stencil_8 = (uint16_t) hw_input_global_wrapper_stencil.extract<112, 127>();
  uint16_t _hw_input_global_wrapper_stencil_9 = (uint16_t) hw_input_global_wrapper_stencil.extract<128, 143>();

  uint16_t _277 = (uint16_t)(24);
  uint16_t _278 = _hw_input_global_wrapper_stencil_1 * _277;
  uint16_t _279 = (uint16_t)(30);
  uint16_t _280 = _hw_input_global_wrapper_stencil_2 * _279;
  uint16_t _281 = _hw_input_global_wrapper_stencil_3 * _277;
  uint16_t _282 = _hw_input_global_wrapper_stencil_4 * _279;
  uint16_t _283 = (uint16_t)(37);
  uint16_t _284 = _hw_input_global_wrapper_stencil_5 * _283;
  uint16_t _285 = _hw_input_global_wrapper_stencil_6 * _279;
  uint16_t _286 = _hw_input_global_wrapper_stencil_7 * _277;
  uint16_t _287 = _hw_input_global_wrapper_stencil_8 * _277;
  uint16_t _288 = _hw_input_global_wrapper_stencil_9 * _279;
  uint16_t _289 = _287 + _288;
  uint16_t _290 = _286 + _289;
  uint16_t _291 = _285 + _290;
  uint16_t _292 = _284 + _291;
  uint16_t _293 = _282 + _292;
  uint16_t _294 = _281 + _293;
  uint16_t _295 = _280 + _294;
  uint16_t _296 = _blur_unnormalized_stencil_1 + _295;
  uint16_t _297 = _278 + _296;
  return _297;
}

//store is: blur.stencil(blur_s0_x, blur_s0_y) = (blur_unnormalized.stencil(blur_s0_x, blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_2 = (uint16_t) blur_unnormalized_stencil.extract<0, 15>();

  uint16_t _354 = (uint16_t)(8);
  uint16_t _355 = _blur_unnormalized_stencil_2 >> _354;
  return _355;
}

//store is: hw_output.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi) = blur.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_1 = (uint16_t) blur_stencil.extract<0, 15>();

  return _blur_stencil_1;
}

