cmake_minimum_required(VERSION 3.27)

project(cliffordsim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

message("Build type: " ${CMAKE_BUILD_TYPE})
message("Compiler: " ${CMAKE_CXX_COMPILER})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-all -Wno-unused-function -Wno-deprecated-declarations -pthread")

# Ignore annoying Intel warnings
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -diag-disable=2196")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG -DSERIAL")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")


# -------------------------------- #
# ------ Fetching externals ------ #
# -------------------------------- #

include(FetchContent)

set(EXTERNALS "")

# Python wrappers
find_package(Python 3.10 
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

execute_process(
    COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR
)
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
find_package(nanobind CONFIG REQUIRED)

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

if (DEFINED UTILS_SOURCE)
    set(utils_SOURCE_DIR ${UTILS_SOURCE})
    add_subdirectory(${UTILS_SOURCE} "utils_src")
else()
    FetchContent_Declare(
        utils
        GIT_REPOSITORY https://github.com/eliotheinrich/utils.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    list(APPEND EXTERNALS utils)
endif()

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG d907f9cfb7197a29fa7cbefab310895593f5b105
)
list(APPEND EXTERNALS glaze)

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
    ${dataframe_SOURCE_DIR}/src
    ${utils_SOURCE_DIR}/src
    ${glaze_SOURCE_DIR}/include
    ${eigen_SOURCE_DIR}
)


set(VQSE_DIR ${CMAKE_SOURCE_DIR}/src/Models/VQSE)
set(XZCIRCUIT_DIR ${CMAKE_SOURCE_DIR}/src/Models/XZCircuit)
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
set(NETWORK_CLIFFORD_DIR ${CMAKE_SOURCE_DIR}/src/Models/Network)
set(ENV_SIM_DIR ${CMAKE_SOURCE_DIR}/src/Models/EnvironmentSim)
set(BULK_SIM_DIR ${CMAKE_SOURCE_DIR}/src/Models/BulkMeasurement)
set(RCS_DIR ${CMAKE_SOURCE_DIR}/src/Models/RandomCircuitSampling)
set(RH_DIR ${CMAKE_SOURCE_DIR}/src/Models/RandomHamiltonian)
set(HQ_DIR ${CMAKE_SOURCE_DIR}/src/Models/HQAmplitude)
set(CLIFFORD_CLUSTERING_DIR ${CMAKE_SOURCE_DIR}/src/Models/CliffordClustering)

set(MODEL_DIRS ${CLIFFORD_CLUSTERING_DIR} ${RH_DIR} ${RCS_DIR} ${VQSE_DIR} ${XZCIRCUIT_DIR} ${QA_DIR} ${RC_DIR} ${MC_DIR} ${BS_DIR} ${SOC_DIR} ${GS_DIR} ${PS_DIR} ${BW_DIR} ${PARTNER_DIR} ${GROVERSAT_DIR} ${PHASELESS_DIR} ${NETWORK_CLIFFORD_DIR} ${ENV_SIM_DIR} ${BULK_SIM_DIR} ${HQ_DIR})
set(SIMULATOR_LIBS quantum_automaton random_clifford random_hamiltonian random_circuit_sampling mincut blocksim self_organized graphsim post_selection brickwork_circuit partnersim groversat phaseless network_clifford env_sim bulk_sim)

foreach(DIR IN LISTS MODEL_DIRS) 
    add_subdirectory(${DIR})
endforeach()

if (PS_BUILDING_PYTHON)
    nanobind_add_module(
        pysimulators 
        STABLE_ABI
        NB_STATIC
        src/PySimulators.cpp
    )
    set_target_properties(pysimulators PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(pysimulators PRIVATE src ${MODEL_DIRS})
    target_link_libraries(pysimulators PRIVATE ${SIMULATOR_LIBS} dataframe)

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

    set(CMAKE_DEBUG_POSTFIX "d")
    set_target_properties(main PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(main PUBLIC ${SIMULATOR_LIBS} dataframe)

    # --- TEST --- #
    add_executable(test_sim src/test.cpp)
    target_link_libraries(test_sim ${Python_LIBRARIES})
    target_include_directories(test_sim PRIVATE
        src
        ${MODEL_DIRS}
        ${NB_DIR}/include
    )

    set_target_properties(test_sim PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(test_sim PUBLIC quantum_state clifford_state ${SIMULATOR_LIBS} dataframe)
endif()
