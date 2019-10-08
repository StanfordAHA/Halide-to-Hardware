#include "vhls_target.h"

#include "Linebuffer.h"
#include "halide_math.h"
void vhls_target(
hls::stream<AxiPackedStencil<uint16_t, 10, 1> > &arg_0,
hls::stream<AxiPackedStencil<uint16_t, 10, 1> > &arg_1)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE axis register port=arg_0
#pragma HLS INTERFACE axis register port=arg_1

 // alias the arguments
 hls::stream<AxiPackedStencil<uint16_t, 10, 1> > &_hw_input_stencil_stream = arg_0;
 hls::stream<AxiPackedStencil<uint16_t, 10, 1> > &_hw_output_stencil_stream = arg_1;

 // dispatch_stream(_hw_input_stencil_stream, 2, 10, 10, 100, 1, 1, 10, 1, "hw_output", 1, 0, 100, 0, 10);
 hls::stream<AxiPackedStencil<uint16_t, 10, 1> > _hw_input_stencil_stream_to_hw_output;
#pragma HLS STREAM variable=_hw_input_stencil_stream_to_hw_output depth=1
#pragma HLS RESOURCE variable=_hw_input_stencil_stream_to_hw_output core=FIFO_SRL

 for (int _dim_1 = 0; _dim_1 <= 9; _dim_1 += 1)
 for (int _dim_0 = 0; _dim_0 <= 90; _dim_0 += 10)
 {
#pragma HLS PIPELINE
  PackedStencil<uint16_t, 10, 1> _tmp_stencil = _hw_input_stencil_stream.read();
  if (_dim_0 >= 0 && _dim_0 <= 90 && _dim_1 >= 0 && _dim_1 <= 9)
  {
   _hw_input_stencil_stream_to_hw_output.write(_tmp_stencil);
  }
 }
 (void)0;
 // produce hw_output.stencil.stream
 for (int _hw_output_y___scan_dim_1 = 0; _hw_output_y___scan_dim_1 < 0 + 10; _hw_output_y___scan_dim_1++)
 {
  for (int _hw_output_x___scan_dim_0 = 0; _hw_output_x___scan_dim_0 < 0 + 10; _hw_output_x___scan_dim_0++)
  {
#pragma HLS PIPELINE II=1
   Stencil<uint16_t, 10, 1> _hw_input_stencil;
#pragma HLS ARRAY_PARTITION variable=_hw_input_stencil.value complete dim=0

_hw_input_stencil(0, 0, 0) = 0;
   // produce hw_input.stencil
   _hw_input_stencil = _hw_input_stencil_stream_to_hw_output.read();
   (void)0;
   // consume hw_input.stencil
   Stencil<uint16_t, 10, 1> _hw_output_stencil;
#pragma HLS ARRAY_PARTITION variable=_hw_output_stencil.value complete dim=0

_hw_output_stencil(0, 0, 0) = 0;
   // produce hw_output.stencil
   uint16_t _278 = _hw_input_stencil(0, 0);
   uint16_t _279 = (uint16_t)(5);
   uint16_t _280 = _278 * _279;
   _hw_output_stencil(0, 0) = _280;
   uint16_t _281 = _hw_input_stencil(1, 0);
   uint16_t _282 = (uint16_t)(5);
   uint16_t _283 = _281 * _282;
   _hw_output_stencil(1, 0) = _283;
   uint16_t _284 = _hw_input_stencil(2, 0);
   uint16_t _285 = (uint16_t)(5);
   uint16_t _286 = _284 * _285;
   _hw_output_stencil(2, 0) = _286;
   uint16_t _287 = _hw_input_stencil(3, 0);
   uint16_t _288 = (uint16_t)(5);
   uint16_t _289 = _287 * _288;
   _hw_output_stencil(3, 0) = _289;
   uint16_t _290 = _hw_input_stencil(4, 0);
   uint16_t _291 = (uint16_t)(5);
   uint16_t _292 = _290 * _291;
   _hw_output_stencil(4, 0) = _292;
   uint16_t _293 = _hw_input_stencil(5, 0);
   uint16_t _294 = (uint16_t)(5);
   uint16_t _295 = _293 * _294;
   _hw_output_stencil(5, 0) = _295;
   uint16_t _296 = _hw_input_stencil(6, 0);
   uint16_t _297 = (uint16_t)(5);
   uint16_t _298 = _296 * _297;
   _hw_output_stencil(6, 0) = _298;
   uint16_t _299 = _hw_input_stencil(7, 0);
   uint16_t _300 = (uint16_t)(5);
   uint16_t _301 = _299 * _300;
   _hw_output_stencil(7, 0) = _301;
   uint16_t _302 = _hw_input_stencil(8, 0);
   uint16_t _303 = (uint16_t)(5);
   uint16_t _304 = _302 * _303;
   _hw_output_stencil(8, 0) = _304;
   uint16_t _305 = _hw_input_stencil(9, 0);
   uint16_t _306 = (uint16_t)(5);
   uint16_t _307 = _305 * _306;
   _hw_output_stencil(9, 0) = _307;
   // consume hw_output.stencil
   AxiPackedStencil<uint16_t, 10, 1> _hw_output_stencil_packed = _hw_output_stencil;
   if (_hw_output_x___scan_dim_0 == 9 && _hw_output_y___scan_dim_1 == 9) {
    _hw_output_stencil_packed.last = 1;
   } else {
    _hw_output_stencil_packed.last = 0;
   }
   _hw_output_stencil_stream.write(_hw_output_stencil_packed);
   (void)0;
  } // for _hw_output_x___scan_dim_0
 } // for _hw_output_y___scan_dim_1
} // kernel hls_target_vhls_target


