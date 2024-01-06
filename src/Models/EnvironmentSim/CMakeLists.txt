add_library(env_sim
    EnvironmentSimulator.cpp 
)

target_link_libraries(env_sim PRIVATE clifford_state)