cmake_minimum_required(VERSION 3.11.4)
project(FreeFermion)

set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(EIGEN_DIR $ENV{EIGEN_DIR})
set(CTPL_DIR $ENV{CTPL_DIR})
set(DF_DIR $ENV{DF_DIR})
set(NLOHMANN_DIR $ENV{NLOHMANN_DIR})

add_library(simulator 
    src/Simulator.hpp
)
target_include_directories(simulator PUBLIC ${CTPL_DIR} ${DF_DIR})
target_link_libraries(simulator -pthread)
set_target_properties(simulator PROPERTIES LINKER_LANGUAGE CXX)

set(CLIFFORD_DIR ${CMAKE_SOURCE_DIR}/src/CliffordState)
add_subdirectory(${CLIFFORD_DIR})
set(QA_DIR ${CMAKE_SOURCE_DIR}/src/QuantumAutomaton)
add_subdirectory(${QA_DIR})
set(RC_DIR ${CMAKE_SOURCE_DIR}/src/RandomClifford)
add_subdirectory(${RC_DIR})
set(FF_DIR ${CMAKE_SOURCE_DIR}/src/FreeFermion)
add_subdirectory(${FF_DIR})
set(MC_DIR ${CMAKE_SOURCE_DIR}/src/MinCut)
add_subdirectory(${MC_DIR})

add_executable(test src/main.cpp)
target_include_directories(test PRIVATE 
    src 
    ${CLIFFORD_DIR}
    ${QA_DIR}
    ${RC_DIR}
    ${MC_DIR}
    ${DF_DIR} 
    ${NLOHMANN_DIR}
)
target_link_libraries(test PRIVATE quantum_automaton random_clifford mincut)