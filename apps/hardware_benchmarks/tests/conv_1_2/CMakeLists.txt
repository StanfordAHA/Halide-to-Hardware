add_executable(conv_1_2_process process.cpp)
halide_use_image_io(conv_1_2_process)

halide_generator(conv_1_2.generator SRCS conv_1_2_generator.cpp)

set(LIB conv_1_2)
halide_library_from_generator(${LIB}
  GENERATOR conv_1_2.generator)

target_link_libraries(conv_1_2_process PRIVATE ${LIB})
