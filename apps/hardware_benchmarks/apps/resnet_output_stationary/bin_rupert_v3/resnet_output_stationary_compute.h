#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: input_glb.stencil((input_glb_s0_z_z*8), input_glb_s0_x, input_glb_s0_y) = input_host.stencil((input_glb_s0_z_z*8), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_1 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_1;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 1), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 1), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_1(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_2 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_2;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 2), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 2), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_2(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_3 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_3;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 3), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 3), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_3(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_4 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_4;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 4), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 4), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_4(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_5 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_5;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 5), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 5), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_5(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_6 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_6;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 6), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 6), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_6(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_7 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_7;
}

//store is: input_glb.stencil(((input_glb_s0_z_z*8) + 7), input_glb_s0_x, input_glb_s0_y) = input_host.stencil(((input_glb_s0_z_z*8) + 7), input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil_7(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_8 = (int16_t) (input_host_stencil.extract<0, 15>());

  return _input_host_stencil_8;
}

//store is: kernel_glb.stencil((kernel_glb_s0_w_w*4), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y) = kernel_host.stencil((kernel_glb_s0_w_w*4), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y)
hw_uint<16> hcompute_kernel_glb_stencil(hw_uint<16>& kernel_host_stencil) {
  int16_t _kernel_host_stencil_1 = (int16_t) (kernel_host_stencil.extract<0, 15>());

  return _kernel_host_stencil_1;
}

//store is: kernel_glb.stencil(((kernel_glb_s0_w_w*4) + 1), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y) = kernel_host.stencil(((kernel_glb_s0_w_w*4) + 1), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y)
hw_uint<16> hcompute_kernel_glb_stencil_1(hw_uint<16>& kernel_host_stencil) {
  int16_t _kernel_host_stencil_2 = (int16_t) (kernel_host_stencil.extract<0, 15>());

  return _kernel_host_stencil_2;
}

//store is: kernel_glb.stencil(((kernel_glb_s0_w_w*4) + 2), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y) = kernel_host.stencil(((kernel_glb_s0_w_w*4) + 2), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y)
hw_uint<16> hcompute_kernel_glb_stencil_2(hw_uint<16>& kernel_host_stencil) {
  int16_t _kernel_host_stencil_3 = (int16_t) (kernel_host_stencil.extract<0, 15>());

  return _kernel_host_stencil_3;
}

//store is: kernel_glb.stencil(((kernel_glb_s0_w_w*4) + 3), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y) = kernel_host.stencil(((kernel_glb_s0_w_w*4) + 3), kernel_glb_s0_z, kernel_glb_s0_x, kernel_glb_s0_y)
hw_uint<16> hcompute_kernel_glb_stencil_3(hw_uint<16>& kernel_host_stencil) {
  int16_t _kernel_host_stencil_4 = (int16_t) (kernel_host_stencil.extract<0, 15>());

  return _kernel_host_stencil_4;
}

//store is: output_cgra.stencil(0, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil() {
  int16_t _721 = (int16_t)(0);
  return _721;
}

//store is: output_cgra.stencil(1, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_1() {
  int16_t _724 = (int16_t)(0);
  return _724;
}

//store is: output_cgra.stencil(2, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_2() {
  int16_t _727 = (int16_t)(0);
  return _727;
}

//store is: output_cgra.stencil(3, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_3() {
  int16_t _730 = (int16_t)(0);
  return _730;
}

//store is: output_cgra.stencil(4, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_4() {
  int16_t _733 = (int16_t)(0);
  return _733;
}

//store is: output_cgra.stencil(5, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_5() {
  int16_t _736 = (int16_t)(0);
  return _736;
}

//store is: output_cgra.stencil(6, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_6() {
  int16_t _739 = (int16_t)(0);
  return _739;
}

//store is: output_cgra.stencil(7, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_7() {
  int16_t _742 = (int16_t)(0);
  return _742;
}

//store is: output_cgra.stencil(8, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_8() {
  int16_t _745 = (int16_t)(0);
  return _745;
}

//store is: output_cgra.stencil(9, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_9() {
  int16_t _748 = (int16_t)(0);
  return _748;
}

//store is: output_cgra.stencil(10, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_10() {
  int16_t _751 = (int16_t)(0);
  return _751;
}

//store is: output_cgra.stencil(11, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_11() {
  int16_t _754 = (int16_t)(0);
  return _754;
}

//store is: output_cgra.stencil(12, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_12() {
  int16_t _757 = (int16_t)(0);
  return _757;
}

//store is: output_cgra.stencil(13, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_13() {
  int16_t _760 = (int16_t)(0);
  return _760;
}

//store is: output_cgra.stencil(14, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_14() {
  int16_t _763 = (int16_t)(0);
  return _763;
}

//store is: output_cgra.stencil(15, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_15() {
  int16_t _766 = (int16_t)(0);
  return _766;
}

//store is: input_cgra.stencil((input_cgra_s0_z_z_cgra_z_cgra*8), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_1 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_1;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 1), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 1), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_1(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_2 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_2;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 2), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 2), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_2(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_3 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_3;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 3), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 3), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_3(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_4 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_4;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 4), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 4), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_4(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_5 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_5;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 5), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 5), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_5(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_6 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_6;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 6), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 6), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_6(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_7 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_7;
}

//store is: input_cgra.stencil(((input_cgra_s0_z_z_cgra_z_cgra*8) + 7), input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((((output_cgra_s1_r_z_rz_glb*2) + input_cgra_s0_z_z_cgra_z_cgra)*8) + 7), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil_7(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_8 = (int16_t) (input_glb_stencil.extract<0, 15>());

  return _input_glb_stencil_8;
}

//store is: kernel_cgra.stencil((kernel_cgra_s0_w_w_cgra_w_cgra*4), kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil((((output_glb_s0_w_w_glb*4) + kernel_cgra_s0_w_w_cgra_w_cgra)*4), ((output_cgra_s1_r_z_rz_glb*16) + kernel_cgra_s0_z_z_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_1 = (int16_t) (kernel_glb_stencil.extract<0, 15>());

  return _kernel_glb_stencil_1;
}

//store is: kernel_cgra.stencil(((kernel_cgra_s0_w_w_cgra_w_cgra*4) + 1), kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((((output_glb_s0_w_w_glb*4) + kernel_cgra_s0_w_w_cgra_w_cgra)*4) + 1), ((output_cgra_s1_r_z_rz_glb*16) + kernel_cgra_s0_z_z_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil_1(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_2 = (int16_t) (kernel_glb_stencil.extract<0, 15>());

  return _kernel_glb_stencil_2;
}

//store is: kernel_cgra.stencil(((kernel_cgra_s0_w_w_cgra_w_cgra*4) + 2), kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((((output_glb_s0_w_w_glb*4) + kernel_cgra_s0_w_w_cgra_w_cgra)*4) + 2), ((output_cgra_s1_r_z_rz_glb*16) + kernel_cgra_s0_z_z_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil_2(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_3 = (int16_t) (kernel_glb_stencil.extract<0, 15>());

  return _kernel_glb_stencil_3;
}

//store is: kernel_cgra.stencil(((kernel_cgra_s0_w_w_cgra_w_cgra*4) + 3), kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((((output_glb_s0_w_w_glb*4) + kernel_cgra_s0_w_w_cgra_w_cgra)*4) + 3), ((output_cgra_s1_r_z_rz_glb*16) + kernel_cgra_s0_z_z_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil_3(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_4 = (int16_t) (kernel_glb_stencil.extract<0, 15>());

  return _kernel_glb_stencil_4;
}

//store is: output_cgra.stencil(0, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(0, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(0, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(0, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(0, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_16(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_1 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_10 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_11 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_12 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_13 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_14 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_15 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_16 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_2 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_3 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_4 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_5 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_6 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_7 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_8 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_9 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_1 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_10 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_11 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_12 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_13 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_14 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_15 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_16 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_2 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_3 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_4 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_5 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_6 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_7 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_8 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_9 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_1 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _857 = _kernel_cgra_stencil_1 * _input_cgra_stencil_1;
  int16_t _858 = _kernel_cgra_stencil_2 * _input_cgra_stencil_2;
  int16_t _859 = _kernel_cgra_stencil_3 * _input_cgra_stencil_3;
  int16_t _860 = _kernel_cgra_stencil_4 * _input_cgra_stencil_4;
  int16_t _861 = _kernel_cgra_stencil_5 * _input_cgra_stencil_5;
  int16_t _862 = _kernel_cgra_stencil_6 * _input_cgra_stencil_6;
  int16_t _863 = _kernel_cgra_stencil_7 * _input_cgra_stencil_7;
  int16_t _864 = _kernel_cgra_stencil_8 * _input_cgra_stencil_8;
  int16_t _865 = _kernel_cgra_stencil_9 * _input_cgra_stencil_9;
  int16_t _866 = _kernel_cgra_stencil_10 * _input_cgra_stencil_10;
  int16_t _867 = _kernel_cgra_stencil_11 * _input_cgra_stencil_11;
  int16_t _868 = _kernel_cgra_stencil_12 * _input_cgra_stencil_12;
  int16_t _869 = _kernel_cgra_stencil_13 * _input_cgra_stencil_13;
  int16_t _870 = _kernel_cgra_stencil_14 * _input_cgra_stencil_14;
  int16_t _871 = _kernel_cgra_stencil_15 * _input_cgra_stencil_15;
  int16_t _872 = _kernel_cgra_stencil_16 * _input_cgra_stencil_16;
  int16_t _873 = _871 + _872;
  int16_t _874 = _870 + _873;
  int16_t _875 = _869 + _874;
  int16_t _876 = _868 + _875;
  int16_t _877 = _867 + _876;
  int16_t _878 = _866 + _877;
  int16_t _879 = _865 + _878;
  int16_t _880 = _864 + _879;
  int16_t _881 = _863 + _880;
  int16_t _882 = _862 + _881;
  int16_t _883 = _861 + _882;
  int16_t _884 = _860 + _883;
  int16_t _885 = _859 + _884;
  int16_t _886 = _858 + _885;
  int16_t _887 = _output_cgra_stencil_1 + _886;
  int16_t _888 = _857 + _887;
  return _888;
}

//store is: output_cgra.stencil(1, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(1, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(1, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(1, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(1, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_17(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_17 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_18 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_19 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_20 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_21 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_22 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_23 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_24 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_25 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_26 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_27 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_28 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_29 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_30 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_31 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_32 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_17 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_18 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_19 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_20 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_21 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_22 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_23 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_24 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_25 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_26 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_27 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_28 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_29 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_30 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_31 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_32 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_2 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _990 = _kernel_cgra_stencil_17 * _input_cgra_stencil_17;
  int16_t _991 = _kernel_cgra_stencil_18 * _input_cgra_stencil_18;
  int16_t _992 = _kernel_cgra_stencil_19 * _input_cgra_stencil_19;
  int16_t _993 = _kernel_cgra_stencil_20 * _input_cgra_stencil_20;
  int16_t _994 = _kernel_cgra_stencil_21 * _input_cgra_stencil_21;
  int16_t _995 = _kernel_cgra_stencil_22 * _input_cgra_stencil_22;
  int16_t _996 = _kernel_cgra_stencil_23 * _input_cgra_stencil_23;
  int16_t _997 = _kernel_cgra_stencil_24 * _input_cgra_stencil_24;
  int16_t _998 = _kernel_cgra_stencil_25 * _input_cgra_stencil_25;
  int16_t _999 = _kernel_cgra_stencil_26 * _input_cgra_stencil_26;
  int16_t _1000 = _kernel_cgra_stencil_27 * _input_cgra_stencil_27;
  int16_t _1001 = _kernel_cgra_stencil_28 * _input_cgra_stencil_28;
  int16_t _1002 = _kernel_cgra_stencil_29 * _input_cgra_stencil_29;
  int16_t _1003 = _kernel_cgra_stencil_30 * _input_cgra_stencil_30;
  int16_t _1004 = _kernel_cgra_stencil_31 * _input_cgra_stencil_31;
  int16_t _1005 = _kernel_cgra_stencil_32 * _input_cgra_stencil_32;
  int16_t _1006 = _1004 + _1005;
  int16_t _1007 = _1003 + _1006;
  int16_t _1008 = _1002 + _1007;
  int16_t _1009 = _1001 + _1008;
  int16_t _1010 = _1000 + _1009;
  int16_t _1011 = _999 + _1010;
  int16_t _1012 = _998 + _1011;
  int16_t _1013 = _997 + _1012;
  int16_t _1014 = _996 + _1013;
  int16_t _1015 = _995 + _1014;
  int16_t _1016 = _994 + _1015;
  int16_t _1017 = _993 + _1016;
  int16_t _1018 = _992 + _1017;
  int16_t _1019 = _991 + _1018;
  int16_t _1020 = _output_cgra_stencil_2 + _1019;
  int16_t _1021 = _990 + _1020;
  return _1021;
}

//store is: output_cgra.stencil(2, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(2, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(2, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(2, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(2, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(2, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_18(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_33 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_34 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_35 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_36 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_37 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_38 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_39 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_40 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_41 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_42 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_43 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_44 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_45 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_46 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_47 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_48 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_33 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_34 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_35 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_36 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_37 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_38 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_39 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_40 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_41 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_42 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_43 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_44 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_45 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_46 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_47 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_48 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_3 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1123 = _kernel_cgra_stencil_33 * _input_cgra_stencil_33;
  int16_t _1124 = _kernel_cgra_stencil_34 * _input_cgra_stencil_34;
  int16_t _1125 = _kernel_cgra_stencil_35 * _input_cgra_stencil_35;
  int16_t _1126 = _kernel_cgra_stencil_36 * _input_cgra_stencil_36;
  int16_t _1127 = _kernel_cgra_stencil_37 * _input_cgra_stencil_37;
  int16_t _1128 = _kernel_cgra_stencil_38 * _input_cgra_stencil_38;
  int16_t _1129 = _kernel_cgra_stencil_39 * _input_cgra_stencil_39;
  int16_t _1130 = _kernel_cgra_stencil_40 * _input_cgra_stencil_40;
  int16_t _1131 = _kernel_cgra_stencil_41 * _input_cgra_stencil_41;
  int16_t _1132 = _kernel_cgra_stencil_42 * _input_cgra_stencil_42;
  int16_t _1133 = _kernel_cgra_stencil_43 * _input_cgra_stencil_43;
  int16_t _1134 = _kernel_cgra_stencil_44 * _input_cgra_stencil_44;
  int16_t _1135 = _kernel_cgra_stencil_45 * _input_cgra_stencil_45;
  int16_t _1136 = _kernel_cgra_stencil_46 * _input_cgra_stencil_46;
  int16_t _1137 = _kernel_cgra_stencil_47 * _input_cgra_stencil_47;
  int16_t _1138 = _kernel_cgra_stencil_48 * _input_cgra_stencil_48;
  int16_t _1139 = _1137 + _1138;
  int16_t _1140 = _1136 + _1139;
  int16_t _1141 = _1135 + _1140;
  int16_t _1142 = _1134 + _1141;
  int16_t _1143 = _1133 + _1142;
  int16_t _1144 = _1132 + _1143;
  int16_t _1145 = _1131 + _1144;
  int16_t _1146 = _1130 + _1145;
  int16_t _1147 = _1129 + _1146;
  int16_t _1148 = _1128 + _1147;
  int16_t _1149 = _1127 + _1148;
  int16_t _1150 = _1126 + _1149;
  int16_t _1151 = _1125 + _1150;
  int16_t _1152 = _1124 + _1151;
  int16_t _1153 = _output_cgra_stencil_3 + _1152;
  int16_t _1154 = _1123 + _1153;
  return _1154;
}

//store is: output_cgra.stencil(3, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(3, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(3, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(3, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(3, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(3, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_19(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_49 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_50 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_51 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_52 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_53 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_54 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_55 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_56 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_57 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_58 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_59 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_60 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_61 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_62 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_63 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_64 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_49 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_50 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_51 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_52 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_53 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_54 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_55 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_56 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_57 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_58 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_59 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_60 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_61 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_62 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_63 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_64 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_4 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1256 = _kernel_cgra_stencil_49 * _input_cgra_stencil_49;
  int16_t _1257 = _kernel_cgra_stencil_50 * _input_cgra_stencil_50;
  int16_t _1258 = _kernel_cgra_stencil_51 * _input_cgra_stencil_51;
  int16_t _1259 = _kernel_cgra_stencil_52 * _input_cgra_stencil_52;
  int16_t _1260 = _kernel_cgra_stencil_53 * _input_cgra_stencil_53;
  int16_t _1261 = _kernel_cgra_stencil_54 * _input_cgra_stencil_54;
  int16_t _1262 = _kernel_cgra_stencil_55 * _input_cgra_stencil_55;
  int16_t _1263 = _kernel_cgra_stencil_56 * _input_cgra_stencil_56;
  int16_t _1264 = _kernel_cgra_stencil_57 * _input_cgra_stencil_57;
  int16_t _1265 = _kernel_cgra_stencil_58 * _input_cgra_stencil_58;
  int16_t _1266 = _kernel_cgra_stencil_59 * _input_cgra_stencil_59;
  int16_t _1267 = _kernel_cgra_stencil_60 * _input_cgra_stencil_60;
  int16_t _1268 = _kernel_cgra_stencil_61 * _input_cgra_stencil_61;
  int16_t _1269 = _kernel_cgra_stencil_62 * _input_cgra_stencil_62;
  int16_t _1270 = _kernel_cgra_stencil_63 * _input_cgra_stencil_63;
  int16_t _1271 = _kernel_cgra_stencil_64 * _input_cgra_stencil_64;
  int16_t _1272 = _1270 + _1271;
  int16_t _1273 = _1269 + _1272;
  int16_t _1274 = _1268 + _1273;
  int16_t _1275 = _1267 + _1274;
  int16_t _1276 = _1266 + _1275;
  int16_t _1277 = _1265 + _1276;
  int16_t _1278 = _1264 + _1277;
  int16_t _1279 = _1263 + _1278;
  int16_t _1280 = _1262 + _1279;
  int16_t _1281 = _1261 + _1280;
  int16_t _1282 = _1260 + _1281;
  int16_t _1283 = _1259 + _1282;
  int16_t _1284 = _1258 + _1283;
  int16_t _1285 = _1257 + _1284;
  int16_t _1286 = _output_cgra_stencil_4 + _1285;
  int16_t _1287 = _1256 + _1286;
  return _1287;
}

//store is: output_cgra.stencil(4, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(4, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(4, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(4, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(4, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(4, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_20(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_65 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_66 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_67 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_68 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_69 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_70 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_71 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_72 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_73 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_74 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_75 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_76 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_77 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_78 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_79 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_80 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_65 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_66 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_67 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_68 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_69 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_70 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_71 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_72 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_73 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_74 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_75 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_76 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_77 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_78 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_79 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_80 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_5 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1389 = _kernel_cgra_stencil_65 * _input_cgra_stencil_65;
  int16_t _1390 = _kernel_cgra_stencil_66 * _input_cgra_stencil_66;
  int16_t _1391 = _kernel_cgra_stencil_67 * _input_cgra_stencil_67;
  int16_t _1392 = _kernel_cgra_stencil_68 * _input_cgra_stencil_68;
  int16_t _1393 = _kernel_cgra_stencil_69 * _input_cgra_stencil_69;
  int16_t _1394 = _kernel_cgra_stencil_70 * _input_cgra_stencil_70;
  int16_t _1395 = _kernel_cgra_stencil_71 * _input_cgra_stencil_71;
  int16_t _1396 = _kernel_cgra_stencil_72 * _input_cgra_stencil_72;
  int16_t _1397 = _kernel_cgra_stencil_73 * _input_cgra_stencil_73;
  int16_t _1398 = _kernel_cgra_stencil_74 * _input_cgra_stencil_74;
  int16_t _1399 = _kernel_cgra_stencil_75 * _input_cgra_stencil_75;
  int16_t _1400 = _kernel_cgra_stencil_76 * _input_cgra_stencil_76;
  int16_t _1401 = _kernel_cgra_stencil_77 * _input_cgra_stencil_77;
  int16_t _1402 = _kernel_cgra_stencil_78 * _input_cgra_stencil_78;
  int16_t _1403 = _kernel_cgra_stencil_79 * _input_cgra_stencil_79;
  int16_t _1404 = _kernel_cgra_stencil_80 * _input_cgra_stencil_80;
  int16_t _1405 = _1403 + _1404;
  int16_t _1406 = _1402 + _1405;
  int16_t _1407 = _1401 + _1406;
  int16_t _1408 = _1400 + _1407;
  int16_t _1409 = _1399 + _1408;
  int16_t _1410 = _1398 + _1409;
  int16_t _1411 = _1397 + _1410;
  int16_t _1412 = _1396 + _1411;
  int16_t _1413 = _1395 + _1412;
  int16_t _1414 = _1394 + _1413;
  int16_t _1415 = _1393 + _1414;
  int16_t _1416 = _1392 + _1415;
  int16_t _1417 = _1391 + _1416;
  int16_t _1418 = _1390 + _1417;
  int16_t _1419 = _output_cgra_stencil_5 + _1418;
  int16_t _1420 = _1389 + _1419;
  return _1420;
}

//store is: output_cgra.stencil(5, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(5, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(5, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(5, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(5, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(5, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_21(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_81 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_82 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_83 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_84 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_85 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_86 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_87 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_88 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_89 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_90 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_91 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_92 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_93 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_94 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_95 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_96 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_81 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_82 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_83 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_84 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_85 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_86 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_87 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_88 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_89 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_90 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_91 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_92 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_93 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_94 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_95 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_96 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_6 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1522 = _kernel_cgra_stencil_81 * _input_cgra_stencil_81;
  int16_t _1523 = _kernel_cgra_stencil_82 * _input_cgra_stencil_82;
  int16_t _1524 = _kernel_cgra_stencil_83 * _input_cgra_stencil_83;
  int16_t _1525 = _kernel_cgra_stencil_84 * _input_cgra_stencil_84;
  int16_t _1526 = _kernel_cgra_stencil_85 * _input_cgra_stencil_85;
  int16_t _1527 = _kernel_cgra_stencil_86 * _input_cgra_stencil_86;
  int16_t _1528 = _kernel_cgra_stencil_87 * _input_cgra_stencil_87;
  int16_t _1529 = _kernel_cgra_stencil_88 * _input_cgra_stencil_88;
  int16_t _1530 = _kernel_cgra_stencil_89 * _input_cgra_stencil_89;
  int16_t _1531 = _kernel_cgra_stencil_90 * _input_cgra_stencil_90;
  int16_t _1532 = _kernel_cgra_stencil_91 * _input_cgra_stencil_91;
  int16_t _1533 = _kernel_cgra_stencil_92 * _input_cgra_stencil_92;
  int16_t _1534 = _kernel_cgra_stencil_93 * _input_cgra_stencil_93;
  int16_t _1535 = _kernel_cgra_stencil_94 * _input_cgra_stencil_94;
  int16_t _1536 = _kernel_cgra_stencil_95 * _input_cgra_stencil_95;
  int16_t _1537 = _kernel_cgra_stencil_96 * _input_cgra_stencil_96;
  int16_t _1538 = _1536 + _1537;
  int16_t _1539 = _1535 + _1538;
  int16_t _1540 = _1534 + _1539;
  int16_t _1541 = _1533 + _1540;
  int16_t _1542 = _1532 + _1541;
  int16_t _1543 = _1531 + _1542;
  int16_t _1544 = _1530 + _1543;
  int16_t _1545 = _1529 + _1544;
  int16_t _1546 = _1528 + _1545;
  int16_t _1547 = _1527 + _1546;
  int16_t _1548 = _1526 + _1547;
  int16_t _1549 = _1525 + _1548;
  int16_t _1550 = _1524 + _1549;
  int16_t _1551 = _1523 + _1550;
  int16_t _1552 = _output_cgra_stencil_6 + _1551;
  int16_t _1553 = _1522 + _1552;
  return _1553;
}

//store is: output_cgra.stencil(6, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(6, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(6, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(6, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(6, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(6, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_22(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_100 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_101 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_102 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_103 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_104 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_105 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_106 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_107 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_108 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_109 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_110 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_111 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_112 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_97 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_98 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_99 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_100 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_101 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_102 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_103 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_104 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_105 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_106 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_107 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_108 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_109 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_110 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_111 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_112 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_97 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_98 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_99 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_7 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1655 = _kernel_cgra_stencil_97 * _input_cgra_stencil_97;
  int16_t _1656 = _kernel_cgra_stencil_98 * _input_cgra_stencil_98;
  int16_t _1657 = _kernel_cgra_stencil_99 * _input_cgra_stencil_99;
  int16_t _1658 = _kernel_cgra_stencil_100 * _input_cgra_stencil_100;
  int16_t _1659 = _kernel_cgra_stencil_101 * _input_cgra_stencil_101;
  int16_t _1660 = _kernel_cgra_stencil_102 * _input_cgra_stencil_102;
  int16_t _1661 = _kernel_cgra_stencil_103 * _input_cgra_stencil_103;
  int16_t _1662 = _kernel_cgra_stencil_104 * _input_cgra_stencil_104;
  int16_t _1663 = _kernel_cgra_stencil_105 * _input_cgra_stencil_105;
  int16_t _1664 = _kernel_cgra_stencil_106 * _input_cgra_stencil_106;
  int16_t _1665 = _kernel_cgra_stencil_107 * _input_cgra_stencil_107;
  int16_t _1666 = _kernel_cgra_stencil_108 * _input_cgra_stencil_108;
  int16_t _1667 = _kernel_cgra_stencil_109 * _input_cgra_stencil_109;
  int16_t _1668 = _kernel_cgra_stencil_110 * _input_cgra_stencil_110;
  int16_t _1669 = _kernel_cgra_stencil_111 * _input_cgra_stencil_111;
  int16_t _1670 = _kernel_cgra_stencil_112 * _input_cgra_stencil_112;
  int16_t _1671 = _1669 + _1670;
  int16_t _1672 = _1668 + _1671;
  int16_t _1673 = _1667 + _1672;
  int16_t _1674 = _1666 + _1673;
  int16_t _1675 = _1665 + _1674;
  int16_t _1676 = _1664 + _1675;
  int16_t _1677 = _1663 + _1676;
  int16_t _1678 = _1662 + _1677;
  int16_t _1679 = _1661 + _1678;
  int16_t _1680 = _1660 + _1679;
  int16_t _1681 = _1659 + _1680;
  int16_t _1682 = _1658 + _1681;
  int16_t _1683 = _1657 + _1682;
  int16_t _1684 = _1656 + _1683;
  int16_t _1685 = _output_cgra_stencil_7 + _1684;
  int16_t _1686 = _1655 + _1685;
  return _1686;
}

//store is: output_cgra.stencil(7, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(7, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(7, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(7, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(7, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(7, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_23(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_113 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_114 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_115 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_116 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_117 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_118 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_119 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_120 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_121 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_122 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_123 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_124 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_125 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_126 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_127 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_128 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_113 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_114 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_115 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_116 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_117 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_118 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_119 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_120 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_121 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_122 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_123 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_124 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_125 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_126 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_127 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_128 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_8 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1788 = _kernel_cgra_stencil_113 * _input_cgra_stencil_113;
  int16_t _1789 = _kernel_cgra_stencil_114 * _input_cgra_stencil_114;
  int16_t _1790 = _kernel_cgra_stencil_115 * _input_cgra_stencil_115;
  int16_t _1791 = _kernel_cgra_stencil_116 * _input_cgra_stencil_116;
  int16_t _1792 = _kernel_cgra_stencil_117 * _input_cgra_stencil_117;
  int16_t _1793 = _kernel_cgra_stencil_118 * _input_cgra_stencil_118;
  int16_t _1794 = _kernel_cgra_stencil_119 * _input_cgra_stencil_119;
  int16_t _1795 = _kernel_cgra_stencil_120 * _input_cgra_stencil_120;
  int16_t _1796 = _kernel_cgra_stencil_121 * _input_cgra_stencil_121;
  int16_t _1797 = _kernel_cgra_stencil_122 * _input_cgra_stencil_122;
  int16_t _1798 = _kernel_cgra_stencil_123 * _input_cgra_stencil_123;
  int16_t _1799 = _kernel_cgra_stencil_124 * _input_cgra_stencil_124;
  int16_t _1800 = _kernel_cgra_stencil_125 * _input_cgra_stencil_125;
  int16_t _1801 = _kernel_cgra_stencil_126 * _input_cgra_stencil_126;
  int16_t _1802 = _kernel_cgra_stencil_127 * _input_cgra_stencil_127;
  int16_t _1803 = _kernel_cgra_stencil_128 * _input_cgra_stencil_128;
  int16_t _1804 = _1802 + _1803;
  int16_t _1805 = _1801 + _1804;
  int16_t _1806 = _1800 + _1805;
  int16_t _1807 = _1799 + _1806;
  int16_t _1808 = _1798 + _1807;
  int16_t _1809 = _1797 + _1808;
  int16_t _1810 = _1796 + _1809;
  int16_t _1811 = _1795 + _1810;
  int16_t _1812 = _1794 + _1811;
  int16_t _1813 = _1793 + _1812;
  int16_t _1814 = _1792 + _1813;
  int16_t _1815 = _1791 + _1814;
  int16_t _1816 = _1790 + _1815;
  int16_t _1817 = _1789 + _1816;
  int16_t _1818 = _output_cgra_stencil_8 + _1817;
  int16_t _1819 = _1788 + _1818;
  return _1819;
}

//store is: output_cgra.stencil(8, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(8, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(8, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(8, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(8, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(8, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_24(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_129 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_130 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_131 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_132 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_133 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_134 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_135 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_136 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_137 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_138 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_139 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_140 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_141 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_142 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_143 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_144 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_129 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_130 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_131 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_132 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_133 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_134 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_135 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_136 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_137 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_138 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_139 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_140 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_141 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_142 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_143 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_144 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_9 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _1921 = _kernel_cgra_stencil_129 * _input_cgra_stencil_129;
  int16_t _1922 = _kernel_cgra_stencil_130 * _input_cgra_stencil_130;
  int16_t _1923 = _kernel_cgra_stencil_131 * _input_cgra_stencil_131;
  int16_t _1924 = _kernel_cgra_stencil_132 * _input_cgra_stencil_132;
  int16_t _1925 = _kernel_cgra_stencil_133 * _input_cgra_stencil_133;
  int16_t _1926 = _kernel_cgra_stencil_134 * _input_cgra_stencil_134;
  int16_t _1927 = _kernel_cgra_stencil_135 * _input_cgra_stencil_135;
  int16_t _1928 = _kernel_cgra_stencil_136 * _input_cgra_stencil_136;
  int16_t _1929 = _kernel_cgra_stencil_137 * _input_cgra_stencil_137;
  int16_t _1930 = _kernel_cgra_stencil_138 * _input_cgra_stencil_138;
  int16_t _1931 = _kernel_cgra_stencil_139 * _input_cgra_stencil_139;
  int16_t _1932 = _kernel_cgra_stencil_140 * _input_cgra_stencil_140;
  int16_t _1933 = _kernel_cgra_stencil_141 * _input_cgra_stencil_141;
  int16_t _1934 = _kernel_cgra_stencil_142 * _input_cgra_stencil_142;
  int16_t _1935 = _kernel_cgra_stencil_143 * _input_cgra_stencil_143;
  int16_t _1936 = _kernel_cgra_stencil_144 * _input_cgra_stencil_144;
  int16_t _1937 = _1935 + _1936;
  int16_t _1938 = _1934 + _1937;
  int16_t _1939 = _1933 + _1938;
  int16_t _1940 = _1932 + _1939;
  int16_t _1941 = _1931 + _1940;
  int16_t _1942 = _1930 + _1941;
  int16_t _1943 = _1929 + _1942;
  int16_t _1944 = _1928 + _1943;
  int16_t _1945 = _1927 + _1944;
  int16_t _1946 = _1926 + _1945;
  int16_t _1947 = _1925 + _1946;
  int16_t _1948 = _1924 + _1947;
  int16_t _1949 = _1923 + _1948;
  int16_t _1950 = _1922 + _1949;
  int16_t _1951 = _output_cgra_stencil_9 + _1950;
  int16_t _1952 = _1921 + _1951;
  return _1952;
}

//store is: output_cgra.stencil(9, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(9, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(9, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(9, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(9, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(9, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_25(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_145 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_146 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_147 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_148 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_149 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_150 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_151 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_152 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_153 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_154 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_155 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_156 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_157 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_158 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_159 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_160 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_145 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_146 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_147 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_148 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_149 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_150 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_151 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_152 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_153 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_154 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_155 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_156 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_157 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_158 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_159 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_160 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_10 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2054 = _kernel_cgra_stencil_145 * _input_cgra_stencil_145;
  int16_t _2055 = _kernel_cgra_stencil_146 * _input_cgra_stencil_146;
  int16_t _2056 = _kernel_cgra_stencil_147 * _input_cgra_stencil_147;
  int16_t _2057 = _kernel_cgra_stencil_148 * _input_cgra_stencil_148;
  int16_t _2058 = _kernel_cgra_stencil_149 * _input_cgra_stencil_149;
  int16_t _2059 = _kernel_cgra_stencil_150 * _input_cgra_stencil_150;
  int16_t _2060 = _kernel_cgra_stencil_151 * _input_cgra_stencil_151;
  int16_t _2061 = _kernel_cgra_stencil_152 * _input_cgra_stencil_152;
  int16_t _2062 = _kernel_cgra_stencil_153 * _input_cgra_stencil_153;
  int16_t _2063 = _kernel_cgra_stencil_154 * _input_cgra_stencil_154;
  int16_t _2064 = _kernel_cgra_stencil_155 * _input_cgra_stencil_155;
  int16_t _2065 = _kernel_cgra_stencil_156 * _input_cgra_stencil_156;
  int16_t _2066 = _kernel_cgra_stencil_157 * _input_cgra_stencil_157;
  int16_t _2067 = _kernel_cgra_stencil_158 * _input_cgra_stencil_158;
  int16_t _2068 = _kernel_cgra_stencil_159 * _input_cgra_stencil_159;
  int16_t _2069 = _kernel_cgra_stencil_160 * _input_cgra_stencil_160;
  int16_t _2070 = _2068 + _2069;
  int16_t _2071 = _2067 + _2070;
  int16_t _2072 = _2066 + _2071;
  int16_t _2073 = _2065 + _2072;
  int16_t _2074 = _2064 + _2073;
  int16_t _2075 = _2063 + _2074;
  int16_t _2076 = _2062 + _2075;
  int16_t _2077 = _2061 + _2076;
  int16_t _2078 = _2060 + _2077;
  int16_t _2079 = _2059 + _2078;
  int16_t _2080 = _2058 + _2079;
  int16_t _2081 = _2057 + _2080;
  int16_t _2082 = _2056 + _2081;
  int16_t _2083 = _2055 + _2082;
  int16_t _2084 = _output_cgra_stencil_10 + _2083;
  int16_t _2085 = _2054 + _2084;
  return _2085;
}

//store is: output_cgra.stencil(10, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(10, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(10, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(10, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(10, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(10, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_26(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_161 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_162 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_163 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_164 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_165 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_166 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_167 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_168 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_169 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_170 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_171 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_172 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_173 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_174 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_175 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_176 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_161 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_162 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_163 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_164 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_165 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_166 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_167 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_168 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_169 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_170 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_171 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_172 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_173 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_174 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_175 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_176 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_11 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2187 = _kernel_cgra_stencil_161 * _input_cgra_stencil_161;
  int16_t _2188 = _kernel_cgra_stencil_162 * _input_cgra_stencil_162;
  int16_t _2189 = _kernel_cgra_stencil_163 * _input_cgra_stencil_163;
  int16_t _2190 = _kernel_cgra_stencil_164 * _input_cgra_stencil_164;
  int16_t _2191 = _kernel_cgra_stencil_165 * _input_cgra_stencil_165;
  int16_t _2192 = _kernel_cgra_stencil_166 * _input_cgra_stencil_166;
  int16_t _2193 = _kernel_cgra_stencil_167 * _input_cgra_stencil_167;
  int16_t _2194 = _kernel_cgra_stencil_168 * _input_cgra_stencil_168;
  int16_t _2195 = _kernel_cgra_stencil_169 * _input_cgra_stencil_169;
  int16_t _2196 = _kernel_cgra_stencil_170 * _input_cgra_stencil_170;
  int16_t _2197 = _kernel_cgra_stencil_171 * _input_cgra_stencil_171;
  int16_t _2198 = _kernel_cgra_stencil_172 * _input_cgra_stencil_172;
  int16_t _2199 = _kernel_cgra_stencil_173 * _input_cgra_stencil_173;
  int16_t _2200 = _kernel_cgra_stencil_174 * _input_cgra_stencil_174;
  int16_t _2201 = _kernel_cgra_stencil_175 * _input_cgra_stencil_175;
  int16_t _2202 = _kernel_cgra_stencil_176 * _input_cgra_stencil_176;
  int16_t _2203 = _2201 + _2202;
  int16_t _2204 = _2200 + _2203;
  int16_t _2205 = _2199 + _2204;
  int16_t _2206 = _2198 + _2205;
  int16_t _2207 = _2197 + _2206;
  int16_t _2208 = _2196 + _2207;
  int16_t _2209 = _2195 + _2208;
  int16_t _2210 = _2194 + _2209;
  int16_t _2211 = _2193 + _2210;
  int16_t _2212 = _2192 + _2211;
  int16_t _2213 = _2191 + _2212;
  int16_t _2214 = _2190 + _2213;
  int16_t _2215 = _2189 + _2214;
  int16_t _2216 = _2188 + _2215;
  int16_t _2217 = _output_cgra_stencil_11 + _2216;
  int16_t _2218 = _2187 + _2217;
  return _2218;
}

//store is: output_cgra.stencil(11, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(11, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(11, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(11, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(11, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(11, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_27(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_177 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_178 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_179 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_180 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_181 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_182 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_183 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_184 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_185 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_186 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_187 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_188 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_189 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_190 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_191 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_192 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_177 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_178 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_179 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_180 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_181 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_182 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_183 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_184 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_185 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_186 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_187 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_188 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_189 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_190 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_191 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_192 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_12 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2320 = _kernel_cgra_stencil_177 * _input_cgra_stencil_177;
  int16_t _2321 = _kernel_cgra_stencil_178 * _input_cgra_stencil_178;
  int16_t _2322 = _kernel_cgra_stencil_179 * _input_cgra_stencil_179;
  int16_t _2323 = _kernel_cgra_stencil_180 * _input_cgra_stencil_180;
  int16_t _2324 = _kernel_cgra_stencil_181 * _input_cgra_stencil_181;
  int16_t _2325 = _kernel_cgra_stencil_182 * _input_cgra_stencil_182;
  int16_t _2326 = _kernel_cgra_stencil_183 * _input_cgra_stencil_183;
  int16_t _2327 = _kernel_cgra_stencil_184 * _input_cgra_stencil_184;
  int16_t _2328 = _kernel_cgra_stencil_185 * _input_cgra_stencil_185;
  int16_t _2329 = _kernel_cgra_stencil_186 * _input_cgra_stencil_186;
  int16_t _2330 = _kernel_cgra_stencil_187 * _input_cgra_stencil_187;
  int16_t _2331 = _kernel_cgra_stencil_188 * _input_cgra_stencil_188;
  int16_t _2332 = _kernel_cgra_stencil_189 * _input_cgra_stencil_189;
  int16_t _2333 = _kernel_cgra_stencil_190 * _input_cgra_stencil_190;
  int16_t _2334 = _kernel_cgra_stencil_191 * _input_cgra_stencil_191;
  int16_t _2335 = _kernel_cgra_stencil_192 * _input_cgra_stencil_192;
  int16_t _2336 = _2334 + _2335;
  int16_t _2337 = _2333 + _2336;
  int16_t _2338 = _2332 + _2337;
  int16_t _2339 = _2331 + _2338;
  int16_t _2340 = _2330 + _2339;
  int16_t _2341 = _2329 + _2340;
  int16_t _2342 = _2328 + _2341;
  int16_t _2343 = _2327 + _2342;
  int16_t _2344 = _2326 + _2343;
  int16_t _2345 = _2325 + _2344;
  int16_t _2346 = _2324 + _2345;
  int16_t _2347 = _2323 + _2346;
  int16_t _2348 = _2322 + _2347;
  int16_t _2349 = _2321 + _2348;
  int16_t _2350 = _output_cgra_stencil_12 + _2349;
  int16_t _2351 = _2320 + _2350;
  return _2351;
}

//store is: output_cgra.stencil(12, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(12, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(12, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(12, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(12, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(12, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_28(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_193 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_194 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_195 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_196 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_197 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_198 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_199 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_200 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_201 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_202 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_203 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_204 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_205 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_206 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_207 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_208 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_193 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_194 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_195 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_196 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_197 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_198 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_199 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_200 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_201 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_202 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_203 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_204 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_205 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_206 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_207 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_208 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_13 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2453 = _kernel_cgra_stencil_193 * _input_cgra_stencil_193;
  int16_t _2454 = _kernel_cgra_stencil_194 * _input_cgra_stencil_194;
  int16_t _2455 = _kernel_cgra_stencil_195 * _input_cgra_stencil_195;
  int16_t _2456 = _kernel_cgra_stencil_196 * _input_cgra_stencil_196;
  int16_t _2457 = _kernel_cgra_stencil_197 * _input_cgra_stencil_197;
  int16_t _2458 = _kernel_cgra_stencil_198 * _input_cgra_stencil_198;
  int16_t _2459 = _kernel_cgra_stencil_199 * _input_cgra_stencil_199;
  int16_t _2460 = _kernel_cgra_stencil_200 * _input_cgra_stencil_200;
  int16_t _2461 = _kernel_cgra_stencil_201 * _input_cgra_stencil_201;
  int16_t _2462 = _kernel_cgra_stencil_202 * _input_cgra_stencil_202;
  int16_t _2463 = _kernel_cgra_stencil_203 * _input_cgra_stencil_203;
  int16_t _2464 = _kernel_cgra_stencil_204 * _input_cgra_stencil_204;
  int16_t _2465 = _kernel_cgra_stencil_205 * _input_cgra_stencil_205;
  int16_t _2466 = _kernel_cgra_stencil_206 * _input_cgra_stencil_206;
  int16_t _2467 = _kernel_cgra_stencil_207 * _input_cgra_stencil_207;
  int16_t _2468 = _kernel_cgra_stencil_208 * _input_cgra_stencil_208;
  int16_t _2469 = _2467 + _2468;
  int16_t _2470 = _2466 + _2469;
  int16_t _2471 = _2465 + _2470;
  int16_t _2472 = _2464 + _2471;
  int16_t _2473 = _2463 + _2472;
  int16_t _2474 = _2462 + _2473;
  int16_t _2475 = _2461 + _2474;
  int16_t _2476 = _2460 + _2475;
  int16_t _2477 = _2459 + _2476;
  int16_t _2478 = _2458 + _2477;
  int16_t _2479 = _2457 + _2478;
  int16_t _2480 = _2456 + _2479;
  int16_t _2481 = _2455 + _2480;
  int16_t _2482 = _2454 + _2481;
  int16_t _2483 = _output_cgra_stencil_13 + _2482;
  int16_t _2484 = _2453 + _2483;
  return _2484;
}

//store is: output_cgra.stencil(13, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(13, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(13, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(13, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(13, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(13, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_29(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_209 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_210 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_211 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_212 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_213 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_214 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_215 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_216 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_217 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_218 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_219 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_220 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_221 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_222 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_223 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_224 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_209 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_210 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_211 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_212 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_213 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_214 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_215 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_216 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_217 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_218 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_219 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_220 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_221 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_222 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_223 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_224 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_14 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2586 = _kernel_cgra_stencil_209 * _input_cgra_stencil_209;
  int16_t _2587 = _kernel_cgra_stencil_210 * _input_cgra_stencil_210;
  int16_t _2588 = _kernel_cgra_stencil_211 * _input_cgra_stencil_211;
  int16_t _2589 = _kernel_cgra_stencil_212 * _input_cgra_stencil_212;
  int16_t _2590 = _kernel_cgra_stencil_213 * _input_cgra_stencil_213;
  int16_t _2591 = _kernel_cgra_stencil_214 * _input_cgra_stencil_214;
  int16_t _2592 = _kernel_cgra_stencil_215 * _input_cgra_stencil_215;
  int16_t _2593 = _kernel_cgra_stencil_216 * _input_cgra_stencil_216;
  int16_t _2594 = _kernel_cgra_stencil_217 * _input_cgra_stencil_217;
  int16_t _2595 = _kernel_cgra_stencil_218 * _input_cgra_stencil_218;
  int16_t _2596 = _kernel_cgra_stencil_219 * _input_cgra_stencil_219;
  int16_t _2597 = _kernel_cgra_stencil_220 * _input_cgra_stencil_220;
  int16_t _2598 = _kernel_cgra_stencil_221 * _input_cgra_stencil_221;
  int16_t _2599 = _kernel_cgra_stencil_222 * _input_cgra_stencil_222;
  int16_t _2600 = _kernel_cgra_stencil_223 * _input_cgra_stencil_223;
  int16_t _2601 = _kernel_cgra_stencil_224 * _input_cgra_stencil_224;
  int16_t _2602 = _2600 + _2601;
  int16_t _2603 = _2599 + _2602;
  int16_t _2604 = _2598 + _2603;
  int16_t _2605 = _2597 + _2604;
  int16_t _2606 = _2596 + _2605;
  int16_t _2607 = _2595 + _2606;
  int16_t _2608 = _2594 + _2607;
  int16_t _2609 = _2593 + _2608;
  int16_t _2610 = _2592 + _2609;
  int16_t _2611 = _2591 + _2610;
  int16_t _2612 = _2590 + _2611;
  int16_t _2613 = _2589 + _2612;
  int16_t _2614 = _2588 + _2613;
  int16_t _2615 = _2587 + _2614;
  int16_t _2616 = _output_cgra_stencil_14 + _2615;
  int16_t _2617 = _2586 + _2616;
  return _2617;
}

//store is: output_cgra.stencil(14, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(14, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(14, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(14, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(14, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(14, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_30(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_225 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_226 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_227 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_228 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_229 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_230 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_231 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_232 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_233 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_234 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_235 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_236 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_237 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_238 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_239 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_240 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_225 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_226 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_227 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_228 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_229 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_230 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_231 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_232 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_233 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_234 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_235 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_236 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_237 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_238 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_239 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_240 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_15 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2719 = _kernel_cgra_stencil_225 * _input_cgra_stencil_225;
  int16_t _2720 = _kernel_cgra_stencil_226 * _input_cgra_stencil_226;
  int16_t _2721 = _kernel_cgra_stencil_227 * _input_cgra_stencil_227;
  int16_t _2722 = _kernel_cgra_stencil_228 * _input_cgra_stencil_228;
  int16_t _2723 = _kernel_cgra_stencil_229 * _input_cgra_stencil_229;
  int16_t _2724 = _kernel_cgra_stencil_230 * _input_cgra_stencil_230;
  int16_t _2725 = _kernel_cgra_stencil_231 * _input_cgra_stencil_231;
  int16_t _2726 = _kernel_cgra_stencil_232 * _input_cgra_stencil_232;
  int16_t _2727 = _kernel_cgra_stencil_233 * _input_cgra_stencil_233;
  int16_t _2728 = _kernel_cgra_stencil_234 * _input_cgra_stencil_234;
  int16_t _2729 = _kernel_cgra_stencil_235 * _input_cgra_stencil_235;
  int16_t _2730 = _kernel_cgra_stencil_236 * _input_cgra_stencil_236;
  int16_t _2731 = _kernel_cgra_stencil_237 * _input_cgra_stencil_237;
  int16_t _2732 = _kernel_cgra_stencil_238 * _input_cgra_stencil_238;
  int16_t _2733 = _kernel_cgra_stencil_239 * _input_cgra_stencil_239;
  int16_t _2734 = _kernel_cgra_stencil_240 * _input_cgra_stencil_240;
  int16_t _2735 = _2733 + _2734;
  int16_t _2736 = _2732 + _2735;
  int16_t _2737 = _2731 + _2736;
  int16_t _2738 = _2730 + _2737;
  int16_t _2739 = _2729 + _2738;
  int16_t _2740 = _2728 + _2739;
  int16_t _2741 = _2727 + _2740;
  int16_t _2742 = _2726 + _2741;
  int16_t _2743 = _2725 + _2742;
  int16_t _2744 = _2724 + _2743;
  int16_t _2745 = _2723 + _2744;
  int16_t _2746 = _2722 + _2745;
  int16_t _2747 = _2721 + _2746;
  int16_t _2748 = _2720 + _2747;
  int16_t _2749 = _output_cgra_stencil_15 + _2748;
  int16_t _2750 = _2719 + _2749;
  return _2750;
}

//store is: output_cgra.stencil(15, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(15, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (output_cgra.stencil(15, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(15, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 8, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 9, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 10, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 11, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 12, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 13, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + ((kernel_cgra.stencil(15, 15, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y))) + (kernel_cgra.stencil(15, 14, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, ((output_cgra_s1_x*2) + output_cgra_s1_r_x), ((output_cgra_s1_y*2) + output_cgra_s1_r_y)))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_31(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_241 = (int16_t) (input_cgra_stencil.extract<0, 15>());
  int16_t _input_cgra_stencil_242 = (int16_t) (input_cgra_stencil.extract<16, 31>());
  int16_t _input_cgra_stencil_243 = (int16_t) (input_cgra_stencil.extract<32, 47>());
  int16_t _input_cgra_stencil_244 = (int16_t) (input_cgra_stencil.extract<48, 63>());
  int16_t _input_cgra_stencil_245 = (int16_t) (input_cgra_stencil.extract<64, 79>());
  int16_t _input_cgra_stencil_246 = (int16_t) (input_cgra_stencil.extract<80, 95>());
  int16_t _input_cgra_stencil_247 = (int16_t) (input_cgra_stencil.extract<96, 111>());
  int16_t _input_cgra_stencil_248 = (int16_t) (input_cgra_stencil.extract<112, 127>());
  int16_t _input_cgra_stencil_249 = (int16_t) (input_cgra_stencil.extract<128, 143>());
  int16_t _input_cgra_stencil_250 = (int16_t) (input_cgra_stencil.extract<144, 159>());
  int16_t _input_cgra_stencil_251 = (int16_t) (input_cgra_stencil.extract<160, 175>());
  int16_t _input_cgra_stencil_252 = (int16_t) (input_cgra_stencil.extract<176, 191>());
  int16_t _input_cgra_stencil_253 = (int16_t) (input_cgra_stencil.extract<192, 207>());
  int16_t _input_cgra_stencil_254 = (int16_t) (input_cgra_stencil.extract<208, 223>());
  int16_t _input_cgra_stencil_255 = (int16_t) (input_cgra_stencil.extract<224, 239>());
  int16_t _input_cgra_stencil_256 = (int16_t) (input_cgra_stencil.extract<240, 255>());

  int16_t _kernel_cgra_stencil_241 = (int16_t) (kernel_cgra_stencil.extract<0, 15>());
  int16_t _kernel_cgra_stencil_242 = (int16_t) (kernel_cgra_stencil.extract<16, 31>());
  int16_t _kernel_cgra_stencil_243 = (int16_t) (kernel_cgra_stencil.extract<32, 47>());
  int16_t _kernel_cgra_stencil_244 = (int16_t) (kernel_cgra_stencil.extract<48, 63>());
  int16_t _kernel_cgra_stencil_245 = (int16_t) (kernel_cgra_stencil.extract<64, 79>());
  int16_t _kernel_cgra_stencil_246 = (int16_t) (kernel_cgra_stencil.extract<80, 95>());
  int16_t _kernel_cgra_stencil_247 = (int16_t) (kernel_cgra_stencil.extract<96, 111>());
  int16_t _kernel_cgra_stencil_248 = (int16_t) (kernel_cgra_stencil.extract<112, 127>());
  int16_t _kernel_cgra_stencil_249 = (int16_t) (kernel_cgra_stencil.extract<128, 143>());
  int16_t _kernel_cgra_stencil_250 = (int16_t) (kernel_cgra_stencil.extract<144, 159>());
  int16_t _kernel_cgra_stencil_251 = (int16_t) (kernel_cgra_stencil.extract<160, 175>());
  int16_t _kernel_cgra_stencil_252 = (int16_t) (kernel_cgra_stencil.extract<176, 191>());
  int16_t _kernel_cgra_stencil_253 = (int16_t) (kernel_cgra_stencil.extract<192, 207>());
  int16_t _kernel_cgra_stencil_254 = (int16_t) (kernel_cgra_stencil.extract<208, 223>());
  int16_t _kernel_cgra_stencil_255 = (int16_t) (kernel_cgra_stencil.extract<224, 239>());
  int16_t _kernel_cgra_stencil_256 = (int16_t) (kernel_cgra_stencil.extract<240, 255>());

  int16_t _output_cgra_stencil_16 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  int16_t _2852 = _kernel_cgra_stencil_241 * _input_cgra_stencil_241;
  int16_t _2853 = _kernel_cgra_stencil_242 * _input_cgra_stencil_242;
  int16_t _2854 = _kernel_cgra_stencil_243 * _input_cgra_stencil_243;
  int16_t _2855 = _kernel_cgra_stencil_244 * _input_cgra_stencil_244;
  int16_t _2856 = _kernel_cgra_stencil_245 * _input_cgra_stencil_245;
  int16_t _2857 = _kernel_cgra_stencil_246 * _input_cgra_stencil_246;
  int16_t _2858 = _kernel_cgra_stencil_247 * _input_cgra_stencil_247;
  int16_t _2859 = _kernel_cgra_stencil_248 * _input_cgra_stencil_248;
  int16_t _2860 = _kernel_cgra_stencil_249 * _input_cgra_stencil_249;
  int16_t _2861 = _kernel_cgra_stencil_250 * _input_cgra_stencil_250;
  int16_t _2862 = _kernel_cgra_stencil_251 * _input_cgra_stencil_251;
  int16_t _2863 = _kernel_cgra_stencil_252 * _input_cgra_stencil_252;
  int16_t _2864 = _kernel_cgra_stencil_253 * _input_cgra_stencil_253;
  int16_t _2865 = _kernel_cgra_stencil_254 * _input_cgra_stencil_254;
  int16_t _2866 = _kernel_cgra_stencil_255 * _input_cgra_stencil_255;
  int16_t _2867 = _kernel_cgra_stencil_256 * _input_cgra_stencil_256;
  int16_t _2868 = _2866 + _2867;
  int16_t _2869 = _2865 + _2868;
  int16_t _2870 = _2864 + _2869;
  int16_t _2871 = _2863 + _2870;
  int16_t _2872 = _2862 + _2871;
  int16_t _2873 = _2861 + _2872;
  int16_t _2874 = _2860 + _2873;
  int16_t _2875 = _2859 + _2874;
  int16_t _2876 = _2858 + _2875;
  int16_t _2877 = _2857 + _2876;
  int16_t _2878 = _2856 + _2877;
  int16_t _2879 = _2855 + _2878;
  int16_t _2880 = _2854 + _2879;
  int16_t _2881 = _2853 + _2880;
  int16_t _2882 = _output_cgra_stencil_16 + _2881;
  int16_t _2883 = _2852 + _2882;
  return _2883;
}

//store is: output_glb.stencil((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil((output_glb_s0_w_w_cgra_w_cgra*8), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_17 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_17;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 1), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 1), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_1(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_18 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_18;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 2), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 2), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_2(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_19 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_19;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 3), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 3), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_3(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_20 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_20;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 4), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 4), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_4(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_21 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_21;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 5), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 5), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_5(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_22 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_22;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 6), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 6), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_6(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_23 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_23;
}

//store is: output_glb.stencil(((((output_glb_s0_w_w_glb*2) + output_glb_s0_w_w_cgra_w_cgra)*8) + 7), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(((output_glb_s0_w_w_cgra_w_cgra*8) + 7), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil_7(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_24 = (int16_t) (output_cgra_stencil.extract<0, 15>());

  return _output_cgra_stencil_24;
}

//store is: hw_output.stencil((hw_output_s0_w_w*8), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil((hw_output_s0_w_w*8), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_1 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_1;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 1), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 1), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_1(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_2 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_2;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 2), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 2), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_2(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_3 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_3;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 3), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 3), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_3(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_4 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_4;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 4), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 4), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_4(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_5 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_5;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 5), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 5), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_5(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_6 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_6;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 6), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 6), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_6(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_7 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_7;
}

//store is: hw_output.stencil(((hw_output_s0_w_w*8) + 7), hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(((hw_output_s0_w_w*8) + 7), hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil_7(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_8 = (int16_t) (output_glb_stencil.extract<0, 15>());

  return _output_glb_stencil_8;
}

