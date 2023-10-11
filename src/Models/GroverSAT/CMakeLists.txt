add_library(groversat
    GroverSATSimulator.cpp
)
target_link_libraries(groversat simulator quantum_circuit)
target_include_directories(groversat PRIVATE ${CMAKE_SOURCE_DIR}/src ${QUANTUMCIRCUIT_DIR})
