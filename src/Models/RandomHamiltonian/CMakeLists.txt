add_library(random_hamiltonian
  RandomHamiltonianSimulator.cpp
)

target_link_libraries(random_hamiltonian PRIVATE quantum_state)

list(APPEND MODELS_LIBS random_hamiltonian)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
