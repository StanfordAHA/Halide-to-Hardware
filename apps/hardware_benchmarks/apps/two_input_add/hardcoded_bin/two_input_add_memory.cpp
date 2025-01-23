#include "app.h"
#include "ubuffer.h"
#include "codegen.h"
#include "prog.h"

prog two_input_add() {
  prog prg;
  prg.compute_unit_file = "two_input_add_compute.h";
  prg.name = "two_input_add";

// Stencil<uint16_t, 64, 64> &hw_input_stencil = arg_0;
  prg.add_input("hw_input_stencil");
  prg.buffer_port_widths["hw_input_stencil"] = 16;
// Stencil<uint16_t, 64, 64> &hw_input_b_stencil = arg_1;
  prg.add_input("hw_input_b_stencil");
  prg.buffer_port_widths["hw_input_b_stencil"] = 16;
// Stencil<uint16_t, 64, 64> &hw_output_stencil = arg_2;
  prg.add_output("hw_output_stencil");
  prg.buffer_port_widths["hw_output_stencil"] = 16;

////producing hw_input_b_global_wrapper.stencil
  auto hw_input_b_global_wrapper_s0_y = prg.add_loop("hw_input_b_global_wrapper_s0_y", 0, 64);
  auto hw_input_b_global_wrapper_s0_x_x = hw_input_b_global_wrapper_s0_y->add_loop("hw_input_b_global_wrapper_s0_x_x", 0, 64);

//store is: hw_input_b_global_wrapper.stencil(hw_input_b_global_wrapper_s0_x_x, hw_input_b_global_wrapper_s0_y) = hw_input_b.stencil(hw_input_b_global_wrapper_s0_x_x, hw_input_b_global_wrapper_s0_y)
  auto hcompute_hw_input_b_global_wrapper_stencil = hw_input_b_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_b_global_wrapper_stencil");
  hcompute_hw_input_b_global_wrapper_stencil->add_function("hcompute_hw_input_b_global_wrapper_stencil");
  hcompute_hw_input_b_global_wrapper_stencil->add_load("hw_input_b_stencil", "hw_input_b_global_wrapper_s0_y", "hw_input_b_global_wrapper_s0_x_x");
  prg.buffer_port_widths["hw_input_b_global_wrapper_stencil"] = 16;
  hcompute_hw_input_b_global_wrapper_stencil->add_store("hw_input_b_global_wrapper_stencil", "hw_input_b_global_wrapper_s0_y", "hw_input_b_global_wrapper_s0_x_x");

//consuming hw_input_b_global_wrapper.stencil
////producing hw_input_global_wrapper.stencil
  auto hw_input_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_s0_y", 0, 64);
  auto hw_input_global_wrapper_s0_x_x = hw_input_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_s0_x_x", 0, 64);

//store is: hw_input_global_wrapper.stencil(hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_stencil = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_stencil->add_function("hcompute_hw_input_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_stencil->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "hw_input_global_wrapper_s0_x_x");
  prg.buffer_port_widths["hw_input_global_wrapper_stencil"] = 16;
  hcompute_hw_input_global_wrapper_stencil->add_store("hw_input_global_wrapper_stencil", "hw_input_global_wrapper_s0_y", "hw_input_global_wrapper_s0_x_x");

//consuming hw_input_global_wrapper.stencil
////producing add.stencil
  auto add_s0_y = prg.add_loop("add_s0_y", 0, 64);
  auto add_s0_x_x = add_s0_y->add_loop("add_s0_x_x", 0, 64);

//store is: add.stencil(add_s0_x_x, add_s0_y) = (hw_input_global_wrapper.stencil(add_s0_x_x, add_s0_y) + hw_input_b_global_wrapper.stencil(add_s0_x_x, add_s0_y))
  auto hcompute_add_stencil = add_s0_x_x->add_op("op_hcompute_add_stencil");
  hcompute_add_stencil->add_function("hcompute_add_stencil");
  hcompute_add_stencil->add_load("hw_input_b_global_wrapper_stencil", "add_s0_y", "add_s0_x_x");
  hcompute_add_stencil->add_load("hw_input_global_wrapper_stencil", "add_s0_y", "add_s0_x_x");
  prg.buffer_port_widths["add_stencil"] = 16;
  hcompute_add_stencil->add_store("add_stencil", "add_s0_y", "add_s0_x_x");

//consuming add.stencil
  auto hw_output_s0_y_yi = prg.add_loop("hw_output_s0_y_yi", 0, 64);
  auto hw_output_s0_x_xi_xi = hw_output_s0_y_yi->add_loop("hw_output_s0_x_xi_xi", 0, 64);

//store is: hw_output.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi) = add.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi)
  auto hcompute_hw_output_stencil = hw_output_s0_x_xi_xi->add_op("op_hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_function("hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_load("add_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi");
  hcompute_hw_output_stencil->add_store("hw_output_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi");

  return prg;
}


// empty
