add_executable(unet_process process.cpp)
halide_use_image_io(unet_process)

halide_generator(unet.generator SRCS unet_generator.cpp)

set(LIB unet)
halide_library_from_generator(${LIB}
  GENERATOR unet.generator)

target_link_libraries(unet_process PRIVATE ${LIB})
