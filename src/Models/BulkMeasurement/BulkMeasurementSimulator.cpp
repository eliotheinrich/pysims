#include "BulkMeasurementSimulator.h"

#define DEFAULT_ENV_DIM 1

using namespace dataframe;
using namespace dataframe::utils;

BulkMeasurementSimulator::BulkMeasurementSimulator(ExperimentParams &params, uint32_t) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");
  L = static_cast<uint32_t>(std::sqrt(system_size));
  if (std::abs(std::sqrt(system_size) - L) > 1e-5) {
    std::string error_message = "The system size " + std::to_string(system_size) + " is not a square number.";
    throw std::invalid_argument(error_message);
  }

	mzr_prob = get<double>(params, "mzr_prob");

  circuit_depth = get<int>(params, "circuit_depth");

	state = std::make_shared<QuantumCHPState>(system_size);
}

std::pair<uint32_t, uint32_t> BulkMeasurementSimulator::two_dim_coordinates(uint32_t q, uint32_t length) {
  return std::make_pair(q / length, q % length);
}

uint32_t BulkMeasurementSimulator::site_index(uint32_t s1, uint32_t s2, uint32_t length) {
  return s1*length + s2;
}

void BulkMeasurementSimulator::timesteps(uint32_t num_steps) {
  for (uint32_t i = 0; i < circuit_depth; i++) {
    uint32_t q1 = rand() % system_size;
    auto [s1, s2] = two_dim_coordinates(q1, L);

    uint32_t direction = rand() % 4;

    while ((s1 == 0 && direction == 0) 
        || (s1 == L-1 && direction == 2)
        || (s2 == 0 && direction == 1) 
        || (s2 == L-1 && direction == 3)) {
      direction = rand() % 4;
    }

    uint32_t s1t, s2t;
    if (direction == 0) {
      s1t = s1 - 1;
      s2t = s2;
    } else if (direction == 1) {
      s1t = s1;
      s2t = s2 - 1;
    } else if (direction == 2) {
      s1t = s1 + 1;
      s2t = s2;
    } else {
      s1t = s1;
      s2t = s2 + 1;
    }

    uint32_t q2 = site_index(s1t, s2t, L);
    std::vector<uint32_t> qubits{q1, q2};
    state->random_clifford(qubits);
  }

  // Done with unitary evolution; do measurements.
  uint32_t bulk_size = (L-2)*(L-2);
  for (uint32_t q = 0; q < bulk_size; q++) {
    auto [s1, s2] = two_dim_coordinates(q, L-2);

    uint32_t qt = site_index(s1+1, s2+1, L);
    if (randf() < mzr_prob) {
      state->mzr(qt);
    }
  }
}

SampleMap BulkMeasurementSimulator::take_samples() {
	SampleMap samples;

  std::vector<double> entropy(L);
  for (uint32_t q = 0; q < L; q++) {
    std::vector<uint32_t> qubits(q);
    std::iota(qubits.begin(), qubits.end(), 0);

    entropy[q] = state->entropy(qubits, 2);
  }

  emplace(samples, "surface", entropy);

  //sampler.add_samples(samples, state);
	return samples;
}


