cmake_minimum_required(VERSION 3.13)

project(measure LANGUAGES CXX)

add_subdirectory(measure_lib)
add_subdirectory(measure_test)

enable_testing()
add_test (
  NAME measure_test
  COMMAND $<TARGET_FILE:measure_test>
)

if (MSVC)
    SET_PROPERTY(DIRECTORY PROPERTY VS_STARTUP_PROJECT measure_test)
endif()