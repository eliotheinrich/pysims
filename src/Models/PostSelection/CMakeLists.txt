add_library(post_selection
    PostSelectionCliffordSimulator.cpp 
    GroverProjectionSimulator.cpp
)
target_link_libraries(post_selection PRIVATE random_clifford clifford_state quantum_state)
target_include_directories(post_selection PRIVATE ${RC_DIR})
