add_library(self_organized
    SandpileCliffordSimulator.cpp
    SelfOrganizedCliffordSimulator.cpp
)

target_link_libraries(self_organized PRIVATE clifford_state)
target_include_directories(self_organized PRIVATE ${CMAKE_SOURCE_DIR}/src/Models/RandomClifford)

list(APPEND MODELS_LIBS self_organized)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
