cmake_minimum_required(VERSION 3.27)

project(cliffordsim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-all -Wno-unused-function -Wno-deprecated-declarations -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")
string(REPLACE ":" ";" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# -------------------------------- #
# ---- Fetching dependencies ----- #
# -------------------------------- #

find_package(fmt REQUIRED)
add_definitions(-DFMT_HEADER_ONLY)

foreach(PATH IN LISTS CMAKE_PREFIX_PATH)
    include_directories("${PATH}/include")
endforeach()

include(FetchContent)

set(EXTERNALS "")

# Python wrappers
find_package(Python 3.10 
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

message("${Python_EXECUTABLE}")

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
  GIT_TAG cc4ce0efe5133c23ec55f07d9e753f7dd1abb0b8
)
list(APPEND EXTERNALS glaze)

FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen
    GIT_TAG 3.4
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
    ${glaze_SOURCE_DIR}/include
    ${dataframe_SOURCE_DIR}/src
    ${utils_SOURCE_DIR}/src
    ${eigen_SOURCE_DIR}
)

# Subdirectories append to MODELS_LIBS and MODELS_INCLUDE
set(MODELS_LIBS)
set(MODELS_INCLUDE)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/RandomClifford)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/QuantumAutomaton)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/VQSE)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/Magic)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/XZCircuit)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/MinCut)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/BlockSim)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/SelfOrganized)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/GraphSim)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/PostSelection)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/BrickworkCircuit)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/PartneringSim)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/GroverSAT)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/Phaseless)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/Network)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/EnvironmentSim)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/BulkMeasurement)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/RandomCircuitSampling)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/RandomHamiltonian)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/HQAmplitude)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Models/CliffordClustering)

if (PS_BUILDING_PYTHON)
    nanobind_add_module(
        pysimulators 
        STABLE_ABI
        NB_STATIC
        src/PySimulators.cpp
    )
    set_target_properties(pysimulators PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(pysimulators PRIVATE ${MODELS_LIBS} dataframe)
    target_include_directories(pysimulators PRIVATE ${MODELS_INCLUDE})

    install(
        TARGETS pysimulators
        LIBRARY DESTINATION pysims
    )
else()
    # --- MAIN --- #
    add_executable(main src/test.cpp)
    target_link_libraries(main PUBLIC ${MODELS_LIBS} dataframe)
    target_include_directories(main PRIVATE src ${MODELS_INCLUDE})
endif()
