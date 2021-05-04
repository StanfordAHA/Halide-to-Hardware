#include "app.h"
#include "ubuffer.h"
#include "codegen.h"
#include "prog.h"

prog pyramid_blur() {
  prog prg;
  prg.compute_unit_file = "pyramid_blur_compute.h";
  prg.name = "pyramid_blur";

// Stencil<uint16_t, 64, 64> &hw_in_stencil = arg_0;
  prg.add_input("hw_in_stencil");
  prg.buffer_port_widths["hw_in_stencil"] = 16;
// Stencil<uint16_t, 4, 4> &hw_output_stencil = arg_1;
  prg.add_output("hw_output_stencil");
  prg.buffer_port_widths["hw_output_stencil"] = 16;

////producing hw_in_global_wrapper.stencil
  auto hw_in_global_wrapper_s0_y = prg.add_loop("hw_in_global_wrapper_s0_y", 0, 64);
  auto hw_in_global_wrapper_s0_x = hw_in_global_wrapper_s0_y->add_loop("hw_in_global_wrapper_s0_x", 0, 64);

//store is: hw_in_global_wrapper.stencil(hw_in_global_wrapper_s0_x, hw_in_global_wrapper_s0_y) = hw_in.stencil(hw_in_global_wrapper_s0_x, hw_in_global_wrapper_s0_y)
  auto hcompute_hw_in_global_wrapper_stencil = hw_in_global_wrapper_s0_x->add_op("op_hcompute_hw_in_global_wrapper_stencil");
  hcompute_hw_in_global_wrapper_stencil->add_function("hcompute_hw_in_global_wrapper_stencil");
  hcompute_hw_in_global_wrapper_stencil->add_load("hw_in_stencil", "hw_in_global_wrapper_s0_y", "hw_in_global_wrapper_s0_x");
  prg.buffer_port_widths["hw_in_global_wrapper_stencil"] = 16;
  hcompute_hw_in_global_wrapper_stencil->add_store("hw_in_global_wrapper_stencil", "hw_in_global_wrapper_s0_y", "hw_in_global_wrapper_s0_x");

//consuming hw_in_global_wrapper.stencil
////producing blur3$1.stencil
  auto blur3_1_s0_y = prg.add_loop("blur3_1_s0_y", 0, 4);
  auto index = blur3_1_s0_y->add_loop("index", 0, 4);
  auto shared_y = index->add_loop("shared_y", 0, _274);
  auto shared_x = shared_y->add_loop("shared_x", 0, select((index == 0), 32, select((index == 1), 16, select((index == 2), 8, 4))));

//store is: share_input0.stencil(index) = select((index == 0), hw_in_global_wrapper.stencil((shared_x*2), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)), select((index == 1), blur0$1.stencil((shared_x*2), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)), select((index == 2), blur1$1.stencil((shared_x*2), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)), blur2$1.stencil((shared_x*2), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)))))
  auto hcompute_share_input0_stencil = shared_x->add_op("op_hcompute_share_input0_stencil");
  hcompute_share_input0_stencil->add_function("hcompute_share_input0_stencil");
  prg.buffer_port_widths["blur0_1_stencil"] = 16;
  hcompute_share_input0_stencil->add_load("blur0_1_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "(shared_x*2)");
  prg.buffer_port_widths["blur1_1_stencil"] = 16;
  hcompute_share_input0_stencil->add_load("blur1_1_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "(shared_x*2)");
  prg.buffer_port_widths["blur2_1_stencil"] = 16;
  hcompute_share_input0_stencil->add_load("blur2_1_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "(shared_x*2)");
  hcompute_share_input0_stencil->add_load("hw_in_global_wrapper_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "(shared_x*2)");
  prg.buffer_port_widths["share_input0_stencil"] = 16;
  hcompute_share_input0_stencil->add_store("share_input0_stencil", "index");
  hcompute_share_input0_stencil->compute_unit_needs_index_variable("index");

//store is: share_input1.stencil(index) = select((index == 0), hw_in_global_wrapper.stencil(((shared_x*2) + 1), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)), select((index == 1), blur0$1.stencil(((shared_x*2) + 1), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)), select((index == 2), blur1$1.stencil(((shared_x*2) + 1), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)), blur2$1.stencil(((shared_x*2) + 1), ((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)))))
  auto hcompute_share_input1_stencil = shared_x->add_op("op_hcompute_share_input1_stencil");
  hcompute_share_input1_stencil->add_function("hcompute_share_input1_stencil");
  hcompute_share_input1_stencil->add_load("blur0_1_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "((shared_x*2) + 1)");
  hcompute_share_input1_stencil->add_load("blur1_1_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "((shared_x*2) + 1)");
  hcompute_share_input1_stencil->add_load("blur2_1_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "((shared_x*2) + 1)");
  hcompute_share_input1_stencil->add_load("hw_in_global_wrapper_stencil", "((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2)", "((shared_x*2) + 1)");
  prg.buffer_port_widths["share_input1_stencil"] = 16;
  hcompute_share_input1_stencil->add_store("share_input1_stencil", "index");
  hcompute_share_input1_stencil->compute_unit_needs_index_variable("index");

//store is: share_input2.stencil(index) = select((index == 0), hw_in_global_wrapper.stencil((shared_x*2), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)), select((index == 1), blur0$1.stencil((shared_x*2), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)), select((index == 2), blur1$1.stencil((shared_x*2), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)), blur2$1.stencil((shared_x*2), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)))))
  auto hcompute_share_input2_stencil = shared_x->add_op("op_hcompute_share_input2_stencil");
  hcompute_share_input2_stencil->add_function("hcompute_share_input2_stencil");
  hcompute_share_input2_stencil->add_load("blur0_1_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "(shared_x*2)");
  hcompute_share_input2_stencil->add_load("blur1_1_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "(shared_x*2)");
  hcompute_share_input2_stencil->add_load("blur2_1_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "(shared_x*2)");
  hcompute_share_input2_stencil->add_load("hw_in_global_wrapper_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "(shared_x*2)");
  prg.buffer_port_widths["share_input2_stencil"] = 16;
  hcompute_share_input2_stencil->add_store("share_input2_stencil", "index");
  hcompute_share_input2_stencil->compute_unit_needs_index_variable("index");

//store is: share_input3.stencil(index) = select((index == 0), hw_in_global_wrapper.stencil(((shared_x*2) + 1), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)), select((index == 1), blur0$1.stencil(((shared_x*2) + 1), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)), select((index == 2), blur1$1.stencil(((shared_x*2) + 1), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)), blur2$1.stencil(((shared_x*2) + 1), (((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)))))
  auto hcompute_share_input3_stencil = shared_x->add_op("op_hcompute_share_input3_stencil");
  hcompute_share_input3_stencil->add_function("hcompute_share_input3_stencil");
  hcompute_share_input3_stencil->add_load("blur0_1_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "((shared_x*2) + 1)");
  hcompute_share_input3_stencil->add_load("blur1_1_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "((shared_x*2) + 1)");
  hcompute_share_input3_stencil->add_load("blur2_1_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "((shared_x*2) + 1)");
  hcompute_share_input3_stencil->add_load("hw_in_global_wrapper_stencil", "(((select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)*2) + 1)", "((shared_x*2) + 1)");
  prg.buffer_port_widths["share_input3_stencil"] = 16;
  hcompute_share_input3_stencil->add_store("share_input3_stencil", "index");
  hcompute_share_input3_stencil->compute_unit_needs_index_variable("index");

//store is: share_output.stencil(index) = ((share_input1.stencil(index)*(uint16)63) + ((share_input0.stencil(index)*(uint16)50) + ((share_input3.stencil(index)*(uint16)78) + (share_input2.stencil(index)*(uint16)63))))
  auto hcompute_share_output_stencil = shared_x->add_op("op_hcompute_share_output_stencil");
  hcompute_share_output_stencil->add_function("hcompute_share_output_stencil");
  hcompute_share_output_stencil->add_load("share_input0_stencil", "index");
  hcompute_share_output_stencil->add_load("share_input1_stencil", "index");
  hcompute_share_output_stencil->add_load("share_input2_stencil", "index");
  hcompute_share_output_stencil->add_load("share_input3_stencil", "index");
  prg.buffer_port_widths["share_output_stencil"] = 16;
  hcompute_share_output_stencil->add_store("share_output_stencil", "index");

//store is: blur0$1.stencil(shared_x, (select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)) = share_output.stencil(0)
  auto hcompute_blur0_1_stencil = shared_x->add_op("op_hcompute_blur0_1_stencil");
  hcompute_blur0_1_stencil->add_function("hcompute_blur0_1_stencil");
  hcompute_blur0_1_stencil->add_load("share_output_stencil", "0");
  hcompute_blur0_1_stencil->add_store("blur0_1_stencil", "(select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)", "shared_x");

//store is: blur1$1.stencil(shared_x, (select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)) = share_output.stencil(1)
  auto hcompute_blur1_1_stencil = shared_x->add_op("op_hcompute_blur1_1_stencil");
  hcompute_blur1_1_stencil->add_function("hcompute_blur1_1_stencil");
  hcompute_blur1_1_stencil->add_load("share_output_stencil", "1");
  hcompute_blur1_1_stencil->add_store("blur1_1_stencil", "(select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)", "shared_x");

//store is: blur2$1.stencil(shared_x, (select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)) = share_output.stencil(2)
  auto hcompute_blur2_1_stencil = shared_x->add_op("op_hcompute_blur2_1_stencil");
  hcompute_blur2_1_stencil->add_function("hcompute_blur2_1_stencil");
  hcompute_blur2_1_stencil->add_load("share_output_stencil", "2");
  hcompute_blur2_1_stencil->add_store("blur2_1_stencil", "(select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)", "shared_x");

//store is: blur3$1.stencil(shared_x, (select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)) = share_output.stencil(index)
  auto hcompute_blur3_1_stencil = shared_x->add_op("op_hcompute_blur3_1_stencil");
  hcompute_blur3_1_stencil->add_function("hcompute_blur3_1_stencil");
  hcompute_blur3_1_stencil->add_load("share_output_stencil", "index");
  prg.buffer_port_widths["blur3_1_stencil"] = 16;
  hcompute_blur3_1_stencil->add_store("blur3_1_stencil", "(select((index == 0), (blur3_1_s0_y*8), select((index == 1), (blur3_1_s0_y*4), select((index == 2), (blur3_1_s0_y*2), blur3_1_s0_y))) + shared_y)", "shared_x");

//consuming blur3$1.stencil
  auto hw_output_s0_y_yi = prg.add_loop("hw_output_s0_y_yi", 0, 4);
  auto hw_output_s0_x_xi = hw_output_s0_y_yi->add_loop("hw_output_s0_x_xi", 0, 4);

//store is: hw_output.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi) = blur3$1.stencil(hw_output_s0_x_xi, hw_output_s0_y_yi)
  auto hcompute_hw_output_stencil = hw_output_s0_x_xi->add_op("op_hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_function("hcompute_hw_output_stencil");
  hcompute_hw_output_stencil->add_load("blur3_1_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi");
  hcompute_hw_output_stencil->add_store("hw_output_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi");

  return prg;
}
