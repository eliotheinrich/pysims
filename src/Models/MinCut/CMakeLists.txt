add_library(mincut
    MinCutSimulator.cpp
)

target_link_libraries(mincut PRIVATE graph)
