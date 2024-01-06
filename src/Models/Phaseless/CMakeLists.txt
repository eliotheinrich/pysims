add_library(phaseless
    PhaselessSimulator.cpp 
)

target_link_libraries(phaseless PRIVATE clifford_state)