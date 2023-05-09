#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_s0_x_x*3), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x + (0*96))*3), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_1;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*3) + 1), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*96))*3) + 1), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_1(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_2 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_2;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*3) + 2), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*96))*3) + 2), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_2(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_3 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_3;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*3), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*3), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_1 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_1;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*3) + 1), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*3) + 1), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_1(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_2 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_2;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*3) + 2), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*3) + 2), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_2(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_3 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_3;
}

//store is: blur_unnormalized.stencil((blur_unnormalized_s0_x_x*3), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil() {
  uint16_t _284 = (uint16_t)(0);
  return _284;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*3) + 1), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_1() {
  uint16_t _288 = (uint16_t)(0);
  return _288;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*3) + 2), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_2() {
  uint16_t _293 = (uint16_t)(0);
  return _293;
}

//store is: blur_unnormalized.stencil((blur_unnormalized_s1_x_x*3), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*3), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil((blur_unnormalized_s1_x_x*3), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*3), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_3(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _298 = (uint16_t)(24);
  uint16_t _299 = _hw_input_global_wrapper_global_wrapper_stencil_1 * _298;
  uint16_t _300 = (uint16_t)(30);
  uint16_t _301 = _hw_input_global_wrapper_global_wrapper_stencil_2 * _300;
  uint16_t _302 = _hw_input_global_wrapper_global_wrapper_stencil_3 * _298;
  uint16_t _303 = _hw_input_global_wrapper_global_wrapper_stencil_4 * _300;
  uint16_t _304 = (uint16_t)(37);
  uint16_t _305 = _hw_input_global_wrapper_global_wrapper_stencil_5 * _304;
  uint16_t _306 = _hw_input_global_wrapper_global_wrapper_stencil_6 * _300;
  uint16_t _307 = _hw_input_global_wrapper_global_wrapper_stencil_7 * _298;
  uint16_t _308 = _hw_input_global_wrapper_global_wrapper_stencil_8 * _298;
  uint16_t _309 = _hw_input_global_wrapper_global_wrapper_stencil_9 * _300;
  uint16_t _310 = _308 + _309;
  uint16_t _311 = _307 + _310;
  uint16_t _312 = _306 + _311;
  uint16_t _313 = _305 + _312;
  uint16_t _314 = _303 + _313;
  uint16_t _315 = _302 + _314;
  uint16_t _316 = _301 + _315;
  uint16_t _317 = _blur_unnormalized_stencil_1 + _316;
  uint16_t _318 = _299 + _317;
  return _318;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*3) + 1), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 1), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*3) + 1), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 3), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 1), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 1), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_4(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_2 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_10 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_11 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_12 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_13 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_14 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_15 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_16 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_17 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_18 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _376 = (uint16_t)(24);
  uint16_t _377 = _hw_input_global_wrapper_global_wrapper_stencil_10 * _376;
  uint16_t _378 = (uint16_t)(30);
  uint16_t _379 = _hw_input_global_wrapper_global_wrapper_stencil_11 * _378;
  uint16_t _380 = _hw_input_global_wrapper_global_wrapper_stencil_12 * _376;
  uint16_t _381 = _hw_input_global_wrapper_global_wrapper_stencil_13 * _378;
  uint16_t _382 = (uint16_t)(37);
  uint16_t _383 = _hw_input_global_wrapper_global_wrapper_stencil_14 * _382;
  uint16_t _384 = _hw_input_global_wrapper_global_wrapper_stencil_15 * _378;
  uint16_t _385 = _hw_input_global_wrapper_global_wrapper_stencil_16 * _376;
  uint16_t _386 = _hw_input_global_wrapper_global_wrapper_stencil_17 * _376;
  uint16_t _387 = _hw_input_global_wrapper_global_wrapper_stencil_18 * _378;
  uint16_t _388 = _386 + _387;
  uint16_t _389 = _385 + _388;
  uint16_t _390 = _384 + _389;
  uint16_t _391 = _383 + _390;
  uint16_t _392 = _381 + _391;
  uint16_t _393 = _380 + _392;
  uint16_t _394 = _379 + _393;
  uint16_t _395 = _blur_unnormalized_stencil_2 + _394;
  uint16_t _396 = _377 + _395;
  return _396;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*3) + 2), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*3) + 2), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 3), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 4), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 3), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*3) + 3), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_5(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_3 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_19 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_20 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_21 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_22 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_23 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_24 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_25 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_26 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_27 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _455 = (uint16_t)(24);
  uint16_t _456 = _hw_input_global_wrapper_global_wrapper_stencil_19 * _455;
  uint16_t _457 = (uint16_t)(30);
  uint16_t _458 = _hw_input_global_wrapper_global_wrapper_stencil_20 * _457;
  uint16_t _459 = _hw_input_global_wrapper_global_wrapper_stencil_21 * _455;
  uint16_t _460 = _hw_input_global_wrapper_global_wrapper_stencil_22 * _457;
  uint16_t _461 = (uint16_t)(37);
  uint16_t _462 = _hw_input_global_wrapper_global_wrapper_stencil_23 * _461;
  uint16_t _463 = _hw_input_global_wrapper_global_wrapper_stencil_24 * _457;
  uint16_t _464 = _hw_input_global_wrapper_global_wrapper_stencil_25 * _455;
  uint16_t _465 = _hw_input_global_wrapper_global_wrapper_stencil_26 * _455;
  uint16_t _466 = _hw_input_global_wrapper_global_wrapper_stencil_27 * _457;
  uint16_t _467 = _465 + _466;
  uint16_t _468 = _464 + _467;
  uint16_t _469 = _463 + _468;
  uint16_t _470 = _462 + _469;
  uint16_t _471 = _460 + _470;
  uint16_t _472 = _459 + _471;
  uint16_t _473 = _458 + _472;
  uint16_t _474 = _blur_unnormalized_stencil_3 + _473;
  uint16_t _475 = _456 + _474;
  return _475;
}

//store is: blur.stencil((blur_s0_x_x*3), blur_s0_y) = (blur_unnormalized.stencil((blur_s0_x_x*3), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_4 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _534 = (uint16_t)(8);
  uint16_t _535 = _blur_unnormalized_stencil_4 >> _534;
  return _535;
}

//store is: blur.stencil(((blur_s0_x_x*3) + 1), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*3) + 1), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_1(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_5 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _542 = (uint16_t)(8);
  uint16_t _543 = _blur_unnormalized_stencil_5 >> _542;
  return _543;
}

//store is: blur.stencil(((blur_s0_x_x*3) + 2), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*3) + 2), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_2(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_6 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _551 = (uint16_t)(8);
  uint16_t _552 = _blur_unnormalized_stencil_6 >> _551;
  return _552;
}

//store is: hw_output.stencil((hw_output_s0_x_xii_xii*3), hw_output_s0_y_yii) = blur.stencil((hw_output_s0_x_xii_xii*3), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_1 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_1;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*3) + 1), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*3) + 1), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_1(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_2 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_2;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*3) + 2), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*3) + 2), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_2(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_3 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_3;
}

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi + (0*96))*3), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil((hw_output_global_wrapper_s0_x_xi_xi*3), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_1 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_1;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*96))*3) + 1), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*3) + 1), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_1(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_2 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_2;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*96))*3) + 2), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*3) + 2), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_2(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_3 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_3;
}

