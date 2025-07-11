cmake_minimum_required(VERSION 3.17)

project(measure_lib LANGUAGES CXX)

add_library(${PROJECT_NAME} 
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/measure/cpp_measure.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/measure/measure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/measure/measure_base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/measure/measure_utils.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/measure/qpc_measure.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/measure/rdtsc_measure.h 
    )

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)

if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(${PROJECT_NAME} INTERFACE "/Zc:__cplusplus")
endif()
