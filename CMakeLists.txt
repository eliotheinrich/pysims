cmake_minimum_required(VERSION 3.12)
project(cliffordsim)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})


set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wno-unused-function -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wno-unused-function")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(UTIL_LIBS)

# Add OpenMPI if needed
if (OMPI)
    message("OMPI required")
    find_package(MPI REQUIRED)
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fopenmp -DOMPI")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fopenmp -DOMPI")
    list(APPEND UTIL_LIBS ${MPI_CXX_LIBRARIES})
endif()

set(EIGEN_DIR $ENV{EIGEN_DIR})
set(DF_DIR $ENV{DF_DIR})
set(NLOHMANN_DIR $ENV{NLOHMANN_DIR})

set(DF_DEPENDENCIES ${DF_DIR} ${NLOHMANN_DIR} ${EIGEN_DIR})

# Add BSPL if needed
if (NOT OMPI)
    set(BSPL_DIR $ENV{BSPL_DIR})
    list(APPEND DF_DEPENDENCIES ${BSPL_DIR})
endif()

add_library(simulator 
    src/Entropy.hpp
    src/Graph.cpp
)
target_include_directories(simulator PUBLIC ${DF_DEPENDENCIES})
target_link_libraries(simulator -pthread)
set_target_properties(simulator PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

set(CLIFFORD_DIR ${CMAKE_SOURCE_DIR}/src/CliffordState)
set(QUANTUMSTATE_DIR ${CMAKE_SOURCE_DIR}/src/QuantumState)
set(QA_DIR ${CMAKE_SOURCE_DIR}/src/QuantumAutomaton)
set(RC_DIR ${CMAKE_SOURCE_DIR}/src/RandomClifford)
set(FF_DIR ${CMAKE_SOURCE_DIR}/src/FreeFermion)
set(MC_DIR ${CMAKE_SOURCE_DIR}/src/MinCut)
set(BS_DIR ${CMAKE_SOURCE_DIR}/src/BlockSim)
set(SOC_DIR ${CMAKE_SOURCE_DIR}/src/SelfOrganized)
set(GS_DIR ${CMAKE_SOURCE_DIR}/src/GraphSim)
set(AC_DIR ${CMAKE_SOURCE_DIR}/src/ArithmeticClifford)
set(AC_DIR ${CMAKE_SOURCE_DIR}/src/PostSelection)

set(MODEL_DIRS ${CLIFFORD_DIR} ${QUANTUMSTATE_DIR} ${QA_DIR} ${RC_DIR} ${FF_DIR} ${MC_DIR} ${BS_DIR} ${SOC_DIR} ${GS_DIR} ${AC_DIR})
set(SIMULATOR_LIBS quantum_automaton random_clifford mincut blocksim self_organized graphsim post_selection)

foreach(DIR IN LISTS MODEL_DIRS) 
    add_subdirectory(${DIR})
endforeach()

foreach(LIB IN LISTS SIMULATOR_LIBS)
    set_target_properties(${LIB} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endforeach()

# --- MAIN --- #
add_executable(main src/main.cpp)
target_include_directories(main PRIVATE 
    src 
    ${DF_DEPENDENCIES}
    ${MODEL_DIRS}
)
set_target_properties(main PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(main PRIVATE ${SIMULATOR_LIBS} ${UTIL_LIBS})

# --- TEST --- #
add_executable(test src/test.cpp)
target_include_directories(test PRIVATE
    src
    ${DF_DEPENDENCIES}
    ${MODEL_DIRS}
)
set_target_properties(test PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(test PRIVATE ${SIMULATOR_LIBS} ${UTIL_LIBS})

# --- SIMPLE TEST --- #
add_executable(simple_test src/simpletest.cpp)
target_include_directories(simple_test PRIVATE
    src
    ${DF_DEPENDENCIES}
)
set_target_properties(simple_test PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(simple_test PRIVATE ${UTIL_LIBS})
