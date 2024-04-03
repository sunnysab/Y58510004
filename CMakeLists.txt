cmake_minimum_required(VERSION 3.0)
project(parallel)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenMP)
include_directories(".")

add_executable(parallel main.cpp
        memory.cpp
        kmp.cpp
        simd_search.cpp
        util.cpp)
target_compile_options(parallel PRIVATE "-march=native")

if (OpenMP_CXX_FOUND)
    target_link_libraries(parallel PUBLIC OpenMP::OpenMP_CXX)
endif ()


add_executable(test_kmp test/test_kmp.cpp kmp.cpp)
target_link_libraries(test_kmp PUBLIC gtest_main gtest)

add_executable(test_simd_search test/test_simd.cpp simd_search.cpp)
target_link_libraries(test_simd_search PUBLIC gtest_main gtest)
target_compile_options(test_simd_search PRIVATE "-march=native")