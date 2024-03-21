cmake_minimum_required(VERSION 3.0)
project(parallel)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP)

add_executable(parallel main.cpp
        kmp.cpp
        util.cpp)

if (OpenMP_CXX_FOUND)
    target_link_libraries(parallel PUBLIC OpenMP::OpenMP_CXX)
endif ()
