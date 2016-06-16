cmake_minimum_required(VERSION 3.0)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
   message(FATAL_ERROR "You don't want to configure in the source directory!")
endif()
project(BenchmarkVc)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

find_package(Vc REQUIRED)
find_library(GGBENCH_LIBRARIES benchmark)
find_path(GGBENCH_INCLUDE_DIR benchmark/benchmark.h)
if(NOT GGBENCH_LIBRARIES OR NOT GGBENCH_INCLUDE_DIR)
   message(FATAL_ERROR "Required Google Benchmark library not found (https://github.com/google/benchmark)")
endif()

CHECK_CXX_SOURCE_COMPILES("#include <cxxabi.h>
int main() { return 0; }" cxx_abi_header_works)
if(cxx_abi_header_works)
   add_definitions(-DHAVE_CXX_ABI_H)
endif()
add_definitions(${Vc_DEFINITIONS})

MACRO(add_benchmark title)
    add_executable(${title} ${title}.cpp)
    target_include_directories(${title} PRIVATE "${Vc_INCLUDE_DIR};${GGBENCH_INCLUDE_DIR}")
    target_compile_options(${title} PRIVATE "-std=c++11;${Vc_COMPILE_FLAGS};${Vc_ARCHITECTURE_FLAGS}")
    set_target_properties(${title} PROPERTIES LINK_FLAGS -pthread)
    target_link_libraries(${title} ${Vc_LIBRARIES} ${GGBENCH_LIBRARIES})
    add_custom_target(run_${title} ${title} DEPENDS ${title} COMMENT "Execute ${title} benchmark" VERBATIM)
endmacro()

add_benchmark(arithmetics)
add_benchmark(memorylayout)
add_benchmark(loopunroll)
add_benchmark(quadratic)
