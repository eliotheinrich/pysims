add_library(network_clifford
    NetworkCliffordSimulator.cpp 
)
target_link_libraries(network_clifford graph clifford_state)
target_include_directories(network_clifford PUBLIC ${CLIFFORD_DIR})
