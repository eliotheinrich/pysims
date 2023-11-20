add_library(phaseless
    PhaselessSimulator.cpp 
)
target_link_libraries(phaseless clifford_state)
target_include_directories(phaseless PUBLIC ${CLIFFORD_DIR})
