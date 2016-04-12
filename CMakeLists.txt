cmake_minimum_required(VERSION 3.0)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
   message(FATAL_ERROR "You don't want to configure in the source directory!")
endif()

project(BenchmarkVc)
find_package(Vc REQUIRED)
find_library(GGBENCH_LIBRARIES benchmark)
find_path(GGBENCH_INCLUDE_DIR benchmark)
if(NOT GGBENCH_LIBRARIES OR NOT GGBENCH_INCLUDE_DIR)
   message(FATAL_ERROR "Required Google Benchmark library not found (https://github.com/google/benchmark)")
endif()

add_executable(benchmark main.cpp)
target_include_directories(benchmark PRIVATE "${Vc_INCLUDE_DIR};${GGBENCH_INCLUDE_DIR}")
target_compile_options(benchmark PRIVATE "-std=c++11;${Vc_COMPILE_FLAGS};${Vc_ARCHITECTURE_FLAGS}")
set_target_properties(benchmark PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(benchmark ${Vc_LIBRARIES} ${GGBENCH_LIBRARIES})
