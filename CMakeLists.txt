cmake_minimum_required(VERSION 3.11.4)
project(FreeFermion)

set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(EIGEN_DIR $ENV{EIGEN_DIR})
set(CTPL_DIR $ENV{CTPL_DIR})
set(DF_DIR $ENV{DF_DIR})
set(NLOHMANN_DIR $ENV{NLOHMANN_DIR})

add_executable(test main.cpp FreeFermionSimulator.cpp FreeFermionSimulator.h FreeFermionConfig.h FreeFermionConfig.cpp Simulator.hpp)
target_include_directories(test PRIVATE ${EIGEN_DIR} ${DF_DIR} ${NLOHMANN_DIR})