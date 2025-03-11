add_library(post_selection
    PostSelectionCliffordSimulator.cpp 
)

target_link_libraries(post_selection PRIVATE clifford_state quantum_state)
target_include_directories(post_selection PRIVATE ${CMAKE_SOURCE_DIR}/src/Models/RandomClifford)

list(APPEND MODELS_LIBS post_selection)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
