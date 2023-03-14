add_library(blocksim
	BlockSimulator.cpp
)
target_link_libraries(blocksim simulator)
target_include_directories(blocksim PUBLIC ${CMAKE_SOURCE_DIR}/src ${DF_DIR})