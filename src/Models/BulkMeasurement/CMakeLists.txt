add_library(bulk_sim
    BulkMeasurementSimulator.cpp 
)

target_link_libraries(bulk_sim PRIVATE clifford_state)

list(APPEND MODELS_LIBS bulk_sim)
set(MODELS_LIBS "${MODELS_LIBS}" PARENT_SCOPE)

list(APPEND MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_INCLUDE "${MODELS_INCLUDE}" PARENT_SCOPE)
