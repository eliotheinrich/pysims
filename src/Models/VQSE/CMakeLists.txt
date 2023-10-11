add_library(vqse
    ADAMOptimizer.hpp
    VQSE.hpp
    VQSEConfig.hpp
)
target_link_libraries(vqse simulator quantum_circuit)
target_include_directories(vqse PUBLIC ${CMAKE_SOURCE_DIR}/src)
