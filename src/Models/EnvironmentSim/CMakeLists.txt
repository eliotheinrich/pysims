add_library(env_sim
    EnvironmentSimulator.cpp 
)
target_link_libraries(env_sim clifford_state)
target_include_directories(env_sim PUBLIC ${CLIFFORD_DIR})
