add_executable(harris_process process.cpp)
halide_use_image_io(harris_process)

halide_generator(harris.generator SRCS harris_generator.cpp)

set(LIB harris)
halide_library_from_generator(${LIB}
  GENERATOR harris.generator)

target_link_libraries(harris_process PRIVATE ${LIB})
