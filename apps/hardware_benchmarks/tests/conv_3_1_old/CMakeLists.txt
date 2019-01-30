add_executable(conv_3_1_process process.cpp)
halide_use_image_io(conv_3_1_process)

halide_generator(conv_3_1.generator SRCS conv_3_1_generator.cpp)

set(LIB conv_3_1)
halide_library_from_generator(${LIB}
  GENERATOR conv_3_1.generator)

target_link_libraries(conv_3_1_process PRIVATE ${LIB})
