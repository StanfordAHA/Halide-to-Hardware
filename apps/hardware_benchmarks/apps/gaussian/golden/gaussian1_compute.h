#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.glb.stencil(hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y) = hw_input.stencil((hw_input_global_wrapper_s0_x_x + (0*288)), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_1;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_1 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_1;
}

//store is: blur_unnormalized.stencil(blur_unnormalized_s0_x_x, blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil() {
  uint16_t _261 = (uint16_t)(0);
  return _261;
}

//store is: blur_unnormalized.stencil(blur_unnormalized_s1_x_x, blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(blur_unnormalized_s1_x_x, blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(blur_unnormalized_s1_x_x, blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_1(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_1 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_1 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_2 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_3 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_4 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_5 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_6 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_7 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_8 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_9 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _264 = (uint16_t)(24);
  uint16_t _265 = _hw_input_global_wrapper_global_wrapper_stencil_1 * _264;
  uint16_t _266 = (uint16_t)(30);
  uint16_t _267 = _hw_input_global_wrapper_global_wrapper_stencil_2 * _266;
  uint16_t _268 = _hw_input_global_wrapper_global_wrapper_stencil_3 * _264;
  uint16_t _269 = _hw_input_global_wrapper_global_wrapper_stencil_4 * _266;
  uint16_t _270 = (uint16_t)(37);
  uint16_t _271 = _hw_input_global_wrapper_global_wrapper_stencil_5 * _270;
  uint16_t _272 = _hw_input_global_wrapper_global_wrapper_stencil_6 * _266;
  uint16_t _273 = _hw_input_global_wrapper_global_wrapper_stencil_7 * _264;
  uint16_t _274 = _hw_input_global_wrapper_global_wrapper_stencil_8 * _264;
  uint16_t _275 = _hw_input_global_wrapper_global_wrapper_stencil_9 * _266;
  uint16_t _276 = _274 + _275;
  uint16_t _277 = _273 + _276;
  uint16_t _278 = _272 + _277;
  uint16_t _279 = _271 + _278;
  uint16_t _280 = _269 + _279;
  uint16_t _281 = _268 + _280;
  uint16_t _282 = _267 + _281;
  uint16_t _283 = _blur_unnormalized_stencil_1 + _282;
  uint16_t _284 = _265 + _283;
  return _284;
}

//store is: blur.stencil(blur_s0_x_x, blur_s0_y) = (blur_unnormalized.stencil(blur_s0_x_x, blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_2 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _341 = (uint16_t)(8);
  uint16_t _342 = _blur_unnormalized_stencil_2 >> _341;
  return _342;
}

//store is: hw_output.stencil(hw_output_s0_x_xii_xii, hw_output_s0_y_yii) = blur.stencil(hw_output_s0_x_xii_xii, hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_1 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_1;
}

//store is: hw_output_global_wrapper.glb.stencil((hw_output_global_wrapper_s0_x_xi_xi + (0*288)), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_1 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_1;
}

