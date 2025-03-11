add_library(phaseless
    PhaselessSimulator.cpp 
)

target_link_libraries(phaseless PRIVATE clifford_state)

list(APPEND MODELS_LIBS phaseless)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
