add_executable(zircon_nop_process process.cpp)
halide_use_image_io(zircon_nop_process)

halide_generator(zircon_nop.generator SRCS zircon_nop_generator.cpp)

set(LIB zircon_nop)
halide_library_from_generator(${LIB}
  GENERATOR zircon_nop.generator)

target_link_libraries(zircon_nop_process PRIVATE ${LIB})
