#include <fstream>
#include "unoptimized_unsharp.h"

int main() {
  srand(234);
  ofstream fout("regression_result_unoptimized_unsharp.txt");
  HWStream<hw_uint<16 > > hw_input_stencil_clkwrk_0;
  HWStream<hw_uint<16 > > hw_input_stencil_clkwrk_1;
  HWStream<hw_uint<16 > > hw_input_stencil_clkwrk_2;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_stencil_clkwrk_3;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_stencil_clkwrk_4;
  HWStream<hw_uint<16 > > hw_output_global_wrapper_stencil_clkwrk_5;


  // Loading input data
  srand(1);
  // cmap    : { op_hcompute_hw_input_global_wrapper_glb_stencil[root = 0, hw_input_global_wrapper_s0_y, hw_input_global_wrapper_s0_x_x] -> hw_input_stencil_clkwrk_0[-3 + hw_input_global_wrapper_s0_y, -3 + hw_input_global_wrapper_s0_x_x, 0] : 0 <= hw_input_global_wrapper_s0_y <= 255 and 0 <= hw_input_global_wrapper_s0_x_x <= 67 }
  // read map: { hw_input_stencil_clkwrk_0[i0, i1, 0] -> op_hcompute_hw_input_global_wrapper_glb_stencil[root = 0, hw_input_global_wrapper_s0_y = 3 + i0, hw_input_global_wrapper_s0_x_x = 3 + i1] : -3 <= i0 <= 252 and -3 <= i1 <= 64 }
  // rng     : { op_hcompute_hw_input_global_wrapper_glb_stencil[root = 0, hw_input_global_wrapper_s0_y, hw_input_global_wrapper_s0_x_x] : 0 <= hw_input_global_wrapper_s0_y <= 255 and 0 <= hw_input_global_wrapper_s0_x_x <= 67 }
  // rng card: { 17408 }
  for (int i = 0; i < 17408; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    hw_input_stencil_clkwrk_0.write(value);
  }

  // cmap    : { op_hcompute_hw_input_global_wrapper_glb_stencil_1[root = 0, hw_input_global_wrapper_s0_y, hw_input_global_wrapper_s0_x_x] -> hw_input_stencil_clkwrk_1[-3 + hw_input_global_wrapper_s0_y, -3 + hw_input_global_wrapper_s0_x_x, 1] : 0 <= hw_input_global_wrapper_s0_y <= 255 and 0 <= hw_input_global_wrapper_s0_x_x <= 67 }
  // read map: { hw_input_stencil_clkwrk_1[i0, i1, 1] -> op_hcompute_hw_input_global_wrapper_glb_stencil_1[root = 0, hw_input_global_wrapper_s0_y = 3 + i0, hw_input_global_wrapper_s0_x_x = 3 + i1] : -3 <= i0 <= 252 and -3 <= i1 <= 64 }
  // rng     : { op_hcompute_hw_input_global_wrapper_glb_stencil_1[root = 0, hw_input_global_wrapper_s0_y, hw_input_global_wrapper_s0_x_x] : 0 <= hw_input_global_wrapper_s0_y <= 255 and 0 <= hw_input_global_wrapper_s0_x_x <= 67 }
  // rng card: { 17408 }
  for (int i = 0; i < 17408; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    hw_input_stencil_clkwrk_1.write(value);
  }

  // cmap    : { op_hcompute_hw_input_global_wrapper_glb_stencil_2[root = 0, hw_input_global_wrapper_s0_y, hw_input_global_wrapper_s0_x_x] -> hw_input_stencil_clkwrk_2[-3 + hw_input_global_wrapper_s0_y, -3 + hw_input_global_wrapper_s0_x_x, 2] : 0 <= hw_input_global_wrapper_s0_y <= 255 and 0 <= hw_input_global_wrapper_s0_x_x <= 67 }
  // read map: { hw_input_stencil_clkwrk_2[i0, i1, 2] -> op_hcompute_hw_input_global_wrapper_glb_stencil_2[root = 0, hw_input_global_wrapper_s0_y = 3 + i0, hw_input_global_wrapper_s0_x_x = 3 + i1] : -3 <= i0 <= 252 and -3 <= i1 <= 64 }
  // rng     : { op_hcompute_hw_input_global_wrapper_glb_stencil_2[root = 0, hw_input_global_wrapper_s0_y, hw_input_global_wrapper_s0_x_x] : 0 <= hw_input_global_wrapper_s0_y <= 255 and 0 <= hw_input_global_wrapper_s0_x_x <= 67 }
  // rng card: { 17408 }
  for (int i = 0; i < 17408; i++) {
    hw_uint<16 > value;
    set_at<0, 16, 16>(value, rand() % 256);
    hw_input_stencil_clkwrk_2.write(value);
  }

  unoptimized_unsharp(hw_input_stencil_clkwrk_0, hw_input_stencil_clkwrk_1, hw_input_stencil_clkwrk_2, hw_output_global_wrapper_stencil_clkwrk_3, hw_output_global_wrapper_stencil_clkwrk_4, hw_output_global_wrapper_stencil_clkwrk_5);
  for (int i = 0; i < 15500; i++) {
    auto actual = hw_output_global_wrapper_stencil_clkwrk_3.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 15500; i++) {
    auto actual = hw_output_global_wrapper_stencil_clkwrk_4.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  for (int i = 0; i < 15500; i++) {
    auto actual = hw_output_global_wrapper_stencil_clkwrk_5.read();
    hw_uint<16> actual_lane_0 = actual.extract<0, 15>();
    fout << actual_lane_0 << endl;
  }

  assert(hw_input_stencil_clkwrk_0.is_empty());
  assert(hw_input_stencil_clkwrk_1.is_empty());
  assert(hw_input_stencil_clkwrk_2.is_empty());
  assert(hw_output_global_wrapper_stencil_clkwrk_3.is_empty());
  assert(hw_output_global_wrapper_stencil_clkwrk_4.is_empty());
  assert(hw_output_global_wrapper_stencil_clkwrk_5.is_empty());
  return 0;
}
