add_library(random_clifford
    RandomCliffordSimulator.cpp 
    DrivenRandomCliffordSimulator.hpp
    UndrivenRandomCliffordSimulator.hpp
)
target_link_libraries(random_clifford clifford_state)
target_include_directories(random_clifford PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR} ${NLOHMANN_DIR} ${DF_DIR})
