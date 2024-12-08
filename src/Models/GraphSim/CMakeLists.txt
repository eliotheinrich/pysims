add_library(graphsim
    GraphCliffordSimulator.cpp
)
target_link_libraries(graphsim PRIVATE quantum_automaton clifford_state)
target_include_directories(graphsim PRIVATE ${RC_DIR} ${QA_DIR})
