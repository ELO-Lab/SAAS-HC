cmake_minimum_required(VERSION 3.0)

project(ACO++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -w")

file(GLOB ACOPP_SRC "*.cpp")

add_executable(acothop ${ACOPP_SRC})
