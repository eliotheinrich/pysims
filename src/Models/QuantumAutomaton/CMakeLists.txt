add_library(quantum_automaton
    QuantumAutomatonSimulator.cpp 
)

target_link_libraries(quantum_automaton PRIVATE clifford_state)

list(APPEND MODELS_LIBS quantum_automaton)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
