cmake_minimum_required(VERSION 3.16)
project(sfc_layouts CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-O3 -march=native -Wall -Wextra)
include_directories(include src sim)

enable_testing()

add_executable(test_layouts tests/test_layouts.cpp)
add_test(NAME test_layouts COMMAND test_layouts)

add_executable(test_kernels tests/test_kernels.cpp)
add_test(NAME test_kernels COMMAND test_kernels)

add_executable(test_sim tests/test_sim.cpp)
add_test(NAME test_sim COMMAND test_sim)

add_executable(run_tlb sim/run_tlb.cpp)

add_executable(bench_stencil bench/bench_stencil.cpp)
