cmake_minimum_required(VERSION 3.18)

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

project(cliffordsim)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message("Build type: " ${CMAKE_BUILD_TYPE})
message("Compiler: " ${CMAKE_CXX_COMPILER})

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wno-unused-function -fopenmp -DDEBUG -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wno-unused-function -fopenmp -fPIC")

# Ignore annoying Intel warnings
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -diag-disable=2196")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -diag-disable=2196")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(UTIL_LIBS)

if (SERIAL)
    message("Building for serial execution")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fopenmp -DSERIAL")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fopenmp -DSERIAL")
endif()

include(FetchContent)

# Fetching externals
set(EXTERNALS "")

FetchContent_Declare(
    nlohmann
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG master
    GIT_SHALLOW TRUE
)
list(APPEND EXTERNALS nlohmann)

FetchContent_Declare(
    bspl
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool
    GIT_TAG master
    GIT_SHALLOW TRUE
)
list(APPEND EXTERNALS bspl)

if (DEFINED DATAFRAME_SOURCE)
    set(dataframe_SOURCE_DIR ${DATAFRAME_SOURCE})
else()
    FetchContent_Declare(
        dataframe
        GIT_REPOSITORY https://github.com/eliotheinrich/dataframe
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    list(APPEND EXTERNALS dataframe)
endif()

FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(EIGEN_BUILD_DOC OFF)
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_PKGCONFIG OFF)
list(APPEND EXTERNALS eigen)

FetchContent_MakeAvailable(${EXTERNALS})

include_directories(
    ${nlohmann_SOURCE_DIR}/include 
    ${bspl_SOURCE_DIR}/include 
    ${dataframe_SOURCE_DIR}/src
    ${eigen_SOURCE_DIR}
)

add_library(simulator 
    src/Entropy.hpp
    src/Graph.cpp
)
target_link_libraries(simulator -pthread)
set_target_properties(simulator PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

set(CLIFFORD_DIR ${CMAKE_SOURCE_DIR}/src/CliffordState)
set(QUANTUMCIRCUIT_DIR ${CMAKE_SOURCE_DIR}/src/QuantumCircuit)

set(VQSE_DIR ${CMAKE_SOURCE_DIR}/src/Models/VQSE)
set(QA_DIR ${CMAKE_SOURCE_DIR}/src/Models/QuantumAutomaton)
set(RC_DIR ${CMAKE_SOURCE_DIR}/src/Models/RandomClifford)
set(MC_DIR ${CMAKE_SOURCE_DIR}/src/Models/MinCut)
set(BS_DIR ${CMAKE_SOURCE_DIR}/src/Models/BlockSim)
set(SOC_DIR ${CMAKE_SOURCE_DIR}/src/Models/SelfOrganized)
set(GS_DIR ${CMAKE_SOURCE_DIR}/src/Models/GraphSim)
set(PS_DIR ${CMAKE_SOURCE_DIR}/src/Models/PostSelection)
set(BW_DIR ${CMAKE_SOURCE_DIR}/src/Models/BrickworkCircuit)
set(PARTNER_DIR ${CMAKE_SOURCE_DIR}/src/Models/PartneringSim)
set(GROVERSAT_DIR ${CMAKE_SOURCE_DIR}/src/Models/GroverSAT)
set(PHASELESS_DIR ${CMAKE_SOURCE_DIR}/src/Models/Phaseless)

set(MODEL_DIRS ${CLIFFORD_DIR} ${QUANTUMCIRCUIT_DIR} ${VQSE_DIR} ${QA_DIR} ${RC_DIR} ${MC_DIR} ${BS_DIR} ${SOC_DIR} ${GS_DIR} ${PS_DIR} ${BW_DIR} ${PARTNER_DIR} ${GROVERSAT_DIR} ${PHASELESS_DIR})
set(SIMULATOR_LIBS clifford_state quantum_circuit quantum_automaton random_clifford mincut blocksim self_organized graphsim post_selection brickwork_circuit vqse partnersim groversat phaseless)

foreach(DIR IN LISTS MODEL_DIRS) 
    add_subdirectory(${DIR})
endforeach()

foreach(LIB IN LISTS SIMULATOR_LIBS)
    set_target_properties(${LIB} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endforeach()

message("CXX Release Flags: " ${CMAKE_CXX_FLAGS_RELEASE})

add_library(sim_utils src/utils.cpp)
target_include_directories(sim_utils PRIVATE ${MODEL_DIRS})
target_link_libraries(sim_utils PRIVATE ${SIMULATOR_LIBS})

if (BUILDING_PYTHON)
    # Python wrappers
    find_package(Python 3.10 REQUIRED)

    # Finding nanobind
    execute_process(
    COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
    find_package(nanobind CONFIG REQUIRED)

    nanobind_add_module(pysimulators src/PySimulators.cpp)
    target_include_directories(pysimulators PRIVATE src ${MODEL_DIRS})
    target_link_libraries(pysimulators PRIVATE sim_utils ${SIMULATOR_LIBS})

    install(
        TARGETS pysimulators
        LIBRARY DESTINATION pysims
    )
    
else()
    # --- MAIN --- #
    add_executable(main src/main.cpp)
    target_include_directories(main PRIVATE 
        src 
        ${MODEL_DIRS}
    )
    set_target_properties(main PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(main PRIVATE ${SIMULATOR_LIBS} ${UTIL_LIBS} ${ITENSOR_LINK_FLAGS})

    # --- TEST --- #
    add_executable(test src/test.cpp)
    target_include_directories(test PRIVATE
        src
        ${MODEL_DIRS}
    )

    set_target_properties(test PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(test PRIVATE ${SIMULATOR_LIBS})
endif()