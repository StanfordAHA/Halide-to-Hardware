#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_s0_x_x*16), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x + (0*18))*16), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_1;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 1), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_1(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_2 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_2;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 2), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_2(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_3 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_3;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 3), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_3(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_4 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_4;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 4), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_4(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_5 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_5;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 5), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_5(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_6 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_6;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 6), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_6(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_7 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_7;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 7), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_7(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_8 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_8;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 8), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_8(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_9 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_9;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 9), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_9(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_10 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_10;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 10), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_10(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_11 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_11;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 11), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_11(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_12 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_12;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 12), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_12(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_13 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_13;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 13), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_13(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_14 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_14;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 14), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_14(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_15 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_15;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*18))*16) + 15), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_15(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_16 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_16;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*16), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*16), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_1 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_1;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_1(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_2 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_2;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_2(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_3 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_3;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_3(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_4 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_4;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_4(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_5 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_5;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_5(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_6 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_6;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_6(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_7 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_7;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_7(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_8 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_8;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_8(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_9 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_9;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_9(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_10 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_10;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_10(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_11 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_11;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_11(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_12 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_12;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_12(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_13 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_13;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_13(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_14 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_14;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_14(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_15 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_15;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_15(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_16 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_16;
}

//store is: blur_unnormalized.stencil((blur_unnormalized_s0_x_x*16), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil() {
  uint16_t _414 = (uint16_t)(0);
  return _414;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 1), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_1() {
  uint16_t _418 = (uint16_t)(0);
  return _418;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 2), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_2() {
  uint16_t _423 = (uint16_t)(0);
  return _423;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 3), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_3() {
  uint16_t _428 = (uint16_t)(0);
  return _428;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 4), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_4() {
  uint16_t _433 = (uint16_t)(0);
  return _433;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 5), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_5() {
  uint16_t _438 = (uint16_t)(0);
  return _438;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 6), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_6() {
  uint16_t _443 = (uint16_t)(0);
  return _443;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 7), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_7() {
  uint16_t _448 = (uint16_t)(0);
  return _448;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 8), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_8() {
  uint16_t _453 = (uint16_t)(0);
  return _453;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 9), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_9() {
  uint16_t _458 = (uint16_t)(0);
  return _458;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 10), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_10() {
  uint16_t _463 = (uint16_t)(0);
  return _463;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 11), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_11() {
  uint16_t _468 = (uint16_t)(0);
  return _468;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 12), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_12() {
  uint16_t _473 = (uint16_t)(0);
  return _473;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 13), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_13() {
  uint16_t _478 = (uint16_t)(0);
  return _478;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 14), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_14() {
  uint16_t _483 = (uint16_t)(0);
  return _483;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 15), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_15() {
  uint16_t _488 = (uint16_t)(0);
  return _488;
}

//store is: blur_unnormalized.stencil((blur_unnormalized_s1_x_x*16), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*16), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil((blur_unnormalized_s1_x_x*16), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*16), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_16(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _493 = (uint16_t)(24);
  uint16_t _494 = _hw_input_global_wrapper_global_wrapper_stencil_1 * _493;
  uint16_t _495 = (uint16_t)(30);
  uint16_t _496 = _hw_input_global_wrapper_global_wrapper_stencil_2 * _495;
  uint16_t _497 = _hw_input_global_wrapper_global_wrapper_stencil_3 * _493;
  uint16_t _498 = _hw_input_global_wrapper_global_wrapper_stencil_4 * _495;
  uint16_t _499 = (uint16_t)(37);
  uint16_t _500 = _hw_input_global_wrapper_global_wrapper_stencil_5 * _499;
  uint16_t _501 = _hw_input_global_wrapper_global_wrapper_stencil_6 * _495;
  uint16_t _502 = _hw_input_global_wrapper_global_wrapper_stencil_7 * _493;
  uint16_t _503 = _hw_input_global_wrapper_global_wrapper_stencil_8 * _493;
  uint16_t _504 = _hw_input_global_wrapper_global_wrapper_stencil_9 * _495;
  uint16_t _505 = _503 + _504;
  uint16_t _506 = _502 + _505;
  uint16_t _507 = _501 + _506;
  uint16_t _508 = _500 + _507;
  uint16_t _509 = _498 + _508;
  uint16_t _510 = _497 + _509;
  uint16_t _511 = _496 + _510;
  uint16_t _512 = _blur_unnormalized_stencil_1 + _511;
  uint16_t _513 = _494 + _512;
  return _513;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_17(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _571 = (uint16_t)(24);
  uint16_t _572 = _hw_input_global_wrapper_global_wrapper_stencil_10 * _571;
  uint16_t _573 = (uint16_t)(30);
  uint16_t _574 = _hw_input_global_wrapper_global_wrapper_stencil_11 * _573;
  uint16_t _575 = _hw_input_global_wrapper_global_wrapper_stencil_12 * _571;
  uint16_t _576 = _hw_input_global_wrapper_global_wrapper_stencil_13 * _573;
  uint16_t _577 = (uint16_t)(37);
  uint16_t _578 = _hw_input_global_wrapper_global_wrapper_stencil_14 * _577;
  uint16_t _579 = _hw_input_global_wrapper_global_wrapper_stencil_15 * _573;
  uint16_t _580 = _hw_input_global_wrapper_global_wrapper_stencil_16 * _571;
  uint16_t _581 = _hw_input_global_wrapper_global_wrapper_stencil_17 * _571;
  uint16_t _582 = _hw_input_global_wrapper_global_wrapper_stencil_18 * _573;
  uint16_t _583 = _581 + _582;
  uint16_t _584 = _580 + _583;
  uint16_t _585 = _579 + _584;
  uint16_t _586 = _578 + _585;
  uint16_t _587 = _576 + _586;
  uint16_t _588 = _575 + _587;
  uint16_t _589 = _574 + _588;
  uint16_t _590 = _blur_unnormalized_stencil_2 + _589;
  uint16_t _591 = _572 + _590;
  return _591;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_18(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _650 = (uint16_t)(24);
  uint16_t _651 = _hw_input_global_wrapper_global_wrapper_stencil_19 * _650;
  uint16_t _652 = (uint16_t)(30);
  uint16_t _653 = _hw_input_global_wrapper_global_wrapper_stencil_20 * _652;
  uint16_t _654 = _hw_input_global_wrapper_global_wrapper_stencil_21 * _650;
  uint16_t _655 = _hw_input_global_wrapper_global_wrapper_stencil_22 * _652;
  uint16_t _656 = (uint16_t)(37);
  uint16_t _657 = _hw_input_global_wrapper_global_wrapper_stencil_23 * _656;
  uint16_t _658 = _hw_input_global_wrapper_global_wrapper_stencil_24 * _652;
  uint16_t _659 = _hw_input_global_wrapper_global_wrapper_stencil_25 * _650;
  uint16_t _660 = _hw_input_global_wrapper_global_wrapper_stencil_26 * _650;
  uint16_t _661 = _hw_input_global_wrapper_global_wrapper_stencil_27 * _652;
  uint16_t _662 = _660 + _661;
  uint16_t _663 = _659 + _662;
  uint16_t _664 = _658 + _663;
  uint16_t _665 = _657 + _664;
  uint16_t _666 = _655 + _665;
  uint16_t _667 = _654 + _666;
  uint16_t _668 = _653 + _667;
  uint16_t _669 = _blur_unnormalized_stencil_3 + _668;
  uint16_t _670 = _651 + _669;
  return _670;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_19(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_4 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_28 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_29 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_30 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_31 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_32 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_33 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_34 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_35 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_36 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _729 = (uint16_t)(24);
  uint16_t _730 = _hw_input_global_wrapper_global_wrapper_stencil_28 * _729;
  uint16_t _731 = (uint16_t)(30);
  uint16_t _732 = _hw_input_global_wrapper_global_wrapper_stencil_29 * _731;
  uint16_t _733 = _hw_input_global_wrapper_global_wrapper_stencil_30 * _729;
  uint16_t _734 = _hw_input_global_wrapper_global_wrapper_stencil_31 * _731;
  uint16_t _735 = (uint16_t)(37);
  uint16_t _736 = _hw_input_global_wrapper_global_wrapper_stencil_32 * _735;
  uint16_t _737 = _hw_input_global_wrapper_global_wrapper_stencil_33 * _731;
  uint16_t _738 = _hw_input_global_wrapper_global_wrapper_stencil_34 * _729;
  uint16_t _739 = _hw_input_global_wrapper_global_wrapper_stencil_35 * _729;
  uint16_t _740 = _hw_input_global_wrapper_global_wrapper_stencil_36 * _731;
  uint16_t _741 = _739 + _740;
  uint16_t _742 = _738 + _741;
  uint16_t _743 = _737 + _742;
  uint16_t _744 = _736 + _743;
  uint16_t _745 = _734 + _744;
  uint16_t _746 = _733 + _745;
  uint16_t _747 = _732 + _746;
  uint16_t _748 = _blur_unnormalized_stencil_4 + _747;
  uint16_t _749 = _730 + _748;
  return _749;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_20(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_5 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_37 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_38 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_39 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_40 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_41 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_42 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_43 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_44 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_45 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _808 = (uint16_t)(24);
  uint16_t _809 = _hw_input_global_wrapper_global_wrapper_stencil_37 * _808;
  uint16_t _810 = (uint16_t)(30);
  uint16_t _811 = _hw_input_global_wrapper_global_wrapper_stencil_38 * _810;
  uint16_t _812 = _hw_input_global_wrapper_global_wrapper_stencil_39 * _808;
  uint16_t _813 = _hw_input_global_wrapper_global_wrapper_stencil_40 * _810;
  uint16_t _814 = (uint16_t)(37);
  uint16_t _815 = _hw_input_global_wrapper_global_wrapper_stencil_41 * _814;
  uint16_t _816 = _hw_input_global_wrapper_global_wrapper_stencil_42 * _810;
  uint16_t _817 = _hw_input_global_wrapper_global_wrapper_stencil_43 * _808;
  uint16_t _818 = _hw_input_global_wrapper_global_wrapper_stencil_44 * _808;
  uint16_t _819 = _hw_input_global_wrapper_global_wrapper_stencil_45 * _810;
  uint16_t _820 = _818 + _819;
  uint16_t _821 = _817 + _820;
  uint16_t _822 = _816 + _821;
  uint16_t _823 = _815 + _822;
  uint16_t _824 = _813 + _823;
  uint16_t _825 = _812 + _824;
  uint16_t _826 = _811 + _825;
  uint16_t _827 = _blur_unnormalized_stencil_5 + _826;
  uint16_t _828 = _809 + _827;
  return _828;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_21(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_6 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_46 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_47 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_48 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_49 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_50 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_51 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_52 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_53 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_54 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _887 = (uint16_t)(24);
  uint16_t _888 = _hw_input_global_wrapper_global_wrapper_stencil_46 * _887;
  uint16_t _889 = (uint16_t)(30);
  uint16_t _890 = _hw_input_global_wrapper_global_wrapper_stencil_47 * _889;
  uint16_t _891 = _hw_input_global_wrapper_global_wrapper_stencil_48 * _887;
  uint16_t _892 = _hw_input_global_wrapper_global_wrapper_stencil_49 * _889;
  uint16_t _893 = (uint16_t)(37);
  uint16_t _894 = _hw_input_global_wrapper_global_wrapper_stencil_50 * _893;
  uint16_t _895 = _hw_input_global_wrapper_global_wrapper_stencil_51 * _889;
  uint16_t _896 = _hw_input_global_wrapper_global_wrapper_stencil_52 * _887;
  uint16_t _897 = _hw_input_global_wrapper_global_wrapper_stencil_53 * _887;
  uint16_t _898 = _hw_input_global_wrapper_global_wrapper_stencil_54 * _889;
  uint16_t _899 = _897 + _898;
  uint16_t _900 = _896 + _899;
  uint16_t _901 = _895 + _900;
  uint16_t _902 = _894 + _901;
  uint16_t _903 = _892 + _902;
  uint16_t _904 = _891 + _903;
  uint16_t _905 = _890 + _904;
  uint16_t _906 = _blur_unnormalized_stencil_6 + _905;
  uint16_t _907 = _888 + _906;
  return _907;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_22(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_7 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_55 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_56 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_57 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_58 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_59 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_60 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_61 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_62 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_63 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _966 = (uint16_t)(24);
  uint16_t _967 = _hw_input_global_wrapper_global_wrapper_stencil_55 * _966;
  uint16_t _968 = (uint16_t)(30);
  uint16_t _969 = _hw_input_global_wrapper_global_wrapper_stencil_56 * _968;
  uint16_t _970 = _hw_input_global_wrapper_global_wrapper_stencil_57 * _966;
  uint16_t _971 = _hw_input_global_wrapper_global_wrapper_stencil_58 * _968;
  uint16_t _972 = (uint16_t)(37);
  uint16_t _973 = _hw_input_global_wrapper_global_wrapper_stencil_59 * _972;
  uint16_t _974 = _hw_input_global_wrapper_global_wrapper_stencil_60 * _968;
  uint16_t _975 = _hw_input_global_wrapper_global_wrapper_stencil_61 * _966;
  uint16_t _976 = _hw_input_global_wrapper_global_wrapper_stencil_62 * _966;
  uint16_t _977 = _hw_input_global_wrapper_global_wrapper_stencil_63 * _968;
  uint16_t _978 = _976 + _977;
  uint16_t _979 = _975 + _978;
  uint16_t _980 = _974 + _979;
  uint16_t _981 = _973 + _980;
  uint16_t _982 = _971 + _981;
  uint16_t _983 = _970 + _982;
  uint16_t _984 = _969 + _983;
  uint16_t _985 = _blur_unnormalized_stencil_7 + _984;
  uint16_t _986 = _967 + _985;
  return _986;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_23(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_8 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_64 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_65 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_66 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_67 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_68 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_69 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_70 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_71 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_72 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1045 = (uint16_t)(24);
  uint16_t _1046 = _hw_input_global_wrapper_global_wrapper_stencil_64 * _1045;
  uint16_t _1047 = (uint16_t)(30);
  uint16_t _1048 = _hw_input_global_wrapper_global_wrapper_stencil_65 * _1047;
  uint16_t _1049 = _hw_input_global_wrapper_global_wrapper_stencil_66 * _1045;
  uint16_t _1050 = _hw_input_global_wrapper_global_wrapper_stencil_67 * _1047;
  uint16_t _1051 = (uint16_t)(37);
  uint16_t _1052 = _hw_input_global_wrapper_global_wrapper_stencil_68 * _1051;
  uint16_t _1053 = _hw_input_global_wrapper_global_wrapper_stencil_69 * _1047;
  uint16_t _1054 = _hw_input_global_wrapper_global_wrapper_stencil_70 * _1045;
  uint16_t _1055 = _hw_input_global_wrapper_global_wrapper_stencil_71 * _1045;
  uint16_t _1056 = _hw_input_global_wrapper_global_wrapper_stencil_72 * _1047;
  uint16_t _1057 = _1055 + _1056;
  uint16_t _1058 = _1054 + _1057;
  uint16_t _1059 = _1053 + _1058;
  uint16_t _1060 = _1052 + _1059;
  uint16_t _1061 = _1050 + _1060;
  uint16_t _1062 = _1049 + _1061;
  uint16_t _1063 = _1048 + _1062;
  uint16_t _1064 = _blur_unnormalized_stencil_8 + _1063;
  uint16_t _1065 = _1046 + _1064;
  return _1065;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_24(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_9 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_73 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_74 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_75 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_76 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_77 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_78 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_79 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_80 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_81 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1124 = (uint16_t)(24);
  uint16_t _1125 = _hw_input_global_wrapper_global_wrapper_stencil_73 * _1124;
  uint16_t _1126 = (uint16_t)(30);
  uint16_t _1127 = _hw_input_global_wrapper_global_wrapper_stencil_74 * _1126;
  uint16_t _1128 = _hw_input_global_wrapper_global_wrapper_stencil_75 * _1124;
  uint16_t _1129 = _hw_input_global_wrapper_global_wrapper_stencil_76 * _1126;
  uint16_t _1130 = (uint16_t)(37);
  uint16_t _1131 = _hw_input_global_wrapper_global_wrapper_stencil_77 * _1130;
  uint16_t _1132 = _hw_input_global_wrapper_global_wrapper_stencil_78 * _1126;
  uint16_t _1133 = _hw_input_global_wrapper_global_wrapper_stencil_79 * _1124;
  uint16_t _1134 = _hw_input_global_wrapper_global_wrapper_stencil_80 * _1124;
  uint16_t _1135 = _hw_input_global_wrapper_global_wrapper_stencil_81 * _1126;
  uint16_t _1136 = _1134 + _1135;
  uint16_t _1137 = _1133 + _1136;
  uint16_t _1138 = _1132 + _1137;
  uint16_t _1139 = _1131 + _1138;
  uint16_t _1140 = _1129 + _1139;
  uint16_t _1141 = _1128 + _1140;
  uint16_t _1142 = _1127 + _1141;
  uint16_t _1143 = _blur_unnormalized_stencil_9 + _1142;
  uint16_t _1144 = _1125 + _1143;
  return _1144;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_25(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_10 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_82 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_83 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_84 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_85 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_86 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_87 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_88 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_89 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_90 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1203 = (uint16_t)(24);
  uint16_t _1204 = _hw_input_global_wrapper_global_wrapper_stencil_82 * _1203;
  uint16_t _1205 = (uint16_t)(30);
  uint16_t _1206 = _hw_input_global_wrapper_global_wrapper_stencil_83 * _1205;
  uint16_t _1207 = _hw_input_global_wrapper_global_wrapper_stencil_84 * _1203;
  uint16_t _1208 = _hw_input_global_wrapper_global_wrapper_stencil_85 * _1205;
  uint16_t _1209 = (uint16_t)(37);
  uint16_t _1210 = _hw_input_global_wrapper_global_wrapper_stencil_86 * _1209;
  uint16_t _1211 = _hw_input_global_wrapper_global_wrapper_stencil_87 * _1205;
  uint16_t _1212 = _hw_input_global_wrapper_global_wrapper_stencil_88 * _1203;
  uint16_t _1213 = _hw_input_global_wrapper_global_wrapper_stencil_89 * _1203;
  uint16_t _1214 = _hw_input_global_wrapper_global_wrapper_stencil_90 * _1205;
  uint16_t _1215 = _1213 + _1214;
  uint16_t _1216 = _1212 + _1215;
  uint16_t _1217 = _1211 + _1216;
  uint16_t _1218 = _1210 + _1217;
  uint16_t _1219 = _1208 + _1218;
  uint16_t _1220 = _1207 + _1219;
  uint16_t _1221 = _1206 + _1220;
  uint16_t _1222 = _blur_unnormalized_stencil_10 + _1221;
  uint16_t _1223 = _1204 + _1222;
  return _1223;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_26(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_11 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_91 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_92 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_93 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_94 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_95 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_96 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_97 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_98 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_99 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1282 = (uint16_t)(24);
  uint16_t _1283 = _hw_input_global_wrapper_global_wrapper_stencil_91 * _1282;
  uint16_t _1284 = (uint16_t)(30);
  uint16_t _1285 = _hw_input_global_wrapper_global_wrapper_stencil_92 * _1284;
  uint16_t _1286 = _hw_input_global_wrapper_global_wrapper_stencil_93 * _1282;
  uint16_t _1287 = _hw_input_global_wrapper_global_wrapper_stencil_94 * _1284;
  uint16_t _1288 = (uint16_t)(37);
  uint16_t _1289 = _hw_input_global_wrapper_global_wrapper_stencil_95 * _1288;
  uint16_t _1290 = _hw_input_global_wrapper_global_wrapper_stencil_96 * _1284;
  uint16_t _1291 = _hw_input_global_wrapper_global_wrapper_stencil_97 * _1282;
  uint16_t _1292 = _hw_input_global_wrapper_global_wrapper_stencil_98 * _1282;
  uint16_t _1293 = _hw_input_global_wrapper_global_wrapper_stencil_99 * _1284;
  uint16_t _1294 = _1292 + _1293;
  uint16_t _1295 = _1291 + _1294;
  uint16_t _1296 = _1290 + _1295;
  uint16_t _1297 = _1289 + _1296;
  uint16_t _1298 = _1287 + _1297;
  uint16_t _1299 = _1286 + _1298;
  uint16_t _1300 = _1285 + _1299;
  uint16_t _1301 = _blur_unnormalized_stencil_11 + _1300;
  uint16_t _1302 = _1283 + _1301;
  return _1302;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_27(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_12 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_100 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_101 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_102 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_103 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_104 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_105 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_106 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_107 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_108 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1361 = (uint16_t)(24);
  uint16_t _1362 = _hw_input_global_wrapper_global_wrapper_stencil_100 * _1361;
  uint16_t _1363 = (uint16_t)(30);
  uint16_t _1364 = _hw_input_global_wrapper_global_wrapper_stencil_101 * _1363;
  uint16_t _1365 = _hw_input_global_wrapper_global_wrapper_stencil_102 * _1361;
  uint16_t _1366 = _hw_input_global_wrapper_global_wrapper_stencil_103 * _1363;
  uint16_t _1367 = (uint16_t)(37);
  uint16_t _1368 = _hw_input_global_wrapper_global_wrapper_stencil_104 * _1367;
  uint16_t _1369 = _hw_input_global_wrapper_global_wrapper_stencil_105 * _1363;
  uint16_t _1370 = _hw_input_global_wrapper_global_wrapper_stencil_106 * _1361;
  uint16_t _1371 = _hw_input_global_wrapper_global_wrapper_stencil_107 * _1361;
  uint16_t _1372 = _hw_input_global_wrapper_global_wrapper_stencil_108 * _1363;
  uint16_t _1373 = _1371 + _1372;
  uint16_t _1374 = _1370 + _1373;
  uint16_t _1375 = _1369 + _1374;
  uint16_t _1376 = _1368 + _1375;
  uint16_t _1377 = _1366 + _1376;
  uint16_t _1378 = _1365 + _1377;
  uint16_t _1379 = _1364 + _1378;
  uint16_t _1380 = _blur_unnormalized_stencil_12 + _1379;
  uint16_t _1381 = _1362 + _1380;
  return _1381;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_28(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_13 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_109 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_110 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_111 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_112 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_113 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_114 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_115 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_116 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_117 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1440 = (uint16_t)(24);
  uint16_t _1441 = _hw_input_global_wrapper_global_wrapper_stencil_109 * _1440;
  uint16_t _1442 = (uint16_t)(30);
  uint16_t _1443 = _hw_input_global_wrapper_global_wrapper_stencil_110 * _1442;
  uint16_t _1444 = _hw_input_global_wrapper_global_wrapper_stencil_111 * _1440;
  uint16_t _1445 = _hw_input_global_wrapper_global_wrapper_stencil_112 * _1442;
  uint16_t _1446 = (uint16_t)(37);
  uint16_t _1447 = _hw_input_global_wrapper_global_wrapper_stencil_113 * _1446;
  uint16_t _1448 = _hw_input_global_wrapper_global_wrapper_stencil_114 * _1442;
  uint16_t _1449 = _hw_input_global_wrapper_global_wrapper_stencil_115 * _1440;
  uint16_t _1450 = _hw_input_global_wrapper_global_wrapper_stencil_116 * _1440;
  uint16_t _1451 = _hw_input_global_wrapper_global_wrapper_stencil_117 * _1442;
  uint16_t _1452 = _1450 + _1451;
  uint16_t _1453 = _1449 + _1452;
  uint16_t _1454 = _1448 + _1453;
  uint16_t _1455 = _1447 + _1454;
  uint16_t _1456 = _1445 + _1455;
  uint16_t _1457 = _1444 + _1456;
  uint16_t _1458 = _1443 + _1457;
  uint16_t _1459 = _blur_unnormalized_stencil_13 + _1458;
  uint16_t _1460 = _1441 + _1459;
  return _1460;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_29(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_14 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_118 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_119 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_120 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_121 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_122 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_123 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_124 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_125 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_126 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1519 = (uint16_t)(24);
  uint16_t _1520 = _hw_input_global_wrapper_global_wrapper_stencil_118 * _1519;
  uint16_t _1521 = (uint16_t)(30);
  uint16_t _1522 = _hw_input_global_wrapper_global_wrapper_stencil_119 * _1521;
  uint16_t _1523 = _hw_input_global_wrapper_global_wrapper_stencil_120 * _1519;
  uint16_t _1524 = _hw_input_global_wrapper_global_wrapper_stencil_121 * _1521;
  uint16_t _1525 = (uint16_t)(37);
  uint16_t _1526 = _hw_input_global_wrapper_global_wrapper_stencil_122 * _1525;
  uint16_t _1527 = _hw_input_global_wrapper_global_wrapper_stencil_123 * _1521;
  uint16_t _1528 = _hw_input_global_wrapper_global_wrapper_stencil_124 * _1519;
  uint16_t _1529 = _hw_input_global_wrapper_global_wrapper_stencil_125 * _1519;
  uint16_t _1530 = _hw_input_global_wrapper_global_wrapper_stencil_126 * _1521;
  uint16_t _1531 = _1529 + _1530;
  uint16_t _1532 = _1528 + _1531;
  uint16_t _1533 = _1527 + _1532;
  uint16_t _1534 = _1526 + _1533;
  uint16_t _1535 = _1524 + _1534;
  uint16_t _1536 = _1523 + _1535;
  uint16_t _1537 = _1522 + _1536;
  uint16_t _1538 = _blur_unnormalized_stencil_14 + _1537;
  uint16_t _1539 = _1520 + _1538;
  return _1539;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_30(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_15 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_127 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_128 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_129 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_130 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_131 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_132 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_133 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_134 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_135 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1598 = (uint16_t)(24);
  uint16_t _1599 = _hw_input_global_wrapper_global_wrapper_stencil_127 * _1598;
  uint16_t _1600 = (uint16_t)(30);
  uint16_t _1601 = _hw_input_global_wrapper_global_wrapper_stencil_128 * _1600;
  uint16_t _1602 = _hw_input_global_wrapper_global_wrapper_stencil_129 * _1598;
  uint16_t _1603 = _hw_input_global_wrapper_global_wrapper_stencil_130 * _1600;
  uint16_t _1604 = (uint16_t)(37);
  uint16_t _1605 = _hw_input_global_wrapper_global_wrapper_stencil_131 * _1604;
  uint16_t _1606 = _hw_input_global_wrapper_global_wrapper_stencil_132 * _1600;
  uint16_t _1607 = _hw_input_global_wrapper_global_wrapper_stencil_133 * _1598;
  uint16_t _1608 = _hw_input_global_wrapper_global_wrapper_stencil_134 * _1598;
  uint16_t _1609 = _hw_input_global_wrapper_global_wrapper_stencil_135 * _1600;
  uint16_t _1610 = _1608 + _1609;
  uint16_t _1611 = _1607 + _1610;
  uint16_t _1612 = _1606 + _1611;
  uint16_t _1613 = _1605 + _1612;
  uint16_t _1614 = _1603 + _1613;
  uint16_t _1615 = _1602 + _1614;
  uint16_t _1616 = _1601 + _1615;
  uint16_t _1617 = _blur_unnormalized_stencil_15 + _1616;
  uint16_t _1618 = _1599 + _1617;
  return _1618;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 17), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 17), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 17), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_31(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_16 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_136 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_137 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_138 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_139 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_140 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_141 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_142 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_143 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_144 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1677 = (uint16_t)(24);
  uint16_t _1678 = _hw_input_global_wrapper_global_wrapper_stencil_136 * _1677;
  uint16_t _1679 = (uint16_t)(30);
  uint16_t _1680 = _hw_input_global_wrapper_global_wrapper_stencil_137 * _1679;
  uint16_t _1681 = _hw_input_global_wrapper_global_wrapper_stencil_138 * _1677;
  uint16_t _1682 = _hw_input_global_wrapper_global_wrapper_stencil_139 * _1679;
  uint16_t _1683 = (uint16_t)(37);
  uint16_t _1684 = _hw_input_global_wrapper_global_wrapper_stencil_140 * _1683;
  uint16_t _1685 = _hw_input_global_wrapper_global_wrapper_stencil_141 * _1679;
  uint16_t _1686 = _hw_input_global_wrapper_global_wrapper_stencil_142 * _1677;
  uint16_t _1687 = _hw_input_global_wrapper_global_wrapper_stencil_143 * _1677;
  uint16_t _1688 = _hw_input_global_wrapper_global_wrapper_stencil_144 * _1679;
  uint16_t _1689 = _1687 + _1688;
  uint16_t _1690 = _1686 + _1689;
  uint16_t _1691 = _1685 + _1690;
  uint16_t _1692 = _1684 + _1691;
  uint16_t _1693 = _1682 + _1692;
  uint16_t _1694 = _1681 + _1693;
  uint16_t _1695 = _1680 + _1694;
  uint16_t _1696 = _blur_unnormalized_stencil_16 + _1695;
  uint16_t _1697 = _1678 + _1696;
  return _1697;
}

//store is: blur.stencil((blur_s0_x_x*16), blur_s0_y) = (blur_unnormalized.stencil((blur_s0_x_x*16), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_17 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1756 = (uint16_t)(8);
  uint16_t _1757 = _blur_unnormalized_stencil_17 >> _1756;
  return _1757;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 1), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 1), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_1(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_18 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1764 = (uint16_t)(8);
  uint16_t _1765 = _blur_unnormalized_stencil_18 >> _1764;
  return _1765;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 2), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 2), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_2(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_19 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1773 = (uint16_t)(8);
  uint16_t _1774 = _blur_unnormalized_stencil_19 >> _1773;
  return _1774;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 3), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 3), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_3(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_20 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1782 = (uint16_t)(8);
  uint16_t _1783 = _blur_unnormalized_stencil_20 >> _1782;
  return _1783;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 4), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 4), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_4(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_21 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1791 = (uint16_t)(8);
  uint16_t _1792 = _blur_unnormalized_stencil_21 >> _1791;
  return _1792;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 5), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 5), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_5(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_22 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1800 = (uint16_t)(8);
  uint16_t _1801 = _blur_unnormalized_stencil_22 >> _1800;
  return _1801;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 6), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 6), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_6(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_23 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1809 = (uint16_t)(8);
  uint16_t _1810 = _blur_unnormalized_stencil_23 >> _1809;
  return _1810;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 7), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 7), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_7(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_24 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1818 = (uint16_t)(8);
  uint16_t _1819 = _blur_unnormalized_stencil_24 >> _1818;
  return _1819;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 8), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 8), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_8(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_25 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1827 = (uint16_t)(8);
  uint16_t _1828 = _blur_unnormalized_stencil_25 >> _1827;
  return _1828;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 9), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 9), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_9(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_26 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1836 = (uint16_t)(8);
  uint16_t _1837 = _blur_unnormalized_stencil_26 >> _1836;
  return _1837;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 10), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 10), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_10(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_27 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1845 = (uint16_t)(8);
  uint16_t _1846 = _blur_unnormalized_stencil_27 >> _1845;
  return _1846;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 11), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 11), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_11(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_28 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1854 = (uint16_t)(8);
  uint16_t _1855 = _blur_unnormalized_stencil_28 >> _1854;
  return _1855;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 12), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 12), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_12(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_29 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1863 = (uint16_t)(8);
  uint16_t _1864 = _blur_unnormalized_stencil_29 >> _1863;
  return _1864;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 13), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 13), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_13(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_30 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1872 = (uint16_t)(8);
  uint16_t _1873 = _blur_unnormalized_stencil_30 >> _1872;
  return _1873;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 14), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 14), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_14(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_31 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1881 = (uint16_t)(8);
  uint16_t _1882 = _blur_unnormalized_stencil_31 >> _1881;
  return _1882;
}

//store is: blur.stencil(((blur_s0_x_x*16) + 15), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 15), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_15(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_32 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _1890 = (uint16_t)(8);
  uint16_t _1891 = _blur_unnormalized_stencil_32 >> _1890;
  return _1891;
}

//store is: hw_output.stencil((hw_output_s0_x_xii_xii*16), hw_output_s0_y_yii) = blur.stencil((hw_output_s0_x_xii_xii*16), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_1 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_1;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 1), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 1), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_1(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_2 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_2;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 2), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 2), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_2(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_3 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_3;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 3), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 3), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_3(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_4 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_4;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 4), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 4), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_4(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_5 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_5;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 5), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 5), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_5(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_6 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_6;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 6), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 6), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_6(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_7 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_7;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 7), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 7), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_7(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_8 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_8;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 8), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 8), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_8(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_9 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_9;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 9), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 9), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_9(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_10 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_10;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 10), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 10), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_10(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_11 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_11;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 11), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 11), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_11(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_12 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_12;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 12), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 12), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_12(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_13 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_13;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 13), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 13), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_13(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_14 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_14;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 14), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 14), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_14(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_15 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_15;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 15), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 15), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_15(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_16 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_16;
}

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil((hw_output_global_wrapper_s0_x_xi_xi*16), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_1 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_1;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 1), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 1), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_1(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_2 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_2;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 2), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 2), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_2(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_3 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_3;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 3), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 3), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_3(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_4 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_4;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 4), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 4), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_4(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_5 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_5;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 5), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 5), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_5(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_6 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_6;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 6), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 6), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_6(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_7 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_7;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 7), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 7), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_7(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_8 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_8;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 8), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 8), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_8(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_9 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_9;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 9), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 9), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_9(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_10 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_10;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 10), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 10), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_10(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_11 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_11;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 11), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 11), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_11(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_12 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_12;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 12), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 12), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_12(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_13 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_13;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 13), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 13), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_13(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_14 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_14;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 14), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 14), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_14(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_15 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_15;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*18))*16) + 15), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 15), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_15(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_16 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_16;
}

