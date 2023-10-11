add_library(quantum_automaton
    QuantumAutomatonSimulator.cpp 
)
target_link_libraries(quantum_automaton clifford_state)
target_include_directories(quantum_automaton PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR})
