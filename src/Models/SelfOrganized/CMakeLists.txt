add_library(self_organized
    SandpileCliffordSimulator.cpp
    SelfOrganizedCliffordSimulator.cpp
)
target_link_libraries(self_organized PRIVATE clifford_state)
target_include_directories(self_organized PRIVATE ${RC_DIR})
