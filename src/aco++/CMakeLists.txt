cmake_minimum_required(VERSION 3.0)

project(ACO++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -w")

set(CMAKE_C_COMPILER $ENV{CONDA_PREFIX}/bin/clang)
set(CMAKE_CXX_COMPILER $ENV{CONDA_PREFIX}/bin/clang++)

include_directories($ENV{CONDA_PREFIX}/include/eigen3)
include_directories(./include)

file(GLOB ACOPP_SRC "./*.cpp")
file(GLOB LIBCMAES_SRC "./libcmaes-0.10/src/*.cc")
add_executable(acothop ${LIBCMAES_SRC} ${ACOPP_SRC})
