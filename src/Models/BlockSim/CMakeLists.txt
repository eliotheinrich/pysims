add_library(blocksim
	BlockSimulator.cpp
	RPMSimulator.cpp
)
message(${SAMPLERS_DIR})
target_link_libraries(blocksim ${PFFFT_LIB})
target_include_directories(blocksim PRIVATE ${SAMPLERS_DIR})