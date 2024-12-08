add_library(post_selection
    PostSelectionCliffordSimulator.cpp 
)
target_link_libraries(post_selection PRIVATE clifford_state quantum_state)
target_include_directories(post_selection PRIVATE ${RC_DIR})
