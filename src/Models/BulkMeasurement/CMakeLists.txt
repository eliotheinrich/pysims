add_library(bulk_sim
    BulkMeasurementSimulator.cpp 
)

target_link_libraries(bulk_sim PRIVATE clifford_state)