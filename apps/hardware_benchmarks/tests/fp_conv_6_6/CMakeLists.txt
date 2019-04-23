add_executable(conv_6_6_process process.cpp)
halide_use_image_io(conv_6_6_process)

halide_generator(conv_6_6.generator SRCS conv_6_6_generator.cpp)

set(LIB conv_6_6)
halide_library_from_generator(${LIB}
  GENERATOR conv_6_6.generator)

target_link_libraries(conv_6_6_process PRIVATE ${LIB})
