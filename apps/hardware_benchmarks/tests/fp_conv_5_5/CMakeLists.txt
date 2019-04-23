add_executable(conv_5_5_process process.cpp)
halide_use_image_io(conv_5_5_process)

halide_generator(conv_5_5.generator SRCS conv_5_5_generator.cpp)

set(LIB conv_5_5)
halide_library_from_generator(${LIB}
  GENERATOR conv_5_5.generator)

target_link_libraries(conv_5_5_process PRIVATE ${LIB})
