add_library(quantum_circuit SHARED
    CircuitUtils.cpp
    Instructions.hpp
    QuantumCircuit.cpp
    Statevector.cpp
    DensityMatrix.cpp
    UnitaryState.cpp
    MatrixProductState.cpp
)

# Preparing ITensor
set(ITENSOR_DIR "/data/heinriea/ITensor")
add_library(itensor STATIC IMPORTED)
set_target_properties(itensor PROPERTIES IMPORTED_LOCATION ${ITENSOR_DIR}/lib/libitensor.a)

# TODO find LAPACK and BLAS automatically
set(ITENSOR_FLAGS "-DPLATFORM_lapack -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0")
set(ITENSOR_LINK_FLAGS "-DPLATFORM_lapack -lpthread -lgfortran -L/cm/shared/apps/lapack/gcc/64/3.8.0 -L/cm/shared/apps/blas/gcc/64/3.8.0 -llapack -lblas")

message(${ITENSOR_LINK_FLAGS})
message(${ITENSOR_DIR})
target_link_libraries(quantum_circuit simulator itensor ${ITENSOR_LINK_FLAGS})
target_include_directories(quantum_circuit PUBLIC ${CMAKE_SOURCE_DIR}/src ${ITENSOR_DIR})
