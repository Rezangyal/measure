cmake_minimum_required(VERSION 3.13)

project(measure_test LANGUAGES CXX)

add_executable(${PROJECT_NAME} measure_test.cpp measure_samples.cpp measure_test.h)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()

target_link_libraries(${PROJECT_NAME} measure_lib)
