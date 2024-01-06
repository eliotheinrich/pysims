add_library(random_clifford
    RandomCliffordSimulator.cpp 
)

target_link_directories(random_clifford PRIVATE clifford_state)