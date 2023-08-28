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

//store is: output_cgra.stencil((output_cgra_s0_w_w*8), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil() {
  int16_t _689 = (int16_t)(0);
  return _689;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 1), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_1() {
  int16_t _693 = (int16_t)(0);
  return _693;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 2), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_2() {
  int16_t _698 = (int16_t)(0);
  return _698;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 3), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_3() {
  int16_t _703 = (int16_t)(0);
  return _703;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 4), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_4() {
  int16_t _708 = (int16_t)(0);
  return _708;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 5), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_5() {
  int16_t _713 = (int16_t)(0);
  return _713;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 6), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_6() {
  int16_t _718 = (int16_t)(0);
  return _718;
}

//store is: output_cgra.stencil(((output_cgra_s0_w_w*8) + 7), output_cgra_s0_x, output_cgra_s0_y) = (int16)0
hw_uint<16> hcompute_output_cgra_stencil_7() {
  int16_t _723 = (int16_t)(0);
  return _723;
}

//store is: input_cgra.stencil(input_cgra_s0_z_z_cgra, input_cgra_s0_x, input_cgra_s0_y) = input_glb.stencil(((output_cgra_s1_r_z_rz_glb*8) + input_cgra_s0_z_z_cgra), input_cgra_s0_x, input_cgra_s0_y)
hw_uint<16> hcompute_input_cgra_stencil(hw_uint<16>& input_glb_stencil) {
  int16_t _input_glb_stencil_1 = (int16_t) input_glb_stencil.extract<0, 15>();

  return _input_glb_stencil_1;
}

//store is: kernel_cgra.stencil(kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_w_w_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((output_cgra_s1_r_z_rz_glb*8) + kernel_cgra_s0_z_z_cgra), ((output_glb_s0_w_w_glb*32) + kernel_cgra_s0_w_w_cgra), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil(hw_uint<16>& kernel_glb_stencil) {
  int16_t _kernel_glb_stencil_1 = (int16_t) kernel_glb_stencil.extract<0, 15>();

  return _kernel_glb_stencil_1;
}

//store is: output_cgra.stencil((output_cgra_s1_w_w*8), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil((output_cgra_s1_w_w*8), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, (output_cgra_s1_w_w*8), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_8(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_1 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_2 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_3 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_4 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_5 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_6 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_7 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_8 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_1 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_2 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_3 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_4 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_5 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_6 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_7 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_8 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_1 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _741 = _kernel_cgra_stencil_1 * _input_cgra_stencil_1;
  int16_t _742 = _kernel_cgra_stencil_2 * _input_cgra_stencil_2;
  int16_t _743 = _kernel_cgra_stencil_3 * _input_cgra_stencil_3;
  int16_t _744 = _kernel_cgra_stencil_4 * _input_cgra_stencil_4;
  int16_t _745 = _kernel_cgra_stencil_5 * _input_cgra_stencil_5;
  int16_t _746 = _kernel_cgra_stencil_6 * _input_cgra_stencil_6;
  int16_t _747 = _kernel_cgra_stencil_7 * _input_cgra_stencil_7;
  int16_t _748 = _kernel_cgra_stencil_8 * _input_cgra_stencil_8;
  int16_t _749 = _747 + _748;
  int16_t _750 = _746 + _749;
  int16_t _751 = _745 + _750;
  int16_t _752 = _744 + _751;
  int16_t _753 = _743 + _752;
  int16_t _754 = _742 + _753;
  int16_t _755 = _output_cgra_stencil_1 + _754;
  int16_t _756 = _741 + _755;
  return _756;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 1), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 1), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 1), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_9(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_10 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_11 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_12 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_13 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_14 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_15 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_16 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_9 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_10 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_11 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_12 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_13 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_14 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_15 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_16 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_9 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_2 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _809 = _kernel_cgra_stencil_9 * _input_cgra_stencil_9;
  int16_t _810 = _kernel_cgra_stencil_10 * _input_cgra_stencil_10;
  int16_t _811 = _kernel_cgra_stencil_11 * _input_cgra_stencil_11;
  int16_t _812 = _kernel_cgra_stencil_12 * _input_cgra_stencil_12;
  int16_t _813 = _kernel_cgra_stencil_13 * _input_cgra_stencil_13;
  int16_t _814 = _kernel_cgra_stencil_14 * _input_cgra_stencil_14;
  int16_t _815 = _kernel_cgra_stencil_15 * _input_cgra_stencil_15;
  int16_t _816 = _kernel_cgra_stencil_16 * _input_cgra_stencil_16;
  int16_t _817 = _815 + _816;
  int16_t _818 = _814 + _817;
  int16_t _819 = _813 + _818;
  int16_t _820 = _812 + _819;
  int16_t _821 = _811 + _820;
  int16_t _822 = _810 + _821;
  int16_t _823 = _output_cgra_stencil_2 + _822;
  int16_t _824 = _809 + _823;
  return _824;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 2), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 2), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 2), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_10(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_17 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_18 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_19 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_20 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_21 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_22 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_23 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_24 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_17 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_18 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_19 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_20 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_21 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_22 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_23 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_24 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_3 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _878 = _kernel_cgra_stencil_17 * _input_cgra_stencil_17;
  int16_t _879 = _kernel_cgra_stencil_18 * _input_cgra_stencil_18;
  int16_t _880 = _kernel_cgra_stencil_19 * _input_cgra_stencil_19;
  int16_t _881 = _kernel_cgra_stencil_20 * _input_cgra_stencil_20;
  int16_t _882 = _kernel_cgra_stencil_21 * _input_cgra_stencil_21;
  int16_t _883 = _kernel_cgra_stencil_22 * _input_cgra_stencil_22;
  int16_t _884 = _kernel_cgra_stencil_23 * _input_cgra_stencil_23;
  int16_t _885 = _kernel_cgra_stencil_24 * _input_cgra_stencil_24;
  int16_t _886 = _884 + _885;
  int16_t _887 = _883 + _886;
  int16_t _888 = _882 + _887;
  int16_t _889 = _881 + _888;
  int16_t _890 = _880 + _889;
  int16_t _891 = _879 + _890;
  int16_t _892 = _output_cgra_stencil_3 + _891;
  int16_t _893 = _878 + _892;
  return _893;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 3), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 3), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 3), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_11(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_25 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_26 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_27 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_28 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_29 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_30 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_31 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_32 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_25 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_26 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_27 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_28 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_29 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_30 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_31 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_32 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_4 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _947 = _kernel_cgra_stencil_25 * _input_cgra_stencil_25;
  int16_t _948 = _kernel_cgra_stencil_26 * _input_cgra_stencil_26;
  int16_t _949 = _kernel_cgra_stencil_27 * _input_cgra_stencil_27;
  int16_t _950 = _kernel_cgra_stencil_28 * _input_cgra_stencil_28;
  int16_t _951 = _kernel_cgra_stencil_29 * _input_cgra_stencil_29;
  int16_t _952 = _kernel_cgra_stencil_30 * _input_cgra_stencil_30;
  int16_t _953 = _kernel_cgra_stencil_31 * _input_cgra_stencil_31;
  int16_t _954 = _kernel_cgra_stencil_32 * _input_cgra_stencil_32;
  int16_t _955 = _953 + _954;
  int16_t _956 = _952 + _955;
  int16_t _957 = _951 + _956;
  int16_t _958 = _950 + _957;
  int16_t _959 = _949 + _958;
  int16_t _960 = _948 + _959;
  int16_t _961 = _output_cgra_stencil_4 + _960;
  int16_t _962 = _947 + _961;
  return _962;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 4), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 4), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 4), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_12(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_33 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_34 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_35 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_36 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_37 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_38 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_39 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_40 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_33 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_34 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_35 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_36 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_37 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_38 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_39 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_40 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_5 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1016 = _kernel_cgra_stencil_33 * _input_cgra_stencil_33;
  int16_t _1017 = _kernel_cgra_stencil_34 * _input_cgra_stencil_34;
  int16_t _1018 = _kernel_cgra_stencil_35 * _input_cgra_stencil_35;
  int16_t _1019 = _kernel_cgra_stencil_36 * _input_cgra_stencil_36;
  int16_t _1020 = _kernel_cgra_stencil_37 * _input_cgra_stencil_37;
  int16_t _1021 = _kernel_cgra_stencil_38 * _input_cgra_stencil_38;
  int16_t _1022 = _kernel_cgra_stencil_39 * _input_cgra_stencil_39;
  int16_t _1023 = _kernel_cgra_stencil_40 * _input_cgra_stencil_40;
  int16_t _1024 = _1022 + _1023;
  int16_t _1025 = _1021 + _1024;
  int16_t _1026 = _1020 + _1025;
  int16_t _1027 = _1019 + _1026;
  int16_t _1028 = _1018 + _1027;
  int16_t _1029 = _1017 + _1028;
  int16_t _1030 = _output_cgra_stencil_5 + _1029;
  int16_t _1031 = _1016 + _1030;
  return _1031;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 5), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 5), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 5), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_13(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_41 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_42 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_43 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_44 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_45 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_46 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_47 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_48 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_41 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_42 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_43 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_44 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_45 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_46 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_47 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_48 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_6 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1085 = _kernel_cgra_stencil_41 * _input_cgra_stencil_41;
  int16_t _1086 = _kernel_cgra_stencil_42 * _input_cgra_stencil_42;
  int16_t _1087 = _kernel_cgra_stencil_43 * _input_cgra_stencil_43;
  int16_t _1088 = _kernel_cgra_stencil_44 * _input_cgra_stencil_44;
  int16_t _1089 = _kernel_cgra_stencil_45 * _input_cgra_stencil_45;
  int16_t _1090 = _kernel_cgra_stencil_46 * _input_cgra_stencil_46;
  int16_t _1091 = _kernel_cgra_stencil_47 * _input_cgra_stencil_47;
  int16_t _1092 = _kernel_cgra_stencil_48 * _input_cgra_stencil_48;
  int16_t _1093 = _1091 + _1092;
  int16_t _1094 = _1090 + _1093;
  int16_t _1095 = _1089 + _1094;
  int16_t _1096 = _1088 + _1095;
  int16_t _1097 = _1087 + _1096;
  int16_t _1098 = _1086 + _1097;
  int16_t _1099 = _output_cgra_stencil_6 + _1098;
  int16_t _1100 = _1085 + _1099;
  return _1100;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 6), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 6), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 6), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_14(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_49 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_50 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_51 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_52 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_53 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_54 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_55 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_56 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_49 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_50 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_51 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_52 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_53 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_54 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_55 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_56 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_7 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1154 = _kernel_cgra_stencil_49 * _input_cgra_stencil_49;
  int16_t _1155 = _kernel_cgra_stencil_50 * _input_cgra_stencil_50;
  int16_t _1156 = _kernel_cgra_stencil_51 * _input_cgra_stencil_51;
  int16_t _1157 = _kernel_cgra_stencil_52 * _input_cgra_stencil_52;
  int16_t _1158 = _kernel_cgra_stencil_53 * _input_cgra_stencil_53;
  int16_t _1159 = _kernel_cgra_stencil_54 * _input_cgra_stencil_54;
  int16_t _1160 = _kernel_cgra_stencil_55 * _input_cgra_stencil_55;
  int16_t _1161 = _kernel_cgra_stencil_56 * _input_cgra_stencil_56;
  int16_t _1162 = _1160 + _1161;
  int16_t _1163 = _1159 + _1162;
  int16_t _1164 = _1158 + _1163;
  int16_t _1165 = _1157 + _1164;
  int16_t _1166 = _1156 + _1165;
  int16_t _1167 = _1155 + _1166;
  int16_t _1168 = _output_cgra_stencil_7 + _1167;
  int16_t _1169 = _1154 + _1168;
  return _1169;
}

//store is: output_cgra.stencil(((output_cgra_s1_w_w*8) + 7), output_cgra_s1_x, output_cgra_s1_y) = ((kernel_cgra.stencil(0, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (output_cgra.stencil(((output_cgra_s1_w_w*8) + 7), output_cgra_s1_x, output_cgra_s1_y) + ((kernel_cgra.stencil(1, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(2, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(3, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(4, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(5, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + ((kernel_cgra.stencil(7, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y))) + (kernel_cgra.stencil(6, ((output_cgra_s1_w_w*8) + 7), output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, (output_cgra_s1_r_x + output_cgra_s1_x), (output_cgra_s1_r_y + output_cgra_s1_y)))))))))))
hw_uint<16> hcompute_output_cgra_stencil_15(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  int16_t _input_cgra_stencil_57 = (int16_t) input_cgra_stencil.extract<0, 15>();
  int16_t _input_cgra_stencil_58 = (int16_t) input_cgra_stencil.extract<16, 31>();
  int16_t _input_cgra_stencil_59 = (int16_t) input_cgra_stencil.extract<32, 47>();
  int16_t _input_cgra_stencil_60 = (int16_t) input_cgra_stencil.extract<48, 63>();
  int16_t _input_cgra_stencil_61 = (int16_t) input_cgra_stencil.extract<64, 79>();
  int16_t _input_cgra_stencil_62 = (int16_t) input_cgra_stencil.extract<80, 95>();
  int16_t _input_cgra_stencil_63 = (int16_t) input_cgra_stencil.extract<96, 111>();
  int16_t _input_cgra_stencil_64 = (int16_t) input_cgra_stencil.extract<112, 127>();

  int16_t _kernel_cgra_stencil_57 = (int16_t) kernel_cgra_stencil.extract<0, 15>();
  int16_t _kernel_cgra_stencil_58 = (int16_t) kernel_cgra_stencil.extract<16, 31>();
  int16_t _kernel_cgra_stencil_59 = (int16_t) kernel_cgra_stencil.extract<32, 47>();
  int16_t _kernel_cgra_stencil_60 = (int16_t) kernel_cgra_stencil.extract<48, 63>();
  int16_t _kernel_cgra_stencil_61 = (int16_t) kernel_cgra_stencil.extract<64, 79>();
  int16_t _kernel_cgra_stencil_62 = (int16_t) kernel_cgra_stencil.extract<80, 95>();
  int16_t _kernel_cgra_stencil_63 = (int16_t) kernel_cgra_stencil.extract<96, 111>();
  int16_t _kernel_cgra_stencil_64 = (int16_t) kernel_cgra_stencil.extract<112, 127>();

  int16_t _output_cgra_stencil_8 = (int16_t) output_cgra_stencil.extract<0, 15>();

  int16_t _1223 = _kernel_cgra_stencil_57 * _input_cgra_stencil_57;
  int16_t _1224 = _kernel_cgra_stencil_58 * _input_cgra_stencil_58;
  int16_t _1225 = _kernel_cgra_stencil_59 * _input_cgra_stencil_59;
  int16_t _1226 = _kernel_cgra_stencil_60 * _input_cgra_stencil_60;
  int16_t _1227 = _kernel_cgra_stencil_61 * _input_cgra_stencil_61;
  int16_t _1228 = _kernel_cgra_stencil_62 * _input_cgra_stencil_62;
  int16_t _1229 = _kernel_cgra_stencil_63 * _input_cgra_stencil_63;
  int16_t _1230 = _kernel_cgra_stencil_64 * _input_cgra_stencil_64;
  int16_t _1231 = _1229 + _1230;
  int16_t _1232 = _1228 + _1231;
  int16_t _1233 = _1227 + _1232;
  int16_t _1234 = _1226 + _1233;
  int16_t _1235 = _1225 + _1234;
  int16_t _1236 = _1224 + _1235;
  int16_t _1237 = _output_cgra_stencil_8 + _1236;
  int16_t _1238 = _1223 + _1237;
  return _1238;
}

//store is: output_glb.stencil(((output_glb_s0_w_w_glb*32) + output_glb_s0_w_w_cgra), output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra) = output_cgra.stencil(output_glb_s0_w_w_cgra, output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil(hw_uint<16>& output_cgra_stencil) {
  int16_t _output_cgra_stencil_9 = (int16_t) output_cgra_stencil.extract<0, 15>();

  return _output_cgra_stencil_9;
}

//store is: hw_output.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& output_glb_stencil) {
  int16_t _output_glb_stencil_1 = (int16_t) output_glb_stencil.extract<0, 15>();

  return _output_glb_stencil_1;
}

