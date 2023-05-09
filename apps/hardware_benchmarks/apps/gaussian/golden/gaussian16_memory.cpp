#include "app.h"
#include "ubuffer.h"
#include "codegen.h"
#include "prog.h"

prog gaussian() {
  prog prg;
  prg.compute_unit_file = "gaussian_compute.h";
  prg.name = "gaussian";

// Stencil<uint16_t, 8944, 3922> &hw_input_stencil = arg_1;
  prg.add_input("hw_input_stencil");
  prg.buffer_port_widths["hw_input_stencil"] = 16;
// Stencil<uint16_t, 4608, 3920> &hw_output_global_wrapper_glb_stencil = arg_3;
  prg.add_output("hw_output_global_wrapper_glb_stencil");
  prg.buffer_port_widths["hw_output_global_wrapper_glb_stencil"] = 16;

////producing hw_input_global_wrapper.glb.stencil
  auto hw_input_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_s0_y", 0, 198);
  auto hw_input_global_wrapper_s0_x_x = hw_input_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_s0_x_x", 0, 19);

//store is: hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_s0_x_x*16), hw_input_global_wrapper_s0_y) = hw_input.stencil((hw_input_global_wrapper_s0_x_x*16), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil");
  hcompute_hw_input_global_wrapper_glb_stencil->add_function("hcompute_hw_input_global_wrapper_glb_stencil");
  hcompute_hw_input_global_wrapper_glb_stencil->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "(hw_input_global_wrapper_s0_x_x*16)");
  prg.buffer_port_widths["hw_input_global_wrapper_glb_stencil"] = 16;
  hcompute_hw_input_global_wrapper_glb_stencil->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "(hw_input_global_wrapper_s0_x_x*16)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_1 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_function("hcompute_hw_input_global_wrapper_glb_stencil_1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 1)");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 1)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_2 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_function("hcompute_hw_input_global_wrapper_glb_stencil_2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 2)");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 2)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_3 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_3");
  hcompute_hw_input_global_wrapper_glb_stencil_3->add_function("hcompute_hw_input_global_wrapper_glb_stencil_3");
  hcompute_hw_input_global_wrapper_glb_stencil_3->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 3)");
  hcompute_hw_input_global_wrapper_glb_stencil_3->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 3)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_4 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_4");
  hcompute_hw_input_global_wrapper_glb_stencil_4->add_function("hcompute_hw_input_global_wrapper_glb_stencil_4");
  hcompute_hw_input_global_wrapper_glb_stencil_4->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 4)");
  hcompute_hw_input_global_wrapper_glb_stencil_4->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 4)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_5 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_5");
  hcompute_hw_input_global_wrapper_glb_stencil_5->add_function("hcompute_hw_input_global_wrapper_glb_stencil_5");
  hcompute_hw_input_global_wrapper_glb_stencil_5->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 5)");
  hcompute_hw_input_global_wrapper_glb_stencil_5->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 5)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_6 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_6");
  hcompute_hw_input_global_wrapper_glb_stencil_6->add_function("hcompute_hw_input_global_wrapper_glb_stencil_6");
  hcompute_hw_input_global_wrapper_glb_stencil_6->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 6)");
  hcompute_hw_input_global_wrapper_glb_stencil_6->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 6)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_7 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_7");
  hcompute_hw_input_global_wrapper_glb_stencil_7->add_function("hcompute_hw_input_global_wrapper_glb_stencil_7");
  hcompute_hw_input_global_wrapper_glb_stencil_7->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 7)");
  hcompute_hw_input_global_wrapper_glb_stencil_7->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 7)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_8 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_8");
  hcompute_hw_input_global_wrapper_glb_stencil_8->add_function("hcompute_hw_input_global_wrapper_glb_stencil_8");
  hcompute_hw_input_global_wrapper_glb_stencil_8->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 8)");
  hcompute_hw_input_global_wrapper_glb_stencil_8->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 8)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_9 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_9");
  hcompute_hw_input_global_wrapper_glb_stencil_9->add_function("hcompute_hw_input_global_wrapper_glb_stencil_9");
  hcompute_hw_input_global_wrapper_glb_stencil_9->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 9)");
  hcompute_hw_input_global_wrapper_glb_stencil_9->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 9)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_10 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_10");
  hcompute_hw_input_global_wrapper_glb_stencil_10->add_function("hcompute_hw_input_global_wrapper_glb_stencil_10");
  hcompute_hw_input_global_wrapper_glb_stencil_10->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 10)");
  hcompute_hw_input_global_wrapper_glb_stencil_10->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 10)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_11 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_11");
  hcompute_hw_input_global_wrapper_glb_stencil_11->add_function("hcompute_hw_input_global_wrapper_glb_stencil_11");
  hcompute_hw_input_global_wrapper_glb_stencil_11->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 11)");
  hcompute_hw_input_global_wrapper_glb_stencil_11->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 11)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_12 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_12");
  hcompute_hw_input_global_wrapper_glb_stencil_12->add_function("hcompute_hw_input_global_wrapper_glb_stencil_12");
  hcompute_hw_input_global_wrapper_glb_stencil_12->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 12)");
  hcompute_hw_input_global_wrapper_glb_stencil_12->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 12)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_13 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_13");
  hcompute_hw_input_global_wrapper_glb_stencil_13->add_function("hcompute_hw_input_global_wrapper_glb_stencil_13");
  hcompute_hw_input_global_wrapper_glb_stencil_13->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 13)");
  hcompute_hw_input_global_wrapper_glb_stencil_13->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 13)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_14 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_14");
  hcompute_hw_input_global_wrapper_glb_stencil_14->add_function("hcompute_hw_input_global_wrapper_glb_stencil_14");
  hcompute_hw_input_global_wrapper_glb_stencil_14->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 14)");
  hcompute_hw_input_global_wrapper_glb_stencil_14->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 14)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_15 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_15");
  hcompute_hw_input_global_wrapper_glb_stencil_15->add_function("hcompute_hw_input_global_wrapper_glb_stencil_15");
  hcompute_hw_input_global_wrapper_glb_stencil_15->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 15)");
  hcompute_hw_input_global_wrapper_glb_stencil_15->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*16) + 15)");

//consuming hw_input_global_wrapper.glb.stencil
////producing hw_output.stencil
////producing hw_input_global_wrapper_global_wrapper.stencil
  auto hw_input_global_wrapper_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_global_wrapper_s0_y", 0, 198);
  auto hw_input_global_wrapper_global_wrapper_s0_x_x = hw_input_global_wrapper_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_global_wrapper_s0_x_x", 0, 19);

//store is: hw_input_global_wrapper_global_wrapper.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*16), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*16), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "(hw_input_global_wrapper_global_wrapper_s0_x_x*16)");
  prg.buffer_port_widths["hw_input_global_wrapper_global_wrapper_stencil"] = 16;
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "(hw_input_global_wrapper_global_wrapper_s0_x_x*16)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 1), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_1 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 1)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 1)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 2), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_2 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 2)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 2)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 3), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_3 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_3");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_3->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_3");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_3->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 3)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_3->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 3)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 4), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_4 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_4");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_4->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_4");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_4->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 4)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_4->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 4)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 5), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_5 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_5");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_5->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_5");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_5->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 5)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_5->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 5)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 6), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_6 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_6");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_6->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_6");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_6->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 6)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_6->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 6)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 7), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_7 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_7");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_7->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_7");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_7->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 7)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_7->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 7)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 8), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_8 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_8");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_8->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_8");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_8->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 8)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_8->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 8)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 9), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_9 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_9");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_9->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_9");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_9->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 9)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_9->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 9)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 10), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_10 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_10");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_10->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_10");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_10->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 10)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_10->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 10)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 11), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_11 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_11");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_11->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_11");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_11->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 11)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_11->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 11)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 12), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_12 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_12");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_12->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_12");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_12->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 12)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_12->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 12)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 13), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_13 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_13");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_13->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_13");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_13->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 13)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_13->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 13)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 14), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_14 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_14");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_14->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_14");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_14->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 14)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_14->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 14)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 15), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_15 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_15");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_15->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_15");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_15->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 15)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_15->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*16) + 15)");

//consuming hw_input_global_wrapper_global_wrapper.stencil
////producing blur_unnormalized.stencil
  auto blur_unnormalized_s0_y = prg.add_loop("blur_unnormalized_s0_y", 0, 196);
  auto blur_unnormalized_s0_x_x = blur_unnormalized_s0_y->add_loop("blur_unnormalized_s0_x_x", 0, 18);

//store is: blur_unnormalized.stencil((blur_unnormalized_s0_x_x*16), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil");
  hcompute_blur_unnormalized_stencil->add_function("hcompute_blur_unnormalized_stencil");
  prg.buffer_port_widths["blur_unnormalized_stencil"] = 16;
  hcompute_blur_unnormalized_stencil->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "(blur_unnormalized_s0_x_x*16)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 1), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_1 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_1");
  hcompute_blur_unnormalized_stencil_1->add_function("hcompute_blur_unnormalized_stencil_1");
  hcompute_blur_unnormalized_stencil_1->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 1)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 2), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_2 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_2");
  hcompute_blur_unnormalized_stencil_2->add_function("hcompute_blur_unnormalized_stencil_2");
  hcompute_blur_unnormalized_stencil_2->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 2)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 3), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_3 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_3");
  hcompute_blur_unnormalized_stencil_3->add_function("hcompute_blur_unnormalized_stencil_3");
  hcompute_blur_unnormalized_stencil_3->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 3)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 4), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_4 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_4");
  hcompute_blur_unnormalized_stencil_4->add_function("hcompute_blur_unnormalized_stencil_4");
  hcompute_blur_unnormalized_stencil_4->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 4)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 5), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_5 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_5");
  hcompute_blur_unnormalized_stencil_5->add_function("hcompute_blur_unnormalized_stencil_5");
  hcompute_blur_unnormalized_stencil_5->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 5)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 6), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_6 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_6");
  hcompute_blur_unnormalized_stencil_6->add_function("hcompute_blur_unnormalized_stencil_6");
  hcompute_blur_unnormalized_stencil_6->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 6)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 7), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_7 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_7");
  hcompute_blur_unnormalized_stencil_7->add_function("hcompute_blur_unnormalized_stencil_7");
  hcompute_blur_unnormalized_stencil_7->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 7)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 8), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_8 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_8");
  hcompute_blur_unnormalized_stencil_8->add_function("hcompute_blur_unnormalized_stencil_8");
  hcompute_blur_unnormalized_stencil_8->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 8)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 9), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_9 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_9");
  hcompute_blur_unnormalized_stencil_9->add_function("hcompute_blur_unnormalized_stencil_9");
  hcompute_blur_unnormalized_stencil_9->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 9)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 10), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_10 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_10");
  hcompute_blur_unnormalized_stencil_10->add_function("hcompute_blur_unnormalized_stencil_10");
  hcompute_blur_unnormalized_stencil_10->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 10)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 11), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_11 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_11");
  hcompute_blur_unnormalized_stencil_11->add_function("hcompute_blur_unnormalized_stencil_11");
  hcompute_blur_unnormalized_stencil_11->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 11)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 12), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_12 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_12");
  hcompute_blur_unnormalized_stencil_12->add_function("hcompute_blur_unnormalized_stencil_12");
  hcompute_blur_unnormalized_stencil_12->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 12)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 13), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_13 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_13");
  hcompute_blur_unnormalized_stencil_13->add_function("hcompute_blur_unnormalized_stencil_13");
  hcompute_blur_unnormalized_stencil_13->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 13)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 14), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_14 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_14");
  hcompute_blur_unnormalized_stencil_14->add_function("hcompute_blur_unnormalized_stencil_14");
  hcompute_blur_unnormalized_stencil_14->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 14)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*16) + 15), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_15 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_15");
  hcompute_blur_unnormalized_stencil_15->add_function("hcompute_blur_unnormalized_stencil_15");
  hcompute_blur_unnormalized_stencil_15->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*16) + 15)");
  auto blur_unnormalized_s1_y = prg.add_loop("blur_unnormalized_s1_y", 0, 196);
  auto blur_unnormalized_s1_x_x = blur_unnormalized_s1_y->add_loop("blur_unnormalized_s1_x_x", 0, 18);

//store is: blur_unnormalized.stencil((blur_unnormalized_s1_x_x*16), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*16), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil((blur_unnormalized_s1_x_x*16), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*16), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_16 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_16");
  hcompute_blur_unnormalized_stencil_16->add_function("hcompute_blur_unnormalized_stencil_16");
  hcompute_blur_unnormalized_stencil_16->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x*16)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x*16)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x*16)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x*16)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_16->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_16->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x*16)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 1), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 1), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_17 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_17");
  hcompute_blur_unnormalized_stencil_17->add_function("hcompute_blur_unnormalized_stencil_17");
  hcompute_blur_unnormalized_stencil_17->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 1)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_17->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_17->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 1)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 2), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_18 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_18");
  hcompute_blur_unnormalized_stencil_18->add_function("hcompute_blur_unnormalized_stencil_18");
  hcompute_blur_unnormalized_stencil_18->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 2)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_18->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_18->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 2)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 3), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_19 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_19");
  hcompute_blur_unnormalized_stencil_19->add_function("hcompute_blur_unnormalized_stencil_19");
  hcompute_blur_unnormalized_stencil_19->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 3)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_19->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_19->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 3)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 4), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_20 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_20");
  hcompute_blur_unnormalized_stencil_20->add_function("hcompute_blur_unnormalized_stencil_20");
  hcompute_blur_unnormalized_stencil_20->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 4)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_20->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_20->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 4)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 5), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_21 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_21");
  hcompute_blur_unnormalized_stencil_21->add_function("hcompute_blur_unnormalized_stencil_21");
  hcompute_blur_unnormalized_stencil_21->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 5)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_21->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_21->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 5)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 6), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_22 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_22");
  hcompute_blur_unnormalized_stencil_22->add_function("hcompute_blur_unnormalized_stencil_22");
  hcompute_blur_unnormalized_stencil_22->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 6)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_22->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_22->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 6)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 7), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_23 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_23");
  hcompute_blur_unnormalized_stencil_23->add_function("hcompute_blur_unnormalized_stencil_23");
  hcompute_blur_unnormalized_stencil_23->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 7)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_23->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_23->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 7)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 8), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_24 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_24");
  hcompute_blur_unnormalized_stencil_24->add_function("hcompute_blur_unnormalized_stencil_24");
  hcompute_blur_unnormalized_stencil_24->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 8)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_24->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_24->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 8)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 9), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_25 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_25");
  hcompute_blur_unnormalized_stencil_25->add_function("hcompute_blur_unnormalized_stencil_25");
  hcompute_blur_unnormalized_stencil_25->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 9)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_25->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_25->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 9)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 10), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_26 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_26");
  hcompute_blur_unnormalized_stencil_26->add_function("hcompute_blur_unnormalized_stencil_26");
  hcompute_blur_unnormalized_stencil_26->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 10)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_26->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_26->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 10)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 11), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_27 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_27");
  hcompute_blur_unnormalized_stencil_27->add_function("hcompute_blur_unnormalized_stencil_27");
  hcompute_blur_unnormalized_stencil_27->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 11)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_27->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_27->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 11)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 12), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_28 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_28");
  hcompute_blur_unnormalized_stencil_28->add_function("hcompute_blur_unnormalized_stencil_28");
  hcompute_blur_unnormalized_stencil_28->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 12)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_28->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_28->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 12)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 13), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_29 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_29");
  hcompute_blur_unnormalized_stencil_29->add_function("hcompute_blur_unnormalized_stencil_29");
  hcompute_blur_unnormalized_stencil_29->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 13)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_29->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_29->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 13)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 14), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_30 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_30");
  hcompute_blur_unnormalized_stencil_30->add_function("hcompute_blur_unnormalized_stencil_30");
  hcompute_blur_unnormalized_stencil_30->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 16)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 16)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 14)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 16)");
  hcompute_blur_unnormalized_stencil_30->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_30->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 14)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*16) + 15), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 17), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 17), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 17), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*16) + 16), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_31 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_31");
  hcompute_blur_unnormalized_stencil_31->add_function("hcompute_blur_unnormalized_stencil_31");
  hcompute_blur_unnormalized_stencil_31->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 16)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 17)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 16)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*16) + 17)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 15)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 17)");
  hcompute_blur_unnormalized_stencil_31->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*16) + 16)");
  hcompute_blur_unnormalized_stencil_31->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*16) + 15)");

//consuming blur_unnormalized.stencil
////producing blur.stencil
  auto blur_s0_y = prg.add_loop("blur_s0_y", 0, 196);
  auto blur_s0_x_x = blur_s0_y->add_loop("blur_s0_x_x", 0, 18);

//store is: blur.stencil((blur_s0_x_x*16), blur_s0_y) = (blur_unnormalized.stencil((blur_s0_x_x*16), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil = blur_s0_x_x->add_op("op_hcompute_blur_stencil");
  hcompute_blur_stencil->add_function("hcompute_blur_stencil");
  hcompute_blur_stencil->add_load("blur_unnormalized_stencil", "blur_s0_y", "(blur_s0_x_x*16)");
  prg.buffer_port_widths["blur_stencil"] = 16;
  hcompute_blur_stencil->add_store("blur_stencil", "blur_s0_y", "(blur_s0_x_x*16)");

//store is: blur.stencil(((blur_s0_x_x*16) + 1), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 1), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_1 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_1");
  hcompute_blur_stencil_1->add_function("hcompute_blur_stencil_1");
  hcompute_blur_stencil_1->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 1)");
  hcompute_blur_stencil_1->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 1)");

//store is: blur.stencil(((blur_s0_x_x*16) + 2), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 2), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_2 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_2");
  hcompute_blur_stencil_2->add_function("hcompute_blur_stencil_2");
  hcompute_blur_stencil_2->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 2)");
  hcompute_blur_stencil_2->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 2)");

//store is: blur.stencil(((blur_s0_x_x*16) + 3), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 3), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_3 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_3");
  hcompute_blur_stencil_3->add_function("hcompute_blur_stencil_3");
  hcompute_blur_stencil_3->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 3)");
  hcompute_blur_stencil_3->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 3)");

//store is: blur.stencil(((blur_s0_x_x*16) + 4), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 4), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_4 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_4");
  hcompute_blur_stencil_4->add_function("hcompute_blur_stencil_4");
  hcompute_blur_stencil_4->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 4)");
  hcompute_blur_stencil_4->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 4)");

//store is: blur.stencil(((blur_s0_x_x*16) + 5), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 5), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_5 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_5");
  hcompute_blur_stencil_5->add_function("hcompute_blur_stencil_5");
  hcompute_blur_stencil_5->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 5)");
  hcompute_blur_stencil_5->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 5)");

//store is: blur.stencil(((blur_s0_x_x*16) + 6), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 6), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_6 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_6");
  hcompute_blur_stencil_6->add_function("hcompute_blur_stencil_6");
  hcompute_blur_stencil_6->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 6)");
  hcompute_blur_stencil_6->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 6)");

//store is: blur.stencil(((blur_s0_x_x*16) + 7), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 7), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_7 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_7");
  hcompute_blur_stencil_7->add_function("hcompute_blur_stencil_7");
  hcompute_blur_stencil_7->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 7)");
  hcompute_blur_stencil_7->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 7)");

//store is: blur.stencil(((blur_s0_x_x*16) + 8), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 8), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_8 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_8");
  hcompute_blur_stencil_8->add_function("hcompute_blur_stencil_8");
  hcompute_blur_stencil_8->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 8)");
  hcompute_blur_stencil_8->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 8)");

//store is: blur.stencil(((blur_s0_x_x*16) + 9), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 9), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_9 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_9");
  hcompute_blur_stencil_9->add_function("hcompute_blur_stencil_9");
  hcompute_blur_stencil_9->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 9)");
  hcompute_blur_stencil_9->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 9)");

//store is: blur.stencil(((blur_s0_x_x*16) + 10), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 10), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_10 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_10");
  hcompute_blur_stencil_10->add_function("hcompute_blur_stencil_10");
  hcompute_blur_stencil_10->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 10)");
  hcompute_blur_stencil_10->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 10)");

//store is: blur.stencil(((blur_s0_x_x*16) + 11), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 11), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_11 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_11");
  hcompute_blur_stencil_11->add_function("hcompute_blur_stencil_11");
  hcompute_blur_stencil_11->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 11)");
  hcompute_blur_stencil_11->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 11)");

//store is: blur.stencil(((blur_s0_x_x*16) + 12), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 12), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_12 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_12");
  hcompute_blur_stencil_12->add_function("hcompute_blur_stencil_12");
  hcompute_blur_stencil_12->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 12)");
  hcompute_blur_stencil_12->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 12)");

//store is: blur.stencil(((blur_s0_x_x*16) + 13), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 13), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_13 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_13");
  hcompute_blur_stencil_13->add_function("hcompute_blur_stencil_13");
  hcompute_blur_stencil_13->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 13)");
  hcompute_blur_stencil_13->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 13)");

//store is: blur.stencil(((blur_s0_x_x*16) + 14), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 14), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_14 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_14");
  hcompute_blur_stencil_14->add_function("hcompute_blur_stencil_14");
  hcompute_blur_stencil_14->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 14)");
  hcompute_blur_stencil_14->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 14)");

//store is: blur.stencil(((blur_s0_x_x*16) + 15), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*16) + 15), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_15 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_15");
  hcompute_blur_stencil_15->add_function("hcompute_blur_stencil_15");
  hcompute_blur_stencil_15->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 15)");
  hcompute_blur_stencil_15->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*16) + 15)");

//consuming blur.stencil
  auto hw_output_s0_y_yii = prg.add_loop("hw_output_s0_y_yii", 0, 196);
  auto hw_output_s0_x_xii_xii = hw_output_s0_y_yii->add_loop("hw_output_s0_x_xii_xii", 0, 18);

//store is: hw_output.stencil((hw_output_s0_x_xii_xii*16), hw_output_s0_y_yii) = blur.stencil((hw_output_s0_x_xii_xii*16), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_function("hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_load("blur_stencil", "hw_output_s0_y_yii", "(hw_output_s0_x_xii_xii*16)");
  prg.buffer_port_widths["hw_output_stencil"] = 16;
  hcompute_hw_output_stencil->add_store("hw_output_stencil", "hw_output_s0_y_yii", "(hw_output_s0_x_xii_xii*16)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 1), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 1), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_1 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_1");
  hcompute_hw_output_stencil_1->add_function("hcompute_hw_output_stencil_1");
  hcompute_hw_output_stencil_1->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 1)");
  hcompute_hw_output_stencil_1->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 1)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 2), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 2), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_2 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_2");
  hcompute_hw_output_stencil_2->add_function("hcompute_hw_output_stencil_2");
  hcompute_hw_output_stencil_2->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 2)");
  hcompute_hw_output_stencil_2->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 2)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 3), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 3), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_3 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_3");
  hcompute_hw_output_stencil_3->add_function("hcompute_hw_output_stencil_3");
  hcompute_hw_output_stencil_3->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 3)");
  hcompute_hw_output_stencil_3->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 3)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 4), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 4), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_4 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_4");
  hcompute_hw_output_stencil_4->add_function("hcompute_hw_output_stencil_4");
  hcompute_hw_output_stencil_4->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 4)");
  hcompute_hw_output_stencil_4->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 4)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 5), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 5), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_5 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_5");
  hcompute_hw_output_stencil_5->add_function("hcompute_hw_output_stencil_5");
  hcompute_hw_output_stencil_5->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 5)");
  hcompute_hw_output_stencil_5->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 5)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 6), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 6), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_6 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_6");
  hcompute_hw_output_stencil_6->add_function("hcompute_hw_output_stencil_6");
  hcompute_hw_output_stencil_6->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 6)");
  hcompute_hw_output_stencil_6->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 6)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 7), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 7), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_7 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_7");
  hcompute_hw_output_stencil_7->add_function("hcompute_hw_output_stencil_7");
  hcompute_hw_output_stencil_7->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 7)");
  hcompute_hw_output_stencil_7->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 7)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 8), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 8), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_8 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_8");
  hcompute_hw_output_stencil_8->add_function("hcompute_hw_output_stencil_8");
  hcompute_hw_output_stencil_8->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 8)");
  hcompute_hw_output_stencil_8->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 8)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 9), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 9), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_9 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_9");
  hcompute_hw_output_stencil_9->add_function("hcompute_hw_output_stencil_9");
  hcompute_hw_output_stencil_9->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 9)");
  hcompute_hw_output_stencil_9->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 9)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 10), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 10), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_10 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_10");
  hcompute_hw_output_stencil_10->add_function("hcompute_hw_output_stencil_10");
  hcompute_hw_output_stencil_10->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 10)");
  hcompute_hw_output_stencil_10->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 10)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 11), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 11), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_11 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_11");
  hcompute_hw_output_stencil_11->add_function("hcompute_hw_output_stencil_11");
  hcompute_hw_output_stencil_11->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 11)");
  hcompute_hw_output_stencil_11->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 11)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 12), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 12), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_12 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_12");
  hcompute_hw_output_stencil_12->add_function("hcompute_hw_output_stencil_12");
  hcompute_hw_output_stencil_12->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 12)");
  hcompute_hw_output_stencil_12->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 12)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 13), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 13), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_13 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_13");
  hcompute_hw_output_stencil_13->add_function("hcompute_hw_output_stencil_13");
  hcompute_hw_output_stencil_13->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 13)");
  hcompute_hw_output_stencil_13->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 13)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 14), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 14), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_14 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_14");
  hcompute_hw_output_stencil_14->add_function("hcompute_hw_output_stencil_14");
  hcompute_hw_output_stencil_14->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 14)");
  hcompute_hw_output_stencil_14->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 14)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*16) + 15), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*16) + 15), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_15 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_15");
  hcompute_hw_output_stencil_15->add_function("hcompute_hw_output_stencil_15");
  hcompute_hw_output_stencil_15->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 15)");
  hcompute_hw_output_stencil_15->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*16) + 15)");

//consuming hw_output.stencil
  auto hw_output_global_wrapper_s0_y_yi = prg.add_loop("hw_output_global_wrapper_s0_y_yi", 0, 196);
  auto hw_output_global_wrapper_s0_x_xi_xi = hw_output_global_wrapper_s0_y_yi->add_loop("hw_output_global_wrapper_s0_x_xi_xi", 0, 18);

//store is: hw_output_global_wrapper.glb.stencil((hw_output_global_wrapper_s0_x_xi_xi*16), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil((hw_output_global_wrapper_s0_x_xi_xi*16), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil");
  hcompute_hw_output_global_wrapper_glb_stencil->add_function("hcompute_hw_output_global_wrapper_glb_stencil");
  hcompute_hw_output_global_wrapper_glb_stencil->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "(hw_output_global_wrapper_s0_x_xi_xi*16)");
  hcompute_hw_output_global_wrapper_glb_stencil->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "(hw_output_global_wrapper_s0_x_xi_xi*16)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 1), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 1), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_1 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_function("hcompute_hw_output_global_wrapper_glb_stencil_1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 1)");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 1)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 2), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 2), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_2 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_function("hcompute_hw_output_global_wrapper_glb_stencil_2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 2)");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 2)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 3), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 3), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_3 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_function("hcompute_hw_output_global_wrapper_glb_stencil_3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 3)");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 3)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 4), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 4), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_4 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_function("hcompute_hw_output_global_wrapper_glb_stencil_4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 4)");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 4)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 5), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 5), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_5 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_function("hcompute_hw_output_global_wrapper_glb_stencil_5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 5)");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 5)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 6), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 6), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_6 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_function("hcompute_hw_output_global_wrapper_glb_stencil_6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 6)");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 6)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 7), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 7), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_7 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_7");
  hcompute_hw_output_global_wrapper_glb_stencil_7->add_function("hcompute_hw_output_global_wrapper_glb_stencil_7");
  hcompute_hw_output_global_wrapper_glb_stencil_7->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 7)");
  hcompute_hw_output_global_wrapper_glb_stencil_7->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 7)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 8), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 8), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_8 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_8");
  hcompute_hw_output_global_wrapper_glb_stencil_8->add_function("hcompute_hw_output_global_wrapper_glb_stencil_8");
  hcompute_hw_output_global_wrapper_glb_stencil_8->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 8)");
  hcompute_hw_output_global_wrapper_glb_stencil_8->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 8)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 9), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 9), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_9 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_9");
  hcompute_hw_output_global_wrapper_glb_stencil_9->add_function("hcompute_hw_output_global_wrapper_glb_stencil_9");
  hcompute_hw_output_global_wrapper_glb_stencil_9->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 9)");
  hcompute_hw_output_global_wrapper_glb_stencil_9->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 9)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 10), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 10), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_10 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_10");
  hcompute_hw_output_global_wrapper_glb_stencil_10->add_function("hcompute_hw_output_global_wrapper_glb_stencil_10");
  hcompute_hw_output_global_wrapper_glb_stencil_10->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 10)");
  hcompute_hw_output_global_wrapper_glb_stencil_10->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 10)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 11), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 11), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_11 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_11");
  hcompute_hw_output_global_wrapper_glb_stencil_11->add_function("hcompute_hw_output_global_wrapper_glb_stencil_11");
  hcompute_hw_output_global_wrapper_glb_stencil_11->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 11)");
  hcompute_hw_output_global_wrapper_glb_stencil_11->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 11)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 12), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 12), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_12 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_12");
  hcompute_hw_output_global_wrapper_glb_stencil_12->add_function("hcompute_hw_output_global_wrapper_glb_stencil_12");
  hcompute_hw_output_global_wrapper_glb_stencil_12->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 12)");
  hcompute_hw_output_global_wrapper_glb_stencil_12->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 12)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 13), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 13), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_13 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_13");
  hcompute_hw_output_global_wrapper_glb_stencil_13->add_function("hcompute_hw_output_global_wrapper_glb_stencil_13");
  hcompute_hw_output_global_wrapper_glb_stencil_13->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 13)");
  hcompute_hw_output_global_wrapper_glb_stencil_13->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 13)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 14), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 14), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_14 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_14");
  hcompute_hw_output_global_wrapper_glb_stencil_14->add_function("hcompute_hw_output_global_wrapper_glb_stencil_14");
  hcompute_hw_output_global_wrapper_glb_stencil_14->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 14)");
  hcompute_hw_output_global_wrapper_glb_stencil_14->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 14)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 15), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*16) + 15), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_15 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_15");
  hcompute_hw_output_global_wrapper_glb_stencil_15->add_function("hcompute_hw_output_global_wrapper_glb_stencil_15");
  hcompute_hw_output_global_wrapper_glb_stencil_15->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 15)");
  hcompute_hw_output_global_wrapper_glb_stencil_15->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*16) + 15)");

  return prg;
}


// schedule=3 myunroll=16 mywidth=288
