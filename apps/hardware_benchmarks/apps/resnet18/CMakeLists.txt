add_executable(conv_3_3_process process.cpp)
halide_use_image_io(conv_3_3_process)

halide_generator(conv_3_3.generator SRCS conv_3_3_generator.cpp)

set(LIB conv_3_3)
halide_library_from_generator(${LIB}
  GENERATOR conv_3_3.generator)

target_link_libraries(conv_3_3_process PRIVATE ${LIB})
