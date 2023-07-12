cmake_minimum_required(VERSION 3.1)

project(ACO++)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OMP_NUM_THREADS 1)
set(ENV{OMP_NUM_THREADS} 1)

# add_compile_options(-O3)
# add_compile_options(-w)
set(CMAKE_C_COMPILER $ENV{CONDA_PREFIX}/bin/gcc)
set(CMAKE_CXX_COMPILER $ENV{CONDA_PREFIX}/bin/g++)

# include_directories($ENV{CONDA_PREFIX}/include/eigen3)
# include_directories(./include)
include_directories($ENV{CONDA_PREFIX}/include)

file(GLOB ACOPP_SRC "./*.cpp")
# file(GLOB LIBCMAES_SRC "./libcmaes-0.10/src/*.cc")

add_executable(acothop ${ACOPP_SRC})
# add_executable(acothop ${ACOPP_SRC} ${LIBCMAES_SRC})

target_link_libraries(acothop $ENV{CONDA_PREFIX}/lib/libarmadillo.so )
# target_link_libraries(acothop $ENV{CONDA_PREFIX}/lib/libcmaes.so )
