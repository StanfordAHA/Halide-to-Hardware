)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE s_axilite port=arg_0 bundle=config
#pragma HLS ARRAY_PARTITION variable=arg_0.value complete dim=0
#pragma HLS INTERFACE s_axilite port=arg_1 bundle=config
#pragma HLS ARRAY_PARTITION variable=arg_1.value complete dim=0
#pragma HLS INTERFACE s_axilite port=arg_2 bundle=config
#pragma HLS ARRAY_PARTITION variable=arg_2.value complete dim=0

 // alias the arguments
 Stencil<uint16_t, 64, 64> &hw_input_stencil = arg_0;
 Stencil<uint16_t, 64, 64> &hw_input_b_stencil = arg_1;
 Stencil<uint16_t, 64, 64> &hw_output_stencil = arg_2;
[, ], [, ] // produce hw_input_b_global_wrapper.stencil
 for (int hw_input_b_global_wrapper_s0_y = 0; hw_input_b_global_wrapper_s0_y < 64; hw_input_b_global_wrapper_s0_y++)
 {
  for (int hw_input_b_global_wrapper_s0_x_x = 0; hw_input_b_global_wrapper_s0_x_x < 64; hw_input_b_global_wrapper_s0_x_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _390 = _hw_input_b_stencil(_hw_input_b_global_wrapper_s0_x_x, _hw_input_b_global_wrapper_s0_y);
   _hw_input_b_global_wrapper_stencil(_hw_input_b_global_wrapper_s0_x_x, _hw_input_b_global_wrapper_s0_y) = _390;
  } // for hw_input_b_global_wrapper_s0_x_x
 } // for hw_input_b_global_wrapper_s0_y
 // consume hw_input_b_global_wrapper.stencil
[, ], [, ] // produce hw_input_global_wrapper.stencil
 for (int hw_input_global_wrapper_s0_y = 0; hw_input_global_wrapper_s0_y < 64; hw_input_global_wrapper_s0_y++)
 {
  for (int hw_input_global_wrapper_s0_x_x = 0; hw_input_global_wrapper_s0_x_x < 64; hw_input_global_wrapper_s0_x_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _391 = _hw_input_stencil(_hw_input_global_wrapper_s0_x_x, _hw_input_global_wrapper_s0_y);
   _hw_input_global_wrapper_stencil(_hw_input_global_wrapper_s0_x_x, _hw_input_global_wrapper_s0_y) = _391;
  } // for hw_input_global_wrapper_s0_x_x
 } // for hw_input_global_wrapper_s0_y
 // consume hw_input_global_wrapper.stencil
[, ], [, ] // produce add.stencil
 for (int add_s0_y = 0; add_s0_y < 64; add_s0_y++)
 {
  for (int add_s0_x_x = 0; add_s0_x_x < 64; add_s0_x_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _394 = _hw_input_global_wrapper_stencil(_add_s0_x_x, _add_s0_y);
   uint16_t _395 = _hw_input_b_global_wrapper_stencil(_add_s0_x_x, _add_s0_y);
   uint16_t _396 = _394 + _395;
   _add_stencil(_add_s0_x_x, _add_s0_y) = _396;
  } // for add_s0_x_x
 } // for add_s0_y
 // consume add.stencil
 for (int hw_output_s0_y_yi = 0; hw_output_s0_y_yi < 64; hw_output_s0_y_yi++)
 {
  for (int hw_output_s0_x_xi_xi = 0; hw_output_s0_x_xi_xi < 64; hw_output_s0_x_xi_xi++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _397 = _add_stencil(_hw_output_s0_x_xi_xi, _hw_output_s0_y_yi);
   _hw_output_stencil(_hw_output_s0_x_xi_xi, _hw_output_s0_y_yi) = _397;
  } // for hw_output_s0_x_xi_xi
 } // for hw_output_s0_y_yi
} // kernel hls_targettwo_input_add

