add_executable(down_sample_process process.cpp)
halide_use_image_io(down_sample_process)

halide_generator(down_sample.generator SRCS down_sample_generator.cpp)

set(LIB down_sample)
halide_library_from_generator(${LIB}
  GENERATOR down_sample.generator)

target_link_libraries(down_sample_process PRIVATE ${LIB})
