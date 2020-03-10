add_executable(conv_7_7_process process.cpp)
halide_use_image_io(conv_7_7_process)

halide_generator(conv_7_7.generator SRCS conv_7_7_generator.cpp)

set(LIB conv_7_7)
halide_library_from_generator(${LIB}
  GENERATOR conv_7_7.generator)

target_link_libraries(conv_7_7_process PRIVATE ${LIB})
