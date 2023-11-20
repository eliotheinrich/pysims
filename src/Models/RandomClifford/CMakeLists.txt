add_library(random_clifford
    RandomCliffordSimulator.cpp 
)
target_link_libraries(random_clifford clifford_state)
target_include_directories(random_clifford PUBLIC ${CLIFFORD_DIR})
