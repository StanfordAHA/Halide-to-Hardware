#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_s0_x_x*32), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x + (0*9))*32), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_1;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 1), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_1(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_2 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_2;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 2), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_2(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_3 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_3;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 3), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_3(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_4 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_4;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 4), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_4(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_5 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_5;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 5), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_5(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_6 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_6;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 6), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_6(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_7 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_7;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 7), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_7(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_8 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_8;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 8), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_8(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_9 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_9;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 9), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_9(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_10 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_10;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 10), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_10(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_11 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_11;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 11), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_11(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_12 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_12;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 12), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_12(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_13 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_13;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 13), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_13(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_14 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_14;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 14), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_14(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_15 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_15;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 15), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_15(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_16 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_16;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 16), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_16(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_17 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_17;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 17), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_17(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_18 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_18;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 18), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_18(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_19 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_19;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 19), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_19(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_20 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_20;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 20), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_20(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_21 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_21;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 21), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_21(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_22 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_22;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 22), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_22(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_23 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_23;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 23), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_23(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_24 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_24;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 24), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_24(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_25 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_25;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 25), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_25(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_26 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_26;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 26), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_26(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_27 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_27;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 27), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_27(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_28 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_28;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 28), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_28(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_29 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_29;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 29), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_29(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_30 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_30;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 30), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_30(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_31 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_31;
}

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_s0_y) = hw_input.stencil((((hw_input_global_wrapper_s0_x_x + (0*9))*32) + 31), hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_glb_stencil_31(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_32 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_32;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*32), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*32), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_1 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_1;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_1(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_2 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_2;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_2(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_3 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_3;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_3(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_4 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_4;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_4(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_5 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_5;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_5(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_6 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_6;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_6(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_7 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_7;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_7(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_8 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_8;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_8(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_9 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_9;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_9(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_10 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_10;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_10(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_11 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_11;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_11(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_12 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_12;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_12(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_13 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_13;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_13(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_14 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_14;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_14(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_15 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_15;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_15(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_16 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_16;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_16(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_17 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_17;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_17(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_18 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_18;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_18(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_19 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_19;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_19(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_20 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_20;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_20(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_21 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_21;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_21(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_22 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_22;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_22(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_23 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_23;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_23(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_24 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_24;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_24(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_25 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_25;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_25(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_26 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_26;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_26(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_27 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_27;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_27(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_28 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_28;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_28(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_29 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_29;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_29(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_30 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_30;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_30(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_31 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_31;
}

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_global_wrapper_stencil_31(hw_uint<16>& hw_input_global_wrapper_glb_stencil) {
  uint16_t _hw_input_global_wrapper_glb_stencil_32 = (uint16_t) (hw_input_global_wrapper_glb_stencil.extract<0, 15>());

  return _hw_input_global_wrapper_glb_stencil_32;
}

//store is: blur_unnormalized.stencil((blur_unnormalized_s0_x_x*32), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil() {
  uint16_t _574 = (uint16_t)(0);
  return _574;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 1), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_1() {
  uint16_t _578 = (uint16_t)(0);
  return _578;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 2), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_2() {
  uint16_t _583 = (uint16_t)(0);
  return _583;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 3), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_3() {
  uint16_t _588 = (uint16_t)(0);
  return _588;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 4), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_4() {
  uint16_t _593 = (uint16_t)(0);
  return _593;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 5), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_5() {
  uint16_t _598 = (uint16_t)(0);
  return _598;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 6), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_6() {
  uint16_t _603 = (uint16_t)(0);
  return _603;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 7), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_7() {
  uint16_t _608 = (uint16_t)(0);
  return _608;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 8), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_8() {
  uint16_t _613 = (uint16_t)(0);
  return _613;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 9), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_9() {
  uint16_t _618 = (uint16_t)(0);
  return _618;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 10), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_10() {
  uint16_t _623 = (uint16_t)(0);
  return _623;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 11), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_11() {
  uint16_t _628 = (uint16_t)(0);
  return _628;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 12), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_12() {
  uint16_t _633 = (uint16_t)(0);
  return _633;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 13), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_13() {
  uint16_t _638 = (uint16_t)(0);
  return _638;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 14), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_14() {
  uint16_t _643 = (uint16_t)(0);
  return _643;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 15), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_15() {
  uint16_t _648 = (uint16_t)(0);
  return _648;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 16), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_16() {
  uint16_t _653 = (uint16_t)(0);
  return _653;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 17), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_17() {
  uint16_t _658 = (uint16_t)(0);
  return _658;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 18), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_18() {
  uint16_t _663 = (uint16_t)(0);
  return _663;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 19), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_19() {
  uint16_t _668 = (uint16_t)(0);
  return _668;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 20), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_20() {
  uint16_t _673 = (uint16_t)(0);
  return _673;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 21), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_21() {
  uint16_t _678 = (uint16_t)(0);
  return _678;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 22), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_22() {
  uint16_t _683 = (uint16_t)(0);
  return _683;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 23), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_23() {
  uint16_t _688 = (uint16_t)(0);
  return _688;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 24), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_24() {
  uint16_t _693 = (uint16_t)(0);
  return _693;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 25), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_25() {
  uint16_t _698 = (uint16_t)(0);
  return _698;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 26), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_26() {
  uint16_t _703 = (uint16_t)(0);
  return _703;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 27), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_27() {
  uint16_t _708 = (uint16_t)(0);
  return _708;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 28), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_28() {
  uint16_t _713 = (uint16_t)(0);
  return _713;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 29), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_29() {
  uint16_t _718 = (uint16_t)(0);
  return _718;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 30), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_30() {
  uint16_t _723 = (uint16_t)(0);
  return _723;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 31), blur_unnormalized_s0_y) = (uint16)0
hw_uint<16> hcompute_blur_unnormalized_stencil_31() {
  uint16_t _728 = (uint16_t)(0);
  return _728;
}

//store is: blur_unnormalized.stencil((blur_unnormalized_s1_x_x*32), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*32), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil((blur_unnormalized_s1_x_x*32), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*32), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*32), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_32(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _733 = (uint16_t)(24);
  uint16_t _734 = _hw_input_global_wrapper_global_wrapper_stencil_1 * _733;
  uint16_t _735 = (uint16_t)(30);
  uint16_t _736 = _hw_input_global_wrapper_global_wrapper_stencil_2 * _735;
  uint16_t _737 = _hw_input_global_wrapper_global_wrapper_stencil_3 * _733;
  uint16_t _738 = _hw_input_global_wrapper_global_wrapper_stencil_4 * _735;
  uint16_t _739 = (uint16_t)(37);
  uint16_t _740 = _hw_input_global_wrapper_global_wrapper_stencil_5 * _739;
  uint16_t _741 = _hw_input_global_wrapper_global_wrapper_stencil_6 * _735;
  uint16_t _742 = _hw_input_global_wrapper_global_wrapper_stencil_7 * _733;
  uint16_t _743 = _hw_input_global_wrapper_global_wrapper_stencil_8 * _733;
  uint16_t _744 = _hw_input_global_wrapper_global_wrapper_stencil_9 * _735;
  uint16_t _745 = _743 + _744;
  uint16_t _746 = _742 + _745;
  uint16_t _747 = _741 + _746;
  uint16_t _748 = _740 + _747;
  uint16_t _749 = _738 + _748;
  uint16_t _750 = _737 + _749;
  uint16_t _751 = _736 + _750;
  uint16_t _752 = _blur_unnormalized_stencil_1 + _751;
  uint16_t _753 = _734 + _752;
  return _753;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_33(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _811 = (uint16_t)(24);
  uint16_t _812 = _hw_input_global_wrapper_global_wrapper_stencil_10 * _811;
  uint16_t _813 = (uint16_t)(30);
  uint16_t _814 = _hw_input_global_wrapper_global_wrapper_stencil_11 * _813;
  uint16_t _815 = _hw_input_global_wrapper_global_wrapper_stencil_12 * _811;
  uint16_t _816 = _hw_input_global_wrapper_global_wrapper_stencil_13 * _813;
  uint16_t _817 = (uint16_t)(37);
  uint16_t _818 = _hw_input_global_wrapper_global_wrapper_stencil_14 * _817;
  uint16_t _819 = _hw_input_global_wrapper_global_wrapper_stencil_15 * _813;
  uint16_t _820 = _hw_input_global_wrapper_global_wrapper_stencil_16 * _811;
  uint16_t _821 = _hw_input_global_wrapper_global_wrapper_stencil_17 * _811;
  uint16_t _822 = _hw_input_global_wrapper_global_wrapper_stencil_18 * _813;
  uint16_t _823 = _821 + _822;
  uint16_t _824 = _820 + _823;
  uint16_t _825 = _819 + _824;
  uint16_t _826 = _818 + _825;
  uint16_t _827 = _816 + _826;
  uint16_t _828 = _815 + _827;
  uint16_t _829 = _814 + _828;
  uint16_t _830 = _blur_unnormalized_stencil_2 + _829;
  uint16_t _831 = _812 + _830;
  return _831;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_34(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _890 = (uint16_t)(24);
  uint16_t _891 = _hw_input_global_wrapper_global_wrapper_stencil_19 * _890;
  uint16_t _892 = (uint16_t)(30);
  uint16_t _893 = _hw_input_global_wrapper_global_wrapper_stencil_20 * _892;
  uint16_t _894 = _hw_input_global_wrapper_global_wrapper_stencil_21 * _890;
  uint16_t _895 = _hw_input_global_wrapper_global_wrapper_stencil_22 * _892;
  uint16_t _896 = (uint16_t)(37);
  uint16_t _897 = _hw_input_global_wrapper_global_wrapper_stencil_23 * _896;
  uint16_t _898 = _hw_input_global_wrapper_global_wrapper_stencil_24 * _892;
  uint16_t _899 = _hw_input_global_wrapper_global_wrapper_stencil_25 * _890;
  uint16_t _900 = _hw_input_global_wrapper_global_wrapper_stencil_26 * _890;
  uint16_t _901 = _hw_input_global_wrapper_global_wrapper_stencil_27 * _892;
  uint16_t _902 = _900 + _901;
  uint16_t _903 = _899 + _902;
  uint16_t _904 = _898 + _903;
  uint16_t _905 = _897 + _904;
  uint16_t _906 = _895 + _905;
  uint16_t _907 = _894 + _906;
  uint16_t _908 = _893 + _907;
  uint16_t _909 = _blur_unnormalized_stencil_3 + _908;
  uint16_t _910 = _891 + _909;
  return _910;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_35(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _969 = (uint16_t)(24);
  uint16_t _970 = _hw_input_global_wrapper_global_wrapper_stencil_28 * _969;
  uint16_t _971 = (uint16_t)(30);
  uint16_t _972 = _hw_input_global_wrapper_global_wrapper_stencil_29 * _971;
  uint16_t _973 = _hw_input_global_wrapper_global_wrapper_stencil_30 * _969;
  uint16_t _974 = _hw_input_global_wrapper_global_wrapper_stencil_31 * _971;
  uint16_t _975 = (uint16_t)(37);
  uint16_t _976 = _hw_input_global_wrapper_global_wrapper_stencil_32 * _975;
  uint16_t _977 = _hw_input_global_wrapper_global_wrapper_stencil_33 * _971;
  uint16_t _978 = _hw_input_global_wrapper_global_wrapper_stencil_34 * _969;
  uint16_t _979 = _hw_input_global_wrapper_global_wrapper_stencil_35 * _969;
  uint16_t _980 = _hw_input_global_wrapper_global_wrapper_stencil_36 * _971;
  uint16_t _981 = _979 + _980;
  uint16_t _982 = _978 + _981;
  uint16_t _983 = _977 + _982;
  uint16_t _984 = _976 + _983;
  uint16_t _985 = _974 + _984;
  uint16_t _986 = _973 + _985;
  uint16_t _987 = _972 + _986;
  uint16_t _988 = _blur_unnormalized_stencil_4 + _987;
  uint16_t _989 = _970 + _988;
  return _989;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_36(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1048 = (uint16_t)(24);
  uint16_t _1049 = _hw_input_global_wrapper_global_wrapper_stencil_37 * _1048;
  uint16_t _1050 = (uint16_t)(30);
  uint16_t _1051 = _hw_input_global_wrapper_global_wrapper_stencil_38 * _1050;
  uint16_t _1052 = _hw_input_global_wrapper_global_wrapper_stencil_39 * _1048;
  uint16_t _1053 = _hw_input_global_wrapper_global_wrapper_stencil_40 * _1050;
  uint16_t _1054 = (uint16_t)(37);
  uint16_t _1055 = _hw_input_global_wrapper_global_wrapper_stencil_41 * _1054;
  uint16_t _1056 = _hw_input_global_wrapper_global_wrapper_stencil_42 * _1050;
  uint16_t _1057 = _hw_input_global_wrapper_global_wrapper_stencil_43 * _1048;
  uint16_t _1058 = _hw_input_global_wrapper_global_wrapper_stencil_44 * _1048;
  uint16_t _1059 = _hw_input_global_wrapper_global_wrapper_stencil_45 * _1050;
  uint16_t _1060 = _1058 + _1059;
  uint16_t _1061 = _1057 + _1060;
  uint16_t _1062 = _1056 + _1061;
  uint16_t _1063 = _1055 + _1062;
  uint16_t _1064 = _1053 + _1063;
  uint16_t _1065 = _1052 + _1064;
  uint16_t _1066 = _1051 + _1065;
  uint16_t _1067 = _blur_unnormalized_stencil_5 + _1066;
  uint16_t _1068 = _1049 + _1067;
  return _1068;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_37(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1127 = (uint16_t)(24);
  uint16_t _1128 = _hw_input_global_wrapper_global_wrapper_stencil_46 * _1127;
  uint16_t _1129 = (uint16_t)(30);
  uint16_t _1130 = _hw_input_global_wrapper_global_wrapper_stencil_47 * _1129;
  uint16_t _1131 = _hw_input_global_wrapper_global_wrapper_stencil_48 * _1127;
  uint16_t _1132 = _hw_input_global_wrapper_global_wrapper_stencil_49 * _1129;
  uint16_t _1133 = (uint16_t)(37);
  uint16_t _1134 = _hw_input_global_wrapper_global_wrapper_stencil_50 * _1133;
  uint16_t _1135 = _hw_input_global_wrapper_global_wrapper_stencil_51 * _1129;
  uint16_t _1136 = _hw_input_global_wrapper_global_wrapper_stencil_52 * _1127;
  uint16_t _1137 = _hw_input_global_wrapper_global_wrapper_stencil_53 * _1127;
  uint16_t _1138 = _hw_input_global_wrapper_global_wrapper_stencil_54 * _1129;
  uint16_t _1139 = _1137 + _1138;
  uint16_t _1140 = _1136 + _1139;
  uint16_t _1141 = _1135 + _1140;
  uint16_t _1142 = _1134 + _1141;
  uint16_t _1143 = _1132 + _1142;
  uint16_t _1144 = _1131 + _1143;
  uint16_t _1145 = _1130 + _1144;
  uint16_t _1146 = _blur_unnormalized_stencil_6 + _1145;
  uint16_t _1147 = _1128 + _1146;
  return _1147;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_38(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1206 = (uint16_t)(24);
  uint16_t _1207 = _hw_input_global_wrapper_global_wrapper_stencil_55 * _1206;
  uint16_t _1208 = (uint16_t)(30);
  uint16_t _1209 = _hw_input_global_wrapper_global_wrapper_stencil_56 * _1208;
  uint16_t _1210 = _hw_input_global_wrapper_global_wrapper_stencil_57 * _1206;
  uint16_t _1211 = _hw_input_global_wrapper_global_wrapper_stencil_58 * _1208;
  uint16_t _1212 = (uint16_t)(37);
  uint16_t _1213 = _hw_input_global_wrapper_global_wrapper_stencil_59 * _1212;
  uint16_t _1214 = _hw_input_global_wrapper_global_wrapper_stencil_60 * _1208;
  uint16_t _1215 = _hw_input_global_wrapper_global_wrapper_stencil_61 * _1206;
  uint16_t _1216 = _hw_input_global_wrapper_global_wrapper_stencil_62 * _1206;
  uint16_t _1217 = _hw_input_global_wrapper_global_wrapper_stencil_63 * _1208;
  uint16_t _1218 = _1216 + _1217;
  uint16_t _1219 = _1215 + _1218;
  uint16_t _1220 = _1214 + _1219;
  uint16_t _1221 = _1213 + _1220;
  uint16_t _1222 = _1211 + _1221;
  uint16_t _1223 = _1210 + _1222;
  uint16_t _1224 = _1209 + _1223;
  uint16_t _1225 = _blur_unnormalized_stencil_7 + _1224;
  uint16_t _1226 = _1207 + _1225;
  return _1226;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_39(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1285 = (uint16_t)(24);
  uint16_t _1286 = _hw_input_global_wrapper_global_wrapper_stencil_64 * _1285;
  uint16_t _1287 = (uint16_t)(30);
  uint16_t _1288 = _hw_input_global_wrapper_global_wrapper_stencil_65 * _1287;
  uint16_t _1289 = _hw_input_global_wrapper_global_wrapper_stencil_66 * _1285;
  uint16_t _1290 = _hw_input_global_wrapper_global_wrapper_stencil_67 * _1287;
  uint16_t _1291 = (uint16_t)(37);
  uint16_t _1292 = _hw_input_global_wrapper_global_wrapper_stencil_68 * _1291;
  uint16_t _1293 = _hw_input_global_wrapper_global_wrapper_stencil_69 * _1287;
  uint16_t _1294 = _hw_input_global_wrapper_global_wrapper_stencil_70 * _1285;
  uint16_t _1295 = _hw_input_global_wrapper_global_wrapper_stencil_71 * _1285;
  uint16_t _1296 = _hw_input_global_wrapper_global_wrapper_stencil_72 * _1287;
  uint16_t _1297 = _1295 + _1296;
  uint16_t _1298 = _1294 + _1297;
  uint16_t _1299 = _1293 + _1298;
  uint16_t _1300 = _1292 + _1299;
  uint16_t _1301 = _1290 + _1300;
  uint16_t _1302 = _1289 + _1301;
  uint16_t _1303 = _1288 + _1302;
  uint16_t _1304 = _blur_unnormalized_stencil_8 + _1303;
  uint16_t _1305 = _1286 + _1304;
  return _1305;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_40(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1364 = (uint16_t)(24);
  uint16_t _1365 = _hw_input_global_wrapper_global_wrapper_stencil_73 * _1364;
  uint16_t _1366 = (uint16_t)(30);
  uint16_t _1367 = _hw_input_global_wrapper_global_wrapper_stencil_74 * _1366;
  uint16_t _1368 = _hw_input_global_wrapper_global_wrapper_stencil_75 * _1364;
  uint16_t _1369 = _hw_input_global_wrapper_global_wrapper_stencil_76 * _1366;
  uint16_t _1370 = (uint16_t)(37);
  uint16_t _1371 = _hw_input_global_wrapper_global_wrapper_stencil_77 * _1370;
  uint16_t _1372 = _hw_input_global_wrapper_global_wrapper_stencil_78 * _1366;
  uint16_t _1373 = _hw_input_global_wrapper_global_wrapper_stencil_79 * _1364;
  uint16_t _1374 = _hw_input_global_wrapper_global_wrapper_stencil_80 * _1364;
  uint16_t _1375 = _hw_input_global_wrapper_global_wrapper_stencil_81 * _1366;
  uint16_t _1376 = _1374 + _1375;
  uint16_t _1377 = _1373 + _1376;
  uint16_t _1378 = _1372 + _1377;
  uint16_t _1379 = _1371 + _1378;
  uint16_t _1380 = _1369 + _1379;
  uint16_t _1381 = _1368 + _1380;
  uint16_t _1382 = _1367 + _1381;
  uint16_t _1383 = _blur_unnormalized_stencil_9 + _1382;
  uint16_t _1384 = _1365 + _1383;
  return _1384;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_41(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1443 = (uint16_t)(24);
  uint16_t _1444 = _hw_input_global_wrapper_global_wrapper_stencil_82 * _1443;
  uint16_t _1445 = (uint16_t)(30);
  uint16_t _1446 = _hw_input_global_wrapper_global_wrapper_stencil_83 * _1445;
  uint16_t _1447 = _hw_input_global_wrapper_global_wrapper_stencil_84 * _1443;
  uint16_t _1448 = _hw_input_global_wrapper_global_wrapper_stencil_85 * _1445;
  uint16_t _1449 = (uint16_t)(37);
  uint16_t _1450 = _hw_input_global_wrapper_global_wrapper_stencil_86 * _1449;
  uint16_t _1451 = _hw_input_global_wrapper_global_wrapper_stencil_87 * _1445;
  uint16_t _1452 = _hw_input_global_wrapper_global_wrapper_stencil_88 * _1443;
  uint16_t _1453 = _hw_input_global_wrapper_global_wrapper_stencil_89 * _1443;
  uint16_t _1454 = _hw_input_global_wrapper_global_wrapper_stencil_90 * _1445;
  uint16_t _1455 = _1453 + _1454;
  uint16_t _1456 = _1452 + _1455;
  uint16_t _1457 = _1451 + _1456;
  uint16_t _1458 = _1450 + _1457;
  uint16_t _1459 = _1448 + _1458;
  uint16_t _1460 = _1447 + _1459;
  uint16_t _1461 = _1446 + _1460;
  uint16_t _1462 = _blur_unnormalized_stencil_10 + _1461;
  uint16_t _1463 = _1444 + _1462;
  return _1463;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_42(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1522 = (uint16_t)(24);
  uint16_t _1523 = _hw_input_global_wrapper_global_wrapper_stencil_91 * _1522;
  uint16_t _1524 = (uint16_t)(30);
  uint16_t _1525 = _hw_input_global_wrapper_global_wrapper_stencil_92 * _1524;
  uint16_t _1526 = _hw_input_global_wrapper_global_wrapper_stencil_93 * _1522;
  uint16_t _1527 = _hw_input_global_wrapper_global_wrapper_stencil_94 * _1524;
  uint16_t _1528 = (uint16_t)(37);
  uint16_t _1529 = _hw_input_global_wrapper_global_wrapper_stencil_95 * _1528;
  uint16_t _1530 = _hw_input_global_wrapper_global_wrapper_stencil_96 * _1524;
  uint16_t _1531 = _hw_input_global_wrapper_global_wrapper_stencil_97 * _1522;
  uint16_t _1532 = _hw_input_global_wrapper_global_wrapper_stencil_98 * _1522;
  uint16_t _1533 = _hw_input_global_wrapper_global_wrapper_stencil_99 * _1524;
  uint16_t _1534 = _1532 + _1533;
  uint16_t _1535 = _1531 + _1534;
  uint16_t _1536 = _1530 + _1535;
  uint16_t _1537 = _1529 + _1536;
  uint16_t _1538 = _1527 + _1537;
  uint16_t _1539 = _1526 + _1538;
  uint16_t _1540 = _1525 + _1539;
  uint16_t _1541 = _blur_unnormalized_stencil_11 + _1540;
  uint16_t _1542 = _1523 + _1541;
  return _1542;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_43(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1601 = (uint16_t)(24);
  uint16_t _1602 = _hw_input_global_wrapper_global_wrapper_stencil_100 * _1601;
  uint16_t _1603 = (uint16_t)(30);
  uint16_t _1604 = _hw_input_global_wrapper_global_wrapper_stencil_101 * _1603;
  uint16_t _1605 = _hw_input_global_wrapper_global_wrapper_stencil_102 * _1601;
  uint16_t _1606 = _hw_input_global_wrapper_global_wrapper_stencil_103 * _1603;
  uint16_t _1607 = (uint16_t)(37);
  uint16_t _1608 = _hw_input_global_wrapper_global_wrapper_stencil_104 * _1607;
  uint16_t _1609 = _hw_input_global_wrapper_global_wrapper_stencil_105 * _1603;
  uint16_t _1610 = _hw_input_global_wrapper_global_wrapper_stencil_106 * _1601;
  uint16_t _1611 = _hw_input_global_wrapper_global_wrapper_stencil_107 * _1601;
  uint16_t _1612 = _hw_input_global_wrapper_global_wrapper_stencil_108 * _1603;
  uint16_t _1613 = _1611 + _1612;
  uint16_t _1614 = _1610 + _1613;
  uint16_t _1615 = _1609 + _1614;
  uint16_t _1616 = _1608 + _1615;
  uint16_t _1617 = _1606 + _1616;
  uint16_t _1618 = _1605 + _1617;
  uint16_t _1619 = _1604 + _1618;
  uint16_t _1620 = _blur_unnormalized_stencil_12 + _1619;
  uint16_t _1621 = _1602 + _1620;
  return _1621;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_44(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1680 = (uint16_t)(24);
  uint16_t _1681 = _hw_input_global_wrapper_global_wrapper_stencil_109 * _1680;
  uint16_t _1682 = (uint16_t)(30);
  uint16_t _1683 = _hw_input_global_wrapper_global_wrapper_stencil_110 * _1682;
  uint16_t _1684 = _hw_input_global_wrapper_global_wrapper_stencil_111 * _1680;
  uint16_t _1685 = _hw_input_global_wrapper_global_wrapper_stencil_112 * _1682;
  uint16_t _1686 = (uint16_t)(37);
  uint16_t _1687 = _hw_input_global_wrapper_global_wrapper_stencil_113 * _1686;
  uint16_t _1688 = _hw_input_global_wrapper_global_wrapper_stencil_114 * _1682;
  uint16_t _1689 = _hw_input_global_wrapper_global_wrapper_stencil_115 * _1680;
  uint16_t _1690 = _hw_input_global_wrapper_global_wrapper_stencil_116 * _1680;
  uint16_t _1691 = _hw_input_global_wrapper_global_wrapper_stencil_117 * _1682;
  uint16_t _1692 = _1690 + _1691;
  uint16_t _1693 = _1689 + _1692;
  uint16_t _1694 = _1688 + _1693;
  uint16_t _1695 = _1687 + _1694;
  uint16_t _1696 = _1685 + _1695;
  uint16_t _1697 = _1684 + _1696;
  uint16_t _1698 = _1683 + _1697;
  uint16_t _1699 = _blur_unnormalized_stencil_13 + _1698;
  uint16_t _1700 = _1681 + _1699;
  return _1700;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_45(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1759 = (uint16_t)(24);
  uint16_t _1760 = _hw_input_global_wrapper_global_wrapper_stencil_118 * _1759;
  uint16_t _1761 = (uint16_t)(30);
  uint16_t _1762 = _hw_input_global_wrapper_global_wrapper_stencil_119 * _1761;
  uint16_t _1763 = _hw_input_global_wrapper_global_wrapper_stencil_120 * _1759;
  uint16_t _1764 = _hw_input_global_wrapper_global_wrapper_stencil_121 * _1761;
  uint16_t _1765 = (uint16_t)(37);
  uint16_t _1766 = _hw_input_global_wrapper_global_wrapper_stencil_122 * _1765;
  uint16_t _1767 = _hw_input_global_wrapper_global_wrapper_stencil_123 * _1761;
  uint16_t _1768 = _hw_input_global_wrapper_global_wrapper_stencil_124 * _1759;
  uint16_t _1769 = _hw_input_global_wrapper_global_wrapper_stencil_125 * _1759;
  uint16_t _1770 = _hw_input_global_wrapper_global_wrapper_stencil_126 * _1761;
  uint16_t _1771 = _1769 + _1770;
  uint16_t _1772 = _1768 + _1771;
  uint16_t _1773 = _1767 + _1772;
  uint16_t _1774 = _1766 + _1773;
  uint16_t _1775 = _1764 + _1774;
  uint16_t _1776 = _1763 + _1775;
  uint16_t _1777 = _1762 + _1776;
  uint16_t _1778 = _blur_unnormalized_stencil_14 + _1777;
  uint16_t _1779 = _1760 + _1778;
  return _1779;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_46(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1838 = (uint16_t)(24);
  uint16_t _1839 = _hw_input_global_wrapper_global_wrapper_stencil_127 * _1838;
  uint16_t _1840 = (uint16_t)(30);
  uint16_t _1841 = _hw_input_global_wrapper_global_wrapper_stencil_128 * _1840;
  uint16_t _1842 = _hw_input_global_wrapper_global_wrapper_stencil_129 * _1838;
  uint16_t _1843 = _hw_input_global_wrapper_global_wrapper_stencil_130 * _1840;
  uint16_t _1844 = (uint16_t)(37);
  uint16_t _1845 = _hw_input_global_wrapper_global_wrapper_stencil_131 * _1844;
  uint16_t _1846 = _hw_input_global_wrapper_global_wrapper_stencil_132 * _1840;
  uint16_t _1847 = _hw_input_global_wrapper_global_wrapper_stencil_133 * _1838;
  uint16_t _1848 = _hw_input_global_wrapper_global_wrapper_stencil_134 * _1838;
  uint16_t _1849 = _hw_input_global_wrapper_global_wrapper_stencil_135 * _1840;
  uint16_t _1850 = _1848 + _1849;
  uint16_t _1851 = _1847 + _1850;
  uint16_t _1852 = _1846 + _1851;
  uint16_t _1853 = _1845 + _1852;
  uint16_t _1854 = _1843 + _1853;
  uint16_t _1855 = _1842 + _1854;
  uint16_t _1856 = _1841 + _1855;
  uint16_t _1857 = _blur_unnormalized_stencil_15 + _1856;
  uint16_t _1858 = _1839 + _1857;
  return _1858;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_47(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
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

  uint16_t _1917 = (uint16_t)(24);
  uint16_t _1918 = _hw_input_global_wrapper_global_wrapper_stencil_136 * _1917;
  uint16_t _1919 = (uint16_t)(30);
  uint16_t _1920 = _hw_input_global_wrapper_global_wrapper_stencil_137 * _1919;
  uint16_t _1921 = _hw_input_global_wrapper_global_wrapper_stencil_138 * _1917;
  uint16_t _1922 = _hw_input_global_wrapper_global_wrapper_stencil_139 * _1919;
  uint16_t _1923 = (uint16_t)(37);
  uint16_t _1924 = _hw_input_global_wrapper_global_wrapper_stencil_140 * _1923;
  uint16_t _1925 = _hw_input_global_wrapper_global_wrapper_stencil_141 * _1919;
  uint16_t _1926 = _hw_input_global_wrapper_global_wrapper_stencil_142 * _1917;
  uint16_t _1927 = _hw_input_global_wrapper_global_wrapper_stencil_143 * _1917;
  uint16_t _1928 = _hw_input_global_wrapper_global_wrapper_stencil_144 * _1919;
  uint16_t _1929 = _1927 + _1928;
  uint16_t _1930 = _1926 + _1929;
  uint16_t _1931 = _1925 + _1930;
  uint16_t _1932 = _1924 + _1931;
  uint16_t _1933 = _1922 + _1932;
  uint16_t _1934 = _1921 + _1933;
  uint16_t _1935 = _1920 + _1934;
  uint16_t _1936 = _blur_unnormalized_stencil_16 + _1935;
  uint16_t _1937 = _1918 + _1936;
  return _1937;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_48(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_17 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_145 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_146 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_147 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_148 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_149 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_150 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_151 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_152 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_153 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _1996 = (uint16_t)(24);
  uint16_t _1997 = _hw_input_global_wrapper_global_wrapper_stencil_145 * _1996;
  uint16_t _1998 = (uint16_t)(30);
  uint16_t _1999 = _hw_input_global_wrapper_global_wrapper_stencil_146 * _1998;
  uint16_t _2000 = _hw_input_global_wrapper_global_wrapper_stencil_147 * _1996;
  uint16_t _2001 = _hw_input_global_wrapper_global_wrapper_stencil_148 * _1998;
  uint16_t _2002 = (uint16_t)(37);
  uint16_t _2003 = _hw_input_global_wrapper_global_wrapper_stencil_149 * _2002;
  uint16_t _2004 = _hw_input_global_wrapper_global_wrapper_stencil_150 * _1998;
  uint16_t _2005 = _hw_input_global_wrapper_global_wrapper_stencil_151 * _1996;
  uint16_t _2006 = _hw_input_global_wrapper_global_wrapper_stencil_152 * _1996;
  uint16_t _2007 = _hw_input_global_wrapper_global_wrapper_stencil_153 * _1998;
  uint16_t _2008 = _2006 + _2007;
  uint16_t _2009 = _2005 + _2008;
  uint16_t _2010 = _2004 + _2009;
  uint16_t _2011 = _2003 + _2010;
  uint16_t _2012 = _2001 + _2011;
  uint16_t _2013 = _2000 + _2012;
  uint16_t _2014 = _1999 + _2013;
  uint16_t _2015 = _blur_unnormalized_stencil_17 + _2014;
  uint16_t _2016 = _1997 + _2015;
  return _2016;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_49(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_18 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_154 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_155 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_156 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_157 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_158 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_159 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_160 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_161 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_162 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2075 = (uint16_t)(24);
  uint16_t _2076 = _hw_input_global_wrapper_global_wrapper_stencil_154 * _2075;
  uint16_t _2077 = (uint16_t)(30);
  uint16_t _2078 = _hw_input_global_wrapper_global_wrapper_stencil_155 * _2077;
  uint16_t _2079 = _hw_input_global_wrapper_global_wrapper_stencil_156 * _2075;
  uint16_t _2080 = _hw_input_global_wrapper_global_wrapper_stencil_157 * _2077;
  uint16_t _2081 = (uint16_t)(37);
  uint16_t _2082 = _hw_input_global_wrapper_global_wrapper_stencil_158 * _2081;
  uint16_t _2083 = _hw_input_global_wrapper_global_wrapper_stencil_159 * _2077;
  uint16_t _2084 = _hw_input_global_wrapper_global_wrapper_stencil_160 * _2075;
  uint16_t _2085 = _hw_input_global_wrapper_global_wrapper_stencil_161 * _2075;
  uint16_t _2086 = _hw_input_global_wrapper_global_wrapper_stencil_162 * _2077;
  uint16_t _2087 = _2085 + _2086;
  uint16_t _2088 = _2084 + _2087;
  uint16_t _2089 = _2083 + _2088;
  uint16_t _2090 = _2082 + _2089;
  uint16_t _2091 = _2080 + _2090;
  uint16_t _2092 = _2079 + _2091;
  uint16_t _2093 = _2078 + _2092;
  uint16_t _2094 = _blur_unnormalized_stencil_18 + _2093;
  uint16_t _2095 = _2076 + _2094;
  return _2095;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_50(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_19 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_163 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_164 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_165 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_166 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_167 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_168 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_169 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_170 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_171 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2154 = (uint16_t)(24);
  uint16_t _2155 = _hw_input_global_wrapper_global_wrapper_stencil_163 * _2154;
  uint16_t _2156 = (uint16_t)(30);
  uint16_t _2157 = _hw_input_global_wrapper_global_wrapper_stencil_164 * _2156;
  uint16_t _2158 = _hw_input_global_wrapper_global_wrapper_stencil_165 * _2154;
  uint16_t _2159 = _hw_input_global_wrapper_global_wrapper_stencil_166 * _2156;
  uint16_t _2160 = (uint16_t)(37);
  uint16_t _2161 = _hw_input_global_wrapper_global_wrapper_stencil_167 * _2160;
  uint16_t _2162 = _hw_input_global_wrapper_global_wrapper_stencil_168 * _2156;
  uint16_t _2163 = _hw_input_global_wrapper_global_wrapper_stencil_169 * _2154;
  uint16_t _2164 = _hw_input_global_wrapper_global_wrapper_stencil_170 * _2154;
  uint16_t _2165 = _hw_input_global_wrapper_global_wrapper_stencil_171 * _2156;
  uint16_t _2166 = _2164 + _2165;
  uint16_t _2167 = _2163 + _2166;
  uint16_t _2168 = _2162 + _2167;
  uint16_t _2169 = _2161 + _2168;
  uint16_t _2170 = _2159 + _2169;
  uint16_t _2171 = _2158 + _2170;
  uint16_t _2172 = _2157 + _2171;
  uint16_t _2173 = _blur_unnormalized_stencil_19 + _2172;
  uint16_t _2174 = _2155 + _2173;
  return _2174;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_51(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_20 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_172 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_173 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_174 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_175 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_176 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_177 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_178 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_179 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_180 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2233 = (uint16_t)(24);
  uint16_t _2234 = _hw_input_global_wrapper_global_wrapper_stencil_172 * _2233;
  uint16_t _2235 = (uint16_t)(30);
  uint16_t _2236 = _hw_input_global_wrapper_global_wrapper_stencil_173 * _2235;
  uint16_t _2237 = _hw_input_global_wrapper_global_wrapper_stencil_174 * _2233;
  uint16_t _2238 = _hw_input_global_wrapper_global_wrapper_stencil_175 * _2235;
  uint16_t _2239 = (uint16_t)(37);
  uint16_t _2240 = _hw_input_global_wrapper_global_wrapper_stencil_176 * _2239;
  uint16_t _2241 = _hw_input_global_wrapper_global_wrapper_stencil_177 * _2235;
  uint16_t _2242 = _hw_input_global_wrapper_global_wrapper_stencil_178 * _2233;
  uint16_t _2243 = _hw_input_global_wrapper_global_wrapper_stencil_179 * _2233;
  uint16_t _2244 = _hw_input_global_wrapper_global_wrapper_stencil_180 * _2235;
  uint16_t _2245 = _2243 + _2244;
  uint16_t _2246 = _2242 + _2245;
  uint16_t _2247 = _2241 + _2246;
  uint16_t _2248 = _2240 + _2247;
  uint16_t _2249 = _2238 + _2248;
  uint16_t _2250 = _2237 + _2249;
  uint16_t _2251 = _2236 + _2250;
  uint16_t _2252 = _blur_unnormalized_stencil_20 + _2251;
  uint16_t _2253 = _2234 + _2252;
  return _2253;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_52(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_21 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_181 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_182 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_183 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_184 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_185 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_186 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_187 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_188 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_189 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2312 = (uint16_t)(24);
  uint16_t _2313 = _hw_input_global_wrapper_global_wrapper_stencil_181 * _2312;
  uint16_t _2314 = (uint16_t)(30);
  uint16_t _2315 = _hw_input_global_wrapper_global_wrapper_stencil_182 * _2314;
  uint16_t _2316 = _hw_input_global_wrapper_global_wrapper_stencil_183 * _2312;
  uint16_t _2317 = _hw_input_global_wrapper_global_wrapper_stencil_184 * _2314;
  uint16_t _2318 = (uint16_t)(37);
  uint16_t _2319 = _hw_input_global_wrapper_global_wrapper_stencil_185 * _2318;
  uint16_t _2320 = _hw_input_global_wrapper_global_wrapper_stencil_186 * _2314;
  uint16_t _2321 = _hw_input_global_wrapper_global_wrapper_stencil_187 * _2312;
  uint16_t _2322 = _hw_input_global_wrapper_global_wrapper_stencil_188 * _2312;
  uint16_t _2323 = _hw_input_global_wrapper_global_wrapper_stencil_189 * _2314;
  uint16_t _2324 = _2322 + _2323;
  uint16_t _2325 = _2321 + _2324;
  uint16_t _2326 = _2320 + _2325;
  uint16_t _2327 = _2319 + _2326;
  uint16_t _2328 = _2317 + _2327;
  uint16_t _2329 = _2316 + _2328;
  uint16_t _2330 = _2315 + _2329;
  uint16_t _2331 = _blur_unnormalized_stencil_21 + _2330;
  uint16_t _2332 = _2313 + _2331;
  return _2332;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_53(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_22 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_190 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_191 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_192 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_193 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_194 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_195 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_196 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_197 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_198 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2391 = (uint16_t)(24);
  uint16_t _2392 = _hw_input_global_wrapper_global_wrapper_stencil_190 * _2391;
  uint16_t _2393 = (uint16_t)(30);
  uint16_t _2394 = _hw_input_global_wrapper_global_wrapper_stencil_191 * _2393;
  uint16_t _2395 = _hw_input_global_wrapper_global_wrapper_stencil_192 * _2391;
  uint16_t _2396 = _hw_input_global_wrapper_global_wrapper_stencil_193 * _2393;
  uint16_t _2397 = (uint16_t)(37);
  uint16_t _2398 = _hw_input_global_wrapper_global_wrapper_stencil_194 * _2397;
  uint16_t _2399 = _hw_input_global_wrapper_global_wrapper_stencil_195 * _2393;
  uint16_t _2400 = _hw_input_global_wrapper_global_wrapper_stencil_196 * _2391;
  uint16_t _2401 = _hw_input_global_wrapper_global_wrapper_stencil_197 * _2391;
  uint16_t _2402 = _hw_input_global_wrapper_global_wrapper_stencil_198 * _2393;
  uint16_t _2403 = _2401 + _2402;
  uint16_t _2404 = _2400 + _2403;
  uint16_t _2405 = _2399 + _2404;
  uint16_t _2406 = _2398 + _2405;
  uint16_t _2407 = _2396 + _2406;
  uint16_t _2408 = _2395 + _2407;
  uint16_t _2409 = _2394 + _2408;
  uint16_t _2410 = _blur_unnormalized_stencil_22 + _2409;
  uint16_t _2411 = _2392 + _2410;
  return _2411;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_54(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_23 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_199 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_200 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_201 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_202 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_203 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_204 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_205 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_206 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_207 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2470 = (uint16_t)(24);
  uint16_t _2471 = _hw_input_global_wrapper_global_wrapper_stencil_199 * _2470;
  uint16_t _2472 = (uint16_t)(30);
  uint16_t _2473 = _hw_input_global_wrapper_global_wrapper_stencil_200 * _2472;
  uint16_t _2474 = _hw_input_global_wrapper_global_wrapper_stencil_201 * _2470;
  uint16_t _2475 = _hw_input_global_wrapper_global_wrapper_stencil_202 * _2472;
  uint16_t _2476 = (uint16_t)(37);
  uint16_t _2477 = _hw_input_global_wrapper_global_wrapper_stencil_203 * _2476;
  uint16_t _2478 = _hw_input_global_wrapper_global_wrapper_stencil_204 * _2472;
  uint16_t _2479 = _hw_input_global_wrapper_global_wrapper_stencil_205 * _2470;
  uint16_t _2480 = _hw_input_global_wrapper_global_wrapper_stencil_206 * _2470;
  uint16_t _2481 = _hw_input_global_wrapper_global_wrapper_stencil_207 * _2472;
  uint16_t _2482 = _2480 + _2481;
  uint16_t _2483 = _2479 + _2482;
  uint16_t _2484 = _2478 + _2483;
  uint16_t _2485 = _2477 + _2484;
  uint16_t _2486 = _2475 + _2485;
  uint16_t _2487 = _2474 + _2486;
  uint16_t _2488 = _2473 + _2487;
  uint16_t _2489 = _blur_unnormalized_stencil_23 + _2488;
  uint16_t _2490 = _2471 + _2489;
  return _2490;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_55(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_24 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_208 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_209 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_210 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_211 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_212 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_213 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_214 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_215 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_216 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2549 = (uint16_t)(24);
  uint16_t _2550 = _hw_input_global_wrapper_global_wrapper_stencil_208 * _2549;
  uint16_t _2551 = (uint16_t)(30);
  uint16_t _2552 = _hw_input_global_wrapper_global_wrapper_stencil_209 * _2551;
  uint16_t _2553 = _hw_input_global_wrapper_global_wrapper_stencil_210 * _2549;
  uint16_t _2554 = _hw_input_global_wrapper_global_wrapper_stencil_211 * _2551;
  uint16_t _2555 = (uint16_t)(37);
  uint16_t _2556 = _hw_input_global_wrapper_global_wrapper_stencil_212 * _2555;
  uint16_t _2557 = _hw_input_global_wrapper_global_wrapper_stencil_213 * _2551;
  uint16_t _2558 = _hw_input_global_wrapper_global_wrapper_stencil_214 * _2549;
  uint16_t _2559 = _hw_input_global_wrapper_global_wrapper_stencil_215 * _2549;
  uint16_t _2560 = _hw_input_global_wrapper_global_wrapper_stencil_216 * _2551;
  uint16_t _2561 = _2559 + _2560;
  uint16_t _2562 = _2558 + _2561;
  uint16_t _2563 = _2557 + _2562;
  uint16_t _2564 = _2556 + _2563;
  uint16_t _2565 = _2554 + _2564;
  uint16_t _2566 = _2553 + _2565;
  uint16_t _2567 = _2552 + _2566;
  uint16_t _2568 = _blur_unnormalized_stencil_24 + _2567;
  uint16_t _2569 = _2550 + _2568;
  return _2569;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_56(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_25 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_217 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_218 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_219 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_220 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_221 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_222 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_223 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_224 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_225 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2628 = (uint16_t)(24);
  uint16_t _2629 = _hw_input_global_wrapper_global_wrapper_stencil_217 * _2628;
  uint16_t _2630 = (uint16_t)(30);
  uint16_t _2631 = _hw_input_global_wrapper_global_wrapper_stencil_218 * _2630;
  uint16_t _2632 = _hw_input_global_wrapper_global_wrapper_stencil_219 * _2628;
  uint16_t _2633 = _hw_input_global_wrapper_global_wrapper_stencil_220 * _2630;
  uint16_t _2634 = (uint16_t)(37);
  uint16_t _2635 = _hw_input_global_wrapper_global_wrapper_stencil_221 * _2634;
  uint16_t _2636 = _hw_input_global_wrapper_global_wrapper_stencil_222 * _2630;
  uint16_t _2637 = _hw_input_global_wrapper_global_wrapper_stencil_223 * _2628;
  uint16_t _2638 = _hw_input_global_wrapper_global_wrapper_stencil_224 * _2628;
  uint16_t _2639 = _hw_input_global_wrapper_global_wrapper_stencil_225 * _2630;
  uint16_t _2640 = _2638 + _2639;
  uint16_t _2641 = _2637 + _2640;
  uint16_t _2642 = _2636 + _2641;
  uint16_t _2643 = _2635 + _2642;
  uint16_t _2644 = _2633 + _2643;
  uint16_t _2645 = _2632 + _2644;
  uint16_t _2646 = _2631 + _2645;
  uint16_t _2647 = _blur_unnormalized_stencil_25 + _2646;
  uint16_t _2648 = _2629 + _2647;
  return _2648;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_57(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_26 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_226 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_227 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_228 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_229 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_230 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_231 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_232 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_233 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_234 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2707 = (uint16_t)(24);
  uint16_t _2708 = _hw_input_global_wrapper_global_wrapper_stencil_226 * _2707;
  uint16_t _2709 = (uint16_t)(30);
  uint16_t _2710 = _hw_input_global_wrapper_global_wrapper_stencil_227 * _2709;
  uint16_t _2711 = _hw_input_global_wrapper_global_wrapper_stencil_228 * _2707;
  uint16_t _2712 = _hw_input_global_wrapper_global_wrapper_stencil_229 * _2709;
  uint16_t _2713 = (uint16_t)(37);
  uint16_t _2714 = _hw_input_global_wrapper_global_wrapper_stencil_230 * _2713;
  uint16_t _2715 = _hw_input_global_wrapper_global_wrapper_stencil_231 * _2709;
  uint16_t _2716 = _hw_input_global_wrapper_global_wrapper_stencil_232 * _2707;
  uint16_t _2717 = _hw_input_global_wrapper_global_wrapper_stencil_233 * _2707;
  uint16_t _2718 = _hw_input_global_wrapper_global_wrapper_stencil_234 * _2709;
  uint16_t _2719 = _2717 + _2718;
  uint16_t _2720 = _2716 + _2719;
  uint16_t _2721 = _2715 + _2720;
  uint16_t _2722 = _2714 + _2721;
  uint16_t _2723 = _2712 + _2722;
  uint16_t _2724 = _2711 + _2723;
  uint16_t _2725 = _2710 + _2724;
  uint16_t _2726 = _blur_unnormalized_stencil_26 + _2725;
  uint16_t _2727 = _2708 + _2726;
  return _2727;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_58(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_27 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_235 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_236 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_237 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_238 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_239 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_240 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_241 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_242 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_243 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2786 = (uint16_t)(24);
  uint16_t _2787 = _hw_input_global_wrapper_global_wrapper_stencil_235 * _2786;
  uint16_t _2788 = (uint16_t)(30);
  uint16_t _2789 = _hw_input_global_wrapper_global_wrapper_stencil_236 * _2788;
  uint16_t _2790 = _hw_input_global_wrapper_global_wrapper_stencil_237 * _2786;
  uint16_t _2791 = _hw_input_global_wrapper_global_wrapper_stencil_238 * _2788;
  uint16_t _2792 = (uint16_t)(37);
  uint16_t _2793 = _hw_input_global_wrapper_global_wrapper_stencil_239 * _2792;
  uint16_t _2794 = _hw_input_global_wrapper_global_wrapper_stencil_240 * _2788;
  uint16_t _2795 = _hw_input_global_wrapper_global_wrapper_stencil_241 * _2786;
  uint16_t _2796 = _hw_input_global_wrapper_global_wrapper_stencil_242 * _2786;
  uint16_t _2797 = _hw_input_global_wrapper_global_wrapper_stencil_243 * _2788;
  uint16_t _2798 = _2796 + _2797;
  uint16_t _2799 = _2795 + _2798;
  uint16_t _2800 = _2794 + _2799;
  uint16_t _2801 = _2793 + _2800;
  uint16_t _2802 = _2791 + _2801;
  uint16_t _2803 = _2790 + _2802;
  uint16_t _2804 = _2789 + _2803;
  uint16_t _2805 = _blur_unnormalized_stencil_27 + _2804;
  uint16_t _2806 = _2787 + _2805;
  return _2806;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_59(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_28 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_244 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_245 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_246 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_247 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_248 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_249 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_250 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_251 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_252 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2865 = (uint16_t)(24);
  uint16_t _2866 = _hw_input_global_wrapper_global_wrapper_stencil_244 * _2865;
  uint16_t _2867 = (uint16_t)(30);
  uint16_t _2868 = _hw_input_global_wrapper_global_wrapper_stencil_245 * _2867;
  uint16_t _2869 = _hw_input_global_wrapper_global_wrapper_stencil_246 * _2865;
  uint16_t _2870 = _hw_input_global_wrapper_global_wrapper_stencil_247 * _2867;
  uint16_t _2871 = (uint16_t)(37);
  uint16_t _2872 = _hw_input_global_wrapper_global_wrapper_stencil_248 * _2871;
  uint16_t _2873 = _hw_input_global_wrapper_global_wrapper_stencil_249 * _2867;
  uint16_t _2874 = _hw_input_global_wrapper_global_wrapper_stencil_250 * _2865;
  uint16_t _2875 = _hw_input_global_wrapper_global_wrapper_stencil_251 * _2865;
  uint16_t _2876 = _hw_input_global_wrapper_global_wrapper_stencil_252 * _2867;
  uint16_t _2877 = _2875 + _2876;
  uint16_t _2878 = _2874 + _2877;
  uint16_t _2879 = _2873 + _2878;
  uint16_t _2880 = _2872 + _2879;
  uint16_t _2881 = _2870 + _2880;
  uint16_t _2882 = _2869 + _2881;
  uint16_t _2883 = _2868 + _2882;
  uint16_t _2884 = _blur_unnormalized_stencil_28 + _2883;
  uint16_t _2885 = _2866 + _2884;
  return _2885;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_60(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_29 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_253 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_254 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_255 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_256 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_257 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_258 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_259 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_260 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_261 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _2944 = (uint16_t)(24);
  uint16_t _2945 = _hw_input_global_wrapper_global_wrapper_stencil_253 * _2944;
  uint16_t _2946 = (uint16_t)(30);
  uint16_t _2947 = _hw_input_global_wrapper_global_wrapper_stencil_254 * _2946;
  uint16_t _2948 = _hw_input_global_wrapper_global_wrapper_stencil_255 * _2944;
  uint16_t _2949 = _hw_input_global_wrapper_global_wrapper_stencil_256 * _2946;
  uint16_t _2950 = (uint16_t)(37);
  uint16_t _2951 = _hw_input_global_wrapper_global_wrapper_stencil_257 * _2950;
  uint16_t _2952 = _hw_input_global_wrapper_global_wrapper_stencil_258 * _2946;
  uint16_t _2953 = _hw_input_global_wrapper_global_wrapper_stencil_259 * _2944;
  uint16_t _2954 = _hw_input_global_wrapper_global_wrapper_stencil_260 * _2944;
  uint16_t _2955 = _hw_input_global_wrapper_global_wrapper_stencil_261 * _2946;
  uint16_t _2956 = _2954 + _2955;
  uint16_t _2957 = _2953 + _2956;
  uint16_t _2958 = _2952 + _2957;
  uint16_t _2959 = _2951 + _2958;
  uint16_t _2960 = _2949 + _2959;
  uint16_t _2961 = _2948 + _2960;
  uint16_t _2962 = _2947 + _2961;
  uint16_t _2963 = _blur_unnormalized_stencil_29 + _2962;
  uint16_t _2964 = _2945 + _2963;
  return _2964;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_61(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_30 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_262 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_263 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_264 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_265 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_266 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_267 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_268 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_269 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_270 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _3023 = (uint16_t)(24);
  uint16_t _3024 = _hw_input_global_wrapper_global_wrapper_stencil_262 * _3023;
  uint16_t _3025 = (uint16_t)(30);
  uint16_t _3026 = _hw_input_global_wrapper_global_wrapper_stencil_263 * _3025;
  uint16_t _3027 = _hw_input_global_wrapper_global_wrapper_stencil_264 * _3023;
  uint16_t _3028 = _hw_input_global_wrapper_global_wrapper_stencil_265 * _3025;
  uint16_t _3029 = (uint16_t)(37);
  uint16_t _3030 = _hw_input_global_wrapper_global_wrapper_stencil_266 * _3029;
  uint16_t _3031 = _hw_input_global_wrapper_global_wrapper_stencil_267 * _3025;
  uint16_t _3032 = _hw_input_global_wrapper_global_wrapper_stencil_268 * _3023;
  uint16_t _3033 = _hw_input_global_wrapper_global_wrapper_stencil_269 * _3023;
  uint16_t _3034 = _hw_input_global_wrapper_global_wrapper_stencil_270 * _3025;
  uint16_t _3035 = _3033 + _3034;
  uint16_t _3036 = _3032 + _3035;
  uint16_t _3037 = _3031 + _3036;
  uint16_t _3038 = _3030 + _3037;
  uint16_t _3039 = _3028 + _3038;
  uint16_t _3040 = _3027 + _3039;
  uint16_t _3041 = _3026 + _3040;
  uint16_t _3042 = _blur_unnormalized_stencil_30 + _3041;
  uint16_t _3043 = _3024 + _3042;
  return _3043;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_62(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_31 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_271 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_272 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_273 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_274 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_275 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_276 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_277 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_278 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_279 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _3102 = (uint16_t)(24);
  uint16_t _3103 = _hw_input_global_wrapper_global_wrapper_stencil_271 * _3102;
  uint16_t _3104 = (uint16_t)(30);
  uint16_t _3105 = _hw_input_global_wrapper_global_wrapper_stencil_272 * _3104;
  uint16_t _3106 = _hw_input_global_wrapper_global_wrapper_stencil_273 * _3102;
  uint16_t _3107 = _hw_input_global_wrapper_global_wrapper_stencil_274 * _3104;
  uint16_t _3108 = (uint16_t)(37);
  uint16_t _3109 = _hw_input_global_wrapper_global_wrapper_stencil_275 * _3108;
  uint16_t _3110 = _hw_input_global_wrapper_global_wrapper_stencil_276 * _3104;
  uint16_t _3111 = _hw_input_global_wrapper_global_wrapper_stencil_277 * _3102;
  uint16_t _3112 = _hw_input_global_wrapper_global_wrapper_stencil_278 * _3102;
  uint16_t _3113 = _hw_input_global_wrapper_global_wrapper_stencil_279 * _3104;
  uint16_t _3114 = _3112 + _3113;
  uint16_t _3115 = _3111 + _3114;
  uint16_t _3116 = _3110 + _3115;
  uint16_t _3117 = _3109 + _3116;
  uint16_t _3118 = _3107 + _3117;
  uint16_t _3119 = _3106 + _3118;
  uint16_t _3120 = _3105 + _3119;
  uint16_t _3121 = _blur_unnormalized_stencil_31 + _3120;
  uint16_t _3122 = _3103 + _3121;
  return _3122;
}

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 33), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 33), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 33), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
hw_uint<16> hcompute_blur_unnormalized_stencil_63(hw_uint<16>& blur_unnormalized_stencil, hw_uint<144>& hw_input_global_wrapper_global_wrapper_stencil) {
  uint16_t _blur_unnormalized_stencil_32 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_280 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<0, 15>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_281 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<16, 31>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_282 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<32, 47>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_283 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<48, 63>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_284 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<64, 79>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_285 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<80, 95>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_286 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<96, 111>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_287 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<112, 127>());
  uint16_t _hw_input_global_wrapper_global_wrapper_stencil_288 = (uint16_t) (hw_input_global_wrapper_global_wrapper_stencil.extract<128, 143>());

  uint16_t _3181 = (uint16_t)(24);
  uint16_t _3182 = _hw_input_global_wrapper_global_wrapper_stencil_280 * _3181;
  uint16_t _3183 = (uint16_t)(30);
  uint16_t _3184 = _hw_input_global_wrapper_global_wrapper_stencil_281 * _3183;
  uint16_t _3185 = _hw_input_global_wrapper_global_wrapper_stencil_282 * _3181;
  uint16_t _3186 = _hw_input_global_wrapper_global_wrapper_stencil_283 * _3183;
  uint16_t _3187 = (uint16_t)(37);
  uint16_t _3188 = _hw_input_global_wrapper_global_wrapper_stencil_284 * _3187;
  uint16_t _3189 = _hw_input_global_wrapper_global_wrapper_stencil_285 * _3183;
  uint16_t _3190 = _hw_input_global_wrapper_global_wrapper_stencil_286 * _3181;
  uint16_t _3191 = _hw_input_global_wrapper_global_wrapper_stencil_287 * _3181;
  uint16_t _3192 = _hw_input_global_wrapper_global_wrapper_stencil_288 * _3183;
  uint16_t _3193 = _3191 + _3192;
  uint16_t _3194 = _3190 + _3193;
  uint16_t _3195 = _3189 + _3194;
  uint16_t _3196 = _3188 + _3195;
  uint16_t _3197 = _3186 + _3196;
  uint16_t _3198 = _3185 + _3197;
  uint16_t _3199 = _3184 + _3198;
  uint16_t _3200 = _blur_unnormalized_stencil_32 + _3199;
  uint16_t _3201 = _3182 + _3200;
  return _3201;
}

//store is: blur.stencil((blur_s0_x_x*32), blur_s0_y) = (blur_unnormalized.stencil((blur_s0_x_x*32), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_33 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3260 = (uint16_t)(8);
  uint16_t _3261 = _blur_unnormalized_stencil_33 >> _3260;
  return _3261;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 1), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 1), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_1(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_34 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3268 = (uint16_t)(8);
  uint16_t _3269 = _blur_unnormalized_stencil_34 >> _3268;
  return _3269;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 2), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 2), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_2(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_35 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3277 = (uint16_t)(8);
  uint16_t _3278 = _blur_unnormalized_stencil_35 >> _3277;
  return _3278;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 3), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 3), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_3(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_36 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3286 = (uint16_t)(8);
  uint16_t _3287 = _blur_unnormalized_stencil_36 >> _3286;
  return _3287;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 4), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 4), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_4(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_37 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3295 = (uint16_t)(8);
  uint16_t _3296 = _blur_unnormalized_stencil_37 >> _3295;
  return _3296;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 5), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 5), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_5(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_38 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3304 = (uint16_t)(8);
  uint16_t _3305 = _blur_unnormalized_stencil_38 >> _3304;
  return _3305;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 6), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 6), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_6(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_39 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3313 = (uint16_t)(8);
  uint16_t _3314 = _blur_unnormalized_stencil_39 >> _3313;
  return _3314;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 7), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 7), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_7(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_40 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3322 = (uint16_t)(8);
  uint16_t _3323 = _blur_unnormalized_stencil_40 >> _3322;
  return _3323;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 8), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 8), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_8(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_41 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3331 = (uint16_t)(8);
  uint16_t _3332 = _blur_unnormalized_stencil_41 >> _3331;
  return _3332;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 9), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 9), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_9(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_42 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3340 = (uint16_t)(8);
  uint16_t _3341 = _blur_unnormalized_stencil_42 >> _3340;
  return _3341;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 10), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 10), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_10(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_43 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3349 = (uint16_t)(8);
  uint16_t _3350 = _blur_unnormalized_stencil_43 >> _3349;
  return _3350;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 11), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 11), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_11(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_44 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3358 = (uint16_t)(8);
  uint16_t _3359 = _blur_unnormalized_stencil_44 >> _3358;
  return _3359;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 12), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 12), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_12(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_45 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3367 = (uint16_t)(8);
  uint16_t _3368 = _blur_unnormalized_stencil_45 >> _3367;
  return _3368;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 13), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 13), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_13(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_46 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3376 = (uint16_t)(8);
  uint16_t _3377 = _blur_unnormalized_stencil_46 >> _3376;
  return _3377;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 14), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 14), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_14(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_47 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3385 = (uint16_t)(8);
  uint16_t _3386 = _blur_unnormalized_stencil_47 >> _3385;
  return _3386;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 15), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 15), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_15(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_48 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3394 = (uint16_t)(8);
  uint16_t _3395 = _blur_unnormalized_stencil_48 >> _3394;
  return _3395;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 16), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 16), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_16(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_49 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3403 = (uint16_t)(8);
  uint16_t _3404 = _blur_unnormalized_stencil_49 >> _3403;
  return _3404;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 17), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 17), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_17(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_50 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3412 = (uint16_t)(8);
  uint16_t _3413 = _blur_unnormalized_stencil_50 >> _3412;
  return _3413;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 18), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 18), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_18(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_51 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3421 = (uint16_t)(8);
  uint16_t _3422 = _blur_unnormalized_stencil_51 >> _3421;
  return _3422;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 19), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 19), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_19(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_52 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3430 = (uint16_t)(8);
  uint16_t _3431 = _blur_unnormalized_stencil_52 >> _3430;
  return _3431;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 20), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 20), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_20(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_53 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3439 = (uint16_t)(8);
  uint16_t _3440 = _blur_unnormalized_stencil_53 >> _3439;
  return _3440;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 21), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 21), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_21(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_54 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3448 = (uint16_t)(8);
  uint16_t _3449 = _blur_unnormalized_stencil_54 >> _3448;
  return _3449;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 22), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 22), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_22(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_55 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3457 = (uint16_t)(8);
  uint16_t _3458 = _blur_unnormalized_stencil_55 >> _3457;
  return _3458;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 23), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 23), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_23(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_56 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3466 = (uint16_t)(8);
  uint16_t _3467 = _blur_unnormalized_stencil_56 >> _3466;
  return _3467;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 24), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 24), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_24(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_57 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3475 = (uint16_t)(8);
  uint16_t _3476 = _blur_unnormalized_stencil_57 >> _3475;
  return _3476;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 25), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 25), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_25(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_58 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3484 = (uint16_t)(8);
  uint16_t _3485 = _blur_unnormalized_stencil_58 >> _3484;
  return _3485;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 26), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 26), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_26(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_59 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3493 = (uint16_t)(8);
  uint16_t _3494 = _blur_unnormalized_stencil_59 >> _3493;
  return _3494;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 27), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 27), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_27(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_60 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3502 = (uint16_t)(8);
  uint16_t _3503 = _blur_unnormalized_stencil_60 >> _3502;
  return _3503;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 28), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 28), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_28(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_61 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3511 = (uint16_t)(8);
  uint16_t _3512 = _blur_unnormalized_stencil_61 >> _3511;
  return _3512;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 29), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 29), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_29(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_62 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3520 = (uint16_t)(8);
  uint16_t _3521 = _blur_unnormalized_stencil_62 >> _3520;
  return _3521;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 30), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 30), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_30(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_63 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3529 = (uint16_t)(8);
  uint16_t _3530 = _blur_unnormalized_stencil_63 >> _3529;
  return _3530;
}

//store is: blur.stencil(((blur_s0_x_x*32) + 31), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 31), blur_s0_y)/(uint16)256)
hw_uint<16> hcompute_blur_stencil_31(hw_uint<16>& blur_unnormalized_stencil) {
  uint16_t _blur_unnormalized_stencil_64 = (uint16_t) (blur_unnormalized_stencil.extract<0, 15>());

  uint16_t _3538 = (uint16_t)(8);
  uint16_t _3539 = _blur_unnormalized_stencil_64 >> _3538;
  return _3539;
}

//store is: hw_output.stencil((hw_output_s0_x_xii_xii*32), hw_output_s0_y_yii) = blur.stencil((hw_output_s0_x_xii_xii*32), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_1 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_1;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 1), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 1), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_1(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_2 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_2;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 2), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 2), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_2(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_3 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_3;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 3), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 3), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_3(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_4 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_4;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 4), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 4), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_4(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_5 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_5;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 5), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 5), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_5(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_6 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_6;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 6), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 6), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_6(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_7 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_7;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 7), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 7), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_7(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_8 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_8;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 8), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 8), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_8(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_9 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_9;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 9), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 9), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_9(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_10 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_10;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 10), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 10), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_10(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_11 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_11;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 11), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 11), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_11(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_12 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_12;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 12), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 12), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_12(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_13 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_13;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 13), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 13), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_13(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_14 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_14;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 14), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 14), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_14(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_15 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_15;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 15), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 15), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_15(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_16 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_16;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 16), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 16), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_16(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_17 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_17;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 17), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 17), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_17(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_18 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_18;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 18), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 18), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_18(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_19 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_19;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 19), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 19), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_19(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_20 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_20;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 20), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 20), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_20(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_21 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_21;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 21), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 21), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_21(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_22 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_22;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 22), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 22), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_22(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_23 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_23;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 23), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 23), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_23(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_24 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_24;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 24), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 24), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_24(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_25 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_25;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 25), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 25), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_25(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_26 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_26;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 26), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 26), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_26(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_27 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_27;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 27), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 27), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_27(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_28 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_28;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 28), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 28), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_28(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_29 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_29;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 29), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 29), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_29(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_30 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_30;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 30), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 30), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_30(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_31 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_31;
}

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 31), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 31), hw_output_s0_y_yii)
hw_uint<16> hcompute_hw_output_stencil_31(hw_uint<16>& blur_stencil) {
  uint16_t _blur_stencil_32 = (uint16_t) (blur_stencil.extract<0, 15>());

  return _blur_stencil_32;
}

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil((hw_output_global_wrapper_s0_x_xi_xi*32), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_1 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_1;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 1), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 1), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_1(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_2 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_2;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 2), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 2), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_2(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_3 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_3;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 3), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 3), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_3(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_4 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_4;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 4), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 4), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_4(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_5 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_5;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 5), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 5), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_5(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_6 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_6;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 6), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 6), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_6(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_7 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_7;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 7), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 7), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_7(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_8 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_8;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 8), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 8), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_8(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_9 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_9;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 9), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 9), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_9(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_10 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_10;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 10), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 10), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_10(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_11 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_11;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 11), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 11), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_11(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_12 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_12;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 12), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 12), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_12(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_13 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_13;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 13), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 13), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_13(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_14 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_14;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 14), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 14), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_14(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_15 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_15;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 15), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 15), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_15(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_16 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_16;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 16), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 16), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_16(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_17 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_17;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 17), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 17), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_17(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_18 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_18;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 18), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 18), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_18(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_19 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_19;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 19), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 19), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_19(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_20 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_20;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 20), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 20), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_20(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_21 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_21;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 21), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 21), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_21(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_22 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_22;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 22), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 22), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_22(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_23 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_23;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 23), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 23), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_23(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_24 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_24;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 24), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 24), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_24(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_25 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_25;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 25), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 25), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_25(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_26 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_26;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 26), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 26), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_26(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_27 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_27;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 27), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 27), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_27(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_28 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_28;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 28), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 28), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_28(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_29 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_29;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 29), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 29), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_29(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_30 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_30;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 30), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 30), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_30(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_31 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_31;
}

//store is: hw_output_global_wrapper.glb.stencil((((hw_output_global_wrapper_s0_x_xi_xi + (0*9))*32) + 31), (hw_output_global_wrapper_s0_y_yi + 0)) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 31), hw_output_global_wrapper_s0_y_yi)
hw_uint<16> hcompute_hw_output_global_wrapper_glb_stencil_31(hw_uint<16>& hw_output_stencil) {
  uint16_t _hw_output_stencil_32 = (uint16_t) (hw_output_stencil.extract<0, 15>());

  return _hw_output_stencil_32;
}

