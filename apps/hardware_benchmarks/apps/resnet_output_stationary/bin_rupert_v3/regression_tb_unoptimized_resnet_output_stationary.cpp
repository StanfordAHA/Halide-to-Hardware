#include <fstream>
#include "unoptimized_resnet_output_stationary.h"

int main() {
  srand(234);
  ofstream fout("regression_result_unoptimized_resnet_output_stationary.txt");
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_0;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_1;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_2;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_3;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_4;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_5;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_6;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_7;
  HWStream<hw_uint<16 > > kernel_host_stencil_clkwrk_10;
  HWStream<hw_uint<16 > > kernel_host_stencil_clkwrk_11;
  HWStream<hw_uint<16 > > kernel_host_stencil_clkwrk_8;
  HWStream<hw_uint<16 > > kernel_host_stencil_clkwrk_9;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_12;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_13;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_14;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_15;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_16;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_17;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_18;
  HWStream<hw_uint<16 > > hw_output_stencil_clkwrk_19;


  // Loading input data
  srand(1);
  // cmap    : { op_hcompute_input_glb_stencil[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_0[input_glb_s0_y, input_glb_s0_x, 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_0[i0, i1, i2] -> op_hcompute_input_glb_stencil[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 0 <= i2 <= 24 }
  // rng     : { op_hcompute_input_glb_stencil[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_0.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_1[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_1[input_glb_s0_y, input_glb_s0_x, 1 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_1[i0, i1, i2] -> op_hcompute_input_glb_stencil_1[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -1 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 0 < i2 <= 25 }
  // rng     : { op_hcompute_input_glb_stencil_1[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_1.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_2[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_2[input_glb_s0_y, input_glb_s0_x, 2 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_2[i0, i1, i2] -> op_hcompute_input_glb_stencil_2[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -2 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 2 <= i2 <= 26 }
  // rng     : { op_hcompute_input_glb_stencil_2[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_2.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_3[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_3[input_glb_s0_y, input_glb_s0_x, 3 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_3[i0, i1, i2] -> op_hcompute_input_glb_stencil_3[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -3 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 3 <= i2 <= 27 }
  // rng     : { op_hcompute_input_glb_stencil_3[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_3.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_4[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_4[input_glb_s0_y, input_glb_s0_x, 4 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_4[i0, i1, i2] -> op_hcompute_input_glb_stencil_4[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -4 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 4 <= i2 <= 28 }
  // rng     : { op_hcompute_input_glb_stencil_4[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_4.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_5[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_5[input_glb_s0_y, input_glb_s0_x, 5 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_5[i0, i1, i2] -> op_hcompute_input_glb_stencil_5[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -5 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 5 <= i2 <= 29 }
  // rng     : { op_hcompute_input_glb_stencil_5[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_5.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_6[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_6[input_glb_s0_y, input_glb_s0_x, 6 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_6[i0, i1, i2] -> op_hcompute_input_glb_stencil_6[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -6 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 6 <= i2 <= 30 }
  // rng     : { op_hcompute_input_glb_stencil_6[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_6.write(value);
  }

  // cmap    : { op_hcompute_input_glb_stencil_7[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] -> input_host_stencil_clkwrk_7[input_glb_s0_y, input_glb_s0_x, 7 + 8input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // read map: { input_host_stencil_clkwrk_7[i0, i1, i2] -> op_hcompute_input_glb_stencil_7[root = 0, input_glb_s0_y = i0, input_glb_s0_x = i1, input_glb_s0_z_z] : 8input_glb_s0_z_z = -7 + i2 and 0 <= i0 <= 28 and 0 <= i1 <= 28 and 7 <= i2 <= 31 }
  // rng     : { op_hcompute_input_glb_stencil_7[root = 0, input_glb_s0_y, input_glb_s0_x, input_glb_s0_z_z] : 0 <= input_glb_s0_y <= 28 and 0 <= input_glb_s0_x <= 28 and 0 <= input_glb_s0_z_z <= 3 }
  // rng card: { 3364 }
  for (int i = 0; i < 3364; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_7.write(value);
  }

  // cmap    : { op_hcompute_kernel_glb_stencil_2[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] -> kernel_host_stencil_clkwrk_10[kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, 2 + 4kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // read map: { kernel_host_stencil_clkwrk_10[i0, i1, i2, i3] -> op_hcompute_kernel_glb_stencil_2[root = 0, kernel_glb_s0_y = i0, kernel_glb_s0_x = i1, kernel_glb_s0_z = i2, kernel_glb_s0_w_w] : 4kernel_glb_s0_w_w = -2 + i3 and 0 <= i0 <= 2 and 0 <= i1 <= 2 and 0 <= i2 <= 31 and 2 <= i3 <= 126 }
  // rng     : { op_hcompute_kernel_glb_stencil_2[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // rng card: { 9216 }
  for (int i = 0; i < 9216; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    kernel_host_stencil_clkwrk_10.write(value);
  }

  // cmap    : { op_hcompute_kernel_glb_stencil_3[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] -> kernel_host_stencil_clkwrk_11[kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, 3 + 4kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // read map: { kernel_host_stencil_clkwrk_11[i0, i1, i2, i3] -> op_hcompute_kernel_glb_stencil_3[root = 0, kernel_glb_s0_y = i0, kernel_glb_s0_x = i1, kernel_glb_s0_z = i2, kernel_glb_s0_w_w] : 4kernel_glb_s0_w_w = -3 + i3 and 0 <= i0 <= 2 and 0 <= i1 <= 2 and 0 <= i2 <= 31 and 3 <= i3 <= 127 }
  // rng     : { op_hcompute_kernel_glb_stencil_3[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // rng card: { 9216 }
  for (int i = 0; i < 9216; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    kernel_host_stencil_clkwrk_11.write(value);
  }

  // cmap    : { op_hcompute_kernel_glb_stencil[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] -> kernel_host_stencil_clkwrk_8[kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, 4kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // read map: { kernel_host_stencil_clkwrk_8[i0, i1, i2, i3] -> op_hcompute_kernel_glb_stencil[root = 0, kernel_glb_s0_y = i0, kernel_glb_s0_x = i1, kernel_glb_s0_z = i2, kernel_glb_s0_w_w] : 4kernel_glb_s0_w_w = i3 and 0 <= i0 <= 2 and 0 <= i1 <= 2 and 0 <= i2 <= 31 and 0 <= i3 <= 124 }
  // rng     : { op_hcompute_kernel_glb_stencil[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // rng card: { 9216 }
  for (int i = 0; i < 9216; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    kernel_host_stencil_clkwrk_8.write(value);
  }

  // cmap    : { op_hcompute_kernel_glb_stencil_1[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] -> kernel_host_stencil_clkwrk_9[kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, 1 + 4kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // read map: { kernel_host_stencil_clkwrk_9[i0, i1, i2, i3] -> op_hcompute_kernel_glb_stencil_1[root = 0, kernel_glb_s0_y = i0, kernel_glb_s0_x = i1, kernel_glb_s0_z = i2, kernel_glb_s0_w_w] : 4kernel_glb_s0_w_w = -1 + i3 and 0 <= i0 <= 2 and 0 <= i1 <= 2 and 0 <= i2 <= 31 and 0 < i3 <= 125 }
  // rng     : { op_hcompute_kernel_glb_stencil_1[root = 0, kernel_glb_s0_y, kernel_glb_s0_x, kernel_glb_s0_z, kernel_glb_s0_w_w] : 0 <= kernel_glb_s0_y <= 2 and 0 <= kernel_glb_s0_x <= 2 and 0 <= kernel_glb_s0_z <= 31 and 0 <= kernel_glb_s0_w_w <= 31 }
  // rng card: { 9216 }
  for (int i = 0; i < 9216; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    kernel_host_stencil_clkwrk_9.write(value);
  }

  unoptimized_resnet_output_stationary(input_host_stencil_clkwrk_0, input_host_stencil_clkwrk_1, input_host_stencil_clkwrk_2, input_host_stencil_clkwrk_3, input_host_stencil_clkwrk_4, input_host_stencil_clkwrk_5, input_host_stencil_clkwrk_6, input_host_stencil_clkwrk_7, kernel_host_stencil_clkwrk_10, kernel_host_stencil_clkwrk_11, kernel_host_stencil_clkwrk_8, kernel_host_stencil_clkwrk_9, hw_output_stencil_clkwrk_12, hw_output_stencil_clkwrk_13, hw_output_stencil_clkwrk_14, hw_output_stencil_clkwrk_15, hw_output_stencil_clkwrk_16, hw_output_stencil_clkwrk_17, hw_output_stencil_clkwrk_18, hw_output_stencil_clkwrk_19);
  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_12.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_13.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_14.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_15.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_16.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_17.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_18.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 3136; i++) {
    auto actual = hw_output_stencil_clkwrk_19.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  assert(input_host_stencil_clkwrk_0.is_empty());
  assert(input_host_stencil_clkwrk_1.is_empty());
  assert(input_host_stencil_clkwrk_2.is_empty());
  assert(input_host_stencil_clkwrk_3.is_empty());
  assert(input_host_stencil_clkwrk_4.is_empty());
  assert(input_host_stencil_clkwrk_5.is_empty());
  assert(input_host_stencil_clkwrk_6.is_empty());
  assert(input_host_stencil_clkwrk_7.is_empty());
  assert(kernel_host_stencil_clkwrk_10.is_empty());
  assert(kernel_host_stencil_clkwrk_11.is_empty());
  assert(kernel_host_stencil_clkwrk_8.is_empty());
  assert(kernel_host_stencil_clkwrk_9.is_empty());
  assert(hw_output_stencil_clkwrk_12.is_empty());
  assert(hw_output_stencil_clkwrk_13.is_empty());
  assert(hw_output_stencil_clkwrk_14.is_empty());
  assert(hw_output_stencil_clkwrk_15.is_empty());
  assert(hw_output_stencil_clkwrk_16.is_empty());
  assert(hw_output_stencil_clkwrk_17.is_empty());
  assert(hw_output_stencil_clkwrk_18.is_empty());
  assert(hw_output_stencil_clkwrk_19.is_empty());
  return 0;
}
