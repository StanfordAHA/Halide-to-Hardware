add_executable(up_sample_process process.cpp)
halide_use_image_io(up_sample_process)

halide_generator(up_sample.generator SRCS up_sample_generator.cpp)

set(LIB up_sample)
halide_library_from_generator(${LIB}
  GENERATOR up_sample.generator)

target_link_libraries(up_sample_process PRIVATE ${LIB})
