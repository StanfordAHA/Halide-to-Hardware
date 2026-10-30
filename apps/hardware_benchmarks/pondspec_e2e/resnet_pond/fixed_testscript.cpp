#include "clockwork_testscript.h"
#include "unoptimized_resnet.h"

template<typename T>
void run_clockwork_program(Halide::Runtime::Buffer<T> input_copy_stencil,
                           Halide::Runtime::Buffer<T> kernel_copy_stencil,
                           Halide::Runtime::Buffer<T> output) {
  // input image and run on design
  HWStream<hw_uint<16> > output_stream;
  HWStream<hw_uint<16> > input_copy_stencil_stream;
  for (int y = 0; y < input_copy_stencil.dim(2).extent(); y++) {
  for (int x = 0; x < input_copy_stencil.dim(1).extent(); x++) {
  for (int z = 0; z < input_copy_stencil.dim(0).extent(); z++) {
    //image=z,x,y  read=y,x,z
    hw_uint<16> in_val;
    set_at<0*16, 16, 16>(in_val, input_copy_stencil(z, x, y));
    input_copy_stencil_stream.write(in_val);
  } } }

  HWStream<hw_uint<16> > kernel_copy_stencil_stream;
  for (int y = 0; y < kernel_copy_stencil.dim(3).extent(); y++) {
  for (int x = 0; x < kernel_copy_stencil.dim(2).extent(); x++) {
  for (int w = 0; w < kernel_copy_stencil.dim(1).extent(); w++) {
  for (int z = 0; z < kernel_copy_stencil.dim(0).extent(); z++) {

  //image=z,w,x,y  read=y,x,w,z
    hw_uint<16> in_val;
    set_at<0*16, 16, 16>(in_val, kernel_copy_stencil(z,w,x,y));
    kernel_copy_stencil_stream.write(in_val);
  } } } }

  std::cout << "loaded data\n";
  // run function
  unoptimized_resnet(input_copy_stencil_stream, kernel_copy_stencil_stream, output_stream);
  std::cout << "ran the function\n";
  
  // copy to output
  // image=x,y,w  write=w,y,x
  for (int w = 0; w < output.dim(2).extent(); w++) {
  for (int y = 0; y < output.dim(1).extent(); y++) {
  for (int x = 0; x < output.dim(0).extent(); x++) {
    //std::cout << "read " << w << "x" << y << "x" << x << std::endl;
    hw_uint<16> actual = output_stream.read();
    auto actual_lane_0 = actual.extract<0*16, 15>();
    output(x, y, w) = actual_lane_0;
  } } }
}

template void run_clockwork_program<bool>(Halide::Runtime::Buffer<bool> input_copy_stencil,
                                          Halide::Runtime::Buffer<bool> kernel_copy_stencil,
                                          Halide::Runtime::Buffer<bool> output);

template void run_clockwork_program<uint8_t>(Halide::Runtime::Buffer<uint8_t> input_copy_stencil,
                                             Halide::Runtime::Buffer<uint8_t> kernel_copy_stencil,
                                             Halide::Runtime::Buffer<uint8_t> output);

template void run_clockwork_program<int8_t>(Halide::Runtime::Buffer<int8_t> input_copy_stencil,
                                            Halide::Runtime::Buffer<int8_t> kernel_copy_stencil,
                                            Halide::Runtime::Buffer<int8_t> output);

template void run_clockwork_program<uint16_t>(Halide::Runtime::Buffer<uint16_t> input_copy_stencil,
                                              Halide::Runtime::Buffer<uint16_t> kernel_copy_stencil,
                                              Halide::Runtime::Buffer<uint16_t> output);

template void run_clockwork_program<int16_t>(Halide::Runtime::Buffer<int16_t> input_copy_stencil,
                                             Halide::Runtime::Buffer<int16_t> kernel_copy_stencil,
                                             Halide::Runtime::Buffer<int16_t> output);

template void run_clockwork_program<uint32_t>(Halide::Runtime::Buffer<uint32_t> input_copy_stencil,
                                              Halide::Runtime::Buffer<uint32_t> kernel_copy_stencil,
                                              Halide::Runtime::Buffer<uint32_t> output);

template void run_clockwork_program<int32_t>(Halide::Runtime::Buffer<int32_t> input_copy_stencil,
                                             Halide::Runtime::Buffer<int32_t> kernel_copy_stencil,
                                             Halide::Runtime::Buffer<int32_t> output);

template void run_clockwork_program<float>(Halide::Runtime::Buffer<float> input_copy_stencil,
                                           Halide::Runtime::Buffer<float> kernel_copy_stencil,
                                           Halide::Runtime::Buffer<float> output);

