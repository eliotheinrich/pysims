add_library(network_clifford
    NetworkCliffordSimulator.cpp 
)

target_link_libraries(network_clifford PRIVATE clifford_state)

list(APPEND MODELS_LIBS network_clifford)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
