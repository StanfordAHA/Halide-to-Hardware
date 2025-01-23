#pragma once
#include "hw_classes.h"
#include "clockwork_standard_compute_units.h"


//store is: hw_input_b_global_wrapper.stencil(hw_input_b_global_wrapper_s0_x_x, hw_input_b_global_wrapper_s0_y) = hw_input_b.stencil(hw_input_b_global_wrapper_s0_x_x, hw_input_b_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_b_global_wrapper_stencil(hw_uint<16>& hw_input_b_stencil) {
  uint16_t _hw_input_b_stencil_1 = (uint16_t) (hw_input_b_stencil.extract<0, 15>());

  return _hw_input_b_stencil_1;
}

//store is: hw_input_global_wrapper.stencil(hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y)
hw_uint<16> hcompute_hw_input_global_wrapper_stencil(hw_uint<16>& hw_input_stencil) {
  uint16_t _hw_input_stencil_1 = (uint16_t) (hw_input_stencil.extract<0, 15>());

  return _hw_input_stencil_1;
}

//store is: add.stencil(add_s0_x_x, add_s0_y) = (hw_input_global_wrapper.stencil(add_s0_x_x, add_s0_y) + hw_input_b_global_wrapper.stencil(add_s0_x_x, add_s0_y))
hw_uint<16> hcompute_add_stencil(hw_uint<16>& hw_input_b_global_wrapper_stencil, hw_uint<16>& hw_input_global_wrapper_stencil) {
  uint16_t _hw_input_b_global_wrapper_stencil_1 = (uint16_t) (hw_input_b_global_wrapper_stencil.extract<0, 15>());

  uint16_t _hw_input_global_wrapper_stencil_1 = (uint16_t) (hw_input_global_wrapper_stencil.extract<0, 15>());

  uint16_t _392 = _hw_input_global_wrapper_stencil_1 + _hw_input_b_global_wrapper_stencil_1;
  return _392;
}

//store is: hw_output.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi) = add.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi)
hw_uint<16> hcompute_hw_output_stencil(hw_uint<16>& add_stencil) {
  uint16_t _add_stencil_1 = (uint16_t) (add_stencil.extract<0, 15>());

  return _add_stencil_1;
}

