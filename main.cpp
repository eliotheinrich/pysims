#include "FreeFermionSimulator.h"
#include "FreeFermionConfig.h"
#include <iostream>
#include <Eigen/Core>
using namespace std;

int main() {

    std::vector<FreeFermionConfig> configs = FreeFermionConfig::load_json("/Users/eliotheinrich/Projects/free_fermion/test.json");
    ParallelCompute pc(configs);
    DataFrame df = pc.compute(1);
    df.write_json("data.json");

}