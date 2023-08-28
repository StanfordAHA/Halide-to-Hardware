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

//store is: output_cgra.stencil(0, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil() {
  int16_t _691 = (int16_t)(0);
  return _691;
}

//store is: output_cgra.stencil(1, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_1() {
  int16_t _694 = (int16_t)(0);
  return _694;
}

//store is: output_cgra.stencil(2, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_2() {
  int16_t _697 = (int16_t)(0);
  return _697;
}

//store is: output_cgra.stencil(3, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_3() {
  int16_t _700 = (int16_t)(0);
  return _700;
}

//store is: output_cgra.stencil(4, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_4() {
  int16_t _703 = (int16_t)(0);
  return _703;
}

//store is: output_cgra.stencil(5, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_5() {
  int16_t _706 = (int16_t)(0);
  return _706;
}

//store is: output_cgra.stencil(6, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_6() {
  int16_t _709 = (int16_t)(0);
  return _709;
}

//store is: output_cgra.stencil(7, output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_7() {
  int16_t _712 = (int16_t)(0);
  return _712;
}

//store is: input_cgra.stencil(input_cgra_s0_z_z_cgra, input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((output_cgra_s1_r_z_rz_glb*16) + input_cgra_s0_z_z_cgra), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_1 = (int16_t) input_glb_stencil.extract<0, 15>();

  return _input_glb_stencil_1;
}

//store is: kernel_cgra.stencil(kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_w_w_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((output_cgra_s1_r_z_rz_glb*16) + kernel_cgra_s0_z_z_cgra), ((output_glb_s0_w_w_glb*8) + kernel_cgra_s0_w_w_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_1 = (int16_t) kernel_glb_stencil.extract<0, 15>();

  return _kernel_glb_stencil_1;
}

//store is: output_cgra.stencil(0, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(0, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _723 = _kernel_cgra_stencil_1 * _input_cgra_stencil_1;
  int16_t _724 = _kernel_cgra_stencil_2 * _input_cgra_stencil_2;
  int16_t _725 = _kernel_cgra_stencil_3 * _input_cgra_stencil_3;
  int16_t _726 = _kernel_cgra_stencil_4 * _input_cgra_stencil_4;
  int16_t _727 = _kernel_cgra_stencil_5 * _input_cgra_stencil_5;
  int16_t _728 = _kernel_cgra_stencil_6 * _input_cgra_stencil_6;
  int16_t _729 = _kernel_cgra_stencil_7 * _input_cgra_stencil_7;
  int16_t _730 = _kernel_cgra_stencil_8 * _input_cgra_stencil_8;
  int16_t _731 = _kernel_cgra_stencil_9 * _input_cgra_stencil_9;
  int16_t _732 = _kernel_cgra_stencil_10 * _input_cgra_stencil_10;
  int16_t _733 = _kernel_cgra_stencil_11 * _input_cgra_stencil_11;
  int16_t _734 = _kernel_cgra_stencil_12 * _input_cgra_stencil_12;
  int16_t _735 = _kernel_cgra_stencil_13 * _input_cgra_stencil_13;
  int16_t _736 = _kernel_cgra_stencil_14 * _input_cgra_stencil_14;
  int16_t _737 = _kernel_cgra_stencil_15 * _input_cgra_stencil_15;
  int16_t _738 = _kernel_cgra_stencil_16 * _input_cgra_stencil_16;
  int16_t _739 = _737 + _738;
  int16_t _740 = _736 + _739;
  int16_t _741 = _735 + _740;
  int16_t _742 = _734 + _741;
  int16_t _743 = _733 + _742;
  int16_t _744 = _732 + _743;
  int16_t _745 = _731 + _744;
  int16_t _746 = _730 + _745;
  int16_t _747 = _729 + _746;
  int16_t _748 = _728 + _747;
  int16_t _749 = _727 + _748;
  int16_t _750 = _726 + _749;
  int16_t _751 = _725 + _750;
  int16_t _752 = _724 + _751;
  int16_t _753 = _output_cgra_stencil_1 + _752;
  int16_t _754 = _723 + _753;
  return _754;
}

//store is: output_cgra.stencil(1, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(1, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _854 = _kernel_cgra_stencil_17 * _input_cgra_stencil_17;
  int16_t _855 = _kernel_cgra_stencil_18 * _input_cgra_stencil_18;
  int16_t _856 = _kernel_cgra_stencil_19 * _input_cgra_stencil_19;
  int16_t _857 = _kernel_cgra_stencil_20 * _input_cgra_stencil_20;
  int16_t _858 = _kernel_cgra_stencil_21 * _input_cgra_stencil_21;
  int16_t _859 = _kernel_cgra_stencil_22 * _input_cgra_stencil_22;
  int16_t _860 = _kernel_cgra_stencil_23 * _input_cgra_stencil_23;
  int16_t _861 = _kernel_cgra_stencil_24 * _input_cgra_stencil_24;
  int16_t _862 = _kernel_cgra_stencil_25 * _input_cgra_stencil_25;
  int16_t _863 = _kernel_cgra_stencil_26 * _input_cgra_stencil_26;
  int16_t _864 = _kernel_cgra_stencil_27 * _input_cgra_stencil_27;
  int16_t _865 = _kernel_cgra_stencil_28 * _input_cgra_stencil_28;
  int16_t _866 = _kernel_cgra_stencil_29 * _input_cgra_stencil_29;
  int16_t _867 = _kernel_cgra_stencil_30 * _input_cgra_stencil_30;
  int16_t _868 = _kernel_cgra_stencil_31 * _input_cgra_stencil_31;
  int16_t _869 = _kernel_cgra_stencil_32 * _input_cgra_stencil_32;
  int16_t _870 = _868 + _869;
  int16_t _871 = _867 + _870;
  int16_t _872 = _866 + _871;
  int16_t _873 = _865 + _872;
  int16_t _874 = _864 + _873;
  int16_t _875 = _863 + _874;
  int16_t _876 = _862 + _875;
  int16_t _877 = _861 + _876;
  int16_t _878 = _860 + _877;
  int16_t _879 = _859 + _878;
  int16_t _880 = _858 + _879;
  int16_t _881 = _857 + _880;
  int16_t _882 = _856 + _881;
  int16_t _883 = _855 + _882;
  int16_t _884 = _output_cgra_stencil_2 + _883;
  int16_t _885 = _854 + _884;
  return _885;
}

//store is: output_cgra.stencil(2, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(2, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _985 = _kernel_cgra_stencil_33 * _input_cgra_stencil_33;
  int16_t _986 = _kernel_cgra_stencil_34 * _input_cgra_stencil_34;
  int16_t _987 = _kernel_cgra_stencil_35 * _input_cgra_stencil_35;
  int16_t _988 = _kernel_cgra_stencil_36 * _input_cgra_stencil_36;
  int16_t _989 = _kernel_cgra_stencil_37 * _input_cgra_stencil_37;
  int16_t _990 = _kernel_cgra_stencil_38 * _input_cgra_stencil_38;
  int16_t _991 = _kernel_cgra_stencil_39 * _input_cgra_stencil_39;
  int16_t _992 = _kernel_cgra_stencil_40 * _input_cgra_stencil_40;
  int16_t _993 = _kernel_cgra_stencil_41 * _input_cgra_stencil_41;
  int16_t _994 = _kernel_cgra_stencil_42 * _input_cgra_stencil_42;
  int16_t _995 = _kernel_cgra_stencil_43 * _input_cgra_stencil_43;
  int16_t _996 = _kernel_cgra_stencil_44 * _input_cgra_stencil_44;
  int16_t _997 = _kernel_cgra_stencil_45 * _input_cgra_stencil_45;
  int16_t _998 = _kernel_cgra_stencil_46 * _input_cgra_stencil_46;
  int16_t _999 = _kernel_cgra_stencil_47 * _input_cgra_stencil_47;
  int16_t _1000 = _kernel_cgra_stencil_48 * _input_cgra_stencil_48;
  int16_t _1001 = _999 + _1000;
  int16_t _1002 = _998 + _1001;
  int16_t _1003 = _997 + _1002;
  int16_t _1004 = _996 + _1003;
  int16_t _1005 = _995 + _1004;
  int16_t _1006 = _994 + _1005;
  int16_t _1007 = _993 + _1006;
  int16_t _1008 = _992 + _1007;
  int16_t _1009 = _991 + _1008;
  int16_t _1010 = _990 + _1009;
  int16_t _1011 = _989 + _1010;
  int16_t _1012 = _988 + _1011;
  int16_t _1013 = _987 + _1012;
  int16_t _1014 = _986 + _1013;
  int16_t _1015 = _output_cgra_stencil_3 + _1014;
  int16_t _1016 = _985 + _1015;
  return _1016;
}

//store is: output_cgra.stencil(3, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(3, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _1116 = _kernel_cgra_stencil_49 * _input_cgra_stencil_49;
  int16_t _1117 = _kernel_cgra_stencil_50 * _input_cgra_stencil_50;
  int16_t _1118 = _kernel_cgra_stencil_51 * _input_cgra_stencil_51;
  int16_t _1119 = _kernel_cgra_stencil_52 * _input_cgra_stencil_52;
  int16_t _1120 = _kernel_cgra_stencil_53 * _input_cgra_stencil_53;
  int16_t _1121 = _kernel_cgra_stencil_54 * _input_cgra_stencil_54;
  int16_t _1122 = _kernel_cgra_stencil_55 * _input_cgra_stencil_55;
  int16_t _1123 = _kernel_cgra_stencil_56 * _input_cgra_stencil_56;
  int16_t _1124 = _kernel_cgra_stencil_57 * _input_cgra_stencil_57;
  int16_t _1125 = _kernel_cgra_stencil_58 * _input_cgra_stencil_58;
  int16_t _1126 = _kernel_cgra_stencil_59 * _input_cgra_stencil_59;
  int16_t _1127 = _kernel_cgra_stencil_60 * _input_cgra_stencil_60;
  int16_t _1128 = _kernel_cgra_stencil_61 * _input_cgra_stencil_61;
  int16_t _1129 = _kernel_cgra_stencil_62 * _input_cgra_stencil_62;
  int16_t _1130 = _kernel_cgra_stencil_63 * _input_cgra_stencil_63;
  int16_t _1131 = _kernel_cgra_stencil_64 * _input_cgra_stencil_64;
  int16_t _1132 = _1130 + _1131;
  int16_t _1133 = _1129 + _1132;
  int16_t _1134 = _1128 + _1133;
  int16_t _1135 = _1127 + _1134;
  int16_t _1136 = _1126 + _1135;
  int16_t _1137 = _1125 + _1136;
  int16_t _1138 = _1124 + _1137;
  int16_t _1139 = _1123 + _1138;
  int16_t _1140 = _1122 + _1139;
  int16_t _1141 = _1121 + _1140;
  int16_t _1142 = _1120 + _1141;
  int16_t _1143 = _1119 + _1142;
  int16_t _1144 = _1118 + _1143;
  int16_t _1145 = _1117 + _1144;
  int16_t _1146 = _output_cgra_stencil_4 + _1145;
  int16_t _1147 = _1116 + _1146;
  return _1147;
}

//store is: output_cgra.stencil(4, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(4, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _1247 = _kernel_cgra_stencil_65 * _input_cgra_stencil_65;
  int16_t _1248 = _kernel_cgra_stencil_66 * _input_cgra_stencil_66;
  int16_t _1249 = _kernel_cgra_stencil_67 * _input_cgra_stencil_67;
  int16_t _1250 = _kernel_cgra_stencil_68 * _input_cgra_stencil_68;
  int16_t _1251 = _kernel_cgra_stencil_69 * _input_cgra_stencil_69;
  int16_t _1252 = _kernel_cgra_stencil_70 * _input_cgra_stencil_70;
  int16_t _1253 = _kernel_cgra_stencil_71 * _input_cgra_stencil_71;
  int16_t _1254 = _kernel_cgra_stencil_72 * _input_cgra_stencil_72;
  int16_t _1255 = _kernel_cgra_stencil_73 * _input_cgra_stencil_73;
  int16_t _1256 = _kernel_cgra_stencil_74 * _input_cgra_stencil_74;
  int16_t _1257 = _kernel_cgra_stencil_75 * _input_cgra_stencil_75;
  int16_t _1258 = _kernel_cgra_stencil_76 * _input_cgra_stencil_76;
  int16_t _1259 = _kernel_cgra_stencil_77 * _input_cgra_stencil_77;
  int16_t _1260 = _kernel_cgra_stencil_78 * _input_cgra_stencil_78;
  int16_t _1261 = _kernel_cgra_stencil_79 * _input_cgra_stencil_79;
  int16_t _1262 = _kernel_cgra_stencil_80 * _input_cgra_stencil_80;
  int16_t _1263 = _1261 + _1262;
  int16_t _1264 = _1260 + _1263;
  int16_t _1265 = _1259 + _1264;
  int16_t _1266 = _1258 + _1265;
  int16_t _1267 = _1257 + _1266;
  int16_t _1268 = _1256 + _1267;
  int16_t _1269 = _1255 + _1268;
  int16_t _1270 = _1254 + _1269;
  int16_t _1271 = _1253 + _1270;
  int16_t _1272 = _1252 + _1271;
  int16_t _1273 = _1251 + _1272;
  int16_t _1274 = _1250 + _1273;
  int16_t _1275 = _1249 + _1274;
  int16_t _1276 = _1248 + _1275;
  int16_t _1277 = _output_cgra_stencil_5 + _1276;
  int16_t _1278 = _1247 + _1277;
  return _1278;
}

//store is: output_cgra.stencil(5, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(5, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _1378 = _kernel_cgra_stencil_81 * _input_cgra_stencil_81;
  int16_t _1379 = _kernel_cgra_stencil_82 * _input_cgra_stencil_82;
  int16_t _1380 = _kernel_cgra_stencil_83 * _input_cgra_stencil_83;
  int16_t _1381 = _kernel_cgra_stencil_84 * _input_cgra_stencil_84;
  int16_t _1382 = _kernel_cgra_stencil_85 * _input_cgra_stencil_85;
  int16_t _1383 = _kernel_cgra_stencil_86 * _input_cgra_stencil_86;
  int16_t _1384 = _kernel_cgra_stencil_87 * _input_cgra_stencil_87;
  int16_t _1385 = _kernel_cgra_stencil_88 * _input_cgra_stencil_88;
  int16_t _1386 = _kernel_cgra_stencil_89 * _input_cgra_stencil_89;
  int16_t _1387 = _kernel_cgra_stencil_90 * _input_cgra_stencil_90;
  int16_t _1388 = _kernel_cgra_stencil_91 * _input_cgra_stencil_91;
  int16_t _1389 = _kernel_cgra_stencil_92 * _input_cgra_stencil_92;
  int16_t _1390 = _kernel_cgra_stencil_93 * _input_cgra_stencil_93;
  int16_t _1391 = _kernel_cgra_stencil_94 * _input_cgra_stencil_94;
  int16_t _1392 = _kernel_cgra_stencil_95 * _input_cgra_stencil_95;
  int16_t _1393 = _kernel_cgra_stencil_96 * _input_cgra_stencil_96;
  int16_t _1394 = _1392 + _1393;
  int16_t _1395 = _1391 + _1394;
  int16_t _1396 = _1390 + _1395;
  int16_t _1397 = _1389 + _1396;
  int16_t _1398 = _1388 + _1397;
  int16_t _1399 = _1387 + _1398;
  int16_t _1400 = _1386 + _1399;
  int16_t _1401 = _1385 + _1400;
  int16_t _1402 = _1384 + _1401;
  int16_t _1403 = _1383 + _1402;
  int16_t _1404 = _1382 + _1403;
  int16_t _1405 = _1381 + _1404;
  int16_t _1406 = _1380 + _1405;
  int16_t _1407 = _1379 + _1406;
  int16_t _1408 = _output_cgra_stencil_6 + _1407;
  int16_t _1409 = _1378 + _1408;
  return _1409;
}

//store is: output_cgra.stencil(6, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(6, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _1509 = _kernel_cgra_stencil_97 * _input_cgra_stencil_97;
  int16_t _1510 = _kernel_cgra_stencil_98 * _input_cgra_stencil_98;
  int16_t _1511 = _kernel_cgra_stencil_99 * _input_cgra_stencil_99;
  int16_t _1512 = _kernel_cgra_stencil_100 * _input_cgra_stencil_100;
  int16_t _1513 = _kernel_cgra_stencil_101 * _input_cgra_stencil_101;
  int16_t _1514 = _kernel_cgra_stencil_102 * _input_cgra_stencil_102;
  int16_t _1515 = _kernel_cgra_stencil_103 * _input_cgra_stencil_103;
  int16_t _1516 = _kernel_cgra_stencil_104 * _input_cgra_stencil_104;
  int16_t _1517 = _kernel_cgra_stencil_105 * _input_cgra_stencil_105;
  int16_t _1518 = _kernel_cgra_stencil_106 * _input_cgra_stencil_106;
  int16_t _1519 = _kernel_cgra_stencil_107 * _input_cgra_stencil_107;
  int16_t _1520 = _kernel_cgra_stencil_108 * _input_cgra_stencil_108;
  int16_t _1521 = _kernel_cgra_stencil_109 * _input_cgra_stencil_109;
  int16_t _1522 = _kernel_cgra_stencil_110 * _input_cgra_stencil_110;
  int16_t _1523 = _kernel_cgra_stencil_111 * _input_cgra_stencil_111;
  int16_t _1524 = _kernel_cgra_stencil_112 * _input_cgra_stencil_112;
  int16_t _1525 = _1523 + _1524;
  int16_t _1526 = _1522 + _1525;
  int16_t _1527 = _1521 + _1526;
  int16_t _1528 = _1520 + _1527;
  int16_t _1529 = _1519 + _1528;
  int16_t _1530 = _1518 + _1529;
  int16_t _1531 = _1517 + _1530;
  int16_t _1532 = _1516 + _1531;
  int16_t _1533 = _1515 + _1532;
  int16_t _1534 = _1514 + _1533;
  int16_t _1535 = _1513 + _1534;
  int16_t _1536 = _1512 + _1535;
  int16_t _1537 = _1511 + _1536;
  int16_t _1538 = _1510 + _1537;
  int16_t _1539 = _output_cgra_stencil_7 + _1538;
  int16_t _1540 = _1509 + _1539;
  return _1540;
}

//store is: output_cgra.stencil(7, output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(7, output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(6, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(8, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(8, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(9, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(9, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(10, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(10, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(11, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(11, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(12, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(12, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(13, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(13, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(15, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(15, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(14, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(14, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))))))))))
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

  int16_t _1640 = _kernel_cgra_stencil_113 * _input_cgra_stencil_113;
  int16_t _1641 = _kernel_cgra_stencil_114 * _input_cgra_stencil_114;
  int16_t _1642 = _kernel_cgra_stencil_115 * _input_cgra_stencil_115;
  int16_t _1643 = _kernel_cgra_stencil_116 * _input_cgra_stencil_116;
  int16_t _1644 = _kernel_cgra_stencil_117 * _input_cgra_stencil_117;
  int16_t _1645 = _kernel_cgra_stencil_118 * _input_cgra_stencil_118;
  int16_t _1646 = _kernel_cgra_stencil_119 * _input_cgra_stencil_119;
  int16_t _1647 = _kernel_cgra_stencil_120 * _input_cgra_stencil_120;
  int16_t _1648 = _kernel_cgra_stencil_121 * _input_cgra_stencil_121;
  int16_t _1649 = _kernel_cgra_stencil_122 * _input_cgra_stencil_122;
  int16_t _1650 = _kernel_cgra_stencil_123 * _input_cgra_stencil_123;
  int16_t _1651 = _kernel_cgra_stencil_124 * _input_cgra_stencil_124;
  int16_t _1652 = _kernel_cgra_stencil_125 * _input_cgra_stencil_125;
  int16_t _1653 = _kernel_cgra_stencil_126 * _input_cgra_stencil_126;
  int16_t _1654 = _kernel_cgra_stencil_127 * _input_cgra_stencil_127;
  int16_t _1655 = _kernel_cgra_stencil_128 * _input_cgra_stencil_128;
  int16_t _1656 = _1654 + _1655;
  int16_t _1657 = _1653 + _1656;
  int16_t _1658 = _1652 + _1657;
  int16_t _1659 = _1651 + _1658;
  int16_t _1660 = _1650 + _1659;
  int16_t _1661 = _1649 + _1660;
  int16_t _1662 = _1648 + _1661;
  int16_t _1663 = _1647 + _1662;
  int16_t _1664 = _1646 + _1663;
  int16_t _1665 = _1645 + _1664;
  int16_t _1666 = _1644 + _1665;
  int16_t _1667 = _1643 + _1666;
  int16_t _1668 = _1642 + _1667;
  int16_t _1669 = _1641 + _1668;
  int16_t _1670 = _output_cgra_stencil_8 + _1669;
  int16_t _1671 = _1640 + _1670;
  return _1671;
}

//store is: output_glb.stencil(((output_glb_s0_w_w_glb*8) + output_glb_s0_w_w_cgra), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(output_glb_s0_w_w_cgra, output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_9 = (int16_t) output_cgra_stencil.extract<0, 15>();

  return _output_cgra_stencil_9;
}

//store is: hw_output.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_1 = (int16_t) output_glb_stencil.extract<0, 15>();

  return _output_glb_stencil_1;
}

