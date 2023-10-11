add_library(phaseless
    PhaselessSimulator.cpp 
)
target_link_libraries(phaseless clifford_state)
target_include_directories(phaseless PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR})
