add_library(self_organized
    SandpileCliffordSimulator.cpp
    SelfOrganizedCliffordSimulator.cpp
)
target_link_libraries(self_organized clifford_state)
target_include_directories(self_organized PUBLIC ${CLIFFORD_DIR})
