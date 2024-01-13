#include "clockwork_testscript.h"
#include "unoptimized_depthwise_conv.h"
#include "hw_classes.h"
#include <fstream>
#include <vector>

void run_clockwork_program(RDAI_MemObject **mem_object_list) {
	// input and output memory objects
	uint16_t *input_host_stencil = (uint16_t*) mem_object_list[0]->host_ptr;
	uint16_t *hw_output_global_wrapper_glb_stencil = (uint16_t*) mem_object_list[1]->host_ptr;

	// input and output stream declarations
	HWStream< hw_uint<16> > input_host_stencil_stream;
	HWStream< hw_uint<16> > hw_output_global_wrapper_glb_stencil_stream;
	int idx = 0;

	// provision input stream input_host_stencil_stream
	std::vector<uint16_t> input_host_stencil_stream_tile(7*116*116);   idx=0;
	for (int l2 = 0; l2 < 116; l2++) {
	for (int l1 = 0; l1 < 116; l1++) {
	for (int l0 = 0; l0 < 7; l0++) {
		hw_uint<16> in_val;
		set_at<0, 16, 16>(in_val, hw_uint<16>(input_host_stencil[((l2*812) + ((l1*7) + l0))]));
		input_host_stencil_stream.write(in_val);
		input_host_stencil_stream_tile[idx] = input_host_stencil[((l2*812) + ((l1*7) + l0))];  idx += 1;
	} } } 
	ofstream input_host_stencil_file("bin/input_host_stencil.leraw", ios::binary);
	input_host_stencil_file.write(reinterpret_cast<const char *>(input_host_stencil_stream_tile.data()),
		sizeof(input_host_stencil_stream_tile[0]) * 7 * 116 * 116);
	input_host_stencil_file.close();


	// invoke clockwork program
	unoptimized_depthwise_conv(
		input_host_stencil_stream,
		hw_output_global_wrapper_glb_stencil_stream
	);

	// provision output buffer
	std::vector<uint16_t> hw_output_global_wrapper_glb_stencil_stream_tile(7*114*114);   idx=0;
	for (int l2 = 0; l2 < 114; l2++) {
	for (int l1 = 0; l1 < 114; l1++) {
	for (int l0 = 0; l0 < 7; l0++) {
		hw_uint<16> actual = hw_output_global_wrapper_glb_stencil_stream.read();
		int actual_lane = actual.extract<0, 15>();
		hw_output_global_wrapper_glb_stencil[((l2*798) + ((l1*7) + l0))] = (uint16_t)(actual_lane);
		hw_output_global_wrapper_glb_stencil_stream_tile[idx] = hw_output_global_wrapper_glb_stencil[((l2*798) + ((l1*7) + l0))];  idx += 1;
	} } } 
	ofstream hw_output_file("bin/hw_output.leraw", ios::binary);
	hw_output_file.write(reinterpret_cast<const char *>(hw_output_global_wrapper_glb_stencil_stream_tile.data()),
		sizeof(hw_output_global_wrapper_glb_stencil_stream_tile[0]) * 7 * 114 * 114);
	hw_output_file.close();
	ofstream hw_output_header_file("bin/hw_output_header.txt", ios::binary);
	hw_output_header_file << "P5" << std::endl;
	hw_output_header_file << "7 114 114" << std::endl;
	hw_output_header_file << "65535" << std::endl;
	hw_output_header_file.close();
}

