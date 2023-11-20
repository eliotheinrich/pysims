add_library(groversat
    GroverSATSimulator.cpp
)
target_link_libraries(groversat quantum_circuit)
target_include_directories(groversat PRIVATE ${QUANTUMCIRCUIT_DIR})
