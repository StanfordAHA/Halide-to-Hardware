#include "app.h"
#include "ubuffer.h"
#include "codegen.h"
#include "prog.h"

prog depthwise_conv() {
  prog prg;
  prg.compute_unit_file = "depthwise_conv_compute.h";
  prg.name = "depthwise_conv";

// Stencil<uint16_t, 7, 116, 116> &input_host_stencil = arg_0;
  prg.add_input("input_host_stencil");
  prg.buffer_port_widths["input_host_stencil"] = 16;
// Stencil<uint16_t, 7, 114, 114> &hw_output_global_wrapper_glb_stencil = arg_1;
  prg.add_output("hw_output_global_wrapper_glb_stencil");
  prg.buffer_port_widths["hw_output_global_wrapper_glb_stencil"] = 16;

////producing input_host_global_wrapper.stencil
  auto input_host_global_wrapper_s0_y = prg.add_loop("input_host_global_wrapper_s0_y", 0, 116);
  auto input_host_global_wrapper_s0_x = input_host_global_wrapper_s0_y->add_loop("input_host_global_wrapper_s0_x", 0, 116);

//store is: input_host_global_wrapper.stencil(0, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(0, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil");
  hcompute_input_host_global_wrapper_stencil->add_function("hcompute_input_host_global_wrapper_stencil");
  hcompute_input_host_global_wrapper_stencil->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "0");
  prg.buffer_port_widths["input_host_global_wrapper_stencil"] = 16;
  hcompute_input_host_global_wrapper_stencil->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "0");

//store is: input_host_global_wrapper.stencil(1, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(1, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil_1 = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil_1");
  hcompute_input_host_global_wrapper_stencil_1->add_function("hcompute_input_host_global_wrapper_stencil_1");
  hcompute_input_host_global_wrapper_stencil_1->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "1");
  hcompute_input_host_global_wrapper_stencil_1->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "1");

//store is: input_host_global_wrapper.stencil(2, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(2, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil_2 = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil_2");
  hcompute_input_host_global_wrapper_stencil_2->add_function("hcompute_input_host_global_wrapper_stencil_2");
  hcompute_input_host_global_wrapper_stencil_2->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "2");
  hcompute_input_host_global_wrapper_stencil_2->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "2");

//store is: input_host_global_wrapper.stencil(3, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(3, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil_3 = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil_3");
  hcompute_input_host_global_wrapper_stencil_3->add_function("hcompute_input_host_global_wrapper_stencil_3");
  hcompute_input_host_global_wrapper_stencil_3->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "3");
  hcompute_input_host_global_wrapper_stencil_3->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "3");

//store is: input_host_global_wrapper.stencil(4, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(4, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil_4 = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil_4");
  hcompute_input_host_global_wrapper_stencil_4->add_function("hcompute_input_host_global_wrapper_stencil_4");
  hcompute_input_host_global_wrapper_stencil_4->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "4");
  hcompute_input_host_global_wrapper_stencil_4->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "4");

//store is: input_host_global_wrapper.stencil(5, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(5, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil_5 = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil_5");
  hcompute_input_host_global_wrapper_stencil_5->add_function("hcompute_input_host_global_wrapper_stencil_5");
  hcompute_input_host_global_wrapper_stencil_5->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "5");
  hcompute_input_host_global_wrapper_stencil_5->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "5");

//store is: input_host_global_wrapper.stencil(6, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y) = input_host.stencil(6, input_host_global_wrapper_s0_x, input_host_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_stencil_6 = input_host_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_stencil_6");
  hcompute_input_host_global_wrapper_stencil_6->add_function("hcompute_input_host_global_wrapper_stencil_6");
  hcompute_input_host_global_wrapper_stencil_6->add_load("input_host_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "6");
  hcompute_input_host_global_wrapper_stencil_6->add_store("input_host_global_wrapper_stencil", "input_host_global_wrapper_s0_y", "input_host_global_wrapper_s0_x", "6");

//consuming input_host_global_wrapper.stencil
////producing hw_output.stencil
////producing input_host_global_wrapper_global_wrapper.stencil
  auto input_host_global_wrapper_global_wrapper_s0_y = prg.add_loop("input_host_global_wrapper_global_wrapper_s0_y", 0, 116);
  auto input_host_global_wrapper_global_wrapper_s0_x = input_host_global_wrapper_global_wrapper_s0_y->add_loop("input_host_global_wrapper_global_wrapper_s0_x", 0, 116);

//store is: input_host_global_wrapper_global_wrapper.stencil(0, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(0, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil");
  hcompute_input_host_global_wrapper_global_wrapper_stencil->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil");
  hcompute_input_host_global_wrapper_global_wrapper_stencil->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "0");
  prg.buffer_port_widths["input_host_global_wrapper_global_wrapper_stencil"] = 16;
  hcompute_input_host_global_wrapper_global_wrapper_stencil->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "0");

//store is: input_host_global_wrapper_global_wrapper.stencil(1, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(1, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil_1 = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil_1");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_1->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil_1");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_1->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "1");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_1->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "1");

//store is: input_host_global_wrapper_global_wrapper.stencil(2, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(2, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil_2 = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil_2");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_2->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil_2");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_2->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "2");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_2->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "2");

//store is: input_host_global_wrapper_global_wrapper.stencil(3, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(3, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil_3 = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil_3");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_3->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil_3");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_3->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "3");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_3->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "3");

//store is: input_host_global_wrapper_global_wrapper.stencil(4, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(4, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil_4 = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil_4");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_4->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil_4");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_4->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "4");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_4->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "4");

//store is: input_host_global_wrapper_global_wrapper.stencil(5, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(5, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil_5 = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil_5");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_5->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil_5");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_5->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "5");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_5->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "5");

//store is: input_host_global_wrapper_global_wrapper.stencil(6, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y) = input_host_global_wrapper.stencil(6, input_host_global_wrapper_global_wrapper_s0_x, input_host_global_wrapper_global_wrapper_s0_y)
  auto hcompute_input_host_global_wrapper_global_wrapper_stencil_6 = input_host_global_wrapper_global_wrapper_s0_x->add_op("op_hcompute_input_host_global_wrapper_global_wrapper_stencil_6");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_6->add_function("hcompute_input_host_global_wrapper_global_wrapper_stencil_6");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_6->add_load("input_host_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "6");
  hcompute_input_host_global_wrapper_global_wrapper_stencil_6->add_store("input_host_global_wrapper_global_wrapper_stencil", "input_host_global_wrapper_global_wrapper_s0_y", "input_host_global_wrapper_global_wrapper_s0_x", "6");

//consuming input_host_global_wrapper_global_wrapper.stencil
////producing depthwise_conv.stencil
  auto depthwise_conv_s0_y = prg.add_loop("depthwise_conv_s0_y", 0, 114);
  auto depthwise_conv_s0_x = depthwise_conv_s0_y->add_loop("depthwise_conv_s0_x", 0, 114);

//store is: depthwise_conv.stencil(0, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil");
  hcompute_depthwise_conv_stencil->add_function("hcompute_depthwise_conv_stencil");
  prg.buffer_port_widths["depthwise_conv_stencil"] = 16;
  hcompute_depthwise_conv_stencil->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "0");

//store is: depthwise_conv.stencil(1, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil_1 = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil_1");
  hcompute_depthwise_conv_stencil_1->add_function("hcompute_depthwise_conv_stencil_1");
  hcompute_depthwise_conv_stencil_1->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "1");

//store is: depthwise_conv.stencil(2, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil_2 = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil_2");
  hcompute_depthwise_conv_stencil_2->add_function("hcompute_depthwise_conv_stencil_2");
  hcompute_depthwise_conv_stencil_2->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "2");

//store is: depthwise_conv.stencil(3, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil_3 = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil_3");
  hcompute_depthwise_conv_stencil_3->add_function("hcompute_depthwise_conv_stencil_3");
  hcompute_depthwise_conv_stencil_3->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "3");

//store is: depthwise_conv.stencil(4, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil_4 = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil_4");
  hcompute_depthwise_conv_stencil_4->add_function("hcompute_depthwise_conv_stencil_4");
  hcompute_depthwise_conv_stencil_4->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "4");

//store is: depthwise_conv.stencil(5, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil_5 = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil_5");
  hcompute_depthwise_conv_stencil_5->add_function("hcompute_depthwise_conv_stencil_5");
  hcompute_depthwise_conv_stencil_5->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "5");

//store is: depthwise_conv.stencil(6, depthwise_conv_s0_x, depthwise_conv_s0_y) = (uint16)0
  auto hcompute_depthwise_conv_stencil_6 = depthwise_conv_s0_x->add_op("op_hcompute_depthwise_conv_stencil_6");
  hcompute_depthwise_conv_stencil_6->add_function("hcompute_depthwise_conv_stencil_6");
  hcompute_depthwise_conv_stencil_6->add_store("depthwise_conv_stencil", "depthwise_conv_s0_y", "depthwise_conv_s0_x", "6");
  auto depthwise_conv_s1_y = prg.add_loop("depthwise_conv_s1_y", 0, 114);
  auto depthwise_conv_s1_x = depthwise_conv_s1_y->add_loop("depthwise_conv_s1_x", 0, 114);

//store is: depthwise_conv.stencil(0, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(0, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(0, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(0, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(0, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(0, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_7 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_7");
  hcompute_depthwise_conv_stencil_7->add_function("hcompute_depthwise_conv_stencil_7");
  hcompute_depthwise_conv_stencil_7->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "0");
  hcompute_depthwise_conv_stencil_7->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "0");
  hcompute_depthwise_conv_stencil_7->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "0");

//store is: depthwise_conv.stencil(1, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(1, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(1, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(1, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(1, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(1, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_8 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_8");
  hcompute_depthwise_conv_stencil_8->add_function("hcompute_depthwise_conv_stencil_8");
  hcompute_depthwise_conv_stencil_8->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "1");
  hcompute_depthwise_conv_stencil_8->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "1");
  hcompute_depthwise_conv_stencil_8->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "1");

//store is: depthwise_conv.stencil(2, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(2, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(2, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(2, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(2, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(2, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_9 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_9");
  hcompute_depthwise_conv_stencil_9->add_function("hcompute_depthwise_conv_stencil_9");
  hcompute_depthwise_conv_stencil_9->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "2");
  hcompute_depthwise_conv_stencil_9->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "2");
  hcompute_depthwise_conv_stencil_9->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "2");

//store is: depthwise_conv.stencil(3, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(3, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(3, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(3, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(3, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(3, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_10 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_10");
  hcompute_depthwise_conv_stencil_10->add_function("hcompute_depthwise_conv_stencil_10");
  hcompute_depthwise_conv_stencil_10->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "3");
  hcompute_depthwise_conv_stencil_10->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "3");
  hcompute_depthwise_conv_stencil_10->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "3");

//store is: depthwise_conv.stencil(4, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(4, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(4, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(4, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(4, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(4, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_11 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_11");
  hcompute_depthwise_conv_stencil_11->add_function("hcompute_depthwise_conv_stencil_11");
  hcompute_depthwise_conv_stencil_11->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "4");
  hcompute_depthwise_conv_stencil_11->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "4");
  hcompute_depthwise_conv_stencil_11->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "4");

//store is: depthwise_conv.stencil(5, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(5, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(5, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(5, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(5, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(5, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_12 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_12");
  hcompute_depthwise_conv_stencil_12->add_function("hcompute_depthwise_conv_stencil_12");
  hcompute_depthwise_conv_stencil_12->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "5");
  hcompute_depthwise_conv_stencil_12->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "5");
  hcompute_depthwise_conv_stencil_12->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "5");

//store is: depthwise_conv.stencil(6, depthwise_conv_s1_x, depthwise_conv_s1_y) = ((input_host_global_wrapper_global_wrapper.stencil(6, depthwise_conv_s1_x, depthwise_conv_s1_y)*(uint16)24) + (depthwise_conv.stencil(6, depthwise_conv_s1_x, depthwise_conv_s1_y) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 1), depthwise_conv_s1_y)*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 2), depthwise_conv_s1_y)*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(6, depthwise_conv_s1_x, (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 1))*(uint16)37) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 1))*(uint16)30) + ((input_host_global_wrapper_global_wrapper.stencil(6, depthwise_conv_s1_x, (depthwise_conv_s1_y + 2))*(uint16)24) + ((input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 2), (depthwise_conv_s1_y + 2))*(uint16)24) + (input_host_global_wrapper_global_wrapper.stencil(6, (depthwise_conv_s1_x + 1), (depthwise_conv_s1_y + 2))*(uint16)30))))))))))
  auto hcompute_depthwise_conv_stencil_13 = depthwise_conv_s1_x->add_op("op_hcompute_depthwise_conv_stencil_13");
  hcompute_depthwise_conv_stencil_13->add_function("hcompute_depthwise_conv_stencil_13");
  hcompute_depthwise_conv_stencil_13->add_load("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 1)", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "depthwise_conv_s1_y", "(depthwise_conv_s1_x + 2)", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "depthwise_conv_s1_x", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 1)", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 1)", "(depthwise_conv_s1_x + 2)", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "depthwise_conv_s1_x", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 2)", "6");
  hcompute_depthwise_conv_stencil_13->add_load("input_host_global_wrapper_global_wrapper_stencil", "(depthwise_conv_s1_y + 2)", "(depthwise_conv_s1_x + 1)", "6");
  hcompute_depthwise_conv_stencil_13->add_store("depthwise_conv_stencil", "depthwise_conv_s1_y", "depthwise_conv_s1_x", "6");

//consuming depthwise_conv.stencil
  auto hw_output_s0_y_y_cgra = prg.add_loop("hw_output_s0_y_y_cgra", 0, 114);
  auto hw_output_s0_x_x_cgra = hw_output_s0_y_y_cgra->add_loop("hw_output_s0_x_x_cgra", 0, 114);

//store is: hw_output.stencil(0, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(0, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_function("hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "0");
  prg.buffer_port_widths["hw_output_stencil"] = 16;
  hcompute_hw_output_stencil->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "0");

//store is: hw_output.stencil(1, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(1, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil_1 = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil_1");
  hcompute_hw_output_stencil_1->add_function("hcompute_hw_output_stencil_1");
  hcompute_hw_output_stencil_1->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "1");
  hcompute_hw_output_stencil_1->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "1");

//store is: hw_output.stencil(2, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(2, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil_2 = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil_2");
  hcompute_hw_output_stencil_2->add_function("hcompute_hw_output_stencil_2");
  hcompute_hw_output_stencil_2->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "2");
  hcompute_hw_output_stencil_2->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "2");

//store is: hw_output.stencil(3, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(3, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil_3 = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil_3");
  hcompute_hw_output_stencil_3->add_function("hcompute_hw_output_stencil_3");
  hcompute_hw_output_stencil_3->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "3");
  hcompute_hw_output_stencil_3->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "3");

//store is: hw_output.stencil(4, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(4, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil_4 = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil_4");
  hcompute_hw_output_stencil_4->add_function("hcompute_hw_output_stencil_4");
  hcompute_hw_output_stencil_4->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "4");
  hcompute_hw_output_stencil_4->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "4");

//store is: hw_output.stencil(5, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(5, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil_5 = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil_5");
  hcompute_hw_output_stencil_5->add_function("hcompute_hw_output_stencil_5");
  hcompute_hw_output_stencil_5->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "5");
  hcompute_hw_output_stencil_5->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "5");

//store is: hw_output.stencil(6, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra) = depthwise_conv.stencil(6, hw_output_s0_x_x_cgra, hw_output_s0_y_y_cgra)
  auto hcompute_hw_output_stencil_6 = hw_output_s0_x_x_cgra->add_op("op_hcompute_hw_output_stencil_6");
  hcompute_hw_output_stencil_6->add_function("hcompute_hw_output_stencil_6");
  hcompute_hw_output_stencil_6->add_load("depthwise_conv_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "6");
  hcompute_hw_output_stencil_6->add_store("hw_output_stencil", "hw_output_s0_y_y_cgra", "hw_output_s0_x_x_cgra", "6");

//consuming hw_output.stencil
  auto hw_output_global_wrapper_s0_y_y_glb = prg.add_loop("hw_output_global_wrapper_s0_y_y_glb", 0, 114);
  auto hw_output_global_wrapper_s0_x_x_glb = hw_output_global_wrapper_s0_y_y_glb->add_loop("hw_output_global_wrapper_s0_x_x_glb", 0, 114);

//store is: hw_output_global_wrapper.glb.stencil(0, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(0, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil");
  hcompute_hw_output_global_wrapper_glb_stencil->add_function("hcompute_hw_output_global_wrapper_glb_stencil");
  hcompute_hw_output_global_wrapper_glb_stencil->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "0");
  hcompute_hw_output_global_wrapper_glb_stencil->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "0");

//store is: hw_output_global_wrapper.glb.stencil(1, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(1, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil_1 = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_function("hcompute_hw_output_global_wrapper_glb_stencil_1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "1");
  hcompute_hw_output_global_wrapper_glb_stencil_1->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "1");

//store is: hw_output_global_wrapper.glb.stencil(2, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(2, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil_2 = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_function("hcompute_hw_output_global_wrapper_glb_stencil_2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "2");
  hcompute_hw_output_global_wrapper_glb_stencil_2->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "2");

//store is: hw_output_global_wrapper.glb.stencil(3, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(3, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil_3 = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_function("hcompute_hw_output_global_wrapper_glb_stencil_3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "3");
  hcompute_hw_output_global_wrapper_glb_stencil_3->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "3");

//store is: hw_output_global_wrapper.glb.stencil(4, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(4, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil_4 = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_function("hcompute_hw_output_global_wrapper_glb_stencil_4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "4");
  hcompute_hw_output_global_wrapper_glb_stencil_4->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "4");

//store is: hw_output_global_wrapper.glb.stencil(5, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(5, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil_5 = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_function("hcompute_hw_output_global_wrapper_glb_stencil_5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "5");
  hcompute_hw_output_global_wrapper_glb_stencil_5->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "5");

//store is: hw_output_global_wrapper.glb.stencil(6, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb) = hw_output.stencil(6, hw_output_global_wrapper_s0_x_x_glb, hw_output_global_wrapper_s0_y_y_glb)
  auto hcompute_hw_output_global_wrapper_glb_stencil_6 = hw_output_global_wrapper_s0_x_x_glb->add_op("op_hcompute_hw_output_global_wrapper_glb_stencil_6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_function("hcompute_hw_output_global_wrapper_glb_stencil_6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_load("hw_output_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "6");
  hcompute_hw_output_global_wrapper_glb_stencil_6->add_store("hw_output_global_wrapper_glb_stencil", "hw_output_global_wrapper_s0_y_y_glb", "hw_output_global_wrapper_s0_x_x_glb", "6");

  return prg;
}


// empty
