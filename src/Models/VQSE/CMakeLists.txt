add_library(vqse
    ADAMOptimizer.hpp
    VQSE.hpp
    VQSEConfig.hpp
)
target_link_libraries(vqse quantum_circuit)
target_include_directories(vqse PUBLIC ${QUANTUMCIRCUIT_DIR})
