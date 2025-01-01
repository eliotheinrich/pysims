#include "RandomHamiltonianSimulator.h"
#include <unsupported/Eigen/MatrixFunctions>

using namespace dataframe;
using namespace dataframe::utils;

RandomHamiltonianSimulator::RandomHamiltonianSimulator(ExperimentParams &params, uint32_t num_threads) : Simulator(params), entropy_sampler(params), prob_sampler(params) {
  system_size = get<int>(params, "system_size");
  dt = get<double>(params, "dt");
  mu = get<double>(params, "mu", 0.0);
  sigma = get<double>(params, "sigma", 1.0);
  

  Eigen::setNbThreads(num_threads);
  state = std::make_shared<Statevector>(system_size);

  std::complex<double> ii(0.0, 1.0);
  Eigen::Matrix2cd X; X << 0, 1, 1, 0;
  Eigen::Matrix2cd Y; Y << 0, -ii, ii, 0;
  Eigen::Matrix2cd Z; Z << 1, 0, 0, -1;

  std::normal_distribution<double> dist(0.0, 1.0);
  hamiltonian = Eigen::MatrixXcd::Zero(1u << system_size, 1u << system_size);
  for (uint32_t i = 0; i < system_size; i++) {
    uint32_t j = (i + 1) % system_size;

    std::vector<uint32_t> q1{i};
    std::vector<uint32_t> q2{j};
    Eigen::MatrixXcd coupling = dist(rng)*full_circuit_unitary(X, q1, system_size) * full_circuit_unitary(X, q2, system_size)
                              + dist(rng)*full_circuit_unitary(Y, q1, system_size) * full_circuit_unitary(Y, q2, system_size)
                              + dist(rng)*full_circuit_unitary(Z, q1, system_size) * full_circuit_unitary(Z, q2, system_size);
    hamiltonian += coupling;
  }

  evolution_operator = (ii*dt*hamiltonian).exp();
}

void RandomHamiltonianSimulator::timesteps(uint32_t num_steps) {
  for (uint32_t k = 0; k < num_steps; k++) {
    state->QuantumState::evolve(evolution_operator);
  }
}

data_t RandomHamiltonianSimulator::take_samples() {
  data_t samples;
  entropy_sampler.add_samples(samples, state);
  prob_sampler.add_samples(samples, state);
  return samples;
}
