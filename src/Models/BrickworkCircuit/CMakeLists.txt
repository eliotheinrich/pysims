add_library(brickwork_circuit
    BrickworkCircuitSimulator.cpp
)

target_link_libraries(brickwork_circuit PRIVATE quantum_state)

list(APPEND MODELS_LIBS brickwork_circuit)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
