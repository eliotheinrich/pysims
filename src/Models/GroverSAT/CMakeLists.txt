add_library(groversat
    GroverSATSimulator.cpp
)

target_link_libraries(groversat PRIVATE quantum_state)