add_library(vqse
    ADAMOptimizer.hpp
    VQSE.hpp
    VQSEConfig.hpp
    VQSECircuitConfig.hpp
)

set_target_properties(vqse PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(vqse PRIVATE quantum_state)
