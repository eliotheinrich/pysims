add_library(brickwork_circuit
    BrickworkCircuitSimulator.cpp
)

target_link_libraries(brickwork_circuit PRIVATE quantum_state)