#include <fstream>
#include "unoptimized_depthwise_conv.h"

int main() {
  srand(234);
  ofstream fout("regression_result_unoptimized_depthwise_conv.txt");
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_0;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_1;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_2;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_3;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_4;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_5;
  HWStream<hw_uint<16 > > input_host_stencil_clkwrk_6;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_10;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_11;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_12;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_13;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_7;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_8;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_glb_stencil_clkwrk_9;


  // Loading input data
  srand(1);
  // cmap    : { op_hcompute_input_host_global_wrapper_stencil[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_0[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 0] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_0[i0, i1, 0] -> op_hcompute_input_host_global_wrapper_stencil[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_0.write(value);
  }

  // cmap    : { op_hcompute_input_host_global_wrapper_stencil_1[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_1[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 1] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_1[i0, i1, 1] -> op_hcompute_input_host_global_wrapper_stencil_1[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil_1[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_1.write(value);
  }

  // cmap    : { op_hcompute_input_host_global_wrapper_stencil_2[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_2[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 2] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_2[i0, i1, 2] -> op_hcompute_input_host_global_wrapper_stencil_2[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil_2[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_2.write(value);
  }

  // cmap    : { op_hcompute_input_host_global_wrapper_stencil_3[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_3[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 3] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_3[i0, i1, 3] -> op_hcompute_input_host_global_wrapper_stencil_3[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil_3[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_3.write(value);
  }

  // cmap    : { op_hcompute_input_host_global_wrapper_stencil_4[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_4[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 4] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_4[i0, i1, 4] -> op_hcompute_input_host_global_wrapper_stencil_4[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil_4[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_4.write(value);
  }

  // cmap    : { op_hcompute_input_host_global_wrapper_stencil_5[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_5[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 5] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_5[i0, i1, 5] -> op_hcompute_input_host_global_wrapper_stencil_5[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil_5[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_5.write(value);
  }

  // cmap    : { op_hcompute_input_host_global_wrapper_stencil_6[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] -> input_host_stencil_clkwrk_6[input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x, 6] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // read map: { input_host_stencil_clkwrk_6[i0, i1, 6] -> op_hcompute_input_host_global_wrapper_stencil_6[root = 0, input_host_global_wrapper_s0_y = i0, input_host_global_wrapper_s0_x = i1] : 0 <= i0 <= 115 and 0 <= i1 <= 115 }
  // rng     : { op_hcompute_input_host_global_wrapper_stencil_6[root = 0, input_host_global_wrapper_s0_y, input_host_global_wrapper_s0_x] : 0 <= input_host_global_wrapper_s0_y <= 115 and 0 <= input_host_global_wrapper_s0_x <= 115 }
  // rng card: { 13456 }
  for (int i = 0; i < 13456; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    input_host_stencil_clkwrk_6.write(value);
  }

  unoptimized_depthwise_conv(input_host_stencil_clkwrk_0, input_host_stencil_clkwrk_1, input_host_stencil_clkwrk_2, input_host_stencil_clkwrk_3, input_host_stencil_clkwrk_4, input_host_stencil_clkwrk_5, input_host_stencil_clkwrk_6, hw_output_global_wrapper_glb_stencil_clkwrk_10, hw_output_global_wrapper_glb_stencil_clkwrk_11, hw_output_global_wrapper_glb_stencil_clkwrk_12, hw_output_global_wrapper_glb_stencil_clkwrk_13, hw_output_global_wrapper_glb_stencil_clkwrk_7, hw_output_global_wrapper_glb_stencil_clkwrk_8, hw_output_global_wrapper_glb_stencil_clkwrk_9);
  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_10.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_11.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_12.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_13.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_7.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_8.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 12996; i++) {
    auto actual = hw_output_global_wrapper_glb_stencil_clkwrk_9.read();
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
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_10.is_empty());
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_11.is_empty());
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_12.is_empty());
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_13.is_empty());
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_7.is_empty());
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_8.is_empty());
  assert(hw_output_global_wrapper_glb_stencil_clkwrk_9.is_empty());
  return 0;
}
