add_library(graphsim
    GraphCliffordSimulator.cpp
)
target_link_libraries(graphsim clifford_state random_clifford quantum_automaton)
target_include_directories(graphsim PUBLIC ${RC_DIR} ${QA_DIR} ${CLIFFORD_DIR})
