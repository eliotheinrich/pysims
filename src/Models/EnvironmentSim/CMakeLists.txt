add_library(env_sim
    EnvironmentSimulator.cpp 
)

target_link_libraries(env_sim PRIVATE clifford_state)

list(APPEND MODELS_LIBS env_sim)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
