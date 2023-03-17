add_library(debugsim
    DebugSimulator.hpp
)
target_link_libraries(debugsim clifford_state)
target_include_directories(debugsim PUBLIC ${CMAKE_SOURCE_DIR}/src ${CLIFFORD_DIR} ${NLOHMANN_DIR} ${DF_DIR})
