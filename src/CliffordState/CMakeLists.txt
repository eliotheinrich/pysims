add_library(clifford_state
	CliffordState.hpp
	Tableau.cpp Tableau.h
	QuantumCHPState.cpp QuantumCHPState.h
)
target_link_libraries(clifford_state simulator)
target_include_directories(clifford_state PUBLIC ${CMAKE_SOURCE_DIR}/src ${DF_DIR})