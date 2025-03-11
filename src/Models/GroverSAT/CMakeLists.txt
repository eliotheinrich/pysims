add_library(groversat
    GroverSATSimulator.cpp
)

target_link_libraries(groversat PRIVATE quantum_state)

list(APPEND MODELS_LIBS groversat)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
