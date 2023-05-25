add_library(quantum_state
	QuantumStatevector.hpp
	UnitaryState.hpp
)
target_link_libraries(quantum_state simulator)
target_include_directories(quantum_state PUBLIC ${CMAKE_SOURCE_DIR}/src ${DF_DEPENDENCIES})