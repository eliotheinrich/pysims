#include <iostream>
#include "QuantumState.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include <map>
#include <Frame.h>
#include <Samplers.h>

#include "Models.h"

using namespace dataframe;
using namespace dataframe::utils;

template <class SimulatorType>
void emulate_simulation(ExperimentParams& params, size_t num_threads=1) {
  SimulatorType sim(params, num_threads);

  size_t equilibration_timesteps = get<int>(params, "equilibration_timesteps", 0);
  size_t sampling_timesteps = get<int>(params, "sampling_timesteps", 0);
  size_t measurement_freq = get<int>(params, "measurement_freq", 1);
  size_t temporal_avg = get<int>(params, "temporal_avg", true);

  size_t num_steps = sampling_timesteps / measurement_freq;

  sim.timesteps(equilibration_timesteps);

  for (size_t i = 0; i < num_steps; i++) {
    sim.timesteps(measurement_freq);
    auto samples = sim.take_samples();
  }

  return;
}

bool test_mps() {
  ExperimentParams params;
  params["system_size"] = static_cast<int>(32);
  params["bond_dimension"] = static_cast<int>(64);
  params["beta"] = 1.2;
  params["p"] = 0.5;

  params["unitary_type"] = static_cast<int>(2);
  params["measurement_type"] = static_cast<int>(1);

  params["sample_spin_glass_order"] = static_cast<int>(true);

  params["sample_stabilizer_renyi_entropy"] = static_cast<int>(false);
  params["sre_num_samples"] = static_cast<int>(500);

  params["sampling_timesteps"] = static_cast<int>(60);
  params["measurement_freq"] = static_cast<int>(3);
  params["equilibration_timesteps"] = static_cast<int>(100);

  emulate_simulation<MatrixProductSimulator>(params, 1);

  return true;
}

bool test_quantum_ising() {
  ExperimentParams params;
  params["system_size"] = static_cast<int>(8);
  params["bond_dimension"] = static_cast<int>(32);
  params["h"] = 0.2;

  params["sample_stabilizer_renyi_entropy"] = static_cast<int>(true);
  params["sre_method"] = "exhaustive";
  params["sample_bipartite_magic_mutual_information"] = static_cast<int>(true);
  params["stabilizer_renyi_indices"] = "1,2,3";
  params["orthogonality_level"] = static_cast<int>(1);

  size_t sampling_timesteps = 60;
  size_t measurement_freq = 3;
  size_t equilibration_timesteps = 0;

  QuantumIsingConfig config(params);

  auto slide = config.compute(1);

  return true;
}

int main() {
  test_mps();
  //test_quantum_ising();
}
