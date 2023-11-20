add_library(post_selection
    PostSelectionCliffordSimulator.cpp 
    GroverProjectionSimulator.cpp
)
target_link_libraries(post_selection clifford_state quantum_circuit random_clifford)
target_include_directories(post_selection PUBLIC ${CLIFFORD_DIR} ${QUANTUMCIRCUIT_DIR} ${RC_DIR})
