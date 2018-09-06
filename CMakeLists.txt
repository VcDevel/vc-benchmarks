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

option(USE_INSTALLED_Vc "Search for a Vc installation to build the benchmarks against. Otherwise requires a path to the Vc source directory." OFF)
if(USE_INSTALLED_Vc)
   find_package(Vc 1.0 REQUIRED)
else()
   find_path(Vc_SRC_DIR cmake/VcMacros.cmake
      DOC "Path of the Vc source to benchmark"
      NO_DEFAULT_PATH)
   if(NOT Vc_SRC_DIR)
      message(FATAL_ERROR "You need to specify the src directory of Vc via `-DVc_SRC_DIR=<path to Vc>`")
   endif()

   string(REGEX REPLACE "[][ ():, |!*/]" "" Vc_BUILD_DIR "${Vc_SRC_DIR}")
   set(Vc_BUILD_DIR "${CMAKE_BINARY_DIR}/${Vc_BUILD_DIR}")

   file(MAKE_DIRECTORY "${Vc_BUILD_DIR}")
   execute_process(
      COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${Vc_SRC_DIR}
      WORKING_DIRECTORY ${Vc_BUILD_DIR}
      RESULT_VARIABLE ok)
   if(NOT ok EQUAL 0)
      message(FATAL_ERROR "cmake of Vc failed")
   endif()
   execute_process(
      COMMAND ${CMAKE_COMMAND} --build ${Vc_BUILD_DIR} --target Vc
      WORKING_DIRECTORY ${Vc_BUILD_DIR}
      RESULT_VARIABLE ok)
   if(NOT ok EQUAL 0)
      message(FATAL_ERROR "building libVc failed")
   endif()

   set(Vc_INCLUDE_DIR "${Vc_SRC_DIR}/include/;${Vc_SRC_DIR}")
   find_library(Vc_LIBRARIES Vc
      PATHS "${Vc_BUILD_DIR}"
      NO_DEFAULT_PATH)

   find_path(Vc_CMAKE_MODULES_DIR AddCompilerFlag.cmake
      HINTS "${Vc_SRC_DIR}/cmake")
   list(APPEND CMAKE_MODULE_PATH "${Vc_CMAKE_MODULES_DIR}")
   include("${Vc_CMAKE_MODULES_DIR}/VcMacros.cmake")
   set(Vc_DEFINITIONS)
   set(Vc_COMPILE_FLAGS)
   set(Vc_ARCHITECTURE_FLAGS)
   vc_set_preferred_compiler_flags()
   separate_arguments(Vc_ALL_FLAGS UNIX_COMMAND "${Vc_DEFINITIONS}")
   list(APPEND Vc_ALL_FLAGS ${Vc_COMPILE_FLAGS})
   list(APPEND Vc_ALL_FLAGS ${Vc_ARCHITECTURE_FLAGS})
endif()
find_library(GGBENCH_LIBRARIES benchmark)
find_path(GGBENCH_INCLUDE_DIR benchmark/benchmark.h)
if(NOT GGBENCH_LIBRARIES OR NOT GGBENCH_INCLUDE_DIR)
   message(FATAL_ERROR "Required Google Benchmark library not found (https://github.com/google/benchmark)")
endif()

# -fstack-protector is the default of GCC, but at least Ubuntu changes the default to -fstack-protector-strong, which is crazy
AddCompilerFlag("-fstack-protector")

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
add_benchmark(sincos)
