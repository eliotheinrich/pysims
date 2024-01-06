add_library(random_circuit_sampling
    RandomCircuitSamplingSimulator.cpp
)

target_link_libraries(random_circuit_sampling PRIVATE quantum_state)