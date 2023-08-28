#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: input_glb.stencil(input_glb_s0_z, input_glb_s0_x, input_glb_s0_y) = input_host.stencil(input_glb_s0_z, input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil(hw_uint<16>& input_host_stencil) {
  int16_t _input_host_stencil_1 = (int16_t) input_host_stencil.extract<0, 15>();

  return _input_host_stencil_1;
}

//store is: kernel_glb.stencil(kernel_glb_s0_z, kernel_glb_s0_w, kernel_glb_s0_x, kernel_glb_s0_y) = kernel_host.stencil(kernel_glb_s0_z, kernel_glb_s0_w, kernel_glb_s0_x, kernel_glb_s0_y)
hw_uint<16> hcompute_kernel_glb_stencil(hw_uint<16>& kernel_host_stencil) {
  int16_t _kernel_host_stencil_1 = (int16_t) kernel_host_stencil.extract<0, 15>();

  return _kernel_host_stencil_1;
}

//store is: output_cgra.stencil(0, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil() {
  int16_t _691 = (int16_t)(0);
  return _691;
}

//store is: output_cgra.stencil(1, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_1() {
  int16_t _700 = (int16_t)(0);
  return _700;
}

//store is: output_cgra.stencil(2, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_2() {
  int16_t _709 = (int16_t)(0);
  return _709;
}

//store is: output_cgra.stencil(3, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_3() {
  int16_t _718 = (int16_t)(0);
  return _718;
}

//store is: output_cgra.stencil(4, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_4() {
  int16_t _727 = (int16_t)(0);
  return _727;
}

//store is: output_cgra.stencil(5, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_5() {
  int16_t _736 = (int16_t)(0);
  return _736;
}

//store is: output_cgra.stencil(6, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_6() {
  int16_t _745 = (int16_t)(0);
  return _745;
}

//store is: output_cgra.stencil(7, (((output_glb_s0_x_x_glb*14) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*14))) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_7() {
  int16_t _754 = (int16_t)(0);
  return _754;
}

//store is: input_cgra.stencil(input_cgra_s0_z_z_cgra, (((output_glb_s0_x_x_glb*28) + input_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + input_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = input_glb.stencil(((output_cgra_s1_r_z_rz_glb*16) + input_cgra_s0_z_z_cgra), ((output_glb_s0_x_x_glb*28) + input_cgra_s0_x), ((output_glb_s0_y_y_glb*28) + input_cgra_s0_y))
hw_uint<16> hcompute_input_cgra_stencil(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_1 = (int16_t) input_glb_stencil.extract<0, 15>();

  return _input_glb_stencil_1;
}

//store is: kernel_cgra.stencil(kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_w_w_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((output_cgra_s1_r_z_rz_glb*16) + kernel_cgra_s0_z_z_cgra), ((output_glb_s0_w_w_glb*8) + kernel_cgra_s0_w_w_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_1 = (int16_t) kernel_glb_stencil.extract<0, 15>();

  return _kernel_glb_stencil_1;
}

//store is: output_cgra.stencil(0, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(0, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_8(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_1 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_10 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_11 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_12 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_13 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_14 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_15 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_16 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_2 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_3 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_4 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_5 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_6 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_7 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_8 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_9 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_1 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_10 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_11 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_12 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_13 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_14 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_15 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_16 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_2 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_3 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_4 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_5 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_6 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_7 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_8 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_9 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_1 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _781 = _kernel_cgra_stencil_1 * _input_cgra_stencil_1;
  int16_t _782 = _kernel_cgra_stencil_2 * _input_cgra_stencil_2;
  int16_t _783 = _kernel_cgra_stencil_3 * _input_cgra_stencil_3;
  int16_t _784 = _kernel_cgra_stencil_4 * _input_cgra_stencil_4;
  int16_t _785 = _kernel_cgra_stencil_5 * _input_cgra_stencil_5;
  int16_t _786 = _kernel_cgra_stencil_6 * _input_cgra_stencil_6;
  int16_t _787 = _kernel_cgra_stencil_7 * _input_cgra_stencil_7;
  int16_t _788 = _kernel_cgra_stencil_8 * _input_cgra_stencil_8;
  int16_t _789 = _kernel_cgra_stencil_9 * _input_cgra_stencil_9;
  int16_t _790 = _kernel_cgra_stencil_10 * _input_cgra_stencil_10;
  int16_t _791 = _kernel_cgra_stencil_11 * _input_cgra_stencil_11;
  int16_t _792 = _kernel_cgra_stencil_12 * _input_cgra_stencil_12;
  int16_t _793 = _kernel_cgra_stencil_13 * _input_cgra_stencil_13;
  int16_t _794 = _kernel_cgra_stencil_14 * _input_cgra_stencil_14;
  int16_t _795 = _kernel_cgra_stencil_15 * _input_cgra_stencil_15;
  int16_t _796 = _kernel_cgra_stencil_16 * _input_cgra_stencil_16;
  int16_t _797 = _795 + _796;
  int16_t _798 = _794 + _797;
  int16_t _799 = _793 + _798;
  int16_t _800 = _792 + _799;
  int16_t _801 = _791 + _800;
  int16_t _802 = _790 + _801;
  int16_t _803 = _789 + _802;
  int16_t _804 = _788 + _803;
  int16_t _805 = _787 + _804;
  int16_t _806 = _786 + _805;
  int16_t _807 = _785 + _806;
  int16_t _808 = _784 + _807;
  int16_t _809 = _783 + _808;
  int16_t _810 = _782 + _809;
  int16_t _811 = _output_cgra_stencil_1 + _810;
  int16_t _812 = _781 + _811;
  return _812;
}

//store is: output_cgra.stencil(1, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(1, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_9(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_17 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_18 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_19 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_20 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_21 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_22 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_23 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_24 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_25 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_26 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_27 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_28 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_29 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_30 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_31 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_32 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_17 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_18 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_19 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_20 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_21 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_22 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_23 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_24 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_25 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_26 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_27 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_28 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_29 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_30 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_31 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_32 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_2 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _924 = _kernel_cgra_stencil_17 * _input_cgra_stencil_17;
  int16_t _925 = _kernel_cgra_stencil_18 * _input_cgra_stencil_18;
  int16_t _926 = _kernel_cgra_stencil_19 * _input_cgra_stencil_19;
  int16_t _927 = _kernel_cgra_stencil_20 * _input_cgra_stencil_20;
  int16_t _928 = _kernel_cgra_stencil_21 * _input_cgra_stencil_21;
  int16_t _929 = _kernel_cgra_stencil_22 * _input_cgra_stencil_22;
  int16_t _930 = _kernel_cgra_stencil_23 * _input_cgra_stencil_23;
  int16_t _931 = _kernel_cgra_stencil_24 * _input_cgra_stencil_24;
  int16_t _932 = _kernel_cgra_stencil_25 * _input_cgra_stencil_25;
  int16_t _933 = _kernel_cgra_stencil_26 * _input_cgra_stencil_26;
  int16_t _934 = _kernel_cgra_stencil_27 * _input_cgra_stencil_27;
  int16_t _935 = _kernel_cgra_stencil_28 * _input_cgra_stencil_28;
  int16_t _936 = _kernel_cgra_stencil_29 * _input_cgra_stencil_29;
  int16_t _937 = _kernel_cgra_stencil_30 * _input_cgra_stencil_30;
  int16_t _938 = _kernel_cgra_stencil_31 * _input_cgra_stencil_31;
  int16_t _939 = _kernel_cgra_stencil_32 * _input_cgra_stencil_32;
  int16_t _940 = _938 + _939;
  int16_t _941 = _937 + _940;
  int16_t _942 = _936 + _941;
  int16_t _943 = _935 + _942;
  int16_t _944 = _934 + _943;
  int16_t _945 = _933 + _944;
  int16_t _946 = _932 + _945;
  int16_t _947 = _931 + _946;
  int16_t _948 = _930 + _947;
  int16_t _949 = _929 + _948;
  int16_t _950 = _928 + _949;
  int16_t _951 = _927 + _950;
  int16_t _952 = _926 + _951;
  int16_t _953 = _925 + _952;
  int16_t _954 = _output_cgra_stencil_2 + _953;
  int16_t _955 = _924 + _954;
  return _955;
}

//store is: output_cgra.stencil(2, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(2, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_10(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_33 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_34 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_35 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_36 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_37 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_38 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_39 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_40 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_41 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_42 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_43 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_44 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_45 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_46 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_47 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_48 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_33 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_34 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_35 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_36 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_37 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_38 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_39 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_40 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_41 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_42 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_43 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_44 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_45 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_46 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_47 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_48 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_3 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1067 = _kernel_cgra_stencil_33 * _input_cgra_stencil_33;
  int16_t _1068 = _kernel_cgra_stencil_34 * _input_cgra_stencil_34;
  int16_t _1069 = _kernel_cgra_stencil_35 * _input_cgra_stencil_35;
  int16_t _1070 = _kernel_cgra_stencil_36 * _input_cgra_stencil_36;
  int16_t _1071 = _kernel_cgra_stencil_37 * _input_cgra_stencil_37;
  int16_t _1072 = _kernel_cgra_stencil_38 * _input_cgra_stencil_38;
  int16_t _1073 = _kernel_cgra_stencil_39 * _input_cgra_stencil_39;
  int16_t _1074 = _kernel_cgra_stencil_40 * _input_cgra_stencil_40;
  int16_t _1075 = _kernel_cgra_stencil_41 * _input_cgra_stencil_41;
  int16_t _1076 = _kernel_cgra_stencil_42 * _input_cgra_stencil_42;
  int16_t _1077 = _kernel_cgra_stencil_43 * _input_cgra_stencil_43;
  int16_t _1078 = _kernel_cgra_stencil_44 * _input_cgra_stencil_44;
  int16_t _1079 = _kernel_cgra_stencil_45 * _input_cgra_stencil_45;
  int16_t _1080 = _kernel_cgra_stencil_46 * _input_cgra_stencil_46;
  int16_t _1081 = _kernel_cgra_stencil_47 * _input_cgra_stencil_47;
  int16_t _1082 = _kernel_cgra_stencil_48 * _input_cgra_stencil_48;
  int16_t _1083 = _1081 + _1082;
  int16_t _1084 = _1080 + _1083;
  int16_t _1085 = _1079 + _1084;
  int16_t _1086 = _1078 + _1085;
  int16_t _1087 = _1077 + _1086;
  int16_t _1088 = _1076 + _1087;
  int16_t _1089 = _1075 + _1088;
  int16_t _1090 = _1074 + _1089;
  int16_t _1091 = _1073 + _1090;
  int16_t _1092 = _1072 + _1091;
  int16_t _1093 = _1071 + _1092;
  int16_t _1094 = _1070 + _1093;
  int16_t _1095 = _1069 + _1094;
  int16_t _1096 = _1068 + _1095;
  int16_t _1097 = _output_cgra_stencil_3 + _1096;
  int16_t _1098 = _1067 + _1097;
  return _1098;
}

//store is: output_cgra.stencil(3, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(3, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_11(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_49 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_50 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_51 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_52 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_53 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_54 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_55 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_56 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_57 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_58 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_59 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_60 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_61 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_62 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_63 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_64 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_49 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_50 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_51 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_52 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_53 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_54 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_55 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_56 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_57 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_58 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_59 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_60 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_61 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_62 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_63 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_64 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_4 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1210 = _kernel_cgra_stencil_49 * _input_cgra_stencil_49;
  int16_t _1211 = _kernel_cgra_stencil_50 * _input_cgra_stencil_50;
  int16_t _1212 = _kernel_cgra_stencil_51 * _input_cgra_stencil_51;
  int16_t _1213 = _kernel_cgra_stencil_52 * _input_cgra_stencil_52;
  int16_t _1214 = _kernel_cgra_stencil_53 * _input_cgra_stencil_53;
  int16_t _1215 = _kernel_cgra_stencil_54 * _input_cgra_stencil_54;
  int16_t _1216 = _kernel_cgra_stencil_55 * _input_cgra_stencil_55;
  int16_t _1217 = _kernel_cgra_stencil_56 * _input_cgra_stencil_56;
  int16_t _1218 = _kernel_cgra_stencil_57 * _input_cgra_stencil_57;
  int16_t _1219 = _kernel_cgra_stencil_58 * _input_cgra_stencil_58;
  int16_t _1220 = _kernel_cgra_stencil_59 * _input_cgra_stencil_59;
  int16_t _1221 = _kernel_cgra_stencil_60 * _input_cgra_stencil_60;
  int16_t _1222 = _kernel_cgra_stencil_61 * _input_cgra_stencil_61;
  int16_t _1223 = _kernel_cgra_stencil_62 * _input_cgra_stencil_62;
  int16_t _1224 = _kernel_cgra_stencil_63 * _input_cgra_stencil_63;
  int16_t _1225 = _kernel_cgra_stencil_64 * _input_cgra_stencil_64;
  int16_t _1226 = _1224 + _1225;
  int16_t _1227 = _1223 + _1226;
  int16_t _1228 = _1222 + _1227;
  int16_t _1229 = _1221 + _1228;
  int16_t _1230 = _1220 + _1229;
  int16_t _1231 = _1219 + _1230;
  int16_t _1232 = _1218 + _1231;
  int16_t _1233 = _1217 + _1232;
  int16_t _1234 = _1216 + _1233;
  int16_t _1235 = _1215 + _1234;
  int16_t _1236 = _1214 + _1235;
  int16_t _1237 = _1213 + _1236;
  int16_t _1238 = _1212 + _1237;
  int16_t _1239 = _1211 + _1238;
  int16_t _1240 = _output_cgra_stencil_4 + _1239;
  int16_t _1241 = _1210 + _1240;
  return _1241;
}

//store is: output_cgra.stencil(4, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(4, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_12(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_65 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_66 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_67 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_68 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_69 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_70 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_71 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_72 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_73 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_74 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_75 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_76 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_77 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_78 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_79 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_80 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_65 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_66 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_67 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_68 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_69 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_70 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_71 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_72 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_73 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_74 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_75 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_76 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_77 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_78 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_79 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_80 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_5 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1353 = _kernel_cgra_stencil_65 * _input_cgra_stencil_65;
  int16_t _1354 = _kernel_cgra_stencil_66 * _input_cgra_stencil_66;
  int16_t _1355 = _kernel_cgra_stencil_67 * _input_cgra_stencil_67;
  int16_t _1356 = _kernel_cgra_stencil_68 * _input_cgra_stencil_68;
  int16_t _1357 = _kernel_cgra_stencil_69 * _input_cgra_stencil_69;
  int16_t _1358 = _kernel_cgra_stencil_70 * _input_cgra_stencil_70;
  int16_t _1359 = _kernel_cgra_stencil_71 * _input_cgra_stencil_71;
  int16_t _1360 = _kernel_cgra_stencil_72 * _input_cgra_stencil_72;
  int16_t _1361 = _kernel_cgra_stencil_73 * _input_cgra_stencil_73;
  int16_t _1362 = _kernel_cgra_stencil_74 * _input_cgra_stencil_74;
  int16_t _1363 = _kernel_cgra_stencil_75 * _input_cgra_stencil_75;
  int16_t _1364 = _kernel_cgra_stencil_76 * _input_cgra_stencil_76;
  int16_t _1365 = _kernel_cgra_stencil_77 * _input_cgra_stencil_77;
  int16_t _1366 = _kernel_cgra_stencil_78 * _input_cgra_stencil_78;
  int16_t _1367 = _kernel_cgra_stencil_79 * _input_cgra_stencil_79;
  int16_t _1368 = _kernel_cgra_stencil_80 * _input_cgra_stencil_80;
  int16_t _1369 = _1367 + _1368;
  int16_t _1370 = _1366 + _1369;
  int16_t _1371 = _1365 + _1370;
  int16_t _1372 = _1364 + _1371;
  int16_t _1373 = _1363 + _1372;
  int16_t _1374 = _1362 + _1373;
  int16_t _1375 = _1361 + _1374;
  int16_t _1376 = _1360 + _1375;
  int16_t _1377 = _1359 + _1376;
  int16_t _1378 = _1358 + _1377;
  int16_t _1379 = _1357 + _1378;
  int16_t _1380 = _1356 + _1379;
  int16_t _1381 = _1355 + _1380;
  int16_t _1382 = _1354 + _1381;
  int16_t _1383 = _output_cgra_stencil_5 + _1382;
  int16_t _1384 = _1353 + _1383;
  return _1384;
}

//store is: output_cgra.stencil(5, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(5, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_13(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_81 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_82 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_83 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_84 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_85 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_86 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_87 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_88 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_89 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_90 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_91 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_92 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_93 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_94 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_95 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_96 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_81 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_82 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_83 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_84 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_85 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_86 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_87 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_88 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_89 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_90 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_91 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_92 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_93 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_94 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_95 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_96 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_6 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1496 = _kernel_cgra_stencil_81 * _input_cgra_stencil_81;
  int16_t _1497 = _kernel_cgra_stencil_82 * _input_cgra_stencil_82;
  int16_t _1498 = _kernel_cgra_stencil_83 * _input_cgra_stencil_83;
  int16_t _1499 = _kernel_cgra_stencil_84 * _input_cgra_stencil_84;
  int16_t _1500 = _kernel_cgra_stencil_85 * _input_cgra_stencil_85;
  int16_t _1501 = _kernel_cgra_stencil_86 * _input_cgra_stencil_86;
  int16_t _1502 = _kernel_cgra_stencil_87 * _input_cgra_stencil_87;
  int16_t _1503 = _kernel_cgra_stencil_88 * _input_cgra_stencil_88;
  int16_t _1504 = _kernel_cgra_stencil_89 * _input_cgra_stencil_89;
  int16_t _1505 = _kernel_cgra_stencil_90 * _input_cgra_stencil_90;
  int16_t _1506 = _kernel_cgra_stencil_91 * _input_cgra_stencil_91;
  int16_t _1507 = _kernel_cgra_stencil_92 * _input_cgra_stencil_92;
  int16_t _1508 = _kernel_cgra_stencil_93 * _input_cgra_stencil_93;
  int16_t _1509 = _kernel_cgra_stencil_94 * _input_cgra_stencil_94;
  int16_t _1510 = _kernel_cgra_stencil_95 * _input_cgra_stencil_95;
  int16_t _1511 = _kernel_cgra_stencil_96 * _input_cgra_stencil_96;
  int16_t _1512 = _1510 + _1511;
  int16_t _1513 = _1509 + _1512;
  int16_t _1514 = _1508 + _1513;
  int16_t _1515 = _1507 + _1514;
  int16_t _1516 = _1506 + _1515;
  int16_t _1517 = _1505 + _1516;
  int16_t _1518 = _1504 + _1517;
  int16_t _1519 = _1503 + _1518;
  int16_t _1520 = _1502 + _1519;
  int16_t _1521 = _1501 + _1520;
  int16_t _1522 = _1500 + _1521;
  int16_t _1523 = _1499 + _1522;
  int16_t _1524 = _1498 + _1523;
  int16_t _1525 = _1497 + _1524;
  int16_t _1526 = _output_cgra_stencil_6 + _1525;
  int16_t _1527 = _1496 + _1526;
  return _1527;
}

//store is: output_cgra.stencil(6, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(6, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_14(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_100 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_101 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_102 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_103 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_104 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_105 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_106 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_107 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_108 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_109 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_110 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_111 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_112 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_97 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_98 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_99 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_100 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_101 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_102 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_103 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_104 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_105 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_106 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_107 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_108 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_109 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_110 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_111 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_112 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_97 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_98 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_99 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_7 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1639 = _kernel_cgra_stencil_97 * _input_cgra_stencil_97;
  int16_t _1640 = _kernel_cgra_stencil_98 * _input_cgra_stencil_98;
  int16_t _1641 = _kernel_cgra_stencil_99 * _input_cgra_stencil_99;
  int16_t _1642 = _kernel_cgra_stencil_100 * _input_cgra_stencil_100;
  int16_t _1643 = _kernel_cgra_stencil_101 * _input_cgra_stencil_101;
  int16_t _1644 = _kernel_cgra_stencil_102 * _input_cgra_stencil_102;
  int16_t _1645 = _kernel_cgra_stencil_103 * _input_cgra_stencil_103;
  int16_t _1646 = _kernel_cgra_stencil_104 * _input_cgra_stencil_104;
  int16_t _1647 = _kernel_cgra_stencil_105 * _input_cgra_stencil_105;
  int16_t _1648 = _kernel_cgra_stencil_106 * _input_cgra_stencil_106;
  int16_t _1649 = _kernel_cgra_stencil_107 * _input_cgra_stencil_107;
  int16_t _1650 = _kernel_cgra_stencil_108 * _input_cgra_stencil_108;
  int16_t _1651 = _kernel_cgra_stencil_109 * _input_cgra_stencil_109;
  int16_t _1652 = _kernel_cgra_stencil_110 * _input_cgra_stencil_110;
  int16_t _1653 = _kernel_cgra_stencil_111 * _input_cgra_stencil_111;
  int16_t _1654 = _kernel_cgra_stencil_112 * _input_cgra_stencil_112;
  int16_t _1655 = _1653 + _1654;
  int16_t _1656 = _1652 + _1655;
  int16_t _1657 = _1651 + _1656;
  int16_t _1658 = _1650 + _1657;
  int16_t _1659 = _1649 + _1658;
  int16_t _1660 = _1648 + _1659;
  int16_t _1661 = _1647 + _1660;
  int16_t _1662 = _1646 + _1661;
  int16_t _1663 = _1645 + _1662;
  int16_t _1664 = _1644 + _1663;
  int16_t _1665 = _1643 + _1664;
  int16_t _1666 = _1642 + _1665;
  int16_t _1667 = _1641 + _1666;
  int16_t _1668 = _1640 + _1667;
  int16_t _1669 = _output_cgra_stencil_7 + _1668;
  int16_t _1670 = _1639 + _1669;
  return _1670;
}

//store is: output_cgra.stencil(7, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) = ((kernel_cgra.stencil(0, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(7, (((output_glb_s0_x_x_glb*14) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*14)), (((output_glb_s0_y_y_glb*14) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*14))) + ((kernel_cgra.stencil(1, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(6, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(8, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(9, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(10, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(11, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(12, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(13, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(15, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(14, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (((((output_glb_s0_x_x_glb*14) + output_cgra_s1_x)*2) + output_cgra_s1_r_x) - (output_glb_s0_x_x_glb*28)), (((((output_glb_s0_y_y_glb*14) + output_cgra_s1_y)*2) + output_cgra_s1_r_y) - (output_glb_s0_y_y_glb*28))))))))))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_15(hw_uint<256>& input_cgra_stencil, hw_uint<256>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_113 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_114 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_115 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_116 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_117 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_118 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_119 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_120 = (int16_t) input_cgra_stencil.extract<112, 127>();
  int16_t _input_cgra_stencil_121 = (int16_t) input_cgra_stencil.extract<128, 143>();
  int16_t _input_cgra_stencil_122 = (int16_t) input_cgra_stencil.extract<144, 159>();
  int16_t _input_cgra_stencil_123 = (int16_t) input_cgra_stencil.extract<160, 175>();
  int16_t _input_cgra_stencil_124 = (int16_t) input_cgra_stencil.extract<176, 191>();
  int16_t _input_cgra_stencil_125 = (int16_t) input_cgra_stencil.extract<192, 207>();
  int16_t _input_cgra_stencil_126 = (int16_t) input_cgra_stencil.extract<208, 223>();
  int16_t _input_cgra_stencil_127 = (int16_t) input_cgra_stencil.extract<224, 239>();
  int16_t _input_cgra_stencil_128 = (int16_t) input_cgra_stencil.extract<240, 255>();

  int16_t _kernel_cgra_stencil_113 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_114 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_115 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_116 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_117 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_118 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_119 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_120 = (int16_t) kernel_cgra_stencil.extract<112, 127>();
  int16_t _kernel_cgra_stencil_121 = (int16_t) kernel_cgra_stencil.extract<128, 143>();
  int16_t _kernel_cgra_stencil_122 = (int16_t) kernel_cgra_stencil.extract<144, 159>();
  int16_t _kernel_cgra_stencil_123 = (int16_t) kernel_cgra_stencil.extract<160, 175>();
  int16_t _kernel_cgra_stencil_124 = (int16_t) kernel_cgra_stencil.extract<176, 191>();
  int16_t _kernel_cgra_stencil_125 = (int16_t) kernel_cgra_stencil.extract<192, 207>();
  int16_t _kernel_cgra_stencil_126 = (int16_t) kernel_cgra_stencil.extract<208, 223>();
  int16_t _kernel_cgra_stencil_127 = (int16_t) kernel_cgra_stencil.extract<224, 239>();
  int16_t _kernel_cgra_stencil_128 = (int16_t) kernel_cgra_stencil.extract<240, 255>();

  int16_t _output_cgra_stencil_8 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1782 = _kernel_cgra_stencil_113 * _input_cgra_stencil_113;
  int16_t _1783 = _kernel_cgra_stencil_114 * _input_cgra_stencil_114;
  int16_t _1784 = _kernel_cgra_stencil_115 * _input_cgra_stencil_115;
  int16_t _1785 = _kernel_cgra_stencil_116 * _input_cgra_stencil_116;
  int16_t _1786 = _kernel_cgra_stencil_117 * _input_cgra_stencil_117;
  int16_t _1787 = _kernel_cgra_stencil_118 * _input_cgra_stencil_118;
  int16_t _1788 = _kernel_cgra_stencil_119 * _input_cgra_stencil_119;
  int16_t _1789 = _kernel_cgra_stencil_120 * _input_cgra_stencil_120;
  int16_t _1790 = _kernel_cgra_stencil_121 * _input_cgra_stencil_121;
  int16_t _1791 = _kernel_cgra_stencil_122 * _input_cgra_stencil_122;
  int16_t _1792 = _kernel_cgra_stencil_123 * _input_cgra_stencil_123;
  int16_t _1793 = _kernel_cgra_stencil_124 * _input_cgra_stencil_124;
  int16_t _1794 = _kernel_cgra_stencil_125 * _input_cgra_stencil_125;
  int16_t _1795 = _kernel_cgra_stencil_126 * _input_cgra_stencil_126;
  int16_t _1796 = _kernel_cgra_stencil_127 * _input_cgra_stencil_127;
  int16_t _1797 = _kernel_cgra_stencil_128 * _input_cgra_stencil_128;
  int16_t _1798 = _1796 + _1797;
  int16_t _1799 = _1795 + _1798;
  int16_t _1800 = _1794 + _1799;
  int16_t _1801 = _1793 + _1800;
  int16_t _1802 = _1792 + _1801;
  int16_t _1803 = _1791 + _1802;
  int16_t _1804 = _1790 + _1803;
  int16_t _1805 = _1789 + _1804;
  int16_t _1806 = _1788 + _1805;
  int16_t _1807 = _1787 + _1806;
  int16_t _1808 = _1786 + _1807;
  int16_t _1809 = _1785 + _1808;
  int16_t _1810 = _1784 + _1809;
  int16_t _1811 = _1783 + _1810;
  int16_t _1812 = _output_cgra_stencil_8 + _1811;
  int16_t _1813 = _1782 + _1812;
  return _1813;
}

//store is: output_glb.stencil(((output_glb_s0_w_w_glb*8) + output_glb_s0_w_w_cgra), ((output_glb_s0_x_x_glb*14) + output_glb_s0_x_x_cgra), ((output_glb_s0_y_y_glb*14) + output_glb_s0_y_y_cgra)) = output_cgra.stencil(output_glb_s0_w_w_cgra, output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_9 = (int16_t) output_cgra_stencil.extract<0, 15>();

  return _output_cgra_stencil_9;
}

//store is: hw_output.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_1 = (int16_t) output_glb_stencil.extract<0, 15>();

  return _output_glb_stencil_1;
}

