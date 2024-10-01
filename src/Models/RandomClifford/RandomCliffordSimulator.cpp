#include "RandomCliffordSimulator.h"
#include <cstdint>
#include <glaze/glaze.hpp>

#define DEFAULT_GATE_WIDTH 2

#define DEFAULT_CLIFFORD_SIMULATOR "chp"

#define DEFAULT_PBC true

#define RC_BRICKWORK 0
#define RC_RANDOM_LOCAL 1
#define RC_RANDOM_NONLOCAL 2
#define RC_POWERLAW 3

using namespace dataframe;
using namespace dataframe::utils;

RandomCliffordSimulator::RandomCliffordSimulator(Params &params, uint32_t) : Simulator(params), entropy_sampler(params), interface_sampler(params) {
	system_size = get<int>(params, "system_size");

	mzr_prob = get<double>(params, "mzr_prob");
	gate_width = get<int>(params, "gate_width", DEFAULT_GATE_WIDTH);

  timestep_type = get<int>(params, "timestep_type", RC_BRICKWORK);
  if (timestep_type == RC_POWERLAW) {
    alpha = get<double>(params, "alpha");
  }

	simulator_type = get<std::string>(params, "simulator_type", DEFAULT_CLIFFORD_SIMULATOR);

	sample_avalanche_sizes = get<int>(params, "sample_avalanche_sizes", false);

	offset = false;
	pbc = get<int>(params, "pbc", DEFAULT_PBC);

	sample_sparsity = get<int>(params, "sample_sparsity", false);	

	seed = get<int>(params, "seed", -1);

	start_sampling = false;


  state = std::make_shared<QuantumCHPState>(system_size, seed);
}

void RandomCliffordSimulator::mzr(uint32_t q) {
  if (sample_avalanche_sizes && start_sampling) {
    std::vector<int> surface1 = state->get_entropy_surface<int>(2);
    state->mzr(q);
    std::vector<int> surface2 = state->get_entropy_surface<int>(2);

    int s = 0.0;
    for (uint32_t i = 0; i < system_size; i++) {
      s += std::abs(surface1[i] - surface2[i]);
    }

    interface_sampler.record_size(s);
  } else {
    state->mzr(q);
  }
}

uint32_t RandomCliffordSimulator::randpl() {
	return rc_power_law(1.0, system_size/2.0, -alpha, randf()); 
}

void RandomCliffordSimulator::timestep_powerlaw() {
  uint32_t q1 = rand() % system_size;
  uint32_t dq = randpl();
  uint32_t q2 = (rand() % 2) ? mod(q1 + dq, system_size) : mod(q1 - dq, system_size);

  std::vector<uint32_t> qbits{q1, q2};;
  state->random_clifford(qbits);

  if (randf() < mzr_prob) {
    mzr(rand() % system_size);
  }
}

void RandomCliffordSimulator::timestep_random_local() {
  uint32_t q1 = rand() % system_size;
  uint32_t q2 = (q1 + 1) % system_size;

  std::vector<uint32_t> qbits{q1, q2};;
  state->random_clifford(qbits);

  if (randf() < mzr_prob) {
    mzr(rand() % system_size);
  }
}

void RandomCliffordSimulator::timestep_random_nonlocal() {
  uint32_t q1 = rand() % system_size;
  uint32_t q2 = rand() % system_size;
  while (q2 == q1) {
    q2 = rand() % system_size;
  }

  std::vector<uint32_t> qbits{q1, q2};;
  state->random_clifford(qbits);

  if (randf() < mzr_prob) {
    mzr(rand() % system_size);
  }
}


void RandomCliffordSimulator::timestep_brickwork(uint32_t num_steps) {
	if (system_size % gate_width != 0) {
		throw std::invalid_argument("Invalid gate width. Must divide system size.");
	} if (gate_width % 2 != 0) {
		throw std::invalid_argument("Gate width must be even.");
	}

	for (uint32_t i = 0; i < num_steps; i++) {
		rc_timestep(state, gate_width, offset, pbc);

		// Apply measurements
		for (uint32_t j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
        mzr(j);
			}
		}

		offset = !offset;
	}
}

void RandomCliffordSimulator::timesteps(uint32_t num_steps) {
  if (timestep_type == RC_BRICKWORK) {
    timestep_brickwork(num_steps);
  } else if (timestep_type == RC_RANDOM_LOCAL) {
    timestep_random_local();
  } else if (timestep_type == RC_RANDOM_NONLOCAL) {
    timestep_random_nonlocal();
  } else if (timestep_type == RC_POWERLAW) {
    timestep_powerlaw();
  }
}

data_t RandomCliffordSimulator::take_samples() {
	data_t samples;

	entropy_sampler.add_samples(samples, state);

	std::vector<int> surface = state->get_entropy_surface<int>(2);
	interface_sampler.add_samples(samples, surface);

	if (sample_sparsity) {
		samples.emplace("sparsity", state->sparsity());
	}

	return samples;
}


template<>
struct glz::meta<RandomCliffordSimulator> {
  static constexpr auto value = glz::object(
    "state", &RandomCliffordSimulator::state
  );
};

void RandomCliffordSimulator::deserialize(const std::vector<byte_t>& bytes) {
  auto pe = glz::read_beve(*this, bytes);
  if (pe) {
    std::string error_message = "Error parsing RandomCliffordSimluator from binary.";
    throw std::invalid_argument(error_message);
  }
}

std::vector<byte_t> RandomCliffordSimulator::serialize() const {
  std::vector<byte_t> data;
  glz::write_beve(*this, data);
  return data;
}
