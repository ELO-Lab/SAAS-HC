cmake_minimum_required(VERSION 3.0)

project(ACO++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -w ")

include_directories(./include)
include_directories($ENV{CONDA_PREFIX}/include/eigen3)

file(GLOB ACOPP_SRC "./*.cpp")
add_executable(acothop ${ACOPP_SRC})

target_link_libraries(acothop $ENV{CONDA_PREFIX}/lib/libcmaes.so)
