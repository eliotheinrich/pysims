add_library(mincut
    MinCutSimulator.cpp
)
target_link_libraries(mincut simulator)
target_include_directories(mincut PRIVATE ${CMAKE_SOURCE_DIR}/src ${NLOHMANN_DIR} ${DF_DIR})
