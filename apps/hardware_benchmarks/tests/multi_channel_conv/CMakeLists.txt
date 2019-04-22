add_executable(multi_channel_conv_process process.cpp)
halide_use_image_io(multi_channel_conv_process)

halide_generator(multi_channel_conv.generator SRCS multi_channel_conv_generator.cpp)

set(LIB multi_channel_conv)
halide_library_from_generator(${LIB}
  GENERATOR multi_channel_conv.generator)

target_link_libraries(multi_channel_conv_process PRIVATE ${LIB})
