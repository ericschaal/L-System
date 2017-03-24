cmake_minimum_required(VERSION 3.6)
project(Ass4)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -Wall -Wextra")
endif()

set(SOURCE_FILES main.cpp)
add_executable(Ass4 ${SOURCE_FILES})