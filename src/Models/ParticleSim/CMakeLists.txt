add_library(particlesim
    ParticleSimulator.cpp
)

target_include_directories(particlesim PRIVATE ${SAMPLERS_DIR})
