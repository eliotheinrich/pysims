add_library(graphsim
    GraphCliffordSimulator.cpp
)

target_link_libraries(graphsim PRIVATE quantum_automaton clifford_state)
target_include_directories(graphsim PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/Models/RandomClifford
    ${CMAKE_SOURCE_DIR}/src/Models/QuantumAutomaton
)

list(APPEND MODELS_LIBS graphsim)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
