add_library(mincut
    MinCutSimulator.cpp
)
target_link_libraries(mincut graph)
target_include_directories(mincut PRIVATE ${CMAKE_SOURCE_DIR}/src ${SAMPLERS_DIR})
