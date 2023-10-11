add_library(graphsim
    GraphCliffordSimulator.cpp
)
target_link_libraries(graphsim clifford_state random_clifford quantum_automaton)
target_include_directories(graphsim PUBLIC ${CMAKE_SOURCE_DIR}/src ${RC_DIR} ${QA_DIR} ${CLIFFORD_DIR})
