add_library(graphsim
    GraphCliffordSimulator.cpp
)
target_link_libraries(graphsim clifford_state)
target_include_directories(graphsim PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR} ${NLOHMANN_DIR} ${DF_DIR})
