add_library(self_organized
    SandpileCliffordSimulator.cpp
    ClusterCliffordSimulator.cpp
    BellSandpileSimulator.cpp
)
target_link_libraries(self_organized clifford_state)
target_include_directories(self_organized PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR} ${NLOHMANN_DIR} ${DF_DIR})
