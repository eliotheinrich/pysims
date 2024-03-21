add_library(hq_amplitude
    HQCircuitConfig.hpp
)

target_link_libraries(hq_amplitude PRIVATE quantum_state pyqtools_misc)
