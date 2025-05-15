#include "app.h"
#include "ubuffer.h"
#include "codegen.h"
#include "prog.h"

prog unsharp() {
  prog prg;
  prg.compute_unit_file = "unsharp_compute.h";
  prg.name = "unsharp";

// Stencil<uint16_t, 3, 564, 2506> &hw_input_stencil = arg_1;
  prg.add_input("hw_input_stencil");
  prg.buffer_port_widths["hw_input_stencil"] = 16;
// Stencil<int16_t, 3, 310, 2500> &hw_output_global_wrapper_stencil = arg_3;
  prg.add_output("hw_output_global_wrapper_stencil");
  prg.buffer_port_widths["hw_output_global_wrapper_stencil"] = 16;

////producing hw_input_global_wrapper.glb.stencil
  auto hw_input_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_s0_y", 0, 256);
  auto hw_input_global_wrapper_s0_x_x = hw_input_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_s0_x_x", 0, 68);

//store is: hw_input_global_wrapper.glb.stencil(0, hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(0, (hw_input_global_wrapper_s0_x_x + -3), (hw_input_global_wrapper_s0_y + -3))
  auto hcompute_hw_input_global_wrapper_glb_stencil = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil");
  hcompute_hw_input_global_wrapper_glb_stencil->add_function("hcompute_hw_input_global_wrapper_glb_stencil");
  hcompute_hw_input_global_wrapper_glb_stencil->add_load("hw_input_stencil", "(hw_input_global_wrapper_s0_y + -3)", "(hw_input_global_wrapper_s0_x_x + -3)", "0");
  prg.buffer_port_widths["hw_input_global_wrapper_glb_stencil"] = 16;
  hcompute_hw_input_global_wrapper_glb_stencil->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "hw_input_global_wrapper_s0_x_x", "0");

//store is: hw_input_global_wrapper.glb.stencil(1, hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(1, (hw_input_global_wrapper_s0_x_x + -3), (hw_input_global_wrapper_s0_y + -3))
  auto hcompute_hw_input_global_wrapper_glb_stencil_1 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_function("hcompute_hw_input_global_wrapper_glb_stencil_1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_load("hw_input_stencil", "(hw_input_global_wrapper_s0_y + -3)", "(hw_input_global_wrapper_s0_x_x + -3)", "1");
  hcompute_hw_input_global_wrapper_glb_stencil_1->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "hw_input_global_wrapper_s0_x_x", "1");

//store is: hw_input_global_wrapper.glb.stencil(2, hw_input_global_wrapper_s0_x_x, hw_input_global_wrapper_s0_y) = hw_input.stencil(2, (hw_input_global_wrapper_s0_x_x + -3), (hw_input_global_wrapper_s0_y + -3))
  auto hcompute_hw_input_global_wrapper_glb_stencil_2 = hw_input_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_glb_stencil_2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_function("hcompute_hw_input_global_wrapper_glb_stencil_2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_load("hw_input_stencil", "(hw_input_global_wrapper_s0_y + -3)", "(hw_input_global_wrapper_s0_x_x + -3)", "2");
  hcompute_hw_input_global_wrapper_glb_stencil_2->add_store("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_s0_y", "hw_input_global_wrapper_s0_x_x", "2");

//consuming hw_input_global_wrapper.glb.stencil
////producing hw_output.glb.stencil
////producing hw_input_global_wrapper_global_wrapper.stencil
  auto hw_input_global_wrapper_global_wrapper_s0_y = prg.add_loop("hw_input_global_wrapper_global_wrapper_s0_y", 0, 256);
  auto hw_input_global_wrapper_global_wrapper_s0_x_x = hw_input_global_wrapper_global_wrapper_s0_y->add_loop("hw_input_global_wrapper_global_wrapper_s0_x_x", 0, 68);

//store is: hw_input_global_wrapper_global_wrapper.stencil(0, hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(0, hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "hw_input_global_wrapper_global_wrapper_s0_x_x", "0");
  prg.buffer_port_widths["hw_input_global_wrapper_global_wrapper_stencil"] = 16;
  hcompute_hw_input_global_wrapper_global_wrapper_stencil->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "hw_input_global_wrapper_global_wrapper_s0_x_x", "0");

//store is: hw_input_global_wrapper_global_wrapper.stencil(1, hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(1, hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_1 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "hw_input_global_wrapper_global_wrapper_s0_x_x", "1");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_1->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "hw_input_global_wrapper_global_wrapper_s0_x_x", "1");

//store is: hw_input_global_wrapper_global_wrapper.stencil(2, hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y) = hw_input_global_wrapper.glb.stencil(2, hw_input_global_wrapper_global_wrapper_s0_x_x, hw_input_global_wrapper_global_wrapper_s0_y)
  auto hcompute_hw_input_global_wrapper_global_wrapper_stencil_2 = hw_input_global_wrapper_global_wrapper_s0_x_x->add_op("op_hcompute_hw_input_global_wrapper_global_wrapper_stencil_2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_function("hcompute_hw_input_global_wrapper_global_wrapper_stencil_2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_load("hw_input_global_wrapper_glb_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "hw_input_global_wrapper_global_wrapper_s0_x_x", "2");
  hcompute_hw_input_global_wrapper_global_wrapper_stencil_2->add_store("hw_input_global_wrapper_global_wrapper_stencil", "hw_input_global_wrapper_global_wrapper_s0_y", "hw_input_global_wrapper_global_wrapper_s0_x_x", "2");

//consuming hw_input_global_wrapper_global_wrapper.stencil
////producing gray.stencil
  auto gray_s0_y = prg.add_loop("gray_s0_y", 0, 256);
  auto gray_s0_x_x = gray_s0_y->add_loop("gray_s0_x_x", 0, 68);

//store is: gray.stencil(gray_s0_x_x, gray_s0_y) = (((hw_input_global_wrapper_global_wrapper.stencil(1, gray_s0_x_x, gray_s0_y)*(uint16)150) + ((hw_input_global_wrapper_global_wrapper.stencil(2, gray_s0_x_x, gray_s0_y)*(uint16)29) + (hw_input_global_wrapper_global_wrapper.stencil(0, gray_s0_x_x, gray_s0_y)*(uint16)77)))/(uint16)256)
  auto hcompute_gray_stencil = gray_s0_x_x->add_op("op_hcompute_gray_stencil");
  hcompute_gray_stencil->add_function("hcompute_gray_stencil");
  hcompute_gray_stencil->add_load("hw_input_global_wrapper_global_wrapper_stencil", "gray_s0_y", "gray_s0_x_x", "1");
  hcompute_gray_stencil->add_load("hw_input_global_wrapper_global_wrapper_stencil", "gray_s0_y", "gray_s0_x_x", "2");
  hcompute_gray_stencil->add_load("hw_input_global_wrapper_global_wrapper_stencil", "gray_s0_y", "gray_s0_x_x", "0");
  prg.buffer_port_widths["gray_stencil"] = 16;
  hcompute_gray_stencil->add_store("gray_stencil", "gray_s0_y", "gray_s0_x_x");

//consuming gray.stencil
////producing rom_div_lookupa0

//consuming rom_div_lookupa0
////producing reciprocal.stencil
  auto reciprocal_s0_y = prg.add_loop("reciprocal_s0_y", 0, 250);
  auto reciprocal_s0_x_x = reciprocal_s0_y->add_loop("reciprocal_s0_x_x", 0, 62);

//store is: reciprocal.stencil(reciprocal_s0_x_x, reciprocal_s0_y) = rom_div_lookupa0[(int32(max(gray.stencil((reciprocal_s0_x_x + 3), (reciprocal_s0_y + 3)), (uint16)1)) + -1)]
  auto hcompute_reciprocal_stencil = reciprocal_s0_x_x->add_op("op_hcompute_reciprocal_stencil");
  hcompute_reciprocal_stencil->add_function("hcompute_reciprocal_stencil");
  hcompute_reciprocal_stencil->add_load("gray_stencil", "(reciprocal_s0_y + 3)", "(reciprocal_s0_x_x + 3)");
  prg.buffer_port_widths["reciprocal_stencil"] = 16;
  hcompute_reciprocal_stencil->add_store("reciprocal_stencil", "reciprocal_s0_y", "reciprocal_s0_x_x");
  //hcompute_reciprocal_stencil->index_variable_prefetch_cycle(1);
  hcompute_reciprocal_stencil->add_latency(1);

//consuming reciprocal.stencil
////producing blur_unnormalized.stencil
  auto blur_unnormalized_s0_y = prg.add_loop("blur_unnormalized_s0_y", 0, 250);
  auto blur_unnormalized_s0_x_x = blur_unnormalized_s0_y->add_loop("blur_unnormalized_s0_x_x", 0, 62);

//store is: blur_unnormalized.stencil(blur_unnormalized_s0_x_x, blur_unnormalized_s0_y) = (uint16)0
  auto hcompute_blur_unnormalized_stencil = blur_unnormalized_s0_x_x->add_op("op_hcompute_blur_unnormalized_stencil");
  hcompute_blur_unnormalized_stencil->add_function("hcompute_blur_unnormalized_stencil");
  prg.buffer_port_widths["blur_unnormalized_stencil"] = 16;
  hcompute_blur_unnormalized_stencil->add_store("blur_unnormalized_stencil", "blur_unnormalized_s0_y", "blur_unnormalized_s0_x_x");
  auto blur_unnormalized_s1_y = prg.add_loop("blur_unnormalized_s1_y", 0, 250);
  auto blur_unnormalized_s1_x_x = blur_unnormalized_s1_y->add_loop("blur_unnormalized_s1_x_x", 0, 62);

//store is: blur_unnormalized.stencil(blur_unnormalized_s1_x_x, blur_unnormalized_s1_y) = (gray.stencil((blur_unnormalized_s1_x_x + 1), blur_unnormalized_s1_y) + (blur_unnormalized.stencil(blur_unnormalized_s1_x_x, blur_unnormalized_s1_y) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), blur_unnormalized_s1_y)*(uint16)2) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), blur_unnormalized_s1_y)*(uint16)2) + ((gray.stencil((blur_unnormalized_s1_x_x + 4), blur_unnormalized_s1_y)*(uint16)2) + (gray.stencil((blur_unnormalized_s1_x_x + 5), blur_unnormalized_s1_y) + (gray.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 1)) + ((gray.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 1))*(uint16)3) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 1))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), (blur_unnormalized_s1_y + 1))*(uint16)7) + ((gray.stencil((blur_unnormalized_s1_x_x + 4), (blur_unnormalized_s1_y + 1))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 5), (blur_unnormalized_s1_y + 1))*(uint16)3) + (gray.stencil((blur_unnormalized_s1_x_x + 6), (blur_unnormalized_s1_y + 1)) + ((gray.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 2))*(uint16)2) + ((gray.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 2))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 2))*(uint16)12) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), (blur_unnormalized_s1_y + 2))*(uint16)15) + ((gray.stencil((blur_unnormalized_s1_x_x + 4), (blur_unnormalized_s1_y + 2))*(uint16)12) + ((gray.stencil((blur_unnormalized_s1_x_x + 5), (blur_unnormalized_s1_y + 2))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 6), (blur_unnormalized_s1_y + 2))*(uint16)2) + ((gray.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 3))*(uint16)2) + ((gray.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 3))*(uint16)7) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 3))*(uint16)15) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), (blur_unnormalized_s1_y + 3))*(uint16)18) + ((gray.stencil((blur_unnormalized_s1_x_x + 4), (blur_unnormalized_s1_y + 3))*(uint16)15) + ((gray.stencil((blur_unnormalized_s1_x_x + 5), (blur_unnormalized_s1_y + 3))*(uint16)7) + ((gray.stencil((blur_unnormalized_s1_x_x + 6), (blur_unnormalized_s1_y + 3))*(uint16)2) + ((gray.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 4))*(uint16)2) + ((gray.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 4))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 4))*(uint16)12) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), (blur_unnormalized_s1_y + 4))*(uint16)15) + ((gray.stencil((blur_unnormalized_s1_x_x + 4), (blur_unnormalized_s1_y + 4))*(uint16)12) + ((gray.stencil((blur_unnormalized_s1_x_x + 5), (blur_unnormalized_s1_y + 4))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 6), (blur_unnormalized_s1_y + 4))*(uint16)2) + (gray.stencil(blur_unnormalized_s1_x_x, (blur_unnormalized_s1_y + 5)) + ((gray.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 5))*(uint16)3) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 5))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), (blur_unnormalized_s1_y + 5))*(uint16)7) + ((gray.stencil((blur_unnormalized_s1_x_x + 4), (blur_unnormalized_s1_y + 5))*(uint16)6) + ((gray.stencil((blur_unnormalized_s1_x_x + 5), (blur_unnormalized_s1_y + 5))*(uint16)3) + (gray.stencil((blur_unnormalized_s1_x_x + 6), (blur_unnormalized_s1_y + 5)) + (gray.stencil((blur_unnormalized_s1_x_x + 1), (blur_unnormalized_s1_y + 6)) + ((gray.stencil((blur_unnormalized_s1_x_x + 2), (blur_unnormalized_s1_y + 6))*(uint16)2) + ((gray.stencil((blur_unnormalized_s1_x_x + 3), (blur_unnormalized_s1_y + 6))*(uint16)2) + (gray.stencil((blur_unnormalized_s1_x_x + 5), (blur_unnormalized_s1_y + 6)) + (gray.stencil((blur_unnormalized_s1_x_x + 4), (blur_unnormalized_s1_y + 6))*(uint16)2))))))))))))))))))))))))))))))))))))))))))))))
  auto hcompute_blur_unnormalized_stencil_1 = blur_unnormalized_s1_x_x->add_op("op_hcompute_blur_unnormalized_stencil_1");
  hcompute_blur_unnormalized_stencil_1->add_function("hcompute_blur_unnormalized_stencil_1");
  hcompute_blur_unnormalized_stencil_1->add_load("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "blur_unnormalized_s1_x_x");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x + 6)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "blur_unnormalized_s1_x_x");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 2)", "(blur_unnormalized_s1_x_x + 6)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "blur_unnormalized_s1_x_x");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 3)", "(blur_unnormalized_s1_x_x + 6)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "blur_unnormalized_s1_x_x");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 4)", "(blur_unnormalized_s1_x_x + 6)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "blur_unnormalized_s1_x_x");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 5)", "(blur_unnormalized_s1_x_x + 6)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 6)", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 6)", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 6)", "(blur_unnormalized_s1_x_x + 3)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 6)", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 6)", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x + 4)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "blur_unnormalized_s1_y", "(blur_unnormalized_s1_x_x + 5)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "blur_unnormalized_s1_x_x");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x + 1)");
  hcompute_blur_unnormalized_stencil_1->add_load("gray_stencil", "(blur_unnormalized_s1_y + 1)", "(blur_unnormalized_s1_x_x + 2)");
  hcompute_blur_unnormalized_stencil_1->add_store("blur_unnormalized_stencil", "blur_unnormalized_s1_y", "blur_unnormalized_s1_x_x");

//consuming blur_unnormalized.stencil
////producing sharpen.stencil
  auto sharpen_s0_y = prg.add_loop("sharpen_s0_y", 0, 250);
  auto sharpen_s0_x_x = sharpen_s0_y->add_loop("sharpen_s0_x_x", 0, 62);

//store is: sharpen.stencil(sharpen_s0_x_x, sharpen_s0_y) = uint16(max(min(((int16(gray.stencil((sharpen_s0_x_x + 3), (sharpen_s0_y + 3)))*(int16)2) - int16(uint8((blur_unnormalized.stencil(sharpen_s0_x_x, sharpen_s0_y)/(uint16)256)))), (int16)255), (int16)0))
  auto hcompute_sharpen_stencil = sharpen_s0_x_x->add_op("op_hcompute_sharpen_stencil");
  hcompute_sharpen_stencil->add_function("hcompute_sharpen_stencil");
  hcompute_sharpen_stencil->add_load("blur_unnormalized_stencil", "sharpen_s0_y", "sharpen_s0_x_x");
  hcompute_sharpen_stencil->add_load("gray_stencil", "(sharpen_s0_y + 3)", "(sharpen_s0_x_x + 3)");
  prg.buffer_port_widths["sharpen_stencil"] = 16;
  hcompute_sharpen_stencil->add_store("sharpen_stencil", "sharpen_s0_y", "sharpen_s0_x_x");

//consuming sharpen.stencil
////producing ratio.stencil
  auto ratio_s0_y = prg.add_loop("ratio_s0_y", 0, 250);
  auto ratio_s0_x_x = ratio_s0_y->add_loop("ratio_s0_x_x", 0, 62);

//store is: ratio.stencil(ratio_s0_x_x, ratio_s0_y) = (sharpen.stencil(ratio_s0_x_x, ratio_s0_y)*reciprocal.stencil(ratio_s0_x_x, ratio_s0_y))
  auto hcompute_ratio_stencil = ratio_s0_x_x->add_op("op_hcompute_ratio_stencil");
  hcompute_ratio_stencil->add_function("hcompute_ratio_stencil");
  hcompute_ratio_stencil->add_load("reciprocal_stencil", "ratio_s0_y", "ratio_s0_x_x");
  hcompute_ratio_stencil->add_load("sharpen_stencil", "ratio_s0_y", "ratio_s0_x_x");
  prg.buffer_port_widths["ratio_stencil"] = 16;
  hcompute_ratio_stencil->add_store("ratio_stencil", "ratio_s0_y", "ratio_s0_x_x");

//consuming ratio.stencil
  auto hw_output_s0_y_yi = prg.add_loop("hw_output_s0_y_yi", 0, 250);
  auto hw_output_s0_x_xi_xi = hw_output_s0_y_yi->add_loop("hw_output_s0_x_xi_xi", 0, 62);

//store is: hw_output.glb.stencil(0, hw_output_s0_x_xi_xi, hw_output_s0_y_yi) = int16(((int32(ratio.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi))*int32(hw_input_global_wrapper_global_wrapper.stencil(0, (hw_output_s0_x_xi_xi + 3), (hw_output_s0_y_yi + 3))))/256))
  auto hcompute_hw_output_glb_stencil = hw_output_s0_x_xi_xi->add_op("op_hcompute_hw_output_glb_stencil");
  hcompute_hw_output_glb_stencil->add_function("hcompute_hw_output_glb_stencil");
  hcompute_hw_output_glb_stencil->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(hw_output_s0_y_yi + 3)", "(hw_output_s0_x_xi_xi + 3)", "0");
  hcompute_hw_output_glb_stencil->add_load("ratio_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi");
  prg.buffer_port_widths["hw_output_glb_stencil"] = 16;
  hcompute_hw_output_glb_stencil->add_store("hw_output_glb_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi", "0");

//store is: hw_output.glb.stencil(1, hw_output_s0_x_xi_xi, hw_output_s0_y_yi) = int16(((int32(ratio.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi))*int32(hw_input_global_wrapper_global_wrapper.stencil(1, (hw_output_s0_x_xi_xi + 3), (hw_output_s0_y_yi + 3))))/256))
  auto hcompute_hw_output_glb_stencil_1 = hw_output_s0_x_xi_xi->add_op("op_hcompute_hw_output_glb_stencil_1");
  hcompute_hw_output_glb_stencil_1->add_function("hcompute_hw_output_glb_stencil_1");
  hcompute_hw_output_glb_stencil_1->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(hw_output_s0_y_yi + 3)", "(hw_output_s0_x_xi_xi + 3)", "1");
  hcompute_hw_output_glb_stencil_1->add_load("ratio_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi");
  hcompute_hw_output_glb_stencil_1->add_store("hw_output_glb_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi", "1");

//store is: hw_output.glb.stencil(2, hw_output_s0_x_xi_xi, hw_output_s0_y_yi) = int16(((int32(ratio.stencil(hw_output_s0_x_xi_xi, hw_output_s0_y_yi))*int32(hw_input_global_wrapper_global_wrapper.stencil(2, (hw_output_s0_x_xi_xi + 3), (hw_output_s0_y_yi + 3))))/256))
  auto hcompute_hw_output_glb_stencil_2 = hw_output_s0_x_xi_xi->add_op("op_hcompute_hw_output_glb_stencil_2");
  hcompute_hw_output_glb_stencil_2->add_function("hcompute_hw_output_glb_stencil_2");
  hcompute_hw_output_glb_stencil_2->add_load("hw_input_global_wrapper_global_wrapper_stencil", "(hw_output_s0_y_yi + 3)", "(hw_output_s0_x_xi_xi + 3)", "2");
  hcompute_hw_output_glb_stencil_2->add_load("ratio_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi");
  hcompute_hw_output_glb_stencil_2->add_store("hw_output_glb_stencil", "hw_output_s0_y_yi", "hw_output_s0_x_xi_xi", "2");

//consuming hw_output.glb.stencil
  auto hw_output_global_wrapper_s0_y_yi = prg.add_loop("hw_output_global_wrapper_s0_y_yi", 0, 250);
  auto hw_output_global_wrapper_s0_x_xi_xi = hw_output_global_wrapper_s0_y_yi->add_loop("hw_output_global_wrapper_s0_x_xi_xi", 0, 62);

//store is: hw_output_global_wrapper.stencil(0, hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi) = hw_output.glb.stencil(0, hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_stencil = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_stencil");
  hcompute_hw_output_global_wrapper_stencil->add_function("hcompute_hw_output_global_wrapper_stencil");
  hcompute_hw_output_global_wrapper_stencil->add_load("hw_output_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "hw_output_global_wrapper_s0_x_xi_xi", "0");
  hcompute_hw_output_global_wrapper_stencil->add_store("hw_output_global_wrapper_stencil", "hw_output_global_wrapper_s0_y_yi", "hw_output_global_wrapper_s0_x_xi_xi", "0");

//store is: hw_output_global_wrapper.stencil(1, hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi) = hw_output.glb.stencil(1, hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_stencil_1 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_stencil_1");
  hcompute_hw_output_global_wrapper_stencil_1->add_function("hcompute_hw_output_global_wrapper_stencil_1");
  hcompute_hw_output_global_wrapper_stencil_1->add_load("hw_output_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "hw_output_global_wrapper_s0_x_xi_xi", "1");
  hcompute_hw_output_global_wrapper_stencil_1->add_store("hw_output_global_wrapper_stencil", "hw_output_global_wrapper_s0_y_yi", "hw_output_global_wrapper_s0_x_xi_xi", "1");

//store is: hw_output_global_wrapper.stencil(2, hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi) = hw_output.glb.stencil(2, hw_output_global_wrapper_s0_x_xi_xi, hw_output_global_wrapper_s0_y_yi)
  auto hcompute_hw_output_global_wrapper_stencil_2 = hw_output_global_wrapper_s0_x_xi_xi->add_op("op_hcompute_hw_output_global_wrapper_stencil_2");
  hcompute_hw_output_global_wrapper_stencil_2->add_function("hcompute_hw_output_global_wrapper_stencil_2");
  hcompute_hw_output_global_wrapper_stencil_2->add_load("hw_output_glb_stencil", "hw_output_global_wrapper_s0_y_yi", "hw_output_global_wrapper_s0_x_xi_xi", "2");
  hcompute_hw_output_global_wrapper_stencil_2->add_store("hw_output_global_wrapper_stencil", "hw_output_global_wrapper_s0_y_yi", "hw_output_global_wrapper_s0_x_xi_xi", "2");

  return prg;
}


// mywidth=62 myunroll=1 schedule=3
