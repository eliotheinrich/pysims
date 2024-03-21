add_library(network_clifford
    NetworkCliffordSimulator.cpp 
)

target_link_libraries(network_clifford PRIVATE clifford_state)
