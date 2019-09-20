add_executable(strided_conv_process process.cpp)
halide_use_image_io(strided_conv_process)

halide_generator(strided_conv.generator SRCS strided_conv_generator.cpp)

set(LIB strided_conv)
halide_library_from_generator(${LIB}
  GENERATOR strided_conv.generator)

target_link_libraries(strided_conv_process PRIVATE ${LIB})
