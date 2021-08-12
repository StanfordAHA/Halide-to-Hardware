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

//store is: output_cgra.stencil(0, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil() {
  uint16_t _699 = (uint16_t)(0);
  return _699;
}

//store is: output_cgra.stencil(1, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_1() {
  uint16_t _708 = (uint16_t)(0);
  return _708;
}

//store is: output_cgra.stencil(2, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_2() {
  uint16_t _717 = (uint16_t)(0);
  return _717;
}

//store is: output_cgra.stencil(3, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_3() {
  uint16_t _726 = (uint16_t)(0);
  return _726;
}

//store is: output_cgra.stencil(4, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_4() {
  uint16_t _735 = (uint16_t)(0);
  return _735;
}

//store is: output_cgra.stencil(5, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_5() {
  uint16_t _744 = (uint16_t)(0);
  return _744;
}

//store is: output_cgra.stencil(6, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_6() {
  uint16_t _753 = (uint16_t)(0);
  return _753;
}

//store is: output_cgra.stencil(7, (((output_glb_s0_x_x_glb*28) + output_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = (uint16)0
hw_uint<16> hcompute_output_cgra_stencil_7() {
  uint16_t _762 = (uint16_t)(0);
  return _762;
}

//store is: input_cgra.stencil(input_cgra_s0_z_z_cgra, (((output_glb_s0_x_x_glb*28) + input_cgra_s0_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + input_cgra_s0_y) - (output_glb_s0_y_y_glb*28))) = input_glb.stencil(input_cgra_s0_z_z_cgra, ((output_glb_s0_x_x_glb*28) + input_cgra_s0_x), ((output_glb_s0_y_y_glb*28) + input_cgra_s0_y))
hw_uint<16> hcompute_input_cgra_stencil(hw_uint<16>& input_glb_stencil) {
  uint16_t _input_glb_stencil_1 = (uint16_t) input_glb_stencil.extract<0, 15>();

  return _input_glb_stencil_1;
}

//store is: kernel_cgra.stencil(kernel_cgra_s0_z_z_cgra, kernel_cgra_s0_w_w_cgra, kernel_cgra_s0_x, kernel_cgra_s0_y) = kernel_glb.stencil(kernel_cgra_s0_z_z_cgra, (((output_glb_s0_w_w_glb*8) + kernel_cgra_s0_w_w_cgra) + 56), kernel_cgra_s0_x, kernel_cgra_s0_y)
hw_uint<16> hcompute_kernel_cgra_stencil(hw_uint<16>& kernel_glb_stencil) {
  uint16_t _kernel_glb_stencil_1 = (uint16_t) kernel_glb_stencil.extract<0, 15>();

  return _kernel_glb_stencil_1;
}

//store is: output_cgra.stencil(0, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(0, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 0, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_8(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _786 = _kernel_cgra_stencil_1 * _input_cgra_stencil_1;
  uint16_t _787 = _kernel_cgra_stencil_2 * _input_cgra_stencil_2;
  uint16_t _788 = _kernel_cgra_stencil_3 * _input_cgra_stencil_3;
  uint16_t _789 = _kernel_cgra_stencil_4 * _input_cgra_stencil_4;
  uint16_t _790 = _kernel_cgra_stencil_5 * _input_cgra_stencil_5;
  uint16_t _791 = _kernel_cgra_stencil_6 * _input_cgra_stencil_6;
  uint16_t _792 = _kernel_cgra_stencil_7 * _input_cgra_stencil_7;
  uint16_t _793 = _kernel_cgra_stencil_8 * _input_cgra_stencil_8;
  uint16_t _794 = _792 + _793;
  uint16_t _795 = _791 + _794;
  uint16_t _796 = _790 + _795;
  uint16_t _797 = _789 + _796;
  uint16_t _798 = _788 + _797;
  uint16_t _799 = _787 + _798;
  uint16_t _800 = _output_cgra_stencil_1 + _799;
  uint16_t _801 = _786 + _800;
  return _801;
}

//store is: output_cgra.stencil(1, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(1, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 1, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_9(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _861 = _kernel_cgra_stencil_9 * _input_cgra_stencil_9;
  uint16_t _862 = _kernel_cgra_stencil_10 * _input_cgra_stencil_10;
  uint16_t _863 = _kernel_cgra_stencil_11 * _input_cgra_stencil_11;
  uint16_t _864 = _kernel_cgra_stencil_12 * _input_cgra_stencil_12;
  uint16_t _865 = _kernel_cgra_stencil_13 * _input_cgra_stencil_13;
  uint16_t _866 = _kernel_cgra_stencil_14 * _input_cgra_stencil_14;
  uint16_t _867 = _kernel_cgra_stencil_15 * _input_cgra_stencil_15;
  uint16_t _868 = _kernel_cgra_stencil_16 * _input_cgra_stencil_16;
  uint16_t _869 = _867 + _868;
  uint16_t _870 = _866 + _869;
  uint16_t _871 = _865 + _870;
  uint16_t _872 = _864 + _871;
  uint16_t _873 = _863 + _872;
  uint16_t _874 = _862 + _873;
  uint16_t _875 = _output_cgra_stencil_2 + _874;
  uint16_t _876 = _861 + _875;
  return _876;
}

//store is: output_cgra.stencil(2, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(2, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 2, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_10(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _936 = _kernel_cgra_stencil_17 * _input_cgra_stencil_17;
  uint16_t _937 = _kernel_cgra_stencil_18 * _input_cgra_stencil_18;
  uint16_t _938 = _kernel_cgra_stencil_19 * _input_cgra_stencil_19;
  uint16_t _939 = _kernel_cgra_stencil_20 * _input_cgra_stencil_20;
  uint16_t _940 = _kernel_cgra_stencil_21 * _input_cgra_stencil_21;
  uint16_t _941 = _kernel_cgra_stencil_22 * _input_cgra_stencil_22;
  uint16_t _942 = _kernel_cgra_stencil_23 * _input_cgra_stencil_23;
  uint16_t _943 = _kernel_cgra_stencil_24 * _input_cgra_stencil_24;
  uint16_t _944 = _942 + _943;
  uint16_t _945 = _941 + _944;
  uint16_t _946 = _940 + _945;
  uint16_t _947 = _939 + _946;
  uint16_t _948 = _938 + _947;
  uint16_t _949 = _937 + _948;
  uint16_t _950 = _output_cgra_stencil_3 + _949;
  uint16_t _951 = _936 + _950;
  return _951;
}

//store is: output_cgra.stencil(3, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(3, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 3, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_11(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _1011 = _kernel_cgra_stencil_25 * _input_cgra_stencil_25;
  uint16_t _1012 = _kernel_cgra_stencil_26 * _input_cgra_stencil_26;
  uint16_t _1013 = _kernel_cgra_stencil_27 * _input_cgra_stencil_27;
  uint16_t _1014 = _kernel_cgra_stencil_28 * _input_cgra_stencil_28;
  uint16_t _1015 = _kernel_cgra_stencil_29 * _input_cgra_stencil_29;
  uint16_t _1016 = _kernel_cgra_stencil_30 * _input_cgra_stencil_30;
  uint16_t _1017 = _kernel_cgra_stencil_31 * _input_cgra_stencil_31;
  uint16_t _1018 = _kernel_cgra_stencil_32 * _input_cgra_stencil_32;
  uint16_t _1019 = _1017 + _1018;
  uint16_t _1020 = _1016 + _1019;
  uint16_t _1021 = _1015 + _1020;
  uint16_t _1022 = _1014 + _1021;
  uint16_t _1023 = _1013 + _1022;
  uint16_t _1024 = _1012 + _1023;
  uint16_t _1025 = _output_cgra_stencil_4 + _1024;
  uint16_t _1026 = _1011 + _1025;
  return _1026;
}

//store is: output_cgra.stencil(4, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(4, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 4, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_12(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _1086 = _kernel_cgra_stencil_33 * _input_cgra_stencil_33;
  uint16_t _1087 = _kernel_cgra_stencil_34 * _input_cgra_stencil_34;
  uint16_t _1088 = _kernel_cgra_stencil_35 * _input_cgra_stencil_35;
  uint16_t _1089 = _kernel_cgra_stencil_36 * _input_cgra_stencil_36;
  uint16_t _1090 = _kernel_cgra_stencil_37 * _input_cgra_stencil_37;
  uint16_t _1091 = _kernel_cgra_stencil_38 * _input_cgra_stencil_38;
  uint16_t _1092 = _kernel_cgra_stencil_39 * _input_cgra_stencil_39;
  uint16_t _1093 = _kernel_cgra_stencil_40 * _input_cgra_stencil_40;
  uint16_t _1094 = _1092 + _1093;
  uint16_t _1095 = _1091 + _1094;
  uint16_t _1096 = _1090 + _1095;
  uint16_t _1097 = _1089 + _1096;
  uint16_t _1098 = _1088 + _1097;
  uint16_t _1099 = _1087 + _1098;
  uint16_t _1100 = _output_cgra_stencil_5 + _1099;
  uint16_t _1101 = _1086 + _1100;
  return _1101;
}

//store is: output_cgra.stencil(5, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(5, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 5, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_13(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _1161 = _kernel_cgra_stencil_41 * _input_cgra_stencil_41;
  uint16_t _1162 = _kernel_cgra_stencil_42 * _input_cgra_stencil_42;
  uint16_t _1163 = _kernel_cgra_stencil_43 * _input_cgra_stencil_43;
  uint16_t _1164 = _kernel_cgra_stencil_44 * _input_cgra_stencil_44;
  uint16_t _1165 = _kernel_cgra_stencil_45 * _input_cgra_stencil_45;
  uint16_t _1166 = _kernel_cgra_stencil_46 * _input_cgra_stencil_46;
  uint16_t _1167 = _kernel_cgra_stencil_47 * _input_cgra_stencil_47;
  uint16_t _1168 = _kernel_cgra_stencil_48 * _input_cgra_stencil_48;
  uint16_t _1169 = _1167 + _1168;
  uint16_t _1170 = _1166 + _1169;
  uint16_t _1171 = _1165 + _1170;
  uint16_t _1172 = _1164 + _1171;
  uint16_t _1173 = _1163 + _1172;
  uint16_t _1174 = _1162 + _1173;
  uint16_t _1175 = _output_cgra_stencil_6 + _1174;
  uint16_t _1176 = _1161 + _1175;
  return _1176;
}

//store is: output_cgra.stencil(6, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(6, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 6, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_14(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _1236 = _kernel_cgra_stencil_49 * _input_cgra_stencil_49;
  uint16_t _1237 = _kernel_cgra_stencil_50 * _input_cgra_stencil_50;
  uint16_t _1238 = _kernel_cgra_stencil_51 * _input_cgra_stencil_51;
  uint16_t _1239 = _kernel_cgra_stencil_52 * _input_cgra_stencil_52;
  uint16_t _1240 = _kernel_cgra_stencil_53 * _input_cgra_stencil_53;
  uint16_t _1241 = _kernel_cgra_stencil_54 * _input_cgra_stencil_54;
  uint16_t _1242 = _kernel_cgra_stencil_55 * _input_cgra_stencil_55;
  uint16_t _1243 = _kernel_cgra_stencil_56 * _input_cgra_stencil_56;
  uint16_t _1244 = _1242 + _1243;
  uint16_t _1245 = _1241 + _1244;
  uint16_t _1246 = _1240 + _1245;
  uint16_t _1247 = _1239 + _1246;
  uint16_t _1248 = _1238 + _1247;
  uint16_t _1249 = _1237 + _1248;
  uint16_t _1250 = _output_cgra_stencil_7 + _1249;
  uint16_t _1251 = _1236 + _1250;
  return _1251;
}

//store is: output_cgra.stencil(7, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) = ((kernel_cgra.stencil(0, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(0, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (output_cgra.stencil(7, (((output_glb_s0_x_x_glb*28) + output_cgra_s1_x) - (output_glb_s0_x_x_glb*28)), (((output_glb_s0_y_y_glb*28) + output_cgra_s1_y) - (output_glb_s0_y_y_glb*28))) + ((kernel_cgra.stencil(1, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(1, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(2, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(2, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(3, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(3, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(4, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(4, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(5, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(5, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + ((kernel_cgra.stencil(7, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(7, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28)))) + (kernel_cgra.stencil(6, 7, output_cgra_s1_r_x, output_cgra_s1_r_y)*input_cgra.stencil(6, ((output_cgra_s1_r_x + ((output_glb_s0_x_x_glb*28) + output_cgra_s1_x)) - (output_glb_s0_x_x_glb*28)), ((output_cgra_s1_r_y + ((output_glb_s0_y_y_glb*28) + output_cgra_s1_y)) - (output_glb_s0_y_y_glb*28))))))))))))
hw_uint<16> hcompute_output_cgra_stencil_15(hw_uint<128>& input_cgra_stencil, hw_uint<128>& kernel_cgra_stencil, hw_uint<16>& output_cgra_stencil) {
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

  uint16_t _1311 = _kernel_cgra_stencil_57 * _input_cgra_stencil_57;
  uint16_t _1312 = _kernel_cgra_stencil_58 * _input_cgra_stencil_58;
  uint16_t _1313 = _kernel_cgra_stencil_59 * _input_cgra_stencil_59;
  uint16_t _1314 = _kernel_cgra_stencil_60 * _input_cgra_stencil_60;
  uint16_t _1315 = _kernel_cgra_stencil_61 * _input_cgra_stencil_61;
  uint16_t _1316 = _kernel_cgra_stencil_62 * _input_cgra_stencil_62;
  uint16_t _1317 = _kernel_cgra_stencil_63 * _input_cgra_stencil_63;
  uint16_t _1318 = _kernel_cgra_stencil_64 * _input_cgra_stencil_64;
  uint16_t _1319 = _1317 + _1318;
  uint16_t _1320 = _1316 + _1319;
  uint16_t _1321 = _1315 + _1320;
  uint16_t _1322 = _1314 + _1321;
  uint16_t _1323 = _1313 + _1322;
  uint16_t _1324 = _1312 + _1323;
  uint16_t _1325 = _output_cgra_stencil_8 + _1324;
  uint16_t _1326 = _1311 + _1325;
  return _1326;
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

