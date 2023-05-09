#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: input_glb.stencil(input_glb_s0_z, input_glb_s0_x, input_glb_s0_y) = input_host.stencil(input_glb_s0_z, input_glb_s0_x, input_glb_s0_y)
hw_uint<16> hcompute_input_glb_stencil(hw_uint<16>& input_host_stencil) {
  uint16_t _input_host_stencil_1 = (uint16_t) input_host_stencil.extract<0, 15>();

  return _input_host_stencil_1;
}

//store is: kernel_glb.stencil(kernel_glb_s0_z, (kernel_glb_s0_w + 56), kernel_glb_s0_x, kernel_glb_s0_y) = kernel_host.stencil(kernel_glb_s0_z, kernel_glb_s0_w, kernel_glb_s0_x, kernel_glb_s0_y)
hw_uint<16> hcompute_kernel_glb_stencil(hw_uint<16>& kernel_host_stencil) {
  uint16_t _kernel_host_stencil_1 = (uint16_t) kernel_host_stencil.extract<0, 15>();

  return _kernel_host_stencil_1;
}

//store is: output_cgra.stencil((((output_glb_s0_w_w_glb*8) + output_cgra_s0_w) - (output_glb_s0_w_w_glb*8)), (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil() {
  uint16_t _701 = (uint16_t)(0);
  return _701;
}

//store is: input_cgra.stencil(input_cgra_s0_z_z_cgra, (((output_glb_s0_x_x_glb*28) + input_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + input_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = input_glb.stencil(((output_cgra_s1_r_z_rz_glb*8) + input_cgra_s0_z_z_cgra), ((output_glb_s0_x_x_glb*28) + input_cgra_s0_x), ((output_glb_s0_y_y_glb*28) + input_cgra_s0_y))
hw_uint<16> hcompute_input_cgra_stencil(hw_uint<16>& input_glb_stencil) {
  uint16_t _input_glb_stencil_1 = (uint16_t) input_glb_stencil.extract<0, 15>();

  return _input_glb_stencil_1;
}

//store is: kernel_cgra.stencil(kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_w_w_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(((output_cgra_s1_r_z_rz_glb*8) + kernel_cgra_s0_z_z_cgra), (((output_glb_s0_w_w_glb*8) + kernel_cgra_s0_w_w_cgra) + 56), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil(hw_uint<16>& kernel_glb_stencil) {
  uint16_t _kernel_glb_stencil_1 = (uint16_t) kernel_glb_stencil.extract<0, 15>();

  return _kernel_glb_stencil_1;
}

//store is: output_cgra.stencil(0, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(0, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_1(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_1 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_2 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_3 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_4 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_5 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_6 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_7 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_8 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_1 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_2 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_3 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_4 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_5 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_6 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_7 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_8 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_1 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _732 = _kernel_cgra_stencil_1 * _input_cgra_stencil_1;
  uint16_t _733 = _kernel_cgra_stencil_2 * _input_cgra_stencil_2;
  uint16_t _734 = _kernel_cgra_stencil_3 * _input_cgra_stencil_3;
  uint16_t _735 = _kernel_cgra_stencil_4 * _input_cgra_stencil_4;
  uint16_t _736 = _kernel_cgra_stencil_5 * _input_cgra_stencil_5;
  uint16_t _737 = _kernel_cgra_stencil_6 * _input_cgra_stencil_6;
  uint16_t _738 = _kernel_cgra_stencil_7 * _input_cgra_stencil_7;
  uint16_t _739 = _kernel_cgra_stencil_8 * _input_cgra_stencil_8;
  uint16_t _740 = _738 + _739;
  uint16_t _741 = _737 + _740;
  uint16_t _742 = _736 + _741;
  uint16_t _743 = _735 + _742;
  uint16_t _744 = _734 + _743;
  uint16_t _745 = _733 + _744;
  uint16_t _746 = _output_cgra_stencil_1 + _745;
  uint16_t _747 = _732 + _746;
  return _747;
}

//store is: output_cgra.stencil(1, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(1, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_2(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_10 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_11 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_12 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_13 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_14 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_15 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_16 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_9 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_10 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_11 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_12 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_13 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_14 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_15 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_16 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_9 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_2 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _807 = _kernel_cgra_stencil_9 * _input_cgra_stencil_9;
  uint16_t _808 = _kernel_cgra_stencil_10 * _input_cgra_stencil_10;
  uint16_t _809 = _kernel_cgra_stencil_11 * _input_cgra_stencil_11;
  uint16_t _810 = _kernel_cgra_stencil_12 * _input_cgra_stencil_12;
  uint16_t _811 = _kernel_cgra_stencil_13 * _input_cgra_stencil_13;
  uint16_t _812 = _kernel_cgra_stencil_14 * _input_cgra_stencil_14;
  uint16_t _813 = _kernel_cgra_stencil_15 * _input_cgra_stencil_15;
  uint16_t _814 = _kernel_cgra_stencil_16 * _input_cgra_stencil_16;
  uint16_t _815 = _813 + _814;
  uint16_t _816 = _812 + _815;
  uint16_t _817 = _811 + _816;
  uint16_t _818 = _810 + _817;
  uint16_t _819 = _809 + _818;
  uint16_t _820 = _808 + _819;
  uint16_t _821 = _output_cgra_stencil_2 + _820;
  uint16_t _822 = _807 + _821;
  return _822;
}

//store is: output_cgra.stencil(2, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(2, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_3(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_17 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_18 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_19 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_20 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_21 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_22 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_23 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_24 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_17 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_18 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_19 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_20 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_21 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_22 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_23 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_24 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_3 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _882 = _kernel_cgra_stencil_17 * _input_cgra_stencil_17;
  uint16_t _883 = _kernel_cgra_stencil_18 * _input_cgra_stencil_18;
  uint16_t _884 = _kernel_cgra_stencil_19 * _input_cgra_stencil_19;
  uint16_t _885 = _kernel_cgra_stencil_20 * _input_cgra_stencil_20;
  uint16_t _886 = _kernel_cgra_stencil_21 * _input_cgra_stencil_21;
  uint16_t _887 = _kernel_cgra_stencil_22 * _input_cgra_stencil_22;
  uint16_t _888 = _kernel_cgra_stencil_23 * _input_cgra_stencil_23;
  uint16_t _889 = _kernel_cgra_stencil_24 * _input_cgra_stencil_24;
  uint16_t _890 = _888 + _889;
  uint16_t _891 = _887 + _890;
  uint16_t _892 = _886 + _891;
  uint16_t _893 = _885 + _892;
  uint16_t _894 = _884 + _893;
  uint16_t _895 = _883 + _894;
  uint16_t _896 = _output_cgra_stencil_3 + _895;
  uint16_t _897 = _882 + _896;
  return _897;
}

//store is: output_cgra.stencil(3, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(3, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_4(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_25 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_26 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_27 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_28 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_29 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_30 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_31 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_32 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_25 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_26 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_27 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_28 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_29 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_30 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_31 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_32 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_4 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _957 = _kernel_cgra_stencil_25 * _input_cgra_stencil_25;
  uint16_t _958 = _kernel_cgra_stencil_26 * _input_cgra_stencil_26;
  uint16_t _959 = _kernel_cgra_stencil_27 * _input_cgra_stencil_27;
  uint16_t _960 = _kernel_cgra_stencil_28 * _input_cgra_stencil_28;
  uint16_t _961 = _kernel_cgra_stencil_29 * _input_cgra_stencil_29;
  uint16_t _962 = _kernel_cgra_stencil_30 * _input_cgra_stencil_30;
  uint16_t _963 = _kernel_cgra_stencil_31 * _input_cgra_stencil_31;
  uint16_t _964 = _kernel_cgra_stencil_32 * _input_cgra_stencil_32;
  uint16_t _965 = _963 + _964;
  uint16_t _966 = _962 + _965;
  uint16_t _967 = _961 + _966;
  uint16_t _968 = _960 + _967;
  uint16_t _969 = _959 + _968;
  uint16_t _970 = _958 + _969;
  uint16_t _971 = _output_cgra_stencil_4 + _970;
  uint16_t _972 = _957 + _971;
  return _972;
}

//store is: output_cgra.stencil(4, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(4, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_5(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_33 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_34 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_35 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_36 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_37 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_38 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_39 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_40 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_33 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_34 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_35 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_36 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_37 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_38 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_39 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_40 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_5 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _1032 = _kernel_cgra_stencil_33 * _input_cgra_stencil_33;
  uint16_t _1033 = _kernel_cgra_stencil_34 * _input_cgra_stencil_34;
  uint16_t _1034 = _kernel_cgra_stencil_35 * _input_cgra_stencil_35;
  uint16_t _1035 = _kernel_cgra_stencil_36 * _input_cgra_stencil_36;
  uint16_t _1036 = _kernel_cgra_stencil_37 * _input_cgra_stencil_37;
  uint16_t _1037 = _kernel_cgra_stencil_38 * _input_cgra_stencil_38;
  uint16_t _1038 = _kernel_cgra_stencil_39 * _input_cgra_stencil_39;
  uint16_t _1039 = _kernel_cgra_stencil_40 * _input_cgra_stencil_40;
  uint16_t _1040 = _1038 + _1039;
  uint16_t _1041 = _1037 + _1040;
  uint16_t _1042 = _1036 + _1041;
  uint16_t _1043 = _1035 + _1042;
  uint16_t _1044 = _1034 + _1043;
  uint16_t _1045 = _1033 + _1044;
  uint16_t _1046 = _output_cgra_stencil_5 + _1045;
  uint16_t _1047 = _1032 + _1046;
  return _1047;
}

//store is: output_cgra.stencil(5, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(5, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_6(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_41 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_42 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_43 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_44 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_45 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_46 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_47 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_48 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_41 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_42 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_43 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_44 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_45 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_46 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_47 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_48 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_6 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _1107 = _kernel_cgra_stencil_41 * _input_cgra_stencil_41;
  uint16_t _1108 = _kernel_cgra_stencil_42 * _input_cgra_stencil_42;
  uint16_t _1109 = _kernel_cgra_stencil_43 * _input_cgra_stencil_43;
  uint16_t _1110 = _kernel_cgra_stencil_44 * _input_cgra_stencil_44;
  uint16_t _1111 = _kernel_cgra_stencil_45 * _input_cgra_stencil_45;
  uint16_t _1112 = _kernel_cgra_stencil_46 * _input_cgra_stencil_46;
  uint16_t _1113 = _kernel_cgra_stencil_47 * _input_cgra_stencil_47;
  uint16_t _1114 = _kernel_cgra_stencil_48 * _input_cgra_stencil_48;
  uint16_t _1115 = _1113 + _1114;
  uint16_t _1116 = _1112 + _1115;
  uint16_t _1117 = _1111 + _1116;
  uint16_t _1118 = _1110 + _1117;
  uint16_t _1119 = _1109 + _1118;
  uint16_t _1120 = _1108 + _1119;
  uint16_t _1121 = _output_cgra_stencil_6 + _1120;
  uint16_t _1122 = _1107 + _1121;
  return _1122;
}

//store is: output_cgra.stencil(6, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(6, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_7(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_49 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_50 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_51 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_52 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_53 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_54 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_55 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_56 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_49 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_50 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_51 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_52 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_53 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_54 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_55 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_56 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_7 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _1182 = _kernel_cgra_stencil_49 * _input_cgra_stencil_49;
  uint16_t _1183 = _kernel_cgra_stencil_50 * _input_cgra_stencil_50;
  uint16_t _1184 = _kernel_cgra_stencil_51 * _input_cgra_stencil_51;
  uint16_t _1185 = _kernel_cgra_stencil_52 * _input_cgra_stencil_52;
  uint16_t _1186 = _kernel_cgra_stencil_53 * _input_cgra_stencil_53;
  uint16_t _1187 = _kernel_cgra_stencil_54 * _input_cgra_stencil_54;
  uint16_t _1188 = _kernel_cgra_stencil_55 * _input_cgra_stencil_55;
  uint16_t _1189 = _kernel_cgra_stencil_56 * _input_cgra_stencil_56;
  uint16_t _1190 = _1188 + _1189;
  uint16_t _1191 = _1187 + _1190;
  uint16_t _1192 = _1186 + _1191;
  uint16_t _1193 = _1185 + _1192;
  uint16_t _1194 = _1184 + _1193;
  uint16_t _1195 = _1183 + _1194;
  uint16_t _1196 = _output_cgra_stencil_7 + _1195;
  uint16_t _1197 = _1182 + _1196;
  return _1197;
}

//store is: output_cgra.stencil(7, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(7, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_8(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
  uint16_t _input_cgra_stencil_57 = (uint16_t) input_cgra_stencil.extract<0, 15>();
  uint16_t _input_cgra_stencil_58 = (uint16_t) input_cgra_stencil.extract<16, 31>();
  uint16_t _input_cgra_stencil_59 = (uint16_t) input_cgra_stencil.extract<32, 47>();
  uint16_t _input_cgra_stencil_60 = (uint16_t) input_cgra_stencil.extract<48, 63>();
  uint16_t _input_cgra_stencil_61 = (uint16_t) input_cgra_stencil.extract<64, 79>();
  uint16_t _input_cgra_stencil_62 = (uint16_t) input_cgra_stencil.extract<80, 95>();
  uint16_t _input_cgra_stencil_63 = (uint16_t) input_cgra_stencil.extract<96, 111>();
  uint16_t _input_cgra_stencil_64 = (uint16_t) input_cgra_stencil.extract<112, 127>();

  uint16_t _kernel_cgra_stencil_57 = (uint16_t) kernel_cgra_stencil.extract<0, 15>();
  uint16_t _kernel_cgra_stencil_58 = (uint16_t) kernel_cgra_stencil.extract<16, 31>();
  uint16_t _kernel_cgra_stencil_59 = (uint16_t) kernel_cgra_stencil.extract<32, 47>();
  uint16_t _kernel_cgra_stencil_60 = (uint16_t) kernel_cgra_stencil.extract<48, 63>();
  uint16_t _kernel_cgra_stencil_61 = (uint16_t) kernel_cgra_stencil.extract<64, 79>();
  uint16_t _kernel_cgra_stencil_62 = (uint16_t) kernel_cgra_stencil.extract<80, 95>();
  uint16_t _kernel_cgra_stencil_63 = (uint16_t) kernel_cgra_stencil.extract<96, 111>();
  uint16_t _kernel_cgra_stencil_64 = (uint16_t) kernel_cgra_stencil.extract<112, 127>();

  uint16_t _output_cgra_stencil_8 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  uint16_t _1257 = _kernel_cgra_stencil_57 * _input_cgra_stencil_57;
  uint16_t _1258 = _kernel_cgra_stencil_58 * _input_cgra_stencil_58;
  uint16_t _1259 = _kernel_cgra_stencil_59 * _input_cgra_stencil_59;
  uint16_t _1260 = _kernel_cgra_stencil_60 * _input_cgra_stencil_60;
  uint16_t _1261 = _kernel_cgra_stencil_61 * _input_cgra_stencil_61;
  uint16_t _1262 = _kernel_cgra_stencil_62 * _input_cgra_stencil_62;
  uint16_t _1263 = _kernel_cgra_stencil_63 * _input_cgra_stencil_63;
  uint16_t _1264 = _kernel_cgra_stencil_64 * _input_cgra_stencil_64;
  uint16_t _1265 = _1263 + _1264;
  uint16_t _1266 = _1262 + _1265;
  uint16_t _1267 = _1261 + _1266;
  uint16_t _1268 = _1260 + _1267;
  uint16_t _1269 = _1259 + _1268;
  uint16_t _1270 = _1258 + _1269;
  uint16_t _1271 = _output_cgra_stencil_8 + _1270;
  uint16_t _1272 = _1257 + _1271;
  return _1272;
}

//store is: output_glb.stencil(((output_glb_s0_w_w_glb*8) + output_glb_s0_w_w_cgra), ((output_glb_s0_x_x_glb*28) + output_glb_s0_x_x_cgra), ((output_glb_s0_y_y_glb*28) + output_glb_s0_y_y_cgra)) = output_cgra.stencil(output_glb_s0_w_w_cgra, output_glb_s0_x_x_cgra, output_glb_s0_y_y_cgra)
hw_uint<16> hcompute_output_glb_stencil(hw_uint<16>& output_cgra_stencil) {
  uint16_t _output_cgra_stencil_9 = (uint16_t) output_cgra_stencil.extract<0, 15>();

  return _output_cgra_stencil_9;
}

//store is: hw_output.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi) = output_glb.stencil(hw_output_s0_w, hw_output_s0_x_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& output_glb_stencil) {
  uint16_t _output_glb_stencil_1 = (uint16_t) output_glb_stencil.extract<0, 15>();

  return _output_glb_stencil_1;
}

