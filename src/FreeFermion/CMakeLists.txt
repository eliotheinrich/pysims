add_library(free_fermion 
    FreeFermionSimulator.cpp
    FreeFermionConfig.cpp
)
target_link_libraries(free_fermion simulator)
target_include_directories(free_fermion PRIVATE ${CMAKE_SOURCE_DIR}/src ${EIGEN_DIR} ${DF_DIR} ${NLOHMANN_DIR})
