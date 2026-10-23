add_executable(conv_3_3_u8_process process.cpp)
halide_use_image_io(conv_3_3_u8_process)

halide_generator(conv_3_3_u8.generator SRCS conv_3_3_u8_generator.cpp)

set(LIB conv_3_3_u8)
halide_library_from_generator(${LIB}
  GENERATOR conv_3_3_u8.generator)

target_link_libraries(conv_3_3_u8_process PRIVATE ${LIB})
