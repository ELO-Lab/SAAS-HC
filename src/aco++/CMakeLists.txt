cmake_minimum_required(VERSION 3.0)
project(ACO++)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -w" )

add_executable(acothop thop.cpp utilities.cpp ants.cpp inout.cpp unix_timer.cpp ls.cpp parse.cpp acothop.cpp )