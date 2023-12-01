cmake_minimum_required(VERSION 3.15..3.26)

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

project(cliffordsim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})
message("Compiler: " ${CMAKE_CXX_COMPILER})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-deprecated-declarations -pthread -fopenmp")

# Ignore annoying Intel warnings
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -diag-disable=2196")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG -DSERIAL")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")


# ---------------------------------- #
# -- Manually installed libraries -- #
# ---------------------------------- #

set(PFFFT_DIR $ENV{PFFFT_DIR})
find_library(PFFFT_LIB pffft HINTS ${PFFFT_DIR}/lib)
add_compile_definitions(PFFFT_ENABLE_DOUBLE)

#set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(BLAS_DIR $ENV{BLAS_PATH})
set(LAPACK_DIR $ENV{LAPACK_PATH})
set(LAPACKBLAS gfortran lapack openblas)
set(LAPACKBLAS_DIR ${LAPACK_DIR} ${BLAS_DIR})
link_directories(${LAPACKBLAS_DIR})

set(ITENSOR_DIR $ENV{ITENSOR_DIR})
add_library(itensor STATIC IMPORTED)
set_target_properties(itensor PROPERTIES IMPORTED_LOCATION ${ITENSOR_DIR}/lib/libitensor.a)

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


#if (DEFINED DATAFRAME_SOURCE)
#    set(dataframe_SOURCE_DIR ${DATAFRAME_SOURCE})
#else()
    FetchContent_Declare(
        dataframe
        GIT_REPOSITORY https://github.com/eliotheinrich/dataframe
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    list(APPEND EXTERNALS dataframe)
#endif()

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
    ${PFFFT_DIR}
)

message("dataframe found: ${dataframe_FOUND}")
message("dataframe_SOURCE_DIR = ${dataframe_SOURCE_DIR}")
message(${dataframe_SOURCE_DIR})


add_library(graph
    src/Graph.cpp
)

set(SAMPLERS_DIR ${CMAKE_SOURCE_DIR}/src/Samplers)
add_subdirectory(${SAMPLERS_DIR})

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
set(NETWORK_CLIFFORD_DIR ${CMAKE_SOURCE_DIR}/src/Models/Network)
set(ENV_SIM_DIR ${CMAKE_SOURCE_DIR}/src/Models/EnvironmentSim)
set(RCS_DIR ${CMAKE_SOURCE_DIR}/src/Models/RandomCircuitSampling)

set(MODEL_DIRS ${CLIFFORD_DIR} ${QUANTUMCIRCUIT_DIR} ${RCS_DIR} ${VQSE_DIR} ${QA_DIR} ${RC_DIR} ${MC_DIR} ${BS_DIR} ${SOC_DIR} ${GS_DIR} ${PS_DIR} ${BW_DIR} ${PARTNER_DIR} ${GROVERSAT_DIR} ${PHASELESS_DIR} ${NETWORK_CLIFFORD_DIR} ${ENV_SIM_DIR})
set(SIMULATOR_LIBS clifford_state quantum_circuit quantum_automaton random_clifford random_circuit_sampling mincut blocksim self_organized graphsim post_selection brickwork_circuit vqse partnersim groversat phaseless network_clifford env_sim)

foreach(DIR IN LISTS MODEL_DIRS) 
    add_subdirectory(${DIR})
endforeach()

foreach(LIB IN LISTS SIMLATOR_LIBS)
    target_link_libraries(LIB samplers)
    target_include_directories(LIB PUBLIC ${SAMPLERS_DIR})
endforeach()



#foreach(LIB IN LISTS SIMULATOR_LIBS)
#    set_target_properties(${LIB} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
#endforeach()

add_library(sim_utils STATIC src/utils.cpp)
target_include_directories(sim_utils PRIVATE ${MODEL_DIRS})
# IF LINKER ERROR APPEARS IN MPS SIMULATION, LINK TO itensor HERE!!
target_link_libraries(sim_utils PRIVATE ${LAPACKBLAS} ${SIMULATOR_LIBS})


if (PS_BUILDING_PYTHON)
    nanobind_add_module(
        pysimulators 
        STABLE_ABI
        NB_STATIC
        src/PySimulators.cpp
    )
    set_target_properties(pysimulators PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(pysimulators PRIVATE src ${MODEL_DIRS})
    target_link_libraries(pysimulators PRIVATE blas lapack itensor ${SIMULATOR_LIBS})

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
    target_link_libraries(main PUBLIC ${LAPACKBLAS} itensor ${SIMULATOR_LIBS})

    # --- TEST --- #
    add_executable(test src/test.cpp)
    target_include_directories(test PRIVATE
        src
        ${MODEL_DIRS}
        src/Samplers
    )

    set_target_properties(test PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(test PUBLIC ${LAPACKBLAS} itensor ${SIMULATOR_LIBS})
endif()