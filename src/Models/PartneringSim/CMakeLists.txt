add_library(partnersim
    PartneringSimulator.cpp
)
target_link_libraries(partnersim graph)
target_include_directories(partnersim PRIVATE ${CMAKE_SOURCE_DIR}/src)