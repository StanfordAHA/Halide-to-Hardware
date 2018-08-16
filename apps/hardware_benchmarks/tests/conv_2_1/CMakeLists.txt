add_executable(conv_2_1_process process.cpp)
halide_use_image_io(conv_2_1_process)

halide_generator(conv_2_1.generator SRCS conv_2_1_generator.cpp)

set(LIB conv_2_1)
halide_library_from_generator(${LIB}
  GENERATOR conv_2_1.generator)

target_link_libraries(conv_2_1_process PRIVATE ${LIB})
