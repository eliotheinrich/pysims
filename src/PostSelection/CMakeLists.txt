add_library(post_selection
    PostSelectionCliffordSimulator.cpp 
    GroverProjectionSimulator.cpp
)
target_link_libraries(post_selection clifford_state quantum_state random_clifford)
target_include_directories(post_selection PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR} ${QUANTUMSTATE_DIR} ${RC_DIR} ${NLOHMANN_DIR} ${DF_DIR})
