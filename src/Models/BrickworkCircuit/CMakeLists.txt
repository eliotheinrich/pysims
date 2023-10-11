add_library(brickwork_circuit
    BrickworkCircuitSimulator.cpp
)
target_link_libraries(brickwork_circuit quantum_circuit)
target_include_directories(brickwork_circuit PUBLIC ${CMAKE_SOURCE_DIR}/src ${QUANTUMCIRCUIT_DIR})
