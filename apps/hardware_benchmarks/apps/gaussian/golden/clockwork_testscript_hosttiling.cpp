#include "clockwork_testscript.h"
#include "unoptimized_gaussian.h"
#include "hw_classes.h"
#include <fstream>

void run_clockwork_program(RDAI_MemObject **mem_object_list) {
	// input and output memory objects
  uint16_t *arg_0 = (uint16_t*) mem_object_list[0]->host_ptr;
	uint16_t *hw_input_stencil = (uint16_t*) mem_object_list[1]->host_ptr;
  uint16_t *arg_2 = (uint16_t*) mem_object_list[2]->host_ptr;
	uint16_t *hw_output_stencil = (uint16_t*) mem_object_list[3]->host_ptr;

  int y = arg_0[0];
  int x = arg_2[0];

	// input and output stream declarations
	HWStream< hw_uint<16> > hw_input_stencil_stream;
	HWStream< hw_uint<16> > hw_output_stencil_stream;
  std::cout << "x=" << x << " y=" << y << std::endl;
	// provision input stream 256_stream
	// provision input stream hw_input_stencil_stream
	for (int l1 = 31*y; l1 < 31*y+33; l1++) {
	for (int l0 = 31*x; l0 < 31*x+33; l0++) {
		hw_uint<16> in_val;
		set_at<0, 16, 16>(in_val, hw_uint<16>(hw_input_stencil[((l1*64) + l0)]));
		hw_input_stencil_stream.write(in_val);
	} } 

	// invoke clockwork program
	unoptimized_gaussian(
		hw_input_stencil_stream,
		hw_output_stencil_stream
	);

	// provision output buffer
	for (int l1 = 31*y; l1 < 31*y+31; l1++) {
	for (int l0 = 31*x; l0 < 31*x+31; l0++) {
		hw_uint<16> actual = hw_output_stencil_stream.read();
		int actual_lane = actual.extract<0, 15>();
		hw_output_stencil[((l1*62) + l0)] = (uint16_t) actual_lane;
	} } 
	ofstream hw_output_file("bin/hw_output.leraw", ios::binary);
	hw_output_file.write(reinterpret_cast<const char *>(hw_output_stencil),
		sizeof(hw_output_stencil[0]) * 62 * 62);
	hw_output_file.close();
	ofstream hw_output_header_file("bin/hw_output_header.txt", ios::binary);
	hw_output_header_file << "P5" << std::endl;
	hw_output_header_file << "62 62" << std::endl;
	hw_output_header_file << "65535" << std::endl;
	hw_output_header_file.close();
}

