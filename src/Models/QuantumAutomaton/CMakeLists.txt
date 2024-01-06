add_library(quantum_automaton
    QuantumAutomatonSimulator.cpp 
)

target_link_libraries(quantum_automaton PRIVATE clifford_state)