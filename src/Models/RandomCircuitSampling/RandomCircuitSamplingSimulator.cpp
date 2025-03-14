#include "RandomCircuitSamplingSimulator.h"

#define FULL_HAAR 0
#define BRICKWORK_HAAR 1
#define RANDOM_HAAR 2

using namespace dataframe;
using namespace dataframe::utils;

RandomCircuitSamplingSimulator::RandomCircuitSamplingSimulator(ExperimentParams &params, uint32_t num_threads) : Simulator(params), entropy_sampler(params), prob_sampler(params) {
  system_size = get<int>(params, "system_size");

  mzr_prob = get<double>(params, "mzr_prob");
  evolution_type = get<int>(params, "evolution_type", FULL_HAAR);
  offset = false;

  Eigen::setNbThreads(num_threads);
  state_type = get<int>(params, "state_type", 0);
  if (state_type == 0) {
    state = std::make_shared<Statevector>(system_size);
  } else if (state_type == 1) {
    state = std::make_shared<MatrixProductState>(system_size, 64, 1e-8);
  }

  state->seed(rand());
}

void RandomCircuitSamplingSimulator::full_haar() {
  std::vector<uint32_t> qubits(system_size);
  std::iota(qubits.begin(), qubits.end(), 0);
  state->evolve(haar_unitary(system_size), qubits);
}

void RandomCircuitSamplingSimulator::brickwork_haar() {
  for (uint32_t q = 0; q < system_size/2; q++) {
    uint32_t q1 = offset ? 2*q : (2*q + 1) % system_size;
    uint32_t q2 = offset ? (2*q + 1) % system_size : (2*q + 2) % system_size;

    std::vector<uint32_t> qubits{q1, q2};
    state->evolve(haar_unitary(2), qubits);
  }

  for (uint32_t q = 0; q < system_size; q++) {
    if (randf() < mzr_prob) {
      state->measure(Measurement::computational_basis(q));
    }
  }

  offset = !offset;
}

void RandomCircuitSamplingSimulator::random_haar() {
  uint32_t q1 = rand() % system_size;
  uint32_t q2 = (q1 + 1) % system_size;

  std::vector<uint32_t> qubits{q1, q2};
  state->evolve(haar_unitary(2), qubits);

  uint32_t q = rand() % system_size;
  if (randf() < mzr_prob/2) {
    state->measure(Measurement::computational_basis(q));
  }
}

void RandomCircuitSamplingSimulator::timesteps(uint32_t num_steps) {
  for (uint32_t k = 0; k < num_steps; k++) {
    if (evolution_type == FULL_HAAR) {
      full_haar();
    } else if (evolution_type == BRICKWORK_HAAR) {
      brickwork_haar();
    } else if (evolution_type == RANDOM_HAAR) {
      random_haar();
    }
  }
}

SampleMap RandomCircuitSamplingSimulator::take_samples() {
  SampleMap samples;
  entropy_sampler.add_samples(samples, state);
  prob_sampler.add_samples(samples, state);
  return samples;
}
