#include "app.h"
#include "ubuffer.h"
#include "codegen.h"
#include "prog.h"

prog gaussian() {
  prog prg;
  prg.compute_unit_file = "gaussian_compute.h";
  prg.name = "gaussian";

// Stencil<uint16_t, 8960, 3922> &hw_input_stencil = arg_1;
  prg.add_input("hw_input_stencil");
  prg.buffer_port_widths["hw_input_stencil"] = 16;
// Stencil<uint16_t, 4608, 3920> &hw_output_global_wrapper_glb_stencil = arg_3;
  prg.add_output("hw_output_global_wrapper_glb_stencil");
  prg.buffer_port_widths["hw_output_global_wrapper_glb_stencil"] = 16;

////producing hw_input_global_wrapper.glb.stencil
  auto hw_input_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_s0_y", 0, 198);
  auto hw_input_global_wrapper_s0_x_x = hw_input_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_s0_x_x", 0, 10);

//store is: hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_s0_x_x*32), hw_input_global_wrapper_s0_y) = hw_input.stencil((hw_input_global_wrapper_s0_x_x*32), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil");
  hcompute_hw_input_global_wrapper_glb_stencil->add_function("hcompute_hw_input_global_wrapper_glb_stencil");
  hcompute_hw_input_global_wrapper_glb_stencil->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "(hw_input_global_wrapper_s0_x_x*32)");
  prg.buffer_port_widths["hw_input_global_wrapper_glb_stencil"] = 16;
  hcompute_hw_input_global_wrapper_glb_stencil->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "(hw_input_global_wrapper_s0_x_x*32)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_1 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_function("hcompute_hw_input_global_wrapper_glb_stencil_1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 1)");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 1)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_2 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_function("hcompute_hw_input_global_wrapper_glb_stencil_2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 2)");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 2)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_3 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_3");
  hcompute_hw_input_global_wrapper_glb_stencil_3->add_function("hcompute_hw_input_global_wrapper_glb_stencil_3");
  hcompute_hw_input_global_wrapper_glb_stencil_3->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 3)");
  hcompute_hw_input_global_wrapper_glb_stencil_3->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 3)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_4 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_4");
  hcompute_hw_input_global_wrapper_glb_stencil_4->add_function("hcompute_hw_input_global_wrapper_glb_stencil_4");
  hcompute_hw_input_global_wrapper_glb_stencil_4->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 4)");
  hcompute_hw_input_global_wrapper_glb_stencil_4->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 4)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_5 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_5");
  hcompute_hw_input_global_wrapper_glb_stencil_5->add_function("hcompute_hw_input_global_wrapper_glb_stencil_5");
  hcompute_hw_input_global_wrapper_glb_stencil_5->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 5)");
  hcompute_hw_input_global_wrapper_glb_stencil_5->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 5)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_6 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_6");
  hcompute_hw_input_global_wrapper_glb_stencil_6->add_function("hcompute_hw_input_global_wrapper_glb_stencil_6");
  hcompute_hw_input_global_wrapper_glb_stencil_6->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 6)");
  hcompute_hw_input_global_wrapper_glb_stencil_6->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 6)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_7 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_7");
  hcompute_hw_input_global_wrapper_glb_stencil_7->add_function("hcompute_hw_input_global_wrapper_glb_stencil_7");
  hcompute_hw_input_global_wrapper_glb_stencil_7->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 7)");
  hcompute_hw_input_global_wrapper_glb_stencil_7->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 7)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_8 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_8");
  hcompute_hw_input_global_wrapper_glb_stencil_8->add_function("hcompute_hw_input_global_wrapper_glb_stencil_8");
  hcompute_hw_input_global_wrapper_glb_stencil_8->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 8)");
  hcompute_hw_input_global_wrapper_glb_stencil_8->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 8)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_9 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_9");
  hcompute_hw_input_global_wrapper_glb_stencil_9->add_function("hcompute_hw_input_global_wrapper_glb_stencil_9");
  hcompute_hw_input_global_wrapper_glb_stencil_9->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 9)");
  hcompute_hw_input_global_wrapper_glb_stencil_9->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 9)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_10 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_10");
  hcompute_hw_input_global_wrapper_glb_stencil_10->add_function("hcompute_hw_input_global_wrapper_glb_stencil_10");
  hcompute_hw_input_global_wrapper_glb_stencil_10->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 10)");
  hcompute_hw_input_global_wrapper_glb_stencil_10->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 10)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_11 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_11");
  hcompute_hw_input_global_wrapper_glb_stencil_11->add_function("hcompute_hw_input_global_wrapper_glb_stencil_11");
  hcompute_hw_input_global_wrapper_glb_stencil_11->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 11)");
  hcompute_hw_input_global_wrapper_glb_stencil_11->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 11)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_12 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_12");
  hcompute_hw_input_global_wrapper_glb_stencil_12->add_function("hcompute_hw_input_global_wrapper_glb_stencil_12");
  hcompute_hw_input_global_wrapper_glb_stencil_12->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 12)");
  hcompute_hw_input_global_wrapper_glb_stencil_12->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 12)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_13 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_13");
  hcompute_hw_input_global_wrapper_glb_stencil_13->add_function("hcompute_hw_input_global_wrapper_glb_stencil_13");
  hcompute_hw_input_global_wrapper_glb_stencil_13->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 13)");
  hcompute_hw_input_global_wrapper_glb_stencil_13->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 13)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_14 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_14");
  hcompute_hw_input_global_wrapper_glb_stencil_14->add_function("hcompute_hw_input_global_wrapper_glb_stencil_14");
  hcompute_hw_input_global_wrapper_glb_stencil_14->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 14)");
  hcompute_hw_input_global_wrapper_glb_stencil_14->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 14)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_15 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_15");
  hcompute_hw_input_global_wrapper_glb_stencil_15->add_function("hcompute_hw_input_global_wrapper_glb_stencil_15");
  hcompute_hw_input_global_wrapper_glb_stencil_15->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 15)");
  hcompute_hw_input_global_wrapper_glb_stencil_15->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 15)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_16 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_16");
  hcompute_hw_input_global_wrapper_glb_stencil_16->add_function("hcompute_hw_input_global_wrapper_glb_stencil_16");
  hcompute_hw_input_global_wrapper_glb_stencil_16->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 16)");
  hcompute_hw_input_global_wrapper_glb_stencil_16->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 16)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_17 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_17");
  hcompute_hw_input_global_wrapper_glb_stencil_17->add_function("hcompute_hw_input_global_wrapper_glb_stencil_17");
  hcompute_hw_input_global_wrapper_glb_stencil_17->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 17)");
  hcompute_hw_input_global_wrapper_glb_stencil_17->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 17)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_18 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_18");
  hcompute_hw_input_global_wrapper_glb_stencil_18->add_function("hcompute_hw_input_global_wrapper_glb_stencil_18");
  hcompute_hw_input_global_wrapper_glb_stencil_18->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 18)");
  hcompute_hw_input_global_wrapper_glb_stencil_18->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 18)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_19 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_19");
  hcompute_hw_input_global_wrapper_glb_stencil_19->add_function("hcompute_hw_input_global_wrapper_glb_stencil_19");
  hcompute_hw_input_global_wrapper_glb_stencil_19->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 19)");
  hcompute_hw_input_global_wrapper_glb_stencil_19->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 19)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_20 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_20");
  hcompute_hw_input_global_wrapper_glb_stencil_20->add_function("hcompute_hw_input_global_wrapper_glb_stencil_20");
  hcompute_hw_input_global_wrapper_glb_stencil_20->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 20)");
  hcompute_hw_input_global_wrapper_glb_stencil_20->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 20)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_21 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_21");
  hcompute_hw_input_global_wrapper_glb_stencil_21->add_function("hcompute_hw_input_global_wrapper_glb_stencil_21");
  hcompute_hw_input_global_wrapper_glb_stencil_21->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 21)");
  hcompute_hw_input_global_wrapper_glb_stencil_21->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 21)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_22 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_22");
  hcompute_hw_input_global_wrapper_glb_stencil_22->add_function("hcompute_hw_input_global_wrapper_glb_stencil_22");
  hcompute_hw_input_global_wrapper_glb_stencil_22->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 22)");
  hcompute_hw_input_global_wrapper_glb_stencil_22->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 22)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_23 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_23");
  hcompute_hw_input_global_wrapper_glb_stencil_23->add_function("hcompute_hw_input_global_wrapper_glb_stencil_23");
  hcompute_hw_input_global_wrapper_glb_stencil_23->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 23)");
  hcompute_hw_input_global_wrapper_glb_stencil_23->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 23)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_24 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_24");
  hcompute_hw_input_global_wrapper_glb_stencil_24->add_function("hcompute_hw_input_global_wrapper_glb_stencil_24");
  hcompute_hw_input_global_wrapper_glb_stencil_24->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 24)");
  hcompute_hw_input_global_wrapper_glb_stencil_24->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 24)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_25 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_25");
  hcompute_hw_input_global_wrapper_glb_stencil_25->add_function("hcompute_hw_input_global_wrapper_glb_stencil_25");
  hcompute_hw_input_global_wrapper_glb_stencil_25->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 25)");
  hcompute_hw_input_global_wrapper_glb_stencil_25->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 25)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_26 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_26");
  hcompute_hw_input_global_wrapper_glb_stencil_26->add_function("hcompute_hw_input_global_wrapper_glb_stencil_26");
  hcompute_hw_input_global_wrapper_glb_stencil_26->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 26)");
  hcompute_hw_input_global_wrapper_glb_stencil_26->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 26)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_27 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_27");
  hcompute_hw_input_global_wrapper_glb_stencil_27->add_function("hcompute_hw_input_global_wrapper_glb_stencil_27");
  hcompute_hw_input_global_wrapper_glb_stencil_27->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 27)");
  hcompute_hw_input_global_wrapper_glb_stencil_27->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 27)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_28 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_28");
  hcompute_hw_input_global_wrapper_glb_stencil_28->add_function("hcompute_hw_input_global_wrapper_glb_stencil_28");
  hcompute_hw_input_global_wrapper_glb_stencil_28->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 28)");
  hcompute_hw_input_global_wrapper_glb_stencil_28->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 28)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_29 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_29");
  hcompute_hw_input_global_wrapper_glb_stencil_29->add_function("hcompute_hw_input_global_wrapper_glb_stencil_29");
  hcompute_hw_input_global_wrapper_glb_stencil_29->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 29)");
  hcompute_hw_input_global_wrapper_glb_stencil_29->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 29)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_30 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_30");
  hcompute_hw_input_global_wrapper_glb_stencil_30->add_function("hcompute_hw_input_global_wrapper_glb_stencil_30");
  hcompute_hw_input_global_wrapper_glb_stencil_30->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 30)");
  hcompute_hw_input_global_wrapper_glb_stencil_30->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 30)");

//store is: hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_s0_y) = hw_input.stencil(((hw_input_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_glb_stencil_31 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_31");
  hcompute_hw_input_global_wrapper_glb_stencil_31->add_function("hcompute_hw_input_global_wrapper_glb_stencil_31");
  hcompute_hw_input_global_wrapper_glb_stencil_31->add_load("hw_input_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 31)");
  hcompute_hw_input_global_wrapper_glb_stencil_31->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "((hw_input_global_wrapper_s0_x_x*32) + 31)");

//consuming hw_input_global_wrapper.glb.stencil
////producing hw_output.stencil
////producing hw_input_global_wrapper_global_wrapper.stencil
  auto hw_input_global_wrapper_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_global_wrapper_s0_y", 0, 198);
  auto hw_input_global_wrapper_global_wrapper_s0_x_x = hw_input_global_wrapper_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_global_wrapper_s0_x_x", 0, 10);

//store is: hw_input_global_wrapper_global_wrapper.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*32), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil((hw_input_global_wrapper_global_wrapper_s0_x_x*32), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "(hw_input_global_wrapper_global_wrapper_s0_x_x*32)");
  prg.buffer_port_widths["hw_input_global_wrapper_global_wrapper_stencil"] = 16;
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "(hw_input_global_wrapper_global_wrapper_s0_x_x*32)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 1), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_1 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 1)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 1)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 2), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_2 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 2)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 2)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 3), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_3 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_3");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_3->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_3");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_3->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 3)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_3->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 3)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 4), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_4 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_4");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_4->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_4");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_4->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 4)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_4->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 4)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 5), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_5 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_5");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_5->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_5");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_5->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 5)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_5->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 5)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 6), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_6 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_6");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_6->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_6");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_6->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 6)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_6->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 6)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 7), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_7 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_7");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_7->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_7");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_7->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 7)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_7->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 7)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 8), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_8 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_8");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_8->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_8");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_8->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 8)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_8->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 8)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 9), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_9 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_9");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_9->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_9");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_9->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 9)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_9->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 9)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 10), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_10 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_10");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_10->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_10");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_10->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 10)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_10->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 10)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 11), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_11 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_11");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_11->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_11");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_11->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 11)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_11->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 11)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 12), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_12 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_12");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_12->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_12");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_12->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 12)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_12->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 12)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 13), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_13 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_13");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_13->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_13");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_13->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 13)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_13->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 13)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 14), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_14 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_14");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_14->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_14");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_14->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 14)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_14->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 14)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 15), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_15 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_15");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_15->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_15");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_15->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 15)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_15->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 15)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 16), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_16 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_16");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_16->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_16");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_16->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 16)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_16->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 16)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 17), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_17 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_17");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_17->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_17");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_17->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 17)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_17->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 17)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 18), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_18 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_18");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_18->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_18");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_18->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 18)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_18->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 18)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 19), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_19 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_19");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_19->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_19");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_19->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 19)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_19->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 19)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 20), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_20 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_20");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_20->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_20");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_20->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 20)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_20->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 20)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 21), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_21 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_21");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_21->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_21");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_21->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 21)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_21->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 21)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 22), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_22 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_22");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_22->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_22");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_22->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 22)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_22->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 22)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 23), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_23 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_23");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_23->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_23");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_23->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 23)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_23->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 23)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 24), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_24 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_24");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_24->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_24");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_24->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 24)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_24->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 24)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 25), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_25 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_25");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_25->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_25");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_25->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 25)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_25->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 25)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 26), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_26 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_26");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_26->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_26");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_26->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 26)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_26->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 26)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 27), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_27 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_27");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_27->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_27");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_27->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 27)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_27->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 27)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 28), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_28 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_28");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_28->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_28");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_28->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 28)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_28->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 28)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 29), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_29 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_29");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_29->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_29");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_29->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 29)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_29->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 29)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 30), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_30 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_30");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_30->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_30");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_30->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 30)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_30->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 30)");

//store is: hw_input_global_wrapper_global_wrapper.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 31), hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_31 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_31");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_31->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_31");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_31->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 31)");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_31->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "((hw_input_global_wrapper_global_wrapper_s0_x_x*32) + 31)");

//consuming hw_input_global_wrapper_global_wrapper.stencil
////producing blur_unnormalized.stencil
  auto blur_unnormalized_s0_y = prg.add_loop("blur_unnormalized_s0_y", 0, 196);
  auto blur_unnormalized_s0_x_x = blur_unnormalized_s0_y->add_loop("blur_unnormalized_s0_x_x", 0, 9);

//store is: blur_unnormalized.stencil((blur_unnormalized_s0_x_x*32), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil");
  hcompute_blur_unnormalized_stencil->add_function("hcompute_blur_unnormalized_stencil");
  prg.buffer_port_widths["blur_unnormalized_stencil"] = 16;
  hcompute_blur_unnormalized_stencil->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "(blur_unnormalized_s0_x_x*32)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 1), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_1 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_1");
  hcompute_blur_unnormalized_stencil_1->add_function("hcompute_blur_unnormalized_stencil_1");
  hcompute_blur_unnormalized_stencil_1->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 1)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 2), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_2 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_2");
  hcompute_blur_unnormalized_stencil_2->add_function("hcompute_blur_unnormalized_stencil_2");
  hcompute_blur_unnormalized_stencil_2->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 2)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 3), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_3 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_3");
  hcompute_blur_unnormalized_stencil_3->add_function("hcompute_blur_unnormalized_stencil_3");
  hcompute_blur_unnormalized_stencil_3->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 3)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 4), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_4 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_4");
  hcompute_blur_unnormalized_stencil_4->add_function("hcompute_blur_unnormalized_stencil_4");
  hcompute_blur_unnormalized_stencil_4->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 4)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 5), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_5 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_5");
  hcompute_blur_unnormalized_stencil_5->add_function("hcompute_blur_unnormalized_stencil_5");
  hcompute_blur_unnormalized_stencil_5->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 5)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 6), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_6 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_6");
  hcompute_blur_unnormalized_stencil_6->add_function("hcompute_blur_unnormalized_stencil_6");
  hcompute_blur_unnormalized_stencil_6->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 6)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 7), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_7 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_7");
  hcompute_blur_unnormalized_stencil_7->add_function("hcompute_blur_unnormalized_stencil_7");
  hcompute_blur_unnormalized_stencil_7->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 7)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 8), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_8 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_8");
  hcompute_blur_unnormalized_stencil_8->add_function("hcompute_blur_unnormalized_stencil_8");
  hcompute_blur_unnormalized_stencil_8->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 8)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 9), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_9 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_9");
  hcompute_blur_unnormalized_stencil_9->add_function("hcompute_blur_unnormalized_stencil_9");
  hcompute_blur_unnormalized_stencil_9->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 9)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 10), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_10 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_10");
  hcompute_blur_unnormalized_stencil_10->add_function("hcompute_blur_unnormalized_stencil_10");
  hcompute_blur_unnormalized_stencil_10->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 10)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 11), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_11 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_11");
  hcompute_blur_unnormalized_stencil_11->add_function("hcompute_blur_unnormalized_stencil_11");
  hcompute_blur_unnormalized_stencil_11->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 11)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 12), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_12 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_12");
  hcompute_blur_unnormalized_stencil_12->add_function("hcompute_blur_unnormalized_stencil_12");
  hcompute_blur_unnormalized_stencil_12->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 12)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 13), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_13 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_13");
  hcompute_blur_unnormalized_stencil_13->add_function("hcompute_blur_unnormalized_stencil_13");
  hcompute_blur_unnormalized_stencil_13->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 13)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 14), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_14 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_14");
  hcompute_blur_unnormalized_stencil_14->add_function("hcompute_blur_unnormalized_stencil_14");
  hcompute_blur_unnormalized_stencil_14->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 14)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 15), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_15 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_15");
  hcompute_blur_unnormalized_stencil_15->add_function("hcompute_blur_unnormalized_stencil_15");
  hcompute_blur_unnormalized_stencil_15->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 15)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 16), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_16 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_16");
  hcompute_blur_unnormalized_stencil_16->add_function("hcompute_blur_unnormalized_stencil_16");
  hcompute_blur_unnormalized_stencil_16->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 16)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 17), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_17 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_17");
  hcompute_blur_unnormalized_stencil_17->add_function("hcompute_blur_unnormalized_stencil_17");
  hcompute_blur_unnormalized_stencil_17->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 17)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 18), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_18 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_18");
  hcompute_blur_unnormalized_stencil_18->add_function("hcompute_blur_unnormalized_stencil_18");
  hcompute_blur_unnormalized_stencil_18->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 18)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 19), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_19 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_19");
  hcompute_blur_unnormalized_stencil_19->add_function("hcompute_blur_unnormalized_stencil_19");
  hcompute_blur_unnormalized_stencil_19->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 19)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 20), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_20 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_20");
  hcompute_blur_unnormalized_stencil_20->add_function("hcompute_blur_unnormalized_stencil_20");
  hcompute_blur_unnormalized_stencil_20->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 20)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 21), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_21 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_21");
  hcompute_blur_unnormalized_stencil_21->add_function("hcompute_blur_unnormalized_stencil_21");
  hcompute_blur_unnormalized_stencil_21->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 21)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 22), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_22 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_22");
  hcompute_blur_unnormalized_stencil_22->add_function("hcompute_blur_unnormalized_stencil_22");
  hcompute_blur_unnormalized_stencil_22->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 22)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 23), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_23 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_23");
  hcompute_blur_unnormalized_stencil_23->add_function("hcompute_blur_unnormalized_stencil_23");
  hcompute_blur_unnormalized_stencil_23->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 23)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 24), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_24 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_24");
  hcompute_blur_unnormalized_stencil_24->add_function("hcompute_blur_unnormalized_stencil_24");
  hcompute_blur_unnormalized_stencil_24->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 24)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 25), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_25 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_25");
  hcompute_blur_unnormalized_stencil_25->add_function("hcompute_blur_unnormalized_stencil_25");
  hcompute_blur_unnormalized_stencil_25->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 25)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 26), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_26 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_26");
  hcompute_blur_unnormalized_stencil_26->add_function("hcompute_blur_unnormalized_stencil_26");
  hcompute_blur_unnormalized_stencil_26->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 26)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 27), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_27 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_27");
  hcompute_blur_unnormalized_stencil_27->add_function("hcompute_blur_unnormalized_stencil_27");
  hcompute_blur_unnormalized_stencil_27->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 27)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 28), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_28 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_28");
  hcompute_blur_unnormalized_stencil_28->add_function("hcompute_blur_unnormalized_stencil_28");
  hcompute_blur_unnormalized_stencil_28->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 28)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 29), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_29 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_29");
  hcompute_blur_unnormalized_stencil_29->add_function("hcompute_blur_unnormalized_stencil_29");
  hcompute_blur_unnormalized_stencil_29->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 29)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 30), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_30 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_30");
  hcompute_blur_unnormalized_stencil_30->add_function("hcompute_blur_unnormalized_stencil_30");
  hcompute_blur_unnormalized_stencil_30->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 30)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s0_x_x*32) + 31), blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil_31 = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil_31");
  hcompute_blur_unnormalized_stencil_31->add_function("hcompute_blur_unnormalized_stencil_31");
  hcompute_blur_unnormalized_stencil_31->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "((blur_unnormalized_s0_x_x*32) + 31)");
  auto blur_unnormalized_s1_y = prg.add_loop("blur_unnormalized_s1_y", 0, 196);
  auto blur_unnormalized_s1_x_x = blur_unnormalized_s1_y->add_loop("blur_unnormalized_s1_x_x", 0, 9);

//store is: blur_unnormalized.stencil((blur_unnormalized_s1_x_x*32), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*32), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil((blur_unnormalized_s1_x_x*32), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*32), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil((blur_unnormalized_s1_x_x*32), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_32 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_32");
  hcompute_blur_unnormalized_stencil_32->add_function("hcompute_blur_unnormalized_stencil_32");
  hcompute_blur_unnormalized_stencil_32->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x*32)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x*32)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x*32)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x*32)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_32->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_32->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x*32)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 1), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 1), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_33 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_33");
  hcompute_blur_unnormalized_stencil_33->add_function("hcompute_blur_unnormalized_stencil_33");
  hcompute_blur_unnormalized_stencil_33->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 1)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_33->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_33->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 1)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 2), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 2), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_34 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_34");
  hcompute_blur_unnormalized_stencil_34->add_function("hcompute_blur_unnormalized_stencil_34");
  hcompute_blur_unnormalized_stencil_34->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 2)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_34->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_34->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 2)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 3), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 3), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_35 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_35");
  hcompute_blur_unnormalized_stencil_35->add_function("hcompute_blur_unnormalized_stencil_35");
  hcompute_blur_unnormalized_stencil_35->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 3)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_35->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_35->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 3)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 4), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 4), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_36 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_36");
  hcompute_blur_unnormalized_stencil_36->add_function("hcompute_blur_unnormalized_stencil_36");
  hcompute_blur_unnormalized_stencil_36->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 4)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_36->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_36->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 4)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 5), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 5), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_37 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_37");
  hcompute_blur_unnormalized_stencil_37->add_function("hcompute_blur_unnormalized_stencil_37");
  hcompute_blur_unnormalized_stencil_37->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 5)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_37->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_37->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 5)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 6), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 6), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_38 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_38");
  hcompute_blur_unnormalized_stencil_38->add_function("hcompute_blur_unnormalized_stencil_38");
  hcompute_blur_unnormalized_stencil_38->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 6)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_38->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_38->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 6)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 7), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 7), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_39 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_39");
  hcompute_blur_unnormalized_stencil_39->add_function("hcompute_blur_unnormalized_stencil_39");
  hcompute_blur_unnormalized_stencil_39->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 7)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_39->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_39->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 7)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 8), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 8), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_40 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_40");
  hcompute_blur_unnormalized_stencil_40->add_function("hcompute_blur_unnormalized_stencil_40");
  hcompute_blur_unnormalized_stencil_40->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 8)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_40->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_40->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 8)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 9), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 9), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_41 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_41");
  hcompute_blur_unnormalized_stencil_41->add_function("hcompute_blur_unnormalized_stencil_41");
  hcompute_blur_unnormalized_stencil_41->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 9)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_41->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_41->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 9)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 10), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 10), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_42 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_42");
  hcompute_blur_unnormalized_stencil_42->add_function("hcompute_blur_unnormalized_stencil_42");
  hcompute_blur_unnormalized_stencil_42->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 10)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_42->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_42->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 10)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 11), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 11), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_43 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_43");
  hcompute_blur_unnormalized_stencil_43->add_function("hcompute_blur_unnormalized_stencil_43");
  hcompute_blur_unnormalized_stencil_43->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 11)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_43->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_43->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 11)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 12), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 12), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_44 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_44");
  hcompute_blur_unnormalized_stencil_44->add_function("hcompute_blur_unnormalized_stencil_44");
  hcompute_blur_unnormalized_stencil_44->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 12)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_44->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_44->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 12)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 13), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 13), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_45 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_45");
  hcompute_blur_unnormalized_stencil_45->add_function("hcompute_blur_unnormalized_stencil_45");
  hcompute_blur_unnormalized_stencil_45->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 13)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_45->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_45->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 13)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 14), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 14), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_46 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_46");
  hcompute_blur_unnormalized_stencil_46->add_function("hcompute_blur_unnormalized_stencil_46");
  hcompute_blur_unnormalized_stencil_46->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 14)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_46->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_46->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 14)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 15), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 15), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_47 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_47");
  hcompute_blur_unnormalized_stencil_47->add_function("hcompute_blur_unnormalized_stencil_47");
  hcompute_blur_unnormalized_stencil_47->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 15)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_47->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_47->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 15)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 16), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 16), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_48 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_48");
  hcompute_blur_unnormalized_stencil_48->add_function("hcompute_blur_unnormalized_stencil_48");
  hcompute_blur_unnormalized_stencil_48->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 16)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_48->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_48->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 16)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 17), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 17), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_49 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_49");
  hcompute_blur_unnormalized_stencil_49->add_function("hcompute_blur_unnormalized_stencil_49");
  hcompute_blur_unnormalized_stencil_49->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 17)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_49->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_49->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 17)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 18), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 18), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_50 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_50");
  hcompute_blur_unnormalized_stencil_50->add_function("hcompute_blur_unnormalized_stencil_50");
  hcompute_blur_unnormalized_stencil_50->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 18)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_50->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_50->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 18)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 19), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 19), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_51 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_51");
  hcompute_blur_unnormalized_stencil_51->add_function("hcompute_blur_unnormalized_stencil_51");
  hcompute_blur_unnormalized_stencil_51->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 19)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_51->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_51->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 19)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 20), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 20), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_52 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_52");
  hcompute_blur_unnormalized_stencil_52->add_function("hcompute_blur_unnormalized_stencil_52");
  hcompute_blur_unnormalized_stencil_52->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 20)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_52->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_52->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 20)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 21), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 21), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_53 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_53");
  hcompute_blur_unnormalized_stencil_53->add_function("hcompute_blur_unnormalized_stencil_53");
  hcompute_blur_unnormalized_stencil_53->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 21)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_53->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_53->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 21)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 22), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 22), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_54 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_54");
  hcompute_blur_unnormalized_stencil_54->add_function("hcompute_blur_unnormalized_stencil_54");
  hcompute_blur_unnormalized_stencil_54->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 22)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_54->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_54->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 22)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 23), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 23), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_55 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_55");
  hcompute_blur_unnormalized_stencil_55->add_function("hcompute_blur_unnormalized_stencil_55");
  hcompute_blur_unnormalized_stencil_55->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 23)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_55->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_55->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 23)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 24), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 24), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_56 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_56");
  hcompute_blur_unnormalized_stencil_56->add_function("hcompute_blur_unnormalized_stencil_56");
  hcompute_blur_unnormalized_stencil_56->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 24)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_56->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_56->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 24)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 25), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 25), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_57 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_57");
  hcompute_blur_unnormalized_stencil_57->add_function("hcompute_blur_unnormalized_stencil_57");
  hcompute_blur_unnormalized_stencil_57->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 25)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_57->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_57->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 25)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 26), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 26), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_58 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_58");
  hcompute_blur_unnormalized_stencil_58->add_function("hcompute_blur_unnormalized_stencil_58");
  hcompute_blur_unnormalized_stencil_58->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 26)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_58->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_58->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 26)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 27), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 27), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_59 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_59");
  hcompute_blur_unnormalized_stencil_59->add_function("hcompute_blur_unnormalized_stencil_59");
  hcompute_blur_unnormalized_stencil_59->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 27)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_59->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_59->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 27)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 28), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 28), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_60 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_60");
  hcompute_blur_unnormalized_stencil_60->add_function("hcompute_blur_unnormalized_stencil_60");
  hcompute_blur_unnormalized_stencil_60->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 28)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_60->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_60->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 28)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 29), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 29), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_61 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_61");
  hcompute_blur_unnormalized_stencil_61->add_function("hcompute_blur_unnormalized_stencil_61");
  hcompute_blur_unnormalized_stencil_61->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 29)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_61->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_61->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 29)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 30), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 30), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_62 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_62");
  hcompute_blur_unnormalized_stencil_62->add_function("hcompute_blur_unnormalized_stencil_62");
  hcompute_blur_unnormalized_stencil_62->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 32)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 32)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 30)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 32)");
  hcompute_blur_unnormalized_stencil_62->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_62->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 30)");

//store is: blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y) = ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y)*(uint16)24) + (blur_unnormalized.stencil(((blur_unnormalized_s1_x_x*32) + 31), blur_unnormalized_s1_y) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), blur_unnormalized_s1_y)*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 33), blur_unnormalized_s1_y)*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 1))*(uint16)37) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 33), (blur_unnormalized_s1_y + 1))*(uint16)30) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 31), (blur_unnormalized_s1_y + 2))*(uint16)24) + ((hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 33), (blur_unnormalized_s1_y + 2))*(uint16)24) + (hw_input_global_wrapper_global_wrapper.stencil(((blur_unnormalized_s1_x_x*32) + 32), (blur_unnormalized_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_blur_unnormalized_stencil_63 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_63");
  hcompute_blur_unnormalized_stencil_63->add_function("hcompute_blur_unnormalized_stencil_63");
  hcompute_blur_unnormalized_stencil_63->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 32)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 33)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 32)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 1)", "((blur_unnormalized_s1_x_x*32) + 33)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 31)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 33)");
  hcompute_blur_unnormalized_stencil_63->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(blur_unnormalized_s1_y + 2)", "((blur_unnormalized_s1_x_x*32) + 32)");
  hcompute_blur_unnormalized_stencil_63->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "((blur_unnormalized_s1_x_x*32) + 31)");

//consuming blur_unnormalized.stencil
////producing blur.stencil
  auto blur_s0_y = prg.add_loop("blur_s0_y", 0, 196);
  auto blur_s0_x_x = blur_s0_y->add_loop("blur_s0_x_x", 0, 9);

//store is: blur.stencil((blur_s0_x_x*32), blur_s0_y) = (blur_unnormalized.stencil((blur_s0_x_x*32), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil = blur_s0_x_x->add_op("op_hcompute_blur_stencil");
  hcompute_blur_stencil->add_function("hcompute_blur_stencil");
  hcompute_blur_stencil->add_load("blur_unnormalized_stencil", "blur_s0_y", "(blur_s0_x_x*32)");
  prg.buffer_port_widths["blur_stencil"] = 16;
  hcompute_blur_stencil->add_store("blur_stencil", "blur_s0_y", "(blur_s0_x_x*32)");

//store is: blur.stencil(((blur_s0_x_x*32) + 1), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 1), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_1 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_1");
  hcompute_blur_stencil_1->add_function("hcompute_blur_stencil_1");
  hcompute_blur_stencil_1->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 1)");
  hcompute_blur_stencil_1->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 1)");

//store is: blur.stencil(((blur_s0_x_x*32) + 2), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 2), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_2 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_2");
  hcompute_blur_stencil_2->add_function("hcompute_blur_stencil_2");
  hcompute_blur_stencil_2->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 2)");
  hcompute_blur_stencil_2->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 2)");

//store is: blur.stencil(((blur_s0_x_x*32) + 3), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 3), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_3 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_3");
  hcompute_blur_stencil_3->add_function("hcompute_blur_stencil_3");
  hcompute_blur_stencil_3->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 3)");
  hcompute_blur_stencil_3->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 3)");

//store is: blur.stencil(((blur_s0_x_x*32) + 4), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 4), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_4 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_4");
  hcompute_blur_stencil_4->add_function("hcompute_blur_stencil_4");
  hcompute_blur_stencil_4->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 4)");
  hcompute_blur_stencil_4->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 4)");

//store is: blur.stencil(((blur_s0_x_x*32) + 5), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 5), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_5 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_5");
  hcompute_blur_stencil_5->add_function("hcompute_blur_stencil_5");
  hcompute_blur_stencil_5->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 5)");
  hcompute_blur_stencil_5->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 5)");

//store is: blur.stencil(((blur_s0_x_x*32) + 6), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 6), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_6 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_6");
  hcompute_blur_stencil_6->add_function("hcompute_blur_stencil_6");
  hcompute_blur_stencil_6->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 6)");
  hcompute_blur_stencil_6->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 6)");

//store is: blur.stencil(((blur_s0_x_x*32) + 7), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 7), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_7 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_7");
  hcompute_blur_stencil_7->add_function("hcompute_blur_stencil_7");
  hcompute_blur_stencil_7->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 7)");
  hcompute_blur_stencil_7->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 7)");

//store is: blur.stencil(((blur_s0_x_x*32) + 8), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 8), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_8 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_8");
  hcompute_blur_stencil_8->add_function("hcompute_blur_stencil_8");
  hcompute_blur_stencil_8->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 8)");
  hcompute_blur_stencil_8->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 8)");

//store is: blur.stencil(((blur_s0_x_x*32) + 9), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 9), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_9 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_9");
  hcompute_blur_stencil_9->add_function("hcompute_blur_stencil_9");
  hcompute_blur_stencil_9->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 9)");
  hcompute_blur_stencil_9->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 9)");

//store is: blur.stencil(((blur_s0_x_x*32) + 10), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 10), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_10 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_10");
  hcompute_blur_stencil_10->add_function("hcompute_blur_stencil_10");
  hcompute_blur_stencil_10->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 10)");
  hcompute_blur_stencil_10->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 10)");

//store is: blur.stencil(((blur_s0_x_x*32) + 11), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 11), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_11 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_11");
  hcompute_blur_stencil_11->add_function("hcompute_blur_stencil_11");
  hcompute_blur_stencil_11->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 11)");
  hcompute_blur_stencil_11->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 11)");

//store is: blur.stencil(((blur_s0_x_x*32) + 12), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 12), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_12 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_12");
  hcompute_blur_stencil_12->add_function("hcompute_blur_stencil_12");
  hcompute_blur_stencil_12->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 12)");
  hcompute_blur_stencil_12->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 12)");

//store is: blur.stencil(((blur_s0_x_x*32) + 13), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 13), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_13 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_13");
  hcompute_blur_stencil_13->add_function("hcompute_blur_stencil_13");
  hcompute_blur_stencil_13->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 13)");
  hcompute_blur_stencil_13->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 13)");

//store is: blur.stencil(((blur_s0_x_x*32) + 14), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 14), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_14 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_14");
  hcompute_blur_stencil_14->add_function("hcompute_blur_stencil_14");
  hcompute_blur_stencil_14->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 14)");
  hcompute_blur_stencil_14->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 14)");

//store is: blur.stencil(((blur_s0_x_x*32) + 15), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 15), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_15 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_15");
  hcompute_blur_stencil_15->add_function("hcompute_blur_stencil_15");
  hcompute_blur_stencil_15->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 15)");
  hcompute_blur_stencil_15->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 15)");

//store is: blur.stencil(((blur_s0_x_x*32) + 16), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 16), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_16 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_16");
  hcompute_blur_stencil_16->add_function("hcompute_blur_stencil_16");
  hcompute_blur_stencil_16->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 16)");
  hcompute_blur_stencil_16->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 16)");

//store is: blur.stencil(((blur_s0_x_x*32) + 17), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 17), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_17 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_17");
  hcompute_blur_stencil_17->add_function("hcompute_blur_stencil_17");
  hcompute_blur_stencil_17->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 17)");
  hcompute_blur_stencil_17->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 17)");

//store is: blur.stencil(((blur_s0_x_x*32) + 18), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 18), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_18 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_18");
  hcompute_blur_stencil_18->add_function("hcompute_blur_stencil_18");
  hcompute_blur_stencil_18->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 18)");
  hcompute_blur_stencil_18->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 18)");

//store is: blur.stencil(((blur_s0_x_x*32) + 19), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 19), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_19 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_19");
  hcompute_blur_stencil_19->add_function("hcompute_blur_stencil_19");
  hcompute_blur_stencil_19->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 19)");
  hcompute_blur_stencil_19->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 19)");

//store is: blur.stencil(((blur_s0_x_x*32) + 20), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 20), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_20 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_20");
  hcompute_blur_stencil_20->add_function("hcompute_blur_stencil_20");
  hcompute_blur_stencil_20->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 20)");
  hcompute_blur_stencil_20->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 20)");

//store is: blur.stencil(((blur_s0_x_x*32) + 21), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 21), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_21 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_21");
  hcompute_blur_stencil_21->add_function("hcompute_blur_stencil_21");
  hcompute_blur_stencil_21->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 21)");
  hcompute_blur_stencil_21->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 21)");

//store is: blur.stencil(((blur_s0_x_x*32) + 22), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 22), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_22 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_22");
  hcompute_blur_stencil_22->add_function("hcompute_blur_stencil_22");
  hcompute_blur_stencil_22->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 22)");
  hcompute_blur_stencil_22->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 22)");

//store is: blur.stencil(((blur_s0_x_x*32) + 23), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 23), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_23 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_23");
  hcompute_blur_stencil_23->add_function("hcompute_blur_stencil_23");
  hcompute_blur_stencil_23->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 23)");
  hcompute_blur_stencil_23->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 23)");

//store is: blur.stencil(((blur_s0_x_x*32) + 24), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 24), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_24 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_24");
  hcompute_blur_stencil_24->add_function("hcompute_blur_stencil_24");
  hcompute_blur_stencil_24->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 24)");
  hcompute_blur_stencil_24->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 24)");

//store is: blur.stencil(((blur_s0_x_x*32) + 25), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 25), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_25 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_25");
  hcompute_blur_stencil_25->add_function("hcompute_blur_stencil_25");
  hcompute_blur_stencil_25->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 25)");
  hcompute_blur_stencil_25->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 25)");

//store is: blur.stencil(((blur_s0_x_x*32) + 26), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 26), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_26 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_26");
  hcompute_blur_stencil_26->add_function("hcompute_blur_stencil_26");
  hcompute_blur_stencil_26->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 26)");
  hcompute_blur_stencil_26->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 26)");

//store is: blur.stencil(((blur_s0_x_x*32) + 27), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 27), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_27 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_27");
  hcompute_blur_stencil_27->add_function("hcompute_blur_stencil_27");
  hcompute_blur_stencil_27->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 27)");
  hcompute_blur_stencil_27->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 27)");

//store is: blur.stencil(((blur_s0_x_x*32) + 28), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 28), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_28 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_28");
  hcompute_blur_stencil_28->add_function("hcompute_blur_stencil_28");
  hcompute_blur_stencil_28->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 28)");
  hcompute_blur_stencil_28->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 28)");

//store is: blur.stencil(((blur_s0_x_x*32) + 29), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 29), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_29 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_29");
  hcompute_blur_stencil_29->add_function("hcompute_blur_stencil_29");
  hcompute_blur_stencil_29->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 29)");
  hcompute_blur_stencil_29->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 29)");

//store is: blur.stencil(((blur_s0_x_x*32) + 30), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 30), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_30 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_30");
  hcompute_blur_stencil_30->add_function("hcompute_blur_stencil_30");
  hcompute_blur_stencil_30->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 30)");
  hcompute_blur_stencil_30->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 30)");

//store is: blur.stencil(((blur_s0_x_x*32) + 31), blur_s0_y) = (blur_unnormalized.stencil(((blur_s0_x_x*32) + 31), blur_s0_y)/(uint16)256)
  auto hcompute_blur_stencil_31 = blur_s0_x_x->add_op("op_hcompute_blur_stencil_31");
  hcompute_blur_stencil_31->add_function("hcompute_blur_stencil_31");
  hcompute_blur_stencil_31->add_load("blur_unnormalized_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 31)");
  hcompute_blur_stencil_31->add_store("blur_stencil", "blur_s0_y", "((blur_s0_x_x*32) + 31)");

//consuming blur.stencil
  auto hw_output_s0_y_yii = prg.add_loop("hw_output_s0_y_yii", 0, 196);
  auto hw_output_s0_x_xii_xii = hw_output_s0_y_yii->add_loop("hw_output_s0_x_xii_xii", 0, 9);

//store is: hw_output.stencil((hw_output_s0_x_xii_xii*32), hw_output_s0_y_yii) = blur.stencil((hw_output_s0_x_xii_xii*32), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_function("hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_load("blur_stencil", "hw_output_s0_y_yii", "(hw_output_s0_x_xii_xii*32)");
  prg.buffer_port_widths["hw_output_stencil"] = 16;
  hcompute_hw_output_stencil->add_store("hw_output_stencil", "hw_output_s0_y_yii", "(hw_output_s0_x_xii_xii*32)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 1), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 1), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_1 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_1");
  hcompute_hw_output_stencil_1->add_function("hcompute_hw_output_stencil_1");
  hcompute_hw_output_stencil_1->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 1)");
  hcompute_hw_output_stencil_1->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 1)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 2), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 2), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_2 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_2");
  hcompute_hw_output_stencil_2->add_function("hcompute_hw_output_stencil_2");
  hcompute_hw_output_stencil_2->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 2)");
  hcompute_hw_output_stencil_2->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 2)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 3), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 3), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_3 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_3");
  hcompute_hw_output_stencil_3->add_function("hcompute_hw_output_stencil_3");
  hcompute_hw_output_stencil_3->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 3)");
  hcompute_hw_output_stencil_3->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 3)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 4), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 4), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_4 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_4");
  hcompute_hw_output_stencil_4->add_function("hcompute_hw_output_stencil_4");
  hcompute_hw_output_stencil_4->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 4)");
  hcompute_hw_output_stencil_4->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 4)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 5), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 5), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_5 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_5");
  hcompute_hw_output_stencil_5->add_function("hcompute_hw_output_stencil_5");
  hcompute_hw_output_stencil_5->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 5)");
  hcompute_hw_output_stencil_5->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 5)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 6), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 6), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_6 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_6");
  hcompute_hw_output_stencil_6->add_function("hcompute_hw_output_stencil_6");
  hcompute_hw_output_stencil_6->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 6)");
  hcompute_hw_output_stencil_6->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 6)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 7), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 7), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_7 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_7");
  hcompute_hw_output_stencil_7->add_function("hcompute_hw_output_stencil_7");
  hcompute_hw_output_stencil_7->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 7)");
  hcompute_hw_output_stencil_7->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 7)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 8), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 8), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_8 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_8");
  hcompute_hw_output_stencil_8->add_function("hcompute_hw_output_stencil_8");
  hcompute_hw_output_stencil_8->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 8)");
  hcompute_hw_output_stencil_8->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 8)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 9), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 9), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_9 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_9");
  hcompute_hw_output_stencil_9->add_function("hcompute_hw_output_stencil_9");
  hcompute_hw_output_stencil_9->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 9)");
  hcompute_hw_output_stencil_9->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 9)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 10), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 10), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_10 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_10");
  hcompute_hw_output_stencil_10->add_function("hcompute_hw_output_stencil_10");
  hcompute_hw_output_stencil_10->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 10)");
  hcompute_hw_output_stencil_10->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 10)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 11), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 11), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_11 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_11");
  hcompute_hw_output_stencil_11->add_function("hcompute_hw_output_stencil_11");
  hcompute_hw_output_stencil_11->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 11)");
  hcompute_hw_output_stencil_11->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 11)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 12), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 12), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_12 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_12");
  hcompute_hw_output_stencil_12->add_function("hcompute_hw_output_stencil_12");
  hcompute_hw_output_stencil_12->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 12)");
  hcompute_hw_output_stencil_12->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 12)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 13), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 13), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_13 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_13");
  hcompute_hw_output_stencil_13->add_function("hcompute_hw_output_stencil_13");
  hcompute_hw_output_stencil_13->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 13)");
  hcompute_hw_output_stencil_13->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 13)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 14), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 14), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_14 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_14");
  hcompute_hw_output_stencil_14->add_function("hcompute_hw_output_stencil_14");
  hcompute_hw_output_stencil_14->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 14)");
  hcompute_hw_output_stencil_14->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 14)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 15), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 15), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_15 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_15");
  hcompute_hw_output_stencil_15->add_function("hcompute_hw_output_stencil_15");
  hcompute_hw_output_stencil_15->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 15)");
  hcompute_hw_output_stencil_15->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 15)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 16), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 16), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_16 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_16");
  hcompute_hw_output_stencil_16->add_function("hcompute_hw_output_stencil_16");
  hcompute_hw_output_stencil_16->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 16)");
  hcompute_hw_output_stencil_16->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 16)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 17), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 17), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_17 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_17");
  hcompute_hw_output_stencil_17->add_function("hcompute_hw_output_stencil_17");
  hcompute_hw_output_stencil_17->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 17)");
  hcompute_hw_output_stencil_17->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 17)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 18), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 18), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_18 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_18");
  hcompute_hw_output_stencil_18->add_function("hcompute_hw_output_stencil_18");
  hcompute_hw_output_stencil_18->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 18)");
  hcompute_hw_output_stencil_18->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 18)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 19), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 19), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_19 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_19");
  hcompute_hw_output_stencil_19->add_function("hcompute_hw_output_stencil_19");
  hcompute_hw_output_stencil_19->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 19)");
  hcompute_hw_output_stencil_19->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 19)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 20), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 20), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_20 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_20");
  hcompute_hw_output_stencil_20->add_function("hcompute_hw_output_stencil_20");
  hcompute_hw_output_stencil_20->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 20)");
  hcompute_hw_output_stencil_20->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 20)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 21), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 21), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_21 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_21");
  hcompute_hw_output_stencil_21->add_function("hcompute_hw_output_stencil_21");
  hcompute_hw_output_stencil_21->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 21)");
  hcompute_hw_output_stencil_21->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 21)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 22), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 22), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_22 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_22");
  hcompute_hw_output_stencil_22->add_function("hcompute_hw_output_stencil_22");
  hcompute_hw_output_stencil_22->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 22)");
  hcompute_hw_output_stencil_22->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 22)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 23), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 23), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_23 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_23");
  hcompute_hw_output_stencil_23->add_function("hcompute_hw_output_stencil_23");
  hcompute_hw_output_stencil_23->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 23)");
  hcompute_hw_output_stencil_23->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 23)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 24), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 24), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_24 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_24");
  hcompute_hw_output_stencil_24->add_function("hcompute_hw_output_stencil_24");
  hcompute_hw_output_stencil_24->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 24)");
  hcompute_hw_output_stencil_24->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 24)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 25), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 25), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_25 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_25");
  hcompute_hw_output_stencil_25->add_function("hcompute_hw_output_stencil_25");
  hcompute_hw_output_stencil_25->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 25)");
  hcompute_hw_output_stencil_25->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 25)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 26), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 26), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_26 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_26");
  hcompute_hw_output_stencil_26->add_function("hcompute_hw_output_stencil_26");
  hcompute_hw_output_stencil_26->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 26)");
  hcompute_hw_output_stencil_26->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 26)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 27), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 27), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_27 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_27");
  hcompute_hw_output_stencil_27->add_function("hcompute_hw_output_stencil_27");
  hcompute_hw_output_stencil_27->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 27)");
  hcompute_hw_output_stencil_27->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 27)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 28), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 28), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_28 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_28");
  hcompute_hw_output_stencil_28->add_function("hcompute_hw_output_stencil_28");
  hcompute_hw_output_stencil_28->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 28)");
  hcompute_hw_output_stencil_28->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 28)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 29), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 29), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_29 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_29");
  hcompute_hw_output_stencil_29->add_function("hcompute_hw_output_stencil_29");
  hcompute_hw_output_stencil_29->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 29)");
  hcompute_hw_output_stencil_29->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 29)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 30), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 30), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_30 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_30");
  hcompute_hw_output_stencil_30->add_function("hcompute_hw_output_stencil_30");
  hcompute_hw_output_stencil_30->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 30)");
  hcompute_hw_output_stencil_30->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 30)");

//store is: hw_output.stencil(((hw_output_s0_x_xii_xii*32) + 31), hw_output_s0_y_yii) = blur.stencil(((hw_output_s0_x_xii_xii*32) + 31), hw_output_s0_y_yii)
  auto hcompute_hw_output_stencil_31 = hw_output_s0_x_xii_xii->add_op("op_hcompute_hw_output_stencil_31");
  hcompute_hw_output_stencil_31->add_function("hcompute_hw_output_stencil_31");
  hcompute_hw_output_stencil_31->add_load("blur_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 31)");
  hcompute_hw_output_stencil_31->add_store("hw_output_stencil", "hw_output_s0_y_yii", "((hw_output_s0_x_xii_xii*32) + 31)");

//consuming hw_output.stencil
  auto hw_output_global_wrapper_s0_y_yi = prg.add_loop("hw_output_global_wrapper_s0_y_yi", 0, 196);
  auto hw_output_global_wrapper_s0_x_xi_xi = hw_output_global_wrapper_s0_y_yi->add_loop("hw_output_global_wrapper_s0_x_xi_xi", 0, 9);

//store is: hw_output_global_wrapper.glb.stencil((hw_output_global_wrapper_s0_x_xi_xi*32), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil((hw_output_global_wrapper_s0_x_xi_xi*32), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil");
  hcompute_hw_output_global_wrapper_glb_stencil->add_function("hcompute_hw_output_global_wrapper_glb_stencil");
  hcompute_hw_output_global_wrapper_glb_stencil->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "(hw_output_global_wrapper_s0_x_xi_xi*32)");
  hcompute_hw_output_global_wrapper_glb_stencil->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "(hw_output_global_wrapper_s0_x_xi_xi*32)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 1), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 1), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_1 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_function("hcompute_hw_output_global_wrapper_glb_stencil_1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 1)");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 1)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 2), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 2), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_2 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_function("hcompute_hw_output_global_wrapper_glb_stencil_2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 2)");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 2)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 3), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 3), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_3 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_function("hcompute_hw_output_global_wrapper_glb_stencil_3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 3)");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 3)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 4), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 4), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_4 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_function("hcompute_hw_output_global_wrapper_glb_stencil_4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 4)");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 4)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 5), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 5), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_5 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_function("hcompute_hw_output_global_wrapper_glb_stencil_5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 5)");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 5)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 6), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 6), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_6 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_function("hcompute_hw_output_global_wrapper_glb_stencil_6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 6)");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 6)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 7), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 7), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_7 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_7");
  hcompute_hw_output_global_wrapper_glb_stencil_7->add_function("hcompute_hw_output_global_wrapper_glb_stencil_7");
  hcompute_hw_output_global_wrapper_glb_stencil_7->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 7)");
  hcompute_hw_output_global_wrapper_glb_stencil_7->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 7)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 8), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 8), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_8 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_8");
  hcompute_hw_output_global_wrapper_glb_stencil_8->add_function("hcompute_hw_output_global_wrapper_glb_stencil_8");
  hcompute_hw_output_global_wrapper_glb_stencil_8->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 8)");
  hcompute_hw_output_global_wrapper_glb_stencil_8->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 8)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 9), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 9), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_9 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_9");
  hcompute_hw_output_global_wrapper_glb_stencil_9->add_function("hcompute_hw_output_global_wrapper_glb_stencil_9");
  hcompute_hw_output_global_wrapper_glb_stencil_9->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 9)");
  hcompute_hw_output_global_wrapper_glb_stencil_9->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 9)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 10), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 10), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_10 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_10");
  hcompute_hw_output_global_wrapper_glb_stencil_10->add_function("hcompute_hw_output_global_wrapper_glb_stencil_10");
  hcompute_hw_output_global_wrapper_glb_stencil_10->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 10)");
  hcompute_hw_output_global_wrapper_glb_stencil_10->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 10)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 11), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 11), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_11 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_11");
  hcompute_hw_output_global_wrapper_glb_stencil_11->add_function("hcompute_hw_output_global_wrapper_glb_stencil_11");
  hcompute_hw_output_global_wrapper_glb_stencil_11->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 11)");
  hcompute_hw_output_global_wrapper_glb_stencil_11->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 11)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 12), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 12), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_12 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_12");
  hcompute_hw_output_global_wrapper_glb_stencil_12->add_function("hcompute_hw_output_global_wrapper_glb_stencil_12");
  hcompute_hw_output_global_wrapper_glb_stencil_12->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 12)");
  hcompute_hw_output_global_wrapper_glb_stencil_12->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 12)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 13), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 13), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_13 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_13");
  hcompute_hw_output_global_wrapper_glb_stencil_13->add_function("hcompute_hw_output_global_wrapper_glb_stencil_13");
  hcompute_hw_output_global_wrapper_glb_stencil_13->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 13)");
  hcompute_hw_output_global_wrapper_glb_stencil_13->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 13)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 14), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 14), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_14 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_14");
  hcompute_hw_output_global_wrapper_glb_stencil_14->add_function("hcompute_hw_output_global_wrapper_glb_stencil_14");
  hcompute_hw_output_global_wrapper_glb_stencil_14->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 14)");
  hcompute_hw_output_global_wrapper_glb_stencil_14->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 14)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 15), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 15), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_15 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_15");
  hcompute_hw_output_global_wrapper_glb_stencil_15->add_function("hcompute_hw_output_global_wrapper_glb_stencil_15");
  hcompute_hw_output_global_wrapper_glb_stencil_15->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 15)");
  hcompute_hw_output_global_wrapper_glb_stencil_15->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 15)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 16), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 16), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_16 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_16");
  hcompute_hw_output_global_wrapper_glb_stencil_16->add_function("hcompute_hw_output_global_wrapper_glb_stencil_16");
  hcompute_hw_output_global_wrapper_glb_stencil_16->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 16)");
  hcompute_hw_output_global_wrapper_glb_stencil_16->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 16)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 17), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 17), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_17 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_17");
  hcompute_hw_output_global_wrapper_glb_stencil_17->add_function("hcompute_hw_output_global_wrapper_glb_stencil_17");
  hcompute_hw_output_global_wrapper_glb_stencil_17->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 17)");
  hcompute_hw_output_global_wrapper_glb_stencil_17->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 17)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 18), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 18), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_18 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_18");
  hcompute_hw_output_global_wrapper_glb_stencil_18->add_function("hcompute_hw_output_global_wrapper_glb_stencil_18");
  hcompute_hw_output_global_wrapper_glb_stencil_18->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 18)");
  hcompute_hw_output_global_wrapper_glb_stencil_18->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 18)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 19), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 19), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_19 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_19");
  hcompute_hw_output_global_wrapper_glb_stencil_19->add_function("hcompute_hw_output_global_wrapper_glb_stencil_19");
  hcompute_hw_output_global_wrapper_glb_stencil_19->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 19)");
  hcompute_hw_output_global_wrapper_glb_stencil_19->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 19)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 20), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 20), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_20 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_20");
  hcompute_hw_output_global_wrapper_glb_stencil_20->add_function("hcompute_hw_output_global_wrapper_glb_stencil_20");
  hcompute_hw_output_global_wrapper_glb_stencil_20->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 20)");
  hcompute_hw_output_global_wrapper_glb_stencil_20->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 20)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 21), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 21), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_21 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_21");
  hcompute_hw_output_global_wrapper_glb_stencil_21->add_function("hcompute_hw_output_global_wrapper_glb_stencil_21");
  hcompute_hw_output_global_wrapper_glb_stencil_21->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 21)");
  hcompute_hw_output_global_wrapper_glb_stencil_21->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 21)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 22), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 22), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_22 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_22");
  hcompute_hw_output_global_wrapper_glb_stencil_22->add_function("hcompute_hw_output_global_wrapper_glb_stencil_22");
  hcompute_hw_output_global_wrapper_glb_stencil_22->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 22)");
  hcompute_hw_output_global_wrapper_glb_stencil_22->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 22)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 23), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 23), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_23 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_23");
  hcompute_hw_output_global_wrapper_glb_stencil_23->add_function("hcompute_hw_output_global_wrapper_glb_stencil_23");
  hcompute_hw_output_global_wrapper_glb_stencil_23->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 23)");
  hcompute_hw_output_global_wrapper_glb_stencil_23->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 23)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 24), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 24), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_24 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_24");
  hcompute_hw_output_global_wrapper_glb_stencil_24->add_function("hcompute_hw_output_global_wrapper_glb_stencil_24");
  hcompute_hw_output_global_wrapper_glb_stencil_24->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 24)");
  hcompute_hw_output_global_wrapper_glb_stencil_24->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 24)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 25), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 25), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_25 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_25");
  hcompute_hw_output_global_wrapper_glb_stencil_25->add_function("hcompute_hw_output_global_wrapper_glb_stencil_25");
  hcompute_hw_output_global_wrapper_glb_stencil_25->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 25)");
  hcompute_hw_output_global_wrapper_glb_stencil_25->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 25)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 26), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 26), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_26 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_26");
  hcompute_hw_output_global_wrapper_glb_stencil_26->add_function("hcompute_hw_output_global_wrapper_glb_stencil_26");
  hcompute_hw_output_global_wrapper_glb_stencil_26->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 26)");
  hcompute_hw_output_global_wrapper_glb_stencil_26->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 26)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 27), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 27), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_27 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_27");
  hcompute_hw_output_global_wrapper_glb_stencil_27->add_function("hcompute_hw_output_global_wrapper_glb_stencil_27");
  hcompute_hw_output_global_wrapper_glb_stencil_27->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 27)");
  hcompute_hw_output_global_wrapper_glb_stencil_27->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 27)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 28), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 28), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_28 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_28");
  hcompute_hw_output_global_wrapper_glb_stencil_28->add_function("hcompute_hw_output_global_wrapper_glb_stencil_28");
  hcompute_hw_output_global_wrapper_glb_stencil_28->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 28)");
  hcompute_hw_output_global_wrapper_glb_stencil_28->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 28)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 29), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 29), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_29 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_29");
  hcompute_hw_output_global_wrapper_glb_stencil_29->add_function("hcompute_hw_output_global_wrapper_glb_stencil_29");
  hcompute_hw_output_global_wrapper_glb_stencil_29->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 29)");
  hcompute_hw_output_global_wrapper_glb_stencil_29->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 29)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 30), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 30), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_30 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_30");
  hcompute_hw_output_global_wrapper_glb_stencil_30->add_function("hcompute_hw_output_global_wrapper_glb_stencil_30");
  hcompute_hw_output_global_wrapper_glb_stencil_30->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 30)");
  hcompute_hw_output_global_wrapper_glb_stencil_30->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 30)");

//store is: hw_output_global_wrapper.glb.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 31), hw_output_global_wrapper_s0_y_yi) = hw_output.stencil(((hw_output_global_wrapper_s0_x_xi_xi*32) + 31), hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_glb_stencil_31 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_31");
  hcompute_hw_output_global_wrapper_glb_stencil_31->add_function("hcompute_hw_output_global_wrapper_glb_stencil_31");
  hcompute_hw_output_global_wrapper_glb_stencil_31->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 31)");
  hcompute_hw_output_global_wrapper_glb_stencil_31->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "((hw_output_global_wrapper_s0_x_xi_xi*32) + 31)");

  return prg;
}


// schedule=3 myunroll=32 mywidth=288
