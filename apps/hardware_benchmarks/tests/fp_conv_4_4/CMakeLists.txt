add_executable(conv_4_4_process process.cpp)
halide_use_image_io(conv_4_4_process)

halide_generator(conv_4_4.generator SRCS conv_4_4_generator.cpp)

set(LIB conv_4_4)
halide_library_from_generator(${LIB}
  GENERATOR conv_4_4.generator)

target_link_libraries(conv_4_4_process PRIVATE ${LIB})
