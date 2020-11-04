cmake_minimum_required(VERSION 3.16)
project(wgs)

enable_testing()

# Set up language settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Find Halide
find_package(Halide REQUIRED)
find_package(OpenMP)

# Generator
add_executable(wgs.generator halide_wgs_generator.cpp)
target_link_libraries(wgs.generator PRIVATE Halide::Generator)

# Filters
add_halide_library(halide_wgs FROM wgs.generator)

# Main executable
add_executable(wgs_test test.cpp)
target_compile_options(wgs_test PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O2>)
target_link_libraries(wgs_test
                      PRIVATE
                      Halide::Tools
                      halide_wgs
                      $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>)

# Test that the app actually works!
add_test(NAME wgs_app COMMAND wgs_test)
set_tests_properties(wgs_app PROPERTIES
                     LABELS internal_app_tests
                     PASS_REGULAR_EXPRESSION "Success!"
                     SKIP_REGULAR_EXPRESSION "\\[SKIP\\]")
